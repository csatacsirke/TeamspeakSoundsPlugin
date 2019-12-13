#include "stdafx.h"
#include "UpdateHandler.h"

#include <App/plugin.h>
#include <Web/Http.h>


#include <optional>
#include <vector>
#include <filesystem>

namespace TSPlugin {


	static void DownloadAndInstallNewVerison() {

		const optional<vector<uint8_t>> result = Web::HttpRequest(L"users.atw.hu", L"battlechicken/ts/downloads/SoundplayerPlugin_x64.ts3_plugin");
		if (!result) {
			return;
		}

		TCHAR tempDirectoryPath[MAX_PATH];
		GetTempPath(MAX_PATH, tempDirectoryPath);

		using namespace std::filesystem;

		path tempPath = path(tempDirectoryPath) / "SoundplayerPlugin_x64.ts3_plugin";

		ofstream out(tempPath, ios::binary);
		out.write((const char*)result->data(), result->size());
		out.close();


		ShellExecute(0, 0, tempPath.wstring().c_str(), 0, 0, SW_SHOW);

	}




	bool CheckForUpdates() {
		const optional<vector<uint8_t>> result = Web::HttpRequest(L"users.atw.hu", L"battlechicken/ts/version");

		if (!result) {
			return false;
		}



		const CStringA versionOnServer = CStringA((const char*)result->data(), (int)result->size());
		const CStringA currentVersion = ts3plugin_version();

		if (currentVersion.Compare(versionOnServer) >= 0) {
			// we are newer or equal to the server
			return false;
		}

		const CString title = L"Soundplayer plugin by Battlechicken - update available";
		const CString message = L"Newer version exists. Would you like to download it? -- you might have to quit TS after downloading";
		const int messageBoxResult = MessageBox(HWND(0), message, title, MB_YESNO);
		if (messageBoxResult == IDYES) {
			DownloadAndInstallNewVerison();
			return true;
		}

		return false;
	}



}



