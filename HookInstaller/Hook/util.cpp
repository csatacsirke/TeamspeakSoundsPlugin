#include "stdafx.h"
#include "util.h"

// uristen...
bool Bch::StartsWith(tstring str, tstring pref){
	if(str.size() < pref.size()){
		return false;
	}
	for(int i=0; i < (int)pref.size(); ++i){
		if(	pref[i] != str[i] && 
			pref[i]+TEXT('a')-TEXT('A') != str[i] &&
			pref[i] != str[i] +TEXT('a')-TEXT('A') ) return false;
	}
	return true;
}


__int64 FileSize(tstring name)
{
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesEx(name.c_str(), GetFileExInfoStandard, &fad))
        return -1; // error condition, could call GetLastError to find out more
    LARGE_INTEGER size;
    size.HighPart = fad.nFileSizeHigh;
    size.LowPart = fad.nFileSizeLow;
    return size.QuadPart;
}

bool FileExists(LPCTSTR szPath) {
  DWORD dwAttrib = GetFileAttributes(szPath);

  return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

// TODO test
void ListFilesInDirectory(_Out_ std::vector<CString>& files, CString path, CString filter = CString(L"") ){

	if(path.Right(1) != "\\" && path.Right(1) != "/"){
		path += "\\";
	}
	path += "*";
	

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;


	if ((hFind = FindFirstFile(path, &FindFileData)) == INVALID_HANDLE_VALUE)  {
		printf ("FindFirstFile failed (%d)\n", GetLastError());
	} else {
		CString fileName(FindFileData.cFileName);
		
		//if(filter == 0 || fileName.find(filter) == fileName.length() -lstrlen(filter) ){
		if( fileName.Right(filter.GetLength()) == filter ) {
			files.push_back(FindFileData.cFileName);
		}
		
	}

	while(FindNextFile(hFind, &FindFileData)){
		CString fileName(FindFileData.cFileName);
		//if(filter == 0 || fileName.find(filter) == fileName.length() -lstrlen(filter) ){
		if( fileName.Right(filter.GetLength()) == filter ) {
			files.push_back(FindFileData.cFileName);
		}
	}

}


CString ToUnicode(const KBDLLHOOKSTRUCT& hookStruct) {
	const int bufferSize = 10;
	wchar_t buffer[bufferSize];
	BYTE keyboardState[256];
	GetKeyboardState(keyboardState);

	int charsWritten = ToUnicode(hookStruct.vkCode, hookStruct.scanCode, keyboardState, buffer, bufferSize, 0);
		
	if( charsWritten == 1 || charsWritten == 2 ) {
		// ok
	} else {
		ASSERT( charsWritten == 0 );
	}

	return CString(buffer);
}