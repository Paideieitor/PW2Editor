#ifndef _FILE_UTILS_H
#define _FILE_UTILS_H

#include <string>
#include <vector>

#include "Globals.h"

#define PATH_SEPARATOR '\\'
#define NOT_PATH_SEPARATOR '/'

#define MAKE_FILE_PATH(path, fileID) path + PATH_SEPARATOR + to_string(fileID)

#define PATH_FORMAT(path) (string("\"") + path + string("\""))

// Returns true if the path exists in the file system, false otherwise
bool PathExists(const string& path);
// Returns true if the path leads to a file, false otherwise
bool IsFilePath(const string& path);
// Returns the absolute path of a given relative path
string GetAbsolutePath(const string& path);
// Returns the extention of the file in the path if there is any
string GetFileExtension(const string& path);

// Creates a file in the given path, returns false if it fails
bool CreateFile(const string& path);
// Removes a file in the given path, returns false if it fails
bool RemoveFile(const string& path);
// Copies a file to the given path
bool CopyFile(const string& path, const string& copyPath);

// Creates a folder in the given path, returns false if it fails
bool CreateFolder(const string& path);
// Removes a folder and all its contents in the given path, returns the amount of deleted elements
int RemoveFolder(const string& path);

// Returns the list of elements inside a folder path
vector<string> GetFolderElementList(const string& path);

// Changes the path separator to the Klinklang Engine standarized one
void NormalizePathSeparator(string& path);

// Concatenate 2 strings with a path separator
string PathConcat(const string& path, const string& concat);
// Returns the given path string without the file portion
string RemoveFileFromPath(const string& path);
// Returns the file portion of the given path string
string GetFileFromPath(const string& path, bool checkFile = true);
string PathRemoveExtension(const string& path);
string PathGetLastName(const string& path);
string PathEraseLastName(const string& path);
// Takes a full path and separates the file portion from the original path string
bool SeparatePathAndFile(string& path, string& file);

struct FileStream
{
	u8* data;
	u32 length;
	u32 capacity;
};

// Load an empty file stream (this MUST be released with "ReleaseFileStream")
bool LoadEmptyFileStream(FileStream& stream);
// Load a file into a file stream (this MUST be released with "ReleaseFileStream")
bool LoadFileStream(FileStream& stream, const string& path);
// Save a file stream to a file
bool SaveFileStream(const FileStream& stream, const string& path);
// Free file data of a file stream
void ReleaseFileStream(FileStream& stream);

// Get the data of a file stream in a given offset
template<class T>
T FileStreamRead(const FileStream& stream, u32 offset)
{
	return *((T*)&stream.data[offset]);
}

// Get the data of a file stream in a given offset and update the offset to the end of the data
template<class T>
T FileStreamReadUpdate(const FileStream& stream, u32& offset)
{
	T output = FileStreamRead<T>(stream, offset);
	offset += sizeof(T);
	
	return output;
}

// Copy the data from the file stream to a buffer
void FileStreamBufferRead(const FileStream& stream, u32 offset, u8* buffer, u32 length);

// Copy the data from the file stream to a buffer and update the offset to the end of the data
void FileStreamBufferReadUpdate(const FileStream& stream, u32& offset, u8* buffer, u32 length);

// Duplicate the size of the file stream
void FileStreamExpand(FileStream& stream);

// Put data at the back of the file stream
template<class T>
void FileStreamPutBack(FileStream& stream, const T& value)
{
	while (stream.length + sizeof(T) >= stream.capacity)
		FileStreamExpand(stream);

	u8* valuePtr = (u8*)&value;
	for (u32 i = 0; i < sizeof(T); ++i)
		stream.data[stream.length + i] = valuePtr[i];

	stream.length += sizeof(T);
}

// Write a buffer of data at the back of the file stream
void FileStreamBufferWriteBack(FileStream& stream, const u8* buffer, u32 length);

// Replace a set of bytes from the file stream at the offset
template<class T>
bool FileStreamReplace(FileStream& stream, const u32 offset, const T& value)
{
	if (offset + sizeof(T) >= stream.capacity)
		return 0;

	u8* valuePtr = (u8*)&value;
	for (u32 i = 0; i < sizeof(T); ++i)
		stream.data[offset + i] = valuePtr[i];

	return 1;
}

// Get a pointer to an offset of the data stored in the file stream
u8* FileStreamGetDataPtr(const FileStream& stream, u32 offset);

// Check if an offset is inside the file
bool FileStreamEnded(const FileStream& stream, u32 offset);

#endif // _FILE_UTILS_H
