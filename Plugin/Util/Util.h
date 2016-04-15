#pragma once

#include <string>

struct Exception {
	CString errorMessage;
	Exception() {};
	Exception(CStringW msg) {
		errorMessage = msg;
	}
	Exception(CStringA msg) {
		errorMessage = msg;
	}
};



// https://www.arclab.com/en/kb/cppmfc/convert-cstring-unicode-utf-16le-to-utf-8-and-reverse.html (2016.03.26)
CStringA ConvertUnicodeToUTF8(const CStringW& uni);
BOOL DirectoryExists(CString szPath);

namespace Log {

	enum Level {
		Level_Debug, Level_Error, Level_Warning
	};
	static void Write(const wchar_t* msg, Log::Level level = Log::Level_Debug) {
		if(level == Level_Error) {
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);  // Get handle to standard output
			CONSOLE_SCREEN_BUFFER_INFO attributes;
			GetConsoleScreenBufferInfo(hConsole, &attributes);

			SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
			std::wcout << msg;
			SetConsoleTextAttribute(hConsole, attributes.wAttributes);
		} else if(level == Level_Warning) {
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);  // Get handle to standard output
			CONSOLE_SCREEN_BUFFER_INFO attributes;
			GetConsoleScreenBufferInfo(hConsole, &attributes);

			SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE);
			std::wcout << msg;
			SetConsoleTextAttribute(hConsole, attributes.wAttributes);
		} else {
			std::wcout << msg;
		}

		std::wcout << std::endl;
	}

	static void Error(CString error) {
		Write(error, Log::Level_Error);
		MessageBox(0, error, L"Error", 0);
	}
};


template<typename T>
CString ToString(T value) {
	return CString(std::to_string(value).c_str());
}

CStringA GetHotkey(CStringA key);

void ListFilesInDirectory(_Out_ std::vector<CString>& files, CString path, CString filter = L"");

#ifdef _WIN32
#define _strcpy(dest, destSize, src) strcpy_s(dest, destSize, src)
#define snprintf sprintf_s
#else
#define _strcpy(dest, destSize, src) { strncpy(dest, src, destSize-1); (dest)[destSize-1] = '\0'; }
#endif
