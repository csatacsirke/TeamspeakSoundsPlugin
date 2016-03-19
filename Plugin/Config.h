#pragma once

#include <map>
#include <afx.h>

class Config {
	std::map<CString, CString> dictionary;

public:
	void LoadFromFile(CString fileName);
	void SaveToFile(CString fileName);

private:
	void StoreLine(std::wstring line);
};
