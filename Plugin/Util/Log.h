#pragma once


#include <fstream>
#include <iostream>


namespace Log {

	enum class Level {
		Level_Debug, Level_Error, Level_Warning	
	};
	
	void Write(const wchar_t* msg, std::wostream& stream);
	void Write(const wchar_t* msg);
	void Write(const wchar_t* msg, Level level);
	void Error(CString error);
	void Debug(CString msg);
	void Warning(CString msg);

	std::wstring PopLogHistory();
};


