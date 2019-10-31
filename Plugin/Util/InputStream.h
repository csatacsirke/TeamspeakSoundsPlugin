#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>

namespace TSPlugin {

	using std::filesystem::path;


	class InputStream {
	public:

		virtual size_t Read(void* data, size_t size) = 0;
		virtual bool IsEndOfStream() = 0;
	};

	class FileInputStream : InputStream {

		std::ifstream in;

		FileInputStream(const path& path) : in(path, std::ifstream::binary){
			// null
		}

		size_t Read(void* data, size_t size) override {
			in.read((char*)data, size);
			return in.gcount();
		}

		bool IsEndOfStream() override {
			return (bool)in;
		}

	};

	class DataInputStream : InputStream {

		void* Data;
		size_t Size;
		size_t Offset = 0;
		bool EndOfStream = false;
	public:

		DataInputStream(void* data, size_t size) : Data(data), Size(size) {
			// null
		}

		size_t Read(void* data, size_t size) override {
			const size_t end = std::min(Size, Offset + size);
			const size_t dataCountToRead = end - Offset;

			memcpy(data, Data, dataCountToRead);

			Offset += dataCountToRead;

			EndOfStream = dataCountToRead != size;

			return dataCountToRead;
		}

		bool IsEndOfStream() override {
			return EndOfStream;
		}

	};

} // namespace TSPlugin 
