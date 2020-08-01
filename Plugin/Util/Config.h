#pragma once

#include <afx.h>


#include <optional>
#include <sstream>


namespace TSPlugin {

	typedef map<CString, CString> ConfigDictionary;


	template <typename T>
	optional<T> Convert(const wstring& s) {
		std::wstringstream ss(s);

		T result;
		if ((ss >> result).fail() || !(ss >> std::ws).eof()) {
			return nullopt;
		}

		return result;
	}

	class Config {
		ConfigDictionary entries;
		CString fileName;
		mutable std::mutex mutex;
	public:
		const CString defaultFileName = _T("soundplayer.cfg");
	public:
		Config();

		void LoadFromFile(CString fileName);
		void SaveToFile(CString fileName);
		void Save();

		//static Config CreateDefault();

		void Add(CString key, CString value);
		CString Get(CString key);
		bool TryGet(CString key, _Out_ CString& value);
		optional<CString> TryGet(CString key);

		template<class T>
		optional<T> Get(const CString& key) {
			optional<CString> optValue = TryGet(key);
			if (!optValue) {
				return nullopt;
			}
			return Convert<T>(wstring(*optValue));
		}

		//optional<double> GetDouble(const CString& key);
		//optional<float> GetFloat(const CString& key);
		//optional<int> GetInt(const CString& key);

		bool GetBool(const CString& key);


		ConfigDictionary MakeCopyOfEntries() const;
		void SetEntries(const ConfigDictionary& newEntries);



	private:
		void StoreLine(wstring line);

	};



	namespace ConfigKeys {
		static const CString SoundFolder = L"SoundFolder";
		static const CString NormalizeVolume = L"NormalizeVolume";
		static const CString PerceptiveVolumeNormalization = L"PerceptiveVolumeNormalization";
		static const CString Volume = L"Volume";
		static const CString TargetNormalizedVolume = L"TargetNormalizedVolume";
		static const CString QueueCommand = L"QueueCommand";
		static const CString BindCommand = L"BindCommand";
		static const CString CommandStarterCharacter = L"CommandStarterCharacter";
		static const CString ClearBindingAfterUse = L"ClearBindingAfterUse";
		static const CString CanReceiveNetworkSoundData = L"CanReceiveNetworkSoundData";
		static const CString ShouldDisplayOverlay = L"ShouldDisplayOverlay";
		static const CString OverlayFontSize = L"OverlayFontSize";
		static const CString OverlayBackgroundAlpha = L"OverlayBackgroundAlpha";
		
		//static const CString PresetPathTemplate = L"PresetPath%d";
	}

	namespace Global {
		extern Config config;
	}

	// TODO config file
	//static const int soundHotkeyCount = 9;


	namespace Hotkey {
		static const CStringA STOP = "keyword_stop";
		static const CStringA PLAY_QUEUED = "keyword_play_queued";
		static const CStringA REPLAY = "keyword_replay";
		static const CStringA PLAY_RANDOM = "keyword_play_random";
		//static const CStringA PLAY_PRESET_TEMPLATE = "keyword_play_preset_%d";
	}



}
