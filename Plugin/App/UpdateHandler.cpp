#include "stdafx.h"
#include "UpdateHandler.h"

#include <App/plugin.h>
#include <Util/Config.h>
#include <Web/Http.h>


#include <optional>
#include <vector>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace TSPlugin {


	static void DownloadAndInstallNewVerison() {

		const CString updateChannel = Global::config.GetBool(ConfigKeys::BetaVersion) ? L"_beta" : L"";
		const CString endpoint = FormatString(L"battlechicken/ts/downloads/SoundplayerPlugin_x64%s.ts3_plugin", updateChannel);


		const optional<vector<uint8_t>> result = Web::HttpRequest(L"users.atw.hu", endpoint);
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


	struct ChangeEntry {
		std::string date;
		std::vector<std::string> changes;
	};
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ChangeEntry, date, changes)

	static optional<vector<CString>> DownloadChanges(const CStringA& currentVersion, const CStringA& serverVersion) {

		const optional<vector<uint8_t>> optChangesJson = Web::HttpRequest(L"users.atw.hu", L"battlechicken/ts/changes.php");
		if (!optChangesJson) {
			return nullopt;
		}

		

		try {
			auto jsonArray = nlohmann::json::parse(*optChangesJson);

			std::vector<ChangeEntry> changeEntries;
			jsonArray.get_to(changeEntries);

			vector<CString> result;

			for (auto& changeEntry : changeEntries) {
				if (changeEntry.date.compare(currentVersion.GetString()) <= 0) {
					continue;
				}

				if (changeEntry.date.compare(serverVersion.GetString()) > 0) {
					// possible, if git already has a new feature, but its not marked as stable
					continue;
				}

				for (auto& changeString : changeEntry.changes) {
					result.push_back(Utf8ToCString(changeString.c_str()));
				}

			}

			return result;
		} catch (std::exception& e) {
			const char* what = e.what();
			std::ignore = what;
			return nullopt;
		}
		
	}

	static optional<CStringA> DownloadVersionOnServer() {

		const CString updateChannel = Global::config.GetBool(ConfigKeys::BetaVersion) ? L"beta" : L"stable";
		const CString endpoint = FormatString(L"battlechicken/ts/version?channel=%s", updateChannel);

		const optional<vector<uint8_t>> result = Web::HttpRequest(L"users.atw.hu", endpoint);

		if (!result) {
			return nullopt;
		}


		const CStringA versionOnServer = CStringA((const char*)result->data(), (int)result->size());

		return versionOnServer;
	}

	bool CheckForUpdates() {
//#ifdef _DEBUG
//		return false;
//#endif

		const optional<CStringA> optVersionOnServer = DownloadVersionOnServer();
		if (!optVersionOnServer) {
			return false;
		}

		const CStringA versionOnServer = *optVersionOnServer;

		const CStringA currentVersion = ts3plugin_version();

		if (currentVersion.Compare(versionOnServer) >= 0) {
			// we are newer or equal to the server
			return false;
		}


		auto optChanges = DownloadChanges(currentVersion, versionOnServer);
		if (!optChanges) {
			return false;
		}

		const CString title = L"Soundplayer plugin by Battlechicken - update available";
		CString message = L"Newer version exists. Would you like to download it? -- you might have to quit TS after downloading";

		if (optChanges->size() > 0) {
			message += L"\nChanges since this version: \n";
			for (auto& changeLine : *optChanges) {
				message += FormatString(L"- %s\n", changeLine);
			}
		}

		const int messageBoxResult = MessageBox(HWND(0), message, title, MB_YESNO);
		if (messageBoxResult == IDYES) {
			DownloadAndInstallNewVerison();
			return true;
		}

		return false;
	}



}



