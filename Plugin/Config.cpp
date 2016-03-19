#include "stdafx.h"

#include "Config.h"

#include <fstream>
#include <string>
#include <sstream>

void Config::LoadFromFile(CString fileName) {
	std::wifstream in(fileName);

	while(in) {
		std::wstring line;
		std::getline(in, line);
		
		StoreLine(line);
	}
}

void Config::SaveToFile(CString fileName) {
	std::wofstream out(fileName);
	for(auto& elem : dictionary) {
		out << elem.first << " " << elem.second << std::endl;
	}
}

void Config::StoreLine(std::wstring line) {
	std::wstringstream ss(line);
	
	std::wstring key;
	ss >> key;
	std::wstring value;
	std::getline(ss, value);

	dictionary.insert(std::make_pair<CString, CString>(key.c_str(), value.c_str()));
}


