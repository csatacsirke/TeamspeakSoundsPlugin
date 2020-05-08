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
		const auto possibleFilesForCurrentInput = GetPossibleFiles(inputBuffer, allFiles);
		

		interfaceItems.clear();
		shouldRotateList = true;

		for (const fs::path& path : possibleFilesForCurrentInput) {

			if (interfaceItems.size() >= maxVisibleFileCount) {
				break;
			}

			interfaceItems.push_back(make_shared<CommandLineInterfaceItem>(CommandLineInterfaceItem{ path }));
			// ha van possible file, akkor nem rotate
			shouldRotateList = false;
		}

		if (inputBuffer.GetLength() > 0) {
			// empty line as separator
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

		//selectedInterfaceItemIndex = euclidean_reminder((int)selectedInterfaceItemIndex + indexDelta, (int)interfaceItems.size());
		//selectedInterfaceItemIndex = euclidean_reminder((int)selectedInterfaceItemIndex + indexDelta, (int)interfaceItems.size());

		const bool nearFileListEdge =
			(indexDelta > 0 && selectedInterfaceItemIndex > interfaceItems.size() * 3 / 4) ||
			(indexDelta < 0 && selectedInterfaceItemIndex < interfaceItems.size() * 1 / 4);



		if (shouldRotateList && nearFileListEdge) {
			fileListOffset += indexDelta;
		} else {
			selectedInterfaceItemIndex = std::clamp<int64_t>(selectedInterfaceItemIndex + indexDelta, 0, interfaceItems.size() - 1);
		}

		//if (indexDelta > 0) {
		//	if (selectedInterfaceItemIndex < interfaceItems.size() * 3 / 4) {
		//		selectedInterfaceItemIndex = std::min<size_t>(selectedInterfaceItemIndex + indexDelta, interfaceItems.size() - 1);
		//	} else {
		//		fileListOffset += indexDelta;
		//	}
		//} else {
		//	if (selectedInterfaceItemIndex > interfaceItems.size() * 1 / 4) {
		//		selectedInterfaceItemIndex = std::min<size_t>(selectedInterfaceItemIndex + indexDelta, interfaceItems.size() - 1);
		//	} else {
		//		fileListOffset += indexDelta;
		//	}
		//}

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


