#include <stack>

#include "Log.h"

#include "Utils/NarcUtils.h"
#include "Utils/FileUtils.h"

struct NarcHeader
{
	u32 ID = 0;
	u16 byteOrderMark = 0;
	u16 version = 0;
	u32 fileSize = 0;
	u16 chunkSize = 0;
	u16 chunkCount = 0;
};

struct FileAllocationTable
{
	u32 ID = 0;
	u32 chunkSize = 0;
	u16 fileCount = 0;
	u16 reserved = 0;
};

struct FileAllocationTableEntry
{
	u32 start = 0;
	u32 end = 0;
};

struct FileNameTable
{
	u32 ID = 0;
	u32 chunkSize = 0;
};

struct FileNameTableEntry
{
	u32 offset = 0;
	u16 firstFileID = 0;
	u16 utility = 0;
};

struct FileImages
{
	u32 ID = 0;
	u32 chunkSize = 0;
};

struct Narc
{
	NarcHeader header = NarcHeader();
	FileAllocationTable fileAllocTable = FileAllocationTable();
	vector<FileAllocationTableEntry> fileAllocTableEntries = vector<FileAllocationTableEntry>();
	FileNameTable fileNameTable = FileNameTable();
	vector<FileNameTableEntry> fileNameTableEntries = vector<FileNameTableEntry>();
	FileImages fileImages = FileImages();
};

bool VerifyNarc(Narc& narc, FileStream& fileStream, const string& path)
{
	u32 currentByte = 0;

	narc.header = FileStreamReadUpdate<NarcHeader>(fileStream, currentByte);
	if (narc.header.ID != 0x4352414E)
	{
		Log(WARNING, "Invalid NARC ID in file %s", path.c_str());
		ReleaseFileStream(fileStream);
		return false;
	}
	if (narc.header.byteOrderMark != 0xFFFE)
	{
		Log(WARNING, "Invalid NARC byte order mark in file %s", path.c_str());
		ReleaseFileStream(fileStream);
		return false;
	}
	if ((narc.header.version != 0x0100) && (narc.header.version != 0x0000))
	{
		Log(WARNING, "Invalid NARC version in file %s", path.c_str());
		ReleaseFileStream(fileStream);
		return false;
	}
	if (narc.header.chunkSize != 0x10)
	{
		Log(WARNING, "Invalid NARC header size in file %s", path.c_str());
		ReleaseFileStream(fileStream);
		return false;
	}
	if (narc.header.chunkCount != 0x3)
	{
		Log(WARNING, "Invalid NARC chunk count in file %s", path.c_str());
		ReleaseFileStream(fileStream);
		return false;
	}

	narc.fileAllocTable = FileStreamReadUpdate<FileAllocationTable>(fileStream, currentByte);
	if (narc.fileAllocTable.ID != 0x46415442)
	{
		Log(WARNING, "Invalid NARC file allocation table ID in file %s", path.c_str());
		ReleaseFileStream(fileStream);
		return false;
	}
	if (narc.fileAllocTable.reserved != 0x0)
	{
		Log(WARNING, "File allocation table is reserved in NARC file %s", path.c_str());
		ReleaseFileStream(fileStream);
		return false;
	}

	narc.fileAllocTableEntries.reserve(narc.fileAllocTable.fileCount);

	for (u16 fileIdx = 0; fileIdx < narc.fileAllocTable.fileCount; ++fileIdx)
		narc.fileAllocTableEntries.emplace_back(FileStreamReadUpdate<FileAllocationTableEntry>(fileStream, currentByte));

	narc.fileNameTable = FileStreamReadUpdate<FileNameTable>(fileStream, currentByte);

	if (narc.fileNameTable.ID != 0x464E5442)
	{
		Log(WARNING, "Invalid NARC file name table ID in file %s", path.c_str());
		ReleaseFileStream(fileStream);
		return false;
	}
	if (narc.fileNameTable.chunkSize != 0x10)
	{
		Log(WARNING, "NARC file name table chunk size not supported in file %s", path.c_str());
		ReleaseFileStream(fileStream);
		return false;
	}

	do
	{
		narc.fileNameTableEntries.emplace_back(FileNameTableEntry());

		FileNameTableEntry& nameTableEntry = narc.fileNameTableEntries.back();
		nameTableEntry.offset = FileStreamReadUpdate<u32>(fileStream, currentByte);
		nameTableEntry.firstFileID = FileStreamReadUpdate<u16>(fileStream, currentByte);
		nameTableEntry.utility = FileStreamReadUpdate<u16>(fileStream, currentByte);
	} while (currentByte < (narc.header.chunkSize + narc.fileAllocTable.chunkSize + sizeof(FileNameTable) + narc.fileNameTableEntries[0].offset));

	vector<string> fileNames;
	for (u32 fileNameTableIdx = 0; fileNameTableIdx < narc.fileNameTableEntries.size(); ++fileNameTableIdx)
	{
		currentByte = narc.header.chunkSize + narc.fileAllocTable.chunkSize + sizeof(FileNameTable) + narc.fileNameTableEntries[fileNameTableIdx].offset;

		u16 fileID = 0x0000;

		for (u8 length = 0x80; length != 0x00; length = FileStreamReadUpdate<u8>(fileStream, currentByte))
		{
			if (length <= 0x7F)
			{
				for (u8 i = 0; i < length; ++i)
				{
					u8 character = FileStreamReadUpdate<u8>(fileStream, currentByte);
					fileNames[narc.fileNameTableEntries[fileNameTableIdx].firstFileID + fileID] += character;
				}

				++fileID;
			}
			else if (length == 0x80)
			{
				// Reserved
			}
			else if (length <= 0xFF)
			{
				length -= 0x80;
				string directoryName;

				for (u8 i = 0; i < length; ++i)
				{
					u8 character = FileStreamReadUpdate<u8>(fileStream, currentByte);
					directoryName += character;
				}

				u16 directoryID = FileStreamReadUpdate<u16>(fileStream, currentByte);

				fileNames[directoryID] = directoryName;
			}
			else
			{
				Log(WARNING, "Invalid NARC file name table entry ID in file %s", path.c_str());
				ReleaseFileStream(fileStream);
				return false;
			}
		}
	}

	if ((currentByte % 4) != 0)
		currentByte += 4 - (currentByte % 4);

	narc.fileImages = FileStreamReadUpdate<FileImages>(fileStream, currentByte);
	if (narc.fileImages.ID != 0x46494D47)
	{
		Log(WARNING, "Invalid NARC file images ID in file %s", path.c_str());
		ReleaseFileStream(fileStream);
		return false;
	}

	return true;
}

void ExtractFileFromNarc(const Narc& narc, u16 fileID, const FileStream& fileStream, const string& savePath)
{
	u32 offset = narc.header.chunkSize + narc.fileAllocTable.chunkSize + narc.fileNameTable.chunkSize + 8 + narc.fileAllocTableEntries[fileID].start;
	u32 bufferSize = narc.fileAllocTableEntries[fileID].end - narc.fileAllocTableEntries[fileID].start;

	FileStream subStream;
	LoadEmptyFileStream(subStream);

	FileStreamBufferWriteBack(subStream, FileStreamGetDataPtr(fileStream, offset), bufferSize);
	SaveFileStream(subStream, savePath + PATH_SEPARATOR + to_string(fileID));
	ReleaseFileStream(subStream);
}

int NarcUnpack(const string& path, const string& savePath)
{
	FileStream fileStream;
	if (!LoadFileStream(fileStream, path))
		return -1;

	Narc narc;
	if (!VerifyNarc(narc, fileStream, path))
		return -1;

	if (!PathExists(savePath))
		CreateFolder(savePath);

	for (u16 fileID = 0; fileID < narc.fileAllocTable.fileCount; ++fileID)
		ExtractFileFromNarc(narc, fileID, fileStream, savePath);

	ReleaseFileStream(fileStream);
	return (int)narc.fileAllocTable.fileCount;
}

int NarcUnpackSingle(const string& path, const string& savePath, u16 fileID)
{
	if (fileID < 0)
	{
		Log(WARNING, "File ID is out of scope of NARC file %s", path.c_str());
		return -1;
	}

	FileStream fileStream;
	if (!LoadFileStream(fileStream, path))
		return -1;

	Narc narc;
	if (!VerifyNarc(narc, fileStream, path))
		return -1;

	if (fileID > narc.fileAllocTable.fileCount)
	{
		Log(WARNING, "File ID is out of scope of NARC file %s", path.c_str());
		ReleaseFileStream(fileStream);
		return -1;
	}

	if (!PathExists(savePath))
		CreateFolder(savePath);

	ExtractFileFromNarc(narc, fileID, fileStream, savePath);

	ReleaseFileStream(fileStream);
	return 1;
}

int NarcUnpackBundle(const string& path, const string& savePath, const vector<u16>& fileIDs)
{
	FileStream fileStream;
	if (!LoadFileStream(fileStream, path))
		return -1;

	Narc narc;
	if (!VerifyNarc(narc, fileStream, path))
		return -1;

	if (!PathExists(savePath))
		CreateFolder(savePath);

	int extractedFiles = 0;
	for (u32 fileIdx = 0; fileIdx < fileIDs.size(); ++fileIdx)
	{
		if (fileIDs[fileIdx] < 0 || fileIDs[fileIdx] > narc.fileAllocTable.fileCount)
		{
			Log(WARNING, "Skiped file ID %d is out of scope of NARC file %s", fileIDs[fileIdx], path.c_str());
			continue;
		}

		ExtractFileFromNarc(narc, fileIDs[fileIdx], fileStream, savePath);
		++extractedFiles;
	}

	ReleaseFileStream(fileStream);
	return extractedFiles;
}

int NarcUnpackExclude(const string& path, const string& savePath, const vector<u16>& excludeIDs)
{
	FileStream fileStream;
	if (!LoadFileStream(fileStream, path))
		return -1;

	Narc narc;
	if (!VerifyNarc(narc, fileStream, path))
		return -1;

	// Exit if we are excluding all the files
	if (narc.fileAllocTable.fileCount == (u16)excludeIDs.size())
		return 0;

	if (!PathExists(savePath))
		CreateFolder(savePath);

	int extractedFiles = 0;

	u8* exclude = new u8[narc.fileAllocTable.fileCount];
	memset(exclude, 0, narc.fileAllocTable.fileCount);
	for (u32 excludeIdx = 0; excludeIdx < excludeIDs.size(); ++excludeIdx)
		if (excludeIDs[excludeIdx] < narc.fileAllocTable.fileCount)
			exclude[excludeIDs[excludeIdx]] = 1;

	for (u32 fileID = 0; fileID < narc.fileAllocTable.fileCount; ++fileID)
	{
		if (exclude[fileID] == 0)
		{
			ExtractFileFromNarc(narc, fileID, fileStream, savePath);

			++extractedFiles;
		}	
	}

	delete[] exclude;
	ReleaseFileStream(fileStream);
	return extractedFiles;
}
