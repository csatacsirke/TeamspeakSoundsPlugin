#pragma once

extern std::ofstream logFile;

#include <fstream>
#include <iostream>


namespace Log {

	enum Level {
		Level_Debug, Level_Error, Level_Warning
	};

	extern std::ofstream logFile;

	static void Write(const wchar_t* msg, std::ostream& stream) {

		if (stream.fail()) {
			stream.clear();
		}
		stream << msg << std::endl;

		//Write(msg);

		if (stream.fail()) {
			stream.clear();
		}
	}

	static void Write(const wchar_t* msg) {
		Write(msg, logFile);
		Write(msg, std::cout);
	}

	static void Write(const wchar_t* msg, Log::Level level) {

			//if (std::wcout.fail()) {
			//	std::wcout.clear();
			//}

		if (level == Level_Error) {
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);  // Get handle to standard output
			CONSOLE_SCREEN_BUFFER_INFO attributes;
			GetConsoleScreenBufferInfo(hConsole, &attributes);

			SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
			Write(msg);
			//std::wcout << msg;
			SetConsoleTextAttribute(hConsole, attributes.wAttributes);
		} else if (level == Level_Warning) {
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);  // Get handle to standard output
			CONSOLE_SCREEN_BUFFER_INFO attributes;
			GetConsoleScreenBufferInfo(hConsole, &attributes);

			// sájga
			SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
			Write(msg);
			//std::wcout << msg;
			SetConsoleTextAttribute(hConsole, attributes.wAttributes);
		} else {
			Write(msg);
			//std::wcout << msg;
		}

		//Write(std::endl);
		//std::wcout << std::endl;


		//if (std::wcout.fail()) {
		//	std::wcout.clear();
		//}
	}

	static void Error(CString error) {
		Write(error, Log::Level_Error);
		MessageBox(0, error, L"Error", 0);
	}

	static void Debug(CString msg) {
		Write(msg, Log::Level_Debug);
	}

	static void Warning(CString msg) {
		Write(msg, Log::Level_Warning);
	}
};


