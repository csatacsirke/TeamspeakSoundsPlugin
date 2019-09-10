#include "stdafx.h"

#include "Config.h"

#include <fstream>
#include <string>
#include <sstream>


namespace TSPlugin {

	namespace Global {
		// TODO igy nem a legszebb
		Config config;
	}

	Config::Config() {
		using namespace ConfigKey;

		dictionary = {
			{ Volume, L"1.0" },
			{ NormalizeVolume, L"1" },
			{ TargetNormalizedVolume, L"0.2" },
			
		};
	}

	void Config::LoadFromFile(CString fileName) {
		std::unique_lock lock(mutex);

		this->fileName = fileName;

		std::wifstream in(fileName);

		ASSERT((bool)in);

		while (in) {
			std::wstring line;
			std::getline(in, line);

			StoreLine(line);
		}
	}

	void Config::SaveToFile(CString fileName) {
		std::unique_lock lock(mutex);


		std::wofstream out(fileName);
		assert((bool)out);

		for (auto& elem : dictionary) {
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
		dictionary.insert_or_assign(key_cs, value_cs);
	}


	void Config::Add(CString key, CString value) {
		std::unique_lock lock(mutex);

		dictionary.insert_or_assign(key, value);
	}

	CString Config::Get(CString key) {
		std::unique_lock lock(mutex);

		const auto it = dictionary.find(key);
		if (it != dictionary.end()) {
			return it->second;
		}

		return CString();
	}

	bool Config::TryGet(CString key, _Out_ CString& value) {
		std::unique_lock lock(mutex);

		if (dictionary.find(key) != dictionary.end()) {
			value = dictionary[key];
			return true;
		} else {
			return false;
		}
	}

	optional<CString> Config::TryGet(CString key) {
		std::unique_lock lock(mutex);

		if (dictionary.find(key) != dictionary.end()) {
			return dictionary[key];
		} else {
			return nullopt;
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


}
