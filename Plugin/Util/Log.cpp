#include "stdafx.h"
#include "Log.h"

#include <mutex>


namespace Log {
	// extern global
	//std::wofstream logFile("soundplayer_log.txt", std::ostream::app);
	std::recursive_mutex log_mutex;
	std::wstring log_history;
	
	void Write(const wchar_t* msg, std::wostream& stream) {

		if (stream.fail()) {
			stream.clear();
		}
		stream << msg << std::endl;

		//Write(msg);

		if (stream.fail()) {
			stream.clear();
		}
	}

	void Write(const wchar_t* msg) {
		std::lock_guard guard(log_mutex);

		//Write(msg, Log::logFile);
		Write(msg, std::wcout);
		TRACE("%S", msg);
		log_history += msg;
	}

	void Write(const wchar_t* msg, Level level) {

		std::lock_guard guard(log_mutex);

		//if (std::wcout.fail()) {
		//	std::wcout.clear();
		//}

		if (level == Level::Level_Error) {
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);  // Get handle to standard output
			CONSOLE_SCREEN_BUFFER_INFO attributes;
			GetConsoleScreenBufferInfo(hConsole, &attributes);

			SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
			Write(msg);
			//std::wcout << msg;
			SetConsoleTextAttribute(hConsole, attributes.wAttributes);
		} else if (level == Level::Level_Warning) {
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

	void Error(CString error) {
		Write(error, Level::Level_Error);
		MessageBox(0, error, L"Error", 0);
	}

	void Debug(CString msg) {
		Write(CString("[D]") + msg, Level::Level_Debug);
	}

	void Warning(CString msg) {
		Write(msg, Level::Level_Warning);
	}

	std::wstring PopLogHistory() {
		std::lock_guard guard(log_mutex);
		return std::move(log_history);
	}

}




//std::wofstream Log::logFile("soundplayer_log.txt");

