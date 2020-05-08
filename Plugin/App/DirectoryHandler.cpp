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
				while (WaitForSingleObject(directoryChangeListenerHandle, INFINITE) == WAIT_OBJECT_0) {
					delegate.OnDirectoryChanged();
					FindNextChangeNotification(directoryChangeListenerHandle);
				}
			});
		}

		~DirectoryListener() {
			FindCloseChangeNotification(directoryChangeListenerHandle);
			monitorThread.join();
		}
	};


	class DirectoryHandlerImpl : public DirectoryHandler, public DirectoryListenerDelegate {
	protected:
		shared_ptr<const DirectoryData> directoryData = nullptr;
		shared_ptr<DirectoryListener> directoryListener;
	public:
		void UpdateCachedFilesIfNecessary() override;
		shared_ptr<const DirectoryData> GetDirectoryData() override;

	protected:
		void OnDirectoryChanged() override;

	private:
		void InitDirectoryListening(const fs::path& directory);
	};




	static shared_ptr<const DirectoryData> TryGetSoundDirectoryData() {

		if (optional<CString> directoryOrNull = TryGetSoundsDirectory()) {
			shared_ptr<DirectoryData> newDirectoryData = make_shared<DirectoryData>();
			const fs::path directory = directoryOrNull->GetString();
			newDirectoryData->allFiles = ListFilesInDirectory(directory);
			newDirectoryData->base = directory;
			return newDirectoryData;
		}

		return nullptr;
	}


	void DirectoryHandlerImpl::OnDirectoryChanged() {
		directoryData = TryGetSoundDirectoryData();
	}

	void DirectoryHandlerImpl::InitDirectoryListening(const fs::path& directory) {
		
	}

	shared_ptr<DirectoryHandler> DirectoryHandler::Create() {
		return make_shared<DirectoryHandlerImpl>();
	}


	void DirectoryHandlerImpl::UpdateCachedFilesIfNecessary() {
		if (directoryData == nullptr) {
			directoryData = TryGetSoundDirectoryData();
		}
		

		if (directoryListener == nullptr) {
			if (optional<CString> directoryOrNull = TryGetSoundsDirectory()) {
				const fs::path directory = (const wchar_t*)*directoryOrNull;
				directoryListener = make_shared<DirectoryListener>(*this, directory);
			}
		}
	}

	
	shared_ptr<const DirectoryData> DirectoryHandlerImpl::GetDirectoryData() {
		UpdateCachedFilesIfNecessary();
		return directoryData;
	}


} // namespace TSPlugin 
