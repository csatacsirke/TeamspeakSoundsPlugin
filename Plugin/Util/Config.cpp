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
			{ PerceptiveVolumeNormalization, L"0" },
			{ TargetNormalizedVolume, L"0.2" },
			{ QueueCommand, L"q " },
			{ CommandStarterCharacter, L"/" },
			{ ClearBindingAfterUse, L"1" },
			{ BindCommand, L"bind" },
			{ CanReceiveNetworkSoundData, L"0" },
			{ ShouldDisplayOverlay, L"1" },
			{ OverlayBackgroundAlpha, L"96" },
			{ OverlayFontSize, L"18" },
			{ BetaVersion, L"0" },
			{ TwitchSession, L"" },
			{ AuthorizedUsers, L"battlechicken,bogeczki,atrax,ugyismegkurlak" },
		};
	}

	void Config::LoadFromFile(const fs::path& path) {
		std::unique_lock lock(mutex);

		_path = path;

		std::wifstream in(path);

		while (in) {
			std::wstring line;
			std::getline(in, line);

			StoreLine(line);
		}
	}

	void Config::SaveToFile(const fs::path& path) {
		std::unique_lock lock(mutex);


		std::wofstream out(path);
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


		CString value_cs = CString(value.c_str()).Trim();
		CString key_cs = CString(key.c_str()).Trim();
		if (key_cs.GetLength() != 0) {
			entries.insert_or_assign(key_cs, value_cs);
		}
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

	//optional<double> Config::GetDouble(const CString& key) {
	//	optional<CString> optValue = TryGet(key);
	//	if (!optValue) {
	//		return nullopt;
	//	}
	//	return Convert<double>(*optValue);
	//}

	//optional<float> Config::GetFloat(const CString& key) {
	//	optional<CString> optValue = TryGet(key);
	//	if (!optValue) {
	//		return nullopt;
	//	}
	//	return (float)_wtof(*optValue);
	//}

	//optional<int> Config::GetInt(const CString& key) {
	//	optional<CString> optValue = TryGet(key);
	//	if (!optValue) {
	//		return nullopt;
	//	}
	//	return _wtoi(*optValue);
	//}

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
		SaveToFile(_path);
	}


}
