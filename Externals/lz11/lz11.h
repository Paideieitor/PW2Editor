/*
	The functions below are from DSDecmp by Barubary:
	https://github.com/Barubary/dsdecmp
	Only thing done was converting them from C# to C
*/

#ifndef _LZ11_H
#define _LZ11_H

#include <stdlib.h>
#include <cstring>

size_t GetOccurrenceLength(unsigned char* newPtr, size_t newLength, unsigned char* oldPtr, int oldLength, int* disp)
{
	*disp = 0;
	int minDisp = 1;
	if (newLength == 0)
		return 0;
	size_t maxLength = 0;
	// try every possible 'disp' value (disp = oldLength - i)
	int i;
	for (i = 0; i < oldLength - minDisp; i++)
	{
		// work from the start of the old data to the end, to mimic the original implementation's behaviour
		// (and going from start to end or from end to start does not influence the compression ratio anyway)
		unsigned char* currentOldStart = oldPtr + i;
		size_t currentLength = 0;
		// determine the length we can copy if we go back (oldLength - i) bytes
		// always check the next 'newLength' bytes, and not just the available 'old' bytes,
		// as the copied data can also originate from what we're currently trying to compress.
		size_t j;
		for (j = 0; j < newLength; j++)
		{
			// stop when the bytes are no longer the same
			if (*(currentOldStart + j) != *(newPtr + j))
				break;
			currentLength++;
		}

		// update the optimal value
		if (currentLength > maxLength)
		{
			maxLength = currentLength;
			*disp = oldLength - i;

			// if we cannot do better anyway, stop trying.
			if (maxLength == newLength)
				break;
		}
	}
	return maxLength;
}

#define lz11_min(a,b) (((a) < (b)) ? (a) : (b))
size_t LZ11Compress(unsigned char* indata, size_t inLength, unsigned char** out)
{
	// make sure the decompressed size fits in 3 bytes.
	// There should be room for four bytes, however I'm not 100% sure if that can be used
	// in every game, as it may not be a built-in function.
	if (inLength > 0xFFFFFF)
	{
		// puts("File too large!");
		return 0;
	}

	//using twice the original length here to be safe
	unsigned char* writePtr = new unsigned char[inLength * 2];
	memset(writePtr, 0, inLength * 2);
	*out = writePtr;

	// write the compression header first
	*writePtr = (0x11); writePtr++;
	*writePtr = ((unsigned char)(inLength & 0xFF)); writePtr++;
	*writePtr = ((unsigned char)((inLength >> 8) & 0xFF)); writePtr++;
	*writePtr = ((unsigned char)((inLength >> 16) & 0xFF)); writePtr++;

	size_t compressedLength = 4;

	// we do need to buffer the output, as the first byte indicates which blocks are compressed.
	// this version does not use a look-ahead, so we do not need to buffer more than 8 blocks at a time.
	// (a block is at most 4 bytes long)
	unsigned char outbuffer[(8 * 4) + 1];
	outbuffer[0] = 0;
	size_t bufferlength = 1, bufferedBlocks = 0;
	size_t readBytes = 0;
	while (readBytes < inLength)
	{
		// we can only buffer 8 blocks at a time.
		if (bufferedBlocks == 8)
		{
			memcpy(writePtr, outbuffer, bufferlength);
			writePtr += bufferlength;
			compressedLength += bufferlength;
			// reset the buffer
			outbuffer[0] = 0;
			bufferlength = 1;
			bufferedBlocks = 0;
		}

		// determine if we're dealing with a compressed or raw block.
		// it is a compressed block when the next 3 or more bytes can be copied from
		// somewhere in the set of already compressed bytes.
		int disp;
		size_t oldLength = lz11_min(readBytes, 0x1000);
		size_t length = GetOccurrenceLength(indata + readBytes, lz11_min(inLength - readBytes, (size_t)0x10110),
			indata + readBytes - oldLength, (int)oldLength, &disp);
		// length not 3 or more? next byte is raw data
		if (length < 3)
		{
			outbuffer[bufferlength++] = *(indata + (readBytes++));
		}
		else
		{
			// 3 or more bytes can be copied? next (length) bytes will be compressed into 2 bytes
			readBytes += length;

			// mark the next block as compressed
			outbuffer[0] |= (unsigned char)(1 << (7 - bufferedBlocks));

			if (length > 0x110)
			{
				// case 1: 1(B CD E)(F GH) + (0x111)(0x1) = (LEN)(DISP)
				outbuffer[bufferlength] = 0x10;
				outbuffer[bufferlength] |= (unsigned char)(((length - 0x111) >> 12) & 0x0F);
				bufferlength++;
				outbuffer[bufferlength] = (unsigned char)(((length - 0x111) >> 4) & 0xFF);
				bufferlength++;
				outbuffer[bufferlength] = (unsigned char)(((length - 0x111) << 4) & 0xF0);
			}
			else if (length > 0x10)
			{
				// case 0; 0(B C)(D EF) + (0x11)(0x1) = (LEN)(DISP)
				outbuffer[bufferlength] = 0x00;
				outbuffer[bufferlength] |= (unsigned char)(((length - 0x111) >> 4) & 0x0F);
				bufferlength++;
				outbuffer[bufferlength] = (unsigned char)(((length - 0x111) << 4) & 0xF0);
			}
			else
			{
				// case > 1: (A)(B CD) + (0x1)(0x1) = (LEN)(DISP)
				outbuffer[bufferlength] = (unsigned char)(((length - 1) << 4) & 0xF0);
			}
			// the last 1.5 bytes are always the disp
			outbuffer[bufferlength] |= (unsigned char)(((disp - 1) >> 8) & 0x0F);
			bufferlength++;
			outbuffer[bufferlength] = (unsigned char)((disp - 1) & 0xFF);
			bufferlength++;
		}
		bufferedBlocks++;
	}

	// copy the remaining blocks to the output
	if (bufferedBlocks > 0)
	{
		memcpy(writePtr, outbuffer, bufferlength);
		writePtr += bufferlength;
		compressedLength += bufferlength;
	}

	return compressedLength;
}

size_t LZ11Decompress(unsigned char* indata, size_t inLength, unsigned char** out)
{
	size_t readBytes = 0;

	if (indata[0] != 0x11)
	{
		// puts("No LZ11!");
		return 0;
	}
	size_t decompressedSize = (indata[1] | (indata[2] << 8) | (indata[3] << 16));
	readBytes += 4;
	if (decompressedSize == 0)
	{
		decompressedSize = (indata[4] | (indata[5] << 8) | (indata[6] << 16) | (indata[7] << 24));
		readBytes += 4;
	}
	unsigned char* writePtr = new unsigned char[decompressedSize];
	memset(writePtr, 0, decompressedSize);
	*out = writePtr;

	// the maximum 'DISP-1' is still 0xFFF.
	const size_t bufferLength = 0x1000;
	unsigned char* buffer = new unsigned char[bufferLength];
	size_t bufferOffset = 0;

	size_t currentOutSize = 0;
	unsigned char flags = 0, mask = 1;
	while (currentOutSize < decompressedSize)
	{
		if (mask == 1)
		{
			if (readBytes >= inLength)
			{
				// puts("Not enough data!");
				delete[] buffer;
				return 0;
			}
			flags = indata[readBytes]; readBytes++;
			mask = 0x80;
		}
		else
		{
			mask >>= 1;
		}

		// bit = 1 <=> compressed.
		if ((flags & mask) > 0)
		{
			// read the first byte first, which also signals the size of the compressed block
			if (readBytes >= inLength)
			{
				// puts("Not enough data!");
				delete[] buffer;
				return 0;
			}
			unsigned char byte1 = indata[readBytes]; readBytes++;

			size_t length = byte1 >> 4;
			int disp = -1;
			if (length == 0)
			{
				// case 0:
				// data = AB CD EF (with A=0)
				// LEN = ABC + 0x11 == BC + 0x11
				// DISP = DEF + 1

				// we need two more bytes available
				if (readBytes + 1 >= inLength)
				{
					// puts("Not enough data!");
					delete[] buffer;
					return 0;
				}
				unsigned char byte2 = indata[readBytes]; readBytes++;
				unsigned char byte3 = indata[readBytes]; readBytes++;

				length = (((byte1 & 0x0F) << 4) | (byte2 >> 4)) + 0x11;
				disp = (((byte2 & 0x0F) << 8) | byte3) + 0x1;
			}
			else if (length == 1)
			{
				// case 1:
				// data = AB CD EF GH (with A=1)
				// LEN = BCDE + 0x111
				// DISP = FGH + 1

				// we need three more bytes available
				if (readBytes + 2 >= inLength)
				{
					// puts("Not enough data!");
					delete[] buffer;
					return 0;
				}
				unsigned char byte2 = indata[readBytes]; readBytes++;
				unsigned char byte3 = indata[readBytes]; readBytes++;
				unsigned char byte4 = indata[readBytes]; readBytes++;

				length = (((byte1 & 0x0F) << 12) | (byte2 << 4) | (byte3 >> 4)) + 0x111;
				disp = (((byte3 & 0x0F) << 8) | byte4) + 0x1;
			}
			else
			{
				// case other:
				// data = AB CD
				// LEN = A + 1
				// DISP = BCD + 1

				// we need only one more byte available
				if (readBytes >= inLength)
				{
					// puts("Not enough data!");
					delete[] buffer;
					return 0;
				}
				unsigned char byte2 = indata[readBytes]; readBytes++;

				length = ((byte1 & 0xF0) >> 4) + 0x1;
				disp = (((byte1 & 0x0F) << 8) | byte2) + 0x1;
			}

			if (disp > (int)currentOutSize)
			{
				// puts("Cannot go back more than already written.");
				delete[] buffer;
				return 0;
			}
			size_t bufIdx = bufferOffset + bufferLength - disp;
			size_t i;
			for (i = 0; i < length; i++)
			{
				unsigned char next = buffer[bufIdx % bufferLength];
				bufIdx++;
				*writePtr = next; writePtr++;
				buffer[bufferOffset] = next;
				bufferOffset = (bufferOffset + 1) % bufferLength;
			}
			currentOutSize += length;
		}
		else
		{
			if (readBytes >= inLength)
			{
				// puts("Not enough data!");
				delete[] buffer;
				return 0;
			}
			unsigned char next = indata[readBytes]; readBytes++;

			*writePtr = next; writePtr++;
			currentOutSize++;
			buffer[bufferOffset] = next;
			bufferOffset = (bufferOffset + 1) % bufferLength;
		}
	}
	delete[] buffer;
	return decompressedSize;
}

#endif // _LZ11_H
