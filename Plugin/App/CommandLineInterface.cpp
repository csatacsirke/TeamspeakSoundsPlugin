#include "stdafx.h"
#include "CommandLineInterface.h"


#include <Util/Util.h>

namespace TSPlugin {


	CommandLineInterface::CommandLineInterface() {
		UpdateCachedFilesIfNecessary();
	}

	void CommandLineInterface::UpdateCachedFilesIfNecessary() {
		if (allFiles.size() != 0) {
			return;
		}

		if (optional<CString> directoryOrNull = TryGetSoundsDirectory()) {
			fs::path directory = directoryOrNull->GetString();
			allFiles = ListFilesInDirectory(directory);
		}
	}

	void CommandLineInterface::AddInput(const KeyboardHook::KeyData& keyData) {
		unique_lock<mutex> lock(internal_mutex);


		if (keyData.hookData.vkCode == VK_BACK) {
			if (inputBuffer.GetLength() > 0) {
				inputBuffer.Truncate(inputBuffer.GetLength() - 1);
			}
		} else {
			inputBuffer += keyData.unicodeLiteral;
		}

		selectedInterfaceItemIndex = 0;
		

		UpdateInterface();

	}

	void CommandLineInterface::Clear() {
		inputBuffer = L"";
		

		UpdateInterface();

	}

	void CommandLineInterface::UpdateInterface() {

		UpdateCachedFilesIfNecessary();
		possibleFilesForCurrentInput = GetPossibleFiles(inputBuffer, allFiles);


		interfaceItems.clear();

		for (const fs::path& path : possibleFilesForCurrentInput) {

			if (interfaceItems.size() >= maxVisibleFileCount) {
				break;
			}

			interfaceItems.push_back(make_shared<CommandLineInterfaceItem>(CommandLineInterfaceItem{ path }));
		}

		if (inputBuffer.GetLength() > 0) {
			interfaceItems.push_back(make_shared<CommandLineInterfaceItem>(CommandLineInterfaceItem{ }));
		}
		

		for (int index = 0; index < (int)allFiles.size(); ++index) {

			if (interfaceItems.size() >= maxVisibleFileCount) {
				break;

			}

			const int effectiveIndex = euclidean_reminder(index + fileListOffset, (int)allFiles.size());
			
			interfaceItems.push_back(make_shared<CommandLineInterfaceItem>(CommandLineInterfaceItem{ allFiles[effectiveIndex] }));
		}

	}



	CStringA CommandLineInterface::CreateTextInterface() {
		unique_lock<mutex> lock(internal_mutex);

		CStringA info = "Sounds: \n";

		for (int index = 0; index < interfaceItems.size(); ++index) {
			const fs::path& filePath = interfaceItems[index]->filePath;
			
			const CStringA fileUtf = ConvertUnicodeToUTF8(filePath.filename().c_str());

			if (index == selectedInterfaceItemIndex) {
				info += "*";
			}

			info += fileUtf + "\n";
		}

		return info;
	}
	CString CommandLineInterface::CopyBuffer() const {
		unique_lock<mutex> lock(internal_mutex);

		return inputBuffer;
	}

	optional<fs::path> CommandLineInterface::TryGetSelectedFile() {
		unique_lock<mutex> lock(internal_mutex);


		if (selectedInterfaceItemIndex < interfaceItems.size()) {
			return interfaceItems[selectedInterfaceItemIndex]->filePath;
		}

		return nullopt;
	}



	void CommandLineInterface::ShiftSelection(int indexDelta) {
		unique_lock<mutex> lock(internal_mutex);
		
		if (interfaceItems.size() == 0) {
			selectedInterfaceItemIndex = 0;
			return;
		}

		selectedInterfaceItemIndex = euclidean_reminder((int)selectedInterfaceItemIndex + indexDelta, (int)interfaceItems.size());

		UpdateInterface();
	}

	void CommandLineInterface::PageUp() {
		unique_lock<mutex> lock(internal_mutex);

		fileListOffset -= maxVisibleFileCount / 2;

		UpdateInterface();
	}

	void CommandLineInterface::PageDown() {
		unique_lock<mutex> lock(internal_mutex);

		fileListOffset += maxVisibleFileCount / 2;

		UpdateInterface();
	}
	
}


