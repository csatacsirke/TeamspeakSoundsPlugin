#include "stdafx.h"

#include "Config.h"

#include <fstream>
#include <string>
#include <sstream>

namespace Global {
	// TODO igy nem a legszebb
	Config config;
}

void Config::LoadFromFile(CString fileName) {
	this->fileName = fileName;

	std::wifstream in(fileName);

	assert((bool)in);

	while(in) {
		std::wstring line;
		std::getline(in, line);
		
		StoreLine(line);
	}
}

void Config::SaveToFile(CString fileName) {
	std::wofstream out(fileName);
	assert((bool)out);

	for(auto& elem : dictionary) {
		out << (const wchar_t*)elem.first << L" " << (const wchar_t*)elem.second << std::endl;
	}
	out.close();
}

void Config::StoreLine(std::wstring line) {
	std::wstringstream ss(line);
	
	std::wstring key;
	ss >> key;
	std::wstring value;
	std::getline(ss, value);


	CString value_cs = CString(value.c_str()).TrimLeft();
	CString key_cs = key.c_str();
	dictionary.insert(std::make_pair(key_cs, value_cs));
}


Config Config::CreateDefault() {
	Config config;
	config.LoadFromFile(config.defaultFileName);
	return config;
}


void Config::Add(CString key, CString value) {
	//auto asd = std::make_pair<CString, CString>(key, value);
	//std::make_pair<CString, CString>()
	//dictionary.insert_or_assign(std::make_pair(key, value));
	dictionary.insert_or_assign(key, value);
	//dictionary.insert(key, value);
}

CString Config::Get(CString key, CString defaultValue) {
	CString result = defaultValue;
	if(dictionary.find(key) != dictionary.end()) {
		result = dictionary[key];
	} 
	
	return result;
}

bool Config::TryGet(CString key, _Out_ CString& value) {
	if(dictionary.find(key) != dictionary.end()) {
		value = dictionary[key];
		return true;
	} else {
		return false;
	}
}

void Config::Save() {
	SaveToFile(this->fileName);
	//if(_taccess(this->fileName, 0)) {
	//	
	//} else {
	//	assert(0);
	//}
}


