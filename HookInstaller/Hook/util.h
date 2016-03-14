#pragma once

#include <string>
#include <tchar.h>

#ifndef tstring
#define tstring std::basic_string<TCHAR>
#endif


namespace Bch {
	bool StartsWith(tstring str, tstring pref);

}

__int64 FileSize(tstring name);
bool FileExists(LPCTSTR szPath);

template<typename T>
CString ToString(T val) {
	return CString(std::to_string(val).c_str());
}

CString ToUnicode(const KBDLLHOOKSTRUCT& hookStruct);

	