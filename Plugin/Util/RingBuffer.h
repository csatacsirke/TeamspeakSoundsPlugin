#pragma once

template<typename T>
class RingBuffer {
	std::vector<T> buffer;
	size_t startIndex = 0;
	size_t endIndex = 0;
	size_t dataLength = 0;
public:
	RingBuffer() {
		const size_t defaultSize = 30 * 48000;
		Resize(defaultSize);
	}

	RingBuffer(size_t size) {
		Resize(size);
	}

	void Resize(size_t size) {
		std::vector<T> targetBuffer;
		GetData(targetBuffer);

		buffer.resize(size);
		startIndex = 0;
		endIndex = 0;
		dataLength = 0;

		AddData(targetBuffer.data(), targetBuffer.size());
	}

	void AddData(T* data, size_t dataLength) {
		if(dataLength > buffer.size()) {
			size_t offset = dataLength - buffer.size();
			memcpy(buffer.data(), data + offset, buffer.size());
			startIndex = 0;
			endIndex = 0;
			this->dataLength = buffer.size();
		} else {
			if(dataLength < (buffer.size() - endIndex)) {
				memcpy(buffer.data(), data, dataLength);
				endIndex += dataLength;
			} else {
				size_t chunkOneLength = dataLength - (buffer.size() - endIndex);
				size_t chunkTwoLength = dataLength - chunkOneLength;
				memcpy(buffer.data() + endIndex, data, chunkOneLength);
				memcpy(buffer.data(), data, chunkTwoLength);
				endIndex = chunkTwoLength;
			}
			this->dataLength += dataLength;
			if(this->dataLength > buffer.size()) {
				this->dataLength = buffer.size();
				startIndex = endIndex;
			}
		}
	}

	void GetData(std::vector<T>& targetBuffer) {
		targetBuffer.resize(dataLength);
		if(endIndex < startIndex) {
			size_t chunkOneLength = targetBuffer.size() - startIndex;
			memcpy(targetBuffer.data(), buffer.data() + startIndex, chunkOneLength);
			memcpy(targetBuffer.data() + chunkOneLength, buffer.data(), endIndex);
		} else {
			memcpy(targetBuffer.data(), buffer.data() + startIndex, dataLength);
		}

	}

	void Clear() {
		startIndex = 0;
		endIndex = 0;
		dataLength = 0;
	}
};
