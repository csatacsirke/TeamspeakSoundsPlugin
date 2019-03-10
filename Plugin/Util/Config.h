#pragma once

#include <afx.h>

#include <optional>

namespace TSPlugin {

	class Config {
		map<CString, CString> dictionary;
		CString fileName;
	public:
		const CString defaultFileName = _T("soundplayer.cfg");
	public:

		void LoadFromFile(CString fileName);
		void SaveToFile(CString fileName);
		void Save();

		static Config CreateDefault();

		void Add(CString key, CString value);
		CString Get(CString key, CString defaultValue = L"");
		bool TryGet(CString key, _Out_ CString& value);
		optional<CString> TryGet(CString key);

	private:
		void StoreLine(wstring line);

	};

	namespace ConfigKey {
		static const CString SoundFolder = L"SoundFolder";
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
