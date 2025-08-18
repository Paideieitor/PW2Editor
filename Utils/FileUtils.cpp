#include <filesystem>

#include "System.h"

#include "Utils/FileUtils.h"
#include "FileUtils.h"

bool PathExists(const string& path)
{
	return filesystem::exists(path);
}

bool IsFilePath(const string& path)
{
	return !filesystem::is_directory(path);
}

string GetAbsolutePath(const string& path)
{
	std::error_code ec;
	filesystem::path absolute = filesystem::absolute(path, ec);
	if (absolute == filesystem::path())
	{
		Log(CRITICAL, "Error getting absolute path %s (%s)", path.c_str(), ec.message().c_str());
		return string();
	}
	return absolute.u8string();
}

string GetFileExtension(const string& path)
{
	if (!IsFilePath(path))
		return string();

	size_t start = path.find_last_of('.');
	if (start == string::npos)
		return string();

	++start;
	return path.substr(start, path.length() - start);
}

bool CreateFile(const string& path)
{
	string dirPath = RemoveFileFromPath(path);
	if (!PathExists(dirPath))
		CreateFolder(dirPath);

	FILE* file = nullptr;
	fopen_s(&file, path.c_str(), "w");
	if (!file)
		return false;

	fclose(file);
	return true;
}

bool RemoveFile(const string& path)
{
	std::error_code ec;
	if (!filesystem::remove(path, ec))
	{
		Log(CRITICAL, "Error removing file %s (%s)", path.c_str(), ec.message().c_str());
		return false;
	}
	return true;
}

bool CopyFile(const string& path, const string& copyPath)
{
	if (!PathExists(path))
		return false;

	string dirPath = RemoveFileFromPath(copyPath);
	if (!PathExists(dirPath))
		CreateFolder(dirPath);

	std::error_code ec;
	if (!filesystem::copy_file(path, copyPath, filesystem::copy_options::overwrite_existing, ec))
	{
		Log(CRITICAL, "Error copying file %s (%s)", path.c_str(), ec.message().c_str());
		return false;
	}
	return true;
}

bool CreateFolder(const string& path)
{
	std::error_code ec;
	if (!filesystem::create_directories(path, ec))
	{
		Log(CRITICAL, "Error creating directory %s (%s)", path.c_str(), ec.message().c_str());
		return false;
	}
	return true;
}

int RemoveFolder(const string& path)
{
	std::error_code ec;
	int output = (u32)filesystem::remove_all(path, ec);
	if (output == -1)
		Log(CRITICAL, "Error removing folder %s (%s)", path.c_str(), ec.message().c_str());
	return output;
}

vector<string> GetFolderElementList(const string& path)
{
	vector<string> folderList;
	if (IsFilePath(path))
		return folderList;

	for (const filesystem::directory_entry& entry : filesystem::directory_iterator(path))
	{
		string path = entry.path().u8string();
		NormalizePathSeparator(path);

		path = GetFileFromPath(path, 0);

		folderList.push_back(path);
	}

	return folderList;
}

void NormalizePathSeparator(string& path)
{
	for (u32 idx = 0; idx < path.length(); ++idx)
		if (path[idx] == NOT_PATH_SEPARATOR)
			path[idx] = PATH_SEPARATOR;
}

string PathConcat(const string& path, const string& concat)
{
	if (path.empty())
		return concat;
	if (concat.empty())
		return path;
	return path + PATH_SEPARATOR + concat;
}

string RemoveFileFromPath(const string& path)
{
	size_t lastSlash = path.find_last_of(PATH_SEPARATOR);
	if (lastSlash == string::npos)
		return "";
	return path.substr(0, lastSlash);
}

string GetFileFromPath(const string& path, bool checkFile)
{
	if (checkFile && !IsFilePath(path))
		return string();

	size_t lastSlash = path.find_last_of(PATH_SEPARATOR);
	if (lastSlash == string::npos)
		return path;
	return path.substr(lastSlash + 1);
}

string PathRemoveExtension(const string& path)
{
	string extension = GetFileExtension(path);
	if (extension.empty())
		return path;

	return path.substr(0, path.length() - extension.length() - 1);
}

string PathGetLastName(const string& path)
{
	string file = GetFileFromPath(path, false);
	if (file.empty())
		return string();

	return PathRemoveExtension(file);
}

string PathEraseLastName(const string& path)
{
	size_t lastSlash = path.find_last_of(PATH_SEPARATOR);
	if (lastSlash == string::npos)
		return path;
	return path.substr(0, lastSlash);
}

bool SeparatePathAndFile(string& path, string& file)
{
	if (!IsFilePath(path))
		return false;

	size_t lastSlash = path.find_last_of(PATH_SEPARATOR) + 1;
	if (lastSlash == string::npos)
	{
		file = path;
		path = string();
	}
	else
	{
		file = path.substr(lastSlash);
		path = path.substr(0, lastSlash - 1);
	}

	return true;
}

bool LoadEmptyFileStream(FileStream& stream)
{
	stream.length = 0;

	stream.data = new u8[16];
	if (!stream.data)
	{
		Log(WARNING, "Memory allocation failure in file");
		return false;
	}
	stream.capacity = 16;

	return true;
}

bool LoadFileStream(FileStream& stream, const string& path)
{
	stream.length = (u32)filesystem::file_size(path);
	if (!stream.length)
	{
		Log(WARNING, "Empty file %s", path.c_str());
		return false;
	}

	FILE* file = nullptr;
	fopen_s(&file, path.c_str(), "rb");
	if (!file)
	{
		Log(WARNING, "Couldn't open file %s", path.c_str());
		return false;
	}

	stream.data = new u8[stream.length];
	if (!stream.data)
	{
		Log(WARNING, "Memory allocation failure in file %s", path.c_str());
		fclose(file);
		return false;
	}
	stream.capacity = stream.length;

	fread_s(stream.data, stream.length, sizeof(u8), stream.length, file);
	fgetc(file);

	if (!feof(file))
		Log(WARNING, "Unread data in file %s", path.c_str());

	fclose(file);
	return true;
}

bool SaveFileStream(const FileStream& stream, const string& path)
{
	string dirPath = RemoveFileFromPath(path);
	if (!PathExists(dirPath))
		CreateFolder(dirPath);

	FILE* file = nullptr;
	fopen_s(&file, path.c_str(), "wb");
	if (!file)
	{
		Log(WARNING, "Couldn't open file %s", path.c_str());
		return false;
	}

	fwrite(stream.data, sizeof(u8), stream.length, file);

	fclose(file);
	return true;
}

void ReleaseFileStream(FileStream& stream)
{
	delete[] stream.data;

	stream.data = nullptr;
	stream.length = 0;
}

void FileStreamBufferRead(const FileStream& stream, u32 offset, u8* buffer, u32 length)
{
	memcpy(buffer, stream.data + offset, length);
}

void FileStreamBufferReadUpdate(const FileStream& stream, u32& offset, u8* buffer, u32 length)
{
	FileStreamBufferRead(stream, offset, buffer, length);
	offset += length;
}

void FileStreamExpand(FileStream& stream)
{
	stream.capacity *= 2;
	u8* data = new u8[stream.capacity];
	memcpy(data, stream.data, stream.length);

	delete[] stream.data;
	stream.data = data;
}

void FileStreamBufferWriteBack(FileStream& stream, const u8* buffer, u32 length)
{
	while (stream.length + length >= stream.capacity)
		FileStreamExpand(stream);

	memcpy(stream.data + stream.length, buffer, length);
	stream.length += length;
}

u8* FileStreamGetDataPtr(const FileStream& stream, u32 offset)
{
	return stream.data + offset;
}

bool FileStreamEnded(const FileStream& stream, u32 offset)
{
	return offset >= stream.length;
}
