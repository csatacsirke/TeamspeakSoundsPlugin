#include "stdafx.h"

#include "Util.h"
#include "Config.h"
#include "Log.h"
#include "Gui/SoundFolderSelector.h"

#include <App/Globals.h>

namespace TSPlugin {

	CStringW Utf8ToCString(const char* utf8Str) {

		const int utf8StrLen = (int)strlen(utf8Str);

		if (utf8StrLen == 0) {
			return L"";
		}

		CStringW cstr;
		LPWSTR ptr = cstr.GetBuffer(utf8StrLen + 1);

		// CString is UNICODE string so we decode
		const int newLen = MultiByteToWideChar(
			CP_UTF8, 0,
			utf8Str, utf8StrLen, ptr, utf8StrLen + 1
		);

		if (!newLen) {
			cstr.ReleaseBuffer(0);
			return cstr;
		}

		cstr.ReleaseBuffer(newLen);

		return cstr;
	}

	// https://www.arclab.com/en/kb/cppmfc/convert-cstring-unicode-utf-16le-to-utf-8-and-reverse.html (2016.03.26)
	CStringA ConvertUnicodeToUTF8(const wchar_t* uni) {
		//if (uni.IsEmpty()) return ""; // nothing to do

		CStringA utf8;
		int cc = 0;
		// get length (cc) of the new multibyte string excluding the \0 terminator first
		if ((cc = WideCharToMultiByte(CP_UTF8, 0, uni, -1, NULL, 0, 0, 0) - 1) > 0) {
			// convert
			char *buf = utf8.GetBuffer(cc);
			if (buf) WideCharToMultiByte(CP_UTF8, 0, uni, -1, buf, cc, 0, 0);
			utf8.ReleaseBuffer();
		}
		return utf8;
	}


	BOOL DirectoryExists(CString path) {
		// we have to remove the ending \\ (if exists) in order this function to work
		if (path.Right(1) == L"\\") {
			path.Truncate(path.GetLength() - 1);
		}

		DWORD dwAttrib = GetFileAttributes(path);

		return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
			(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
	}



	CStringA GetHotkey(CStringA key) {

		const char* keywords[] = {
			key, 0
			//Hotkey::STOP, 0
		};


		const int length = 1;
		const int size = 256;
		//char hotkeys[length][size];
		char buffer1[size];
		char buffer2[size];
		char* hotkeys[2] = { buffer1, buffer2 };

		const int asd = sizeof hotkeys;


		Global::ts3Functions.getHotkeyFromKeyword(Global::pluginID, keywords, hotkeys, length, size);

		return hotkeys[0];
	}

	// TODO test
	std::vector<fs::path> ListFilesInDirectory(const fs::path& directoryPath, const CString& filter/* = L""*/) {
		std::vector<fs::path> files;

		CString glob = directoryPath.c_str();

		if (glob.Right(1) != "\\" && glob.Right(1) != "/") {
			glob += "\\";
		}
		glob += "*";


		WIN32_FIND_DATA findFileData;
		HANDLE hFind;


		if ((hFind = FindFirstFile(glob, &findFileData)) != INVALID_HANDLE_VALUE) {
			CString fileName(findFileData.cFileName);

			//if(filter == 0 || fileName.find(filter) == fileName.length() -lstrlen(filter) ){
			if (fileName.Right(filter.GetLength()) == filter) {
				if (fileName != L"." && fileName != L"..") {
					files.push_back(directoryPath / findFileData.cFileName);
				}
			}

		}

		while (FindNextFile(hFind, &findFileData)) {
			CString fileName(findFileData.cFileName);
			//if(filter == 0 || fileName.find(filter) == fileName.length() -lstrlen(filter) ){
			if (fileName.Right(filter.GetLength()) == filter) {
				if (fileName != L"." && fileName != L"..") {
					files.push_back(directoryPath / findFileData.cFileName);
				}
			}
		}

		return files;
	}

	static inline FILETIME GetModificationDateForFile(const CString& fileName) {
		WIN32_FIND_DATA findData;
		HANDLE h = FindFirstFile(fileName, &findData);
		FindClose(h);
		
		//return findData.ftLastWriteTime;
		return findData.ftCreationTime;
	}

	struct FileInfo {
		CString directory;
		CString fileName;
		FILETIME ftLastWriteTime;
		FileInfo(const CString& directory, const CString& fileName) : directory(directory), fileName(fileName) { ftLastWriteTime = GetModificationDateForFile(directory + fileName); }
		//bool operator < (const FileInfo& other) const { return CompareFileTime(&ftLastWriteTime, &other.ftLastWriteTime) == 1; }
		operator CString() const { return this->fileName; }
	};

	vector<CString> SortFilesByModificationDate(const CString& directory, const vector<CString>& filesNames) {
		//vector<FileInfo> fileInfos(filesNames.begin(), filesNames.end());
		vector<FileInfo> fileInfos;
		fileInfos.reserve(filesNames.size());
		for (const CString& fileName : filesNames) {
			fileInfos.push_back(FileInfo(directory, fileName));
		}


		std::sort(fileInfos.begin(), fileInfos.end(), [](const auto& a, const auto& b) { return CompareFileTime(&a.ftLastWriteTime, &b.ftLastWriteTime) == 1; });
		vector<CString> sorted(fileInfos.begin(), fileInfos.end());

		return sorted;
	}


	//fs::path PickRandomFile(const fs::path& directoryPath) {
	//	std::vector<CString> files = ListFilesInDirectory(directoryPath);
	//
	//	if (files.size() == 0) return L"";
	//
	//	return files[rand() % files.size()];
	//}


	CString FileNameFromPath(CString path) {
		int i;
		for (i = path.GetLength() - 1; i >= 0; --i) {
			if (path[i] == L'/' || path[i] == L'\\') break;
		}
		return path.Right(path.GetLength() - 1 - i);
	}

	//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
	CString GetLastErrorAsString() {
		//Get the error message, if any.
		DWORD errorMessageID = ::GetLastError();
		if (errorMessageID == 0)
			return CString(); //No error message has been recorded

		LPSTR messageBuffer = nullptr;
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

		CString message(messageBuffer, (int)size);

		//Free the buffer.
		LocalFree(messageBuffer);

		return message;
	}


	CString ErrorToString(DWORD error) {
		if (error == 0)
			return CString(); //No error message has been recorded

		LPSTR messageBuffer = nullptr;
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

		CString message(messageBuffer, (int)size);

		//Free the buffer.
		LocalFree(messageBuffer);

		return message;
	}


	optional<CString> TryGetSoundsDirectory(TryGetSoundsDirectoryOptions options) {

		while (true) {

			CString directory = Global::config.Get(ConfigKeys::SoundFolder);
			if (DirectoryExists(directory)) {
				return directory;
			} else {
				if (options == TryGetSoundsDirectoryOptions::AskGui) {
					SoundFolderSelector dialog;
					auto result = dialog.DoModal();
					if (result == IDOK) {
						continue;
					} else {
						return nullopt;
					}
				} else {
					return nullopt;
				}
			}
		}

		return nullopt;
	}
	std::vector<fs::path> _GetPossibleFiles(const CString& inputString) {

		optional<CString> directoryOrNull = TryGetSoundsDirectory();
		if (!directoryOrNull) {
			return {};
		}


		fs::path directory = directoryOrNull->GetString();
		//CString directory = *directoryOrNull;


		//if (directory.Right(1) != "\\" && directory.Right(1) != "/") {
		//	directory += "\\";
		//}


		vector<fs::path> files = ListFilesInDirectory(directory);

		return GetPossibleFiles(inputString, files);
	}

	std::vector<fs::path> GetPossibleFiles(const CString& inputString, const std::vector<fs::path>& files) {


		std::vector<fs::path> results;

		if (inputString.GetLength() > 0) {
			const CString sanitizedInputString = MakeComparable(inputString);

			for (const fs::path& file : files) {
				const CString sanitizedFileName = MakeComparable(file.filename().c_str());

				if (sanitizedFileName.Find(sanitizedInputString) == 0) {
					results.push_back(file);
				}
			}


			for (const fs::path& file : files) {
				const CString sanitizedFileName = MakeComparable(file.filename().c_str());

				// akkor is a lista v�g�re f�zz�k, ha nem az elej�n van a cucc
				if (sanitizedFileName.Find(sanitizedInputString) > 0) {
					fs::path result = file;
					results.push_back(file);
				}
			}
		}
		


		return results;
	}

	optional<fs::path> TryGetSoundFileForUserInput(const CString& inputString) {

		vector<fs::path> possibleFiles = _GetPossibleFiles(inputString);
		if (possibleFiles.size() >= 1) {
			return possibleFiles.front();
		}

		return nullopt;
	}




	void OpenConsole() {
		AllocConsole();
		FILE* pCout;
		freopen_s(&pCout, "CONOUT$", "w", stdout);

		FILE* pCin;
		freopen_s(&pCin, "CONIN$", "r", stdin);
		std::cout.clear();
		std::wcout.clear();

		std::wstring log_history = Log::PopLogHistory();
		std::wcout << log_history << endl;
	}

}
