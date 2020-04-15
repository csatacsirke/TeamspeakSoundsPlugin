#pragma once

#include "DirectoryHandler.h"

#include <KeyboardHook/KeyboardHookUtils.h>

namespace TSPlugin {



	struct CommandLineInterfaceItem {
		fs::path filePath;

	};


	class CommandLineInterface {
		mutable mutex internal_mutex;

		//vector<fs::path> possibleFilesForCurrentInput;
		
		shared_ptr<DirectoryHandler> directoryHandler = make_shared<DirectoryHandler>();

		vector<shared_ptr<CommandLineInterfaceItem>> interfaceItems;

		
		int fileListOffset = 0;
		int maxVisibleFileCount = 30;
		
		size_t selectedInterfaceItemIndex = 0;
		CString inputBuffer;

	public:

		CommandLineInterface();

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

