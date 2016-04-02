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




