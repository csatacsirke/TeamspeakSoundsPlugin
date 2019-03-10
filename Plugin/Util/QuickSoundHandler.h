#pragma once


#include <Util/Util.h>
#include <Util/Config.h>



#include <filesystem>
#include <regex>


template<class IteratorType>
IteratorType begin(const std::pair<IteratorType, IteratorType>& iterator) {
	return iterator.first;
}

template<class IteratorType>
IteratorType end(const std::pair<IteratorType, IteratorType>& iterator) {
	return iterator.second;
}

namespace TSPlugin {

	namespace fs = std::experimental::filesystem;



	class QuickSoundsFileSystem {
	public:

		QuickSoundsFileSystem() {
			auto soundFolderOrNull = Global::config.TryGet(ConfigKey::SoundFolder);
			if (!soundFolderOrNull) {
				return;
			}

			fs::path basePath = (wstring)*soundFolderOrNull;


			fs::path path = basePath / "QuickSounds";

			CreateDirectory(path.c_str(), NULL);

			for (const auto& p : fs::recursive_directory_iterator(path)) {
				AddShortcutForPath(p.path());
			}
		}

		void AddShortcutForPath(const fs::path& fullPath) {
			CString shortcut = ShortcutFromFileName(fullPath.filename());

			if (shortcut.GetLength() == 0) {
				return;
			}

			shortcutsToPathsMapping.insert({ shortcut, fullPath });
		}


		std::vector<fs::path> FindMatchesForKey(const CString& key) {

			std::vector<fs::path> matches;

			for (auto pair : shortcutsToPathsMapping.equal_range(key)) {
#ifdef _DEBUG
				const CString& key = pair.first;
#endif
				const fs::path& path = pair.second;
				matches.push_back(path);
			}

			return matches;
		}


		CString ShortcutFromFileName(const wstring& fileName) {
			size_t sepatatorIndex = fileName.find(L'.');
			if (sepatatorIndex == wstring::npos) {
				return L"";
			}

			wstring shortcut = fileName.substr(0, sepatatorIndex);
			return shortcut.c_str();
		}

	private:
		std::multimap<CString, fs::path> shortcutsToPathsMapping;
	};


	class QuickSoundHandlerDelegate {
	public:
		virtual void OnQuickSoundMatch(const CString& path) = 0;
	};

	class QuickSoundHandler {
		QuickSoundHandlerDelegate& delegate;
		QuickSoundsFileSystem fileSystem;

		CString capturedSequence;
	public:

		QuickSoundHandler(QuickSoundHandlerDelegate& delegate) : delegate(delegate) {}
		//HookResult TryConsumeEvent(const KeyboardHook::KeyData& keyData);

		HookResult QuickSoundHandler::TryConsumeEvent(const KeyboardHook::KeyData& keyData) {
			CString key = keyData.unicodeLiteral;

			if (key.GetLength() == 0) {
				return HookResult::PassEvent;
			}

			capturedSequence += key;

			if (capturedSequence.GetLength() > 0) {
				if (towlower(capturedSequence[0]) != L'v') {
					capturedSequence = L"";
					return HookResult::PassEvent;
				}
			}

			if (capturedSequence.GetLength() > 1) {
				if (!iswdigit(capturedSequence[1])) {
					capturedSequence = L"";
					return HookResult::PassEvent;
				}
			}

			if (capturedSequence.GetLength() > 2) {
				if (!iswdigit(capturedSequence[2])) {
					capturedSequence = L"";
					return HookResult::PassEvent;
				}
			}

			// ha elso betu akkor no consume, de a második és harmadik már igen
			if (capturedSequence.GetLength() == 2) {
				return HookResult::ConsumeEvent;
			}

			if (capturedSequence.GetLength() == 1) {
				return HookResult::PassEvent;
			}



			auto matches = fileSystem.FindMatchesForKey(capturedSequence);

			capturedSequence = L"";

			if (matches.size() == 0) {
				return HookResult::ConsumeEvent;
			}


			int index = rand() % matches.size();
			delegate.OnQuickSoundMatch(matches.at(index).c_str());


			return HookResult::ConsumeEvent;
		}



	};

}


