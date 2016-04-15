#include "stdafx.h"

#include "Util\Util.h"





// https://www.arclab.com/en/kb/cppmfc/convert-cstring-unicode-utf-16le-to-utf-8-and-reverse.html (2016.03.26)
CStringA ConvertUnicodeToUTF8(const CStringW& uni) {
	if(uni.IsEmpty()) return ""; // nothing to do
	CStringA utf8;
	int cc = 0;
	// get length (cc) of the new multibyte string excluding the \0 terminator first
	if((cc = WideCharToMultiByte(CP_UTF8, 0, uni, -1, NULL, 0, 0, 0) - 1) > 0) {
		// convert
		char *buf = utf8.GetBuffer(cc);
		if(buf) WideCharToMultiByte(CP_UTF8, 0, uni, -1, buf, cc, 0, 0);
		utf8.ReleaseBuffer();
	}
	return utf8;
}


BOOL DirectoryExists(CString path) {
	// we have to remove the ending \\ (if exists) in order this function to work
	if(path.Left(1) == L"\\") {
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


	const int length = 2;
	const int size = 256;
	//char hotkeys[length][size];
	char buffer1[size];
	char buffer2[size];
	char* hotkeys[length] = { buffer1, buffer2};


	Global::ts3Functions.getHotkeyFromKeyword(Global::pluginID, keywords, hotkeys, length, size);

	return hotkeys[0];
}

// TODO test
void ListFilesInDirectory(_Out_ std::vector<CString>& files, CString path, CString filter/* = L""*/) {

	if(path.Right(1) != "\\" && path.Right(1) != "/") {
		path += "\\";
	}
	path += "*";


	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;


	if((hFind = FindFirstFile(path, &FindFileData)) == INVALID_HANDLE_VALUE) {
		printf("FindFirstFile failed (%d)\n", GetLastError());
	} else {
		CString fileName(FindFileData.cFileName);

		//if(filter == 0 || fileName.find(filter) == fileName.length() -lstrlen(filter) ){
		if(fileName.Right(filter.GetLength()) == filter) {
			files.push_back(FindFileData.cFileName);
		}

	}

	while(FindNextFile(hFind, &FindFileData)) {
		CString fileName(FindFileData.cFileName);
		//if(filter == 0 || fileName.find(filter) == fileName.length() -lstrlen(filter) ){
		if(fileName.Right(filter.GetLength()) == filter) {
			files.push_back(FindFileData.cFileName);
		}
	}

}





