#pragma once

#include <KeyboardHook/KeyboardHookUtils.h>

namespace TSPlugin {



	struct CommandLineInterfaceItem {
		fs::path filePath;

	};


	class CommandLineInterface {
		mutable mutex internal_mutex;

		//vector<fs::path> possibleFilesForCurrentInput;
		vector<fs::path> allFiles;

		vector<shared_ptr<CommandLineInterfaceItem>> interfaceItems;

		
		int fileListOffset = 0;
		int maxVisibleFileCount = 30;
		
		size_t selectedInterfaceItemIndex = 0;
		CString inputBuffer;

	public:

		CommandLineInterface();

		void UpdateCachedFilesIfNecessary();

		void AddInput(const KeyboardHook::KeyData& keyData);
		void Clear();


		CStringA CreateTextInterface();
		CString CopyBuffer() const;

		optional<fs::path> TryGetSelectedFile();

		void ShiftSelection(int indexDelta);
		void PageUp();
		void PageDown();

	private:
		void UpdateInterface();

	};

}

