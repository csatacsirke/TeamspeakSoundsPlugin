#include "stdafx.h"

#include "Config.h"

#include <fstream>
#include <string>
#include <sstream>


namespace TSPlugin {

	namespace Global {
		// TODO igy nem a legszebb
		// update: mé' nem?
		Config config;
	}

	Config::Config() {
		using namespace ConfigKeys;

		entries = {
			{ Volume, L"1.0" },
			{ NormalizeVolume, L"1" },
			{ TargetNormalizedVolume, L"0.2" },
			{ QueueCommand, L"q " },
			{ CommandStarterCharacter, L"/" },
			{ ClearBindingAfterUse, L"1" },
			{ BindCommand, L"bind" },
			{ CanReceiveNetworkSoundData, L"1" },

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

		for (auto& elem : entries) {
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
		entries.insert_or_assign(key_cs, value_cs);
	}


	void Config::Add(CString key, CString value) {
		std::unique_lock lock(mutex);

		entries.insert_or_assign(key, value);
	}

	CString Config::Get(CString key) {
		std::unique_lock lock(mutex);

		const auto it = entries.find(key);
		if (it != entries.end()) {
			return it->second;
		}

		return CString();
	}

	bool Config::TryGet(CString key, _Out_ CString& value) {
		std::unique_lock lock(mutex);

		if (entries.find(key) != entries.end()) {
			value = entries[key];
			return true;
		} else {
			return false;
		}
	}

	optional<CString> Config::TryGet(CString key) {
		std::unique_lock lock(mutex);

		if (entries.find(key) != entries.end()) {
			return entries[key];
		} else {
			return nullopt;
		}
	}

	bool Config::GetBool(const CString& key) {
		return _wtoi(Global::config.Get(key)) != 0;
	}


	ConfigDictionary Config::MakeCopyOfEntries() const {
		// na most ebben nem vagyok biztos, hogy tart a lock addig amig lemásolja.... #YOLO
		std::unique_lock lock(mutex);
		return entries;
	}

	void Config::SetEntries(const ConfigDictionary& newEntries) {
		std::unique_lock lock(mutex);
		entries = newEntries;
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
