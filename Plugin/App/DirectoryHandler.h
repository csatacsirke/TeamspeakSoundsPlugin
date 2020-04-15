#pragma once

namespace TSPlugin {


	struct DirectoryData {
		fs::path base;
		vector<fs::path> allFiles;
	};

	class DirectoryHandler {

	public:
		static shared_ptr<DirectoryHandler> Create();

		virtual void UpdateCachedFilesIfNecessary() = 0;
		virtual shared_ptr<const DirectoryData> GetDirectoryData() = 0;

	};

} // namespace TSPlugin 
