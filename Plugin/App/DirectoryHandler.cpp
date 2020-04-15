#include "stdafx.h"
#include "DirectoryHandler.h"

#include <Util/Util.h>


namespace TSPlugin {

	class DirectoryListenerDelegate {
	public:
		virtual void OnDirectoryChanged() = 0;
	};

	class DirectoryListener {
		DirectoryListenerDelegate& delegate;
		HANDLE directoryChangeListenerHandle = NULL;
		thread monitorThread;
	public:
		DirectoryListener(DirectoryListenerDelegate& newDelegate, const fs::path& directory) 
			: delegate(newDelegate)
		{
			constexpr BOOL bWatchSubtree = true;
			constexpr DWORD dwNotifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME;
			directoryChangeListenerHandle = FindFirstChangeNotification(directory.c_str(), bWatchSubtree, dwNotifyFilter);
			monitorThread = thread([this] {
				const DWORD waitResult = WaitForSingleObject(FindNextChangeNotification, INFINITE);
				if (waitResult == WAIT_OBJECT_0) {
					delegate.OnDirectoryChanged();
					FindNextChangeNotification(directoryChangeListenerHandle);
				}
				
			});
		}

		~DirectoryListener() {
			FindCloseChangeNotification(directoryChangeListenerHandle);
		}
	};


	class DirectoryHandlerImpl : public DirectoryHandler, public DirectoryListenerDelegate {
	protected:
		shared_ptr<const DirectoryData> directoryData = nullptr;

	public:
		void UpdateCachedFilesIfNecessary() override;
		shared_ptr<const DirectoryData> GetDirectoryData() override;

	protected:
		void OnDirectoryChanged() override;

		void InitDirectoryListening(const fs::path& directory);
	};


	void DirectoryHandlerImpl::OnDirectoryChanged() {
		directoryData = nullptr;
	}

	void DirectoryHandlerImpl::InitDirectoryListening(const fs::path& directory) {
		
	}

	shared_ptr<DirectoryHandler> DirectoryHandler::Create() {
		return make_shared<DirectoryHandlerImpl>();
	}

	void DirectoryHandlerImpl::UpdateCachedFilesIfNecessary() {
		if (directoryData != nullptr) {
			return;
		}


		if (optional<CString> directoryOrNull = TryGetSoundsDirectory()) {
			shared_ptr<DirectoryData> newDirectoryData = make_shared<DirectoryData>();
			const fs::path directory = directoryOrNull->GetString();
			newDirectoryData->allFiles = ListFilesInDirectory(directory);
			newDirectoryData->base = directory;
			directoryData = newDirectoryData;
		}

	}

	shared_ptr<const DirectoryData> DirectoryHandlerImpl::GetDirectoryData() {
		return directoryData;
	}


} // namespace TSPlugin 
