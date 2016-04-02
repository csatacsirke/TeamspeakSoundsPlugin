#pragma once

#include <afx.h>

class Config {
	std::map<CString, CString> dictionary;

public:
	void LoadFromFile(CString fileName);
	void SaveToFile(CString fileName);

	static Config CreateDefault();

	void Add(CString key, CString value);
	CString Get(CString key, CString asd);

private:
	void StoreLine(std::wstring line);
};
