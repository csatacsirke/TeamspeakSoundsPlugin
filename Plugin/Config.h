#pragma once

#include <afx.h>

class Config {
	std::map<CString, CString> dictionary;

public:
	
	void LoadFromFile(CString fileName);
	void SaveToFile(CString fileName);

	static Config CreateDefault();

	void Add(CString key, CString value);
	CString Get(CString key, CString defaultValue);

private:
	void StoreLine(std::wstring line);
};

namespace Global {
	extern Config config;
}


namespace Hotkey {
	static const CStringA STOP = "keyword_stop";
	static const CStringA PLAY_QUEUED = "keyword_play_queued";
	static const CStringA REPLAY = "keyword_replay";
}