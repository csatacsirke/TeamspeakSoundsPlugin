#include "stdafx.h"



#include "wave.h"
#include <Util/Config.h>
#include <Util/Log.h>
#include <Util/Util.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fstream>
#include <array>

#pragma warning( disable: 4267 )


namespace TSPlugin {




	struct WaveFmtHeader {
		unsigned short formatTag;
		unsigned short channels;
		unsigned int samplesPerSec;
		unsigned int avgBytesPerSec;
		unsigned short blockAlign;
		unsigned short bitsPerSample;


	public:
		// (.cpp ben)
		static optional<WaveFmtHeader> ReadFrom(std::istream& stream);
		WAVEFORMATEX ToWaveFormatEx() const;
	};

	optional<WaveFmtHeader> WaveFmtHeader::ReadFrom(std::istream& stream) {
		uint32_t chunkSize;
		stream.read((char*)&chunkSize, sizeof chunkSize);
		if (!stream) return nullopt;


		WaveFmtHeader header;

		static_assert(sizeof header == 16);

		if (chunkSize != sizeof header) {
			return nullopt;
		}


		stream.read((char*)&header, sizeof header);
		if (!stream) return nullopt;

		return header;
	}
	

	WAVEFORMATEX WaveFmtHeader::ToWaveFormatEx() const {
		WAVEFORMATEX format;

		format.wFormatTag = WAVE_FORMAT_PCM;
		format.nChannels = this->channels;
		format.nSamplesPerSec = this->samplesPerSec;
		format.nAvgBytesPerSec = this->avgBytesPerSec;
		format.nBlockAlign = this->blockAlign;
		format.wBitsPerSample = this->bitsPerSample;
		format.cbSize = 0; // ennek ut�na k�ne n�zni *


		//*cbSize
		// 
		//	Size, in bytes, of extra format information appended to the end 
		//	of the WAVEFORMATEX structure.This information can be used by non 
		//	- PCM formats to store extra attributes for the wFormatTag.
		//	If no extra information is required by the wFormatTag, this member
		//	must be set to 0. For WAVE_FORMAT_PCM formats(and only WAVE_FORMAT_PCM formats),
		//	this member is ignored.When this structure is included 
		//	in a WAVEFORMATEXTENSIBLE structure, this value must be at least 22.


		return format;
	}

	std::shared_ptr<WaveTrack> WaveTrack::MakeFromData(const WAVEFORMATEX& format, std::vector<uint8_t>&& data) {
		std::shared_ptr<WaveTrack> track(new WaveTrack());

		track->format = format;
		track->data = std::move(data);

		return track;
	}

	typedef std::array<char, 4> ChunkId;
	static const ChunkId fmt_id = {'f', 'm', 't', ' ' };
	static const ChunkId data_id = {'d', 'a', 't', 'a' };


	inline optional<ChunkId> ReadChunkHeader(std::istream& stream) {
		ChunkId chunkId;
		stream.read((char*)&chunkId, sizeof chunkId);
		if (!stream) {
			return nullopt;
		}
		return chunkId;
	}

	static void ResampleTo16Bit(WaveTrack& track) {

		vector<uint8_t>& data = track.data;
		auto& format = track.format;

		if (format.wBitsPerSample == 8) {
			format.wBitsPerSample = 16;
			std::vector<uint8_t> buffer16Bit(data.size() * 2, 0);

			for (int i = 0; i < data.size(); ++i) {
				// >implying that its stored in little endian
				buffer16Bit[2 * i] = data[i];
				buffer16Bit[2 * i + 1] = 0;
			}
			std::swap(buffer16Bit, data);
		}

		if (format.wBitsPerSample == 32) {
			format.wBitsPerSample = 16;
			std::vector<uint8_t> newBuffer(data.size() / 2, 0);
			int16_t* buffer16Bit = (int16_t*)newBuffer.data();

			for (int i = 0; i < newBuffer.size() / 2; ++i) {
				const float value_f32 = *reinterpret_cast<float*>(data.data() + 4 * i);
				buffer16Bit[i] = int16_t(double(value_f32) * int64_t(1 << 16));
			}
			std::swap(newBuffer, data);
		}
	}


	static float CalculateMaxVolume(const class WaveTrack& waveTrack) {

		const auto& format = waveTrack.format;

		const int bytesPerSample = format.wBitsPerSample / 8;
		const uint8_t* data = waveTrack.data.data();
		const size_t dataSize = waveTrack.data.size();

		float absMaxSample = 0;

		if (bytesPerSample == 1) {
			for (size_t offset = 0; offset < dataSize; offset += bytesPerSample) {
				const int sample = *reinterpret_cast<const uint8_t*>(data + offset);
				const float sampleAsFloat = float(sample) / float(1 << 8);
				absMaxSample = std::max<float>(abs(sampleAsFloat), absMaxSample);
			}
		} else if (bytesPerSample == 2) {
			for (size_t offset = 0; offset < dataSize; offset += bytesPerSample) {
				const int sample = *reinterpret_cast<const short*>(data + offset);
				const float sampleAsFloat = float(sample) / float(1 << 16);
				absMaxSample = std::max<float>(abs(sampleAsFloat), absMaxSample);
			}
		} else {
			ASSERT(0);
			return 1.0f;
		}

		return absMaxSample;
	}

	static void NormalizeVolume(WaveTrack& track) {


		const float maxVolume = CalculateMaxVolume(track);
		const float targetVolume = float(_wtof(Global::config.Get(ConfigKeys::Volume)));
		const float targetNormalizedVolume = float(_wtof(Global::config.Get(ConfigKeys::TargetNormalizedVolume)));

		const bool normalizeVolume = Global::config.GetBool(ConfigKeys::NormalizeVolume);

		const float multiplier = normalizeVolume ? targetNormalizedVolume / maxVolume * targetVolume : targetVolume;

		short* dataStart = (short*)track.data.data();
		short* dataEnd = (short*)(track.data.data() + track.data.size());
		for (short* data = dataStart; data != dataEnd; ++data) {
			*data = short(float(*data) * multiplier);
		}

	}


	static void PostProcessTrack(WaveTrack& track) {

		
		
		ResampleTo16Bit(track);

		//short* data = (short*)result->data.data();
		vector<uint8_t>& data = track.data;
		auto& format = track.format;

		// Add smooth ending, so there will be no clicking sound because of the abrupt ending
		const int fadeoutMs = 20;
		const size_t fadeoutSampleCount = format.nSamplesPerSec / (1000 / fadeoutMs);
		const size_t fadeoutSize = fadeoutSampleCount * format.nChannels * sizeof(short);
		//if()
		//data.resize(originalSize + fadeoutSize);


		//if (data.size() > sizeof(short)*format.nChannels) {
		if (data.size() > fadeoutSize) {

			const size_t originalSize = data.size() - fadeoutSize;

			if (format.nChannels == 1) {

				short* const start = (short*)(data.data() + originalSize);
				short* const end = (short*)(data.data() + data.size());

				const int64_t sampleCount = end - start;
				int64_t currentIndex = 0;

				for (short* it = start; it < end; ++it, ++currentIndex) {
					short& currentSample = *it;
					currentSample = (short)(currentSample * (sampleCount - currentIndex) / sampleCount);
				}
			} else {
				const size_t stride = format.nChannels;

				for (int channelIndex = 0; channelIndex < format.nChannels; ++channelIndex) {

					short* const start = (short*)(data.data() + originalSize) + channelIndex;
					short* const end = (short*)(data.data() + data.size()) + channelIndex;

					const int64_t sampleCount = (end - start) / stride;
					int64_t currentIndex = 0;

					for (short* it = start; it < end; it += stride, ++currentIndex) {
						short& currentSample = *it;
						currentSample = (short)(currentSample * (sampleCount - currentIndex) / sampleCount);
					}
				}
			}
		}

		NormalizeVolume(track);
	}

	static bool SkipChunk(std::istream& stream) {

		uint32_t chunkSize;
		stream.read((char*)&chunkSize, sizeof chunkSize);
		if (!stream) return false;

		stream.ignore(chunkSize);
		if (!stream) return false;

		return true;
	}

	static std::shared_ptr<WaveTrack> ReadData(const WaveFmtHeader& header, std::istream& stream) {

		std::shared_ptr<WaveTrack> track = make_shared<WaveTrack>();
		track->format = header.ToWaveFormatEx();

		uint32_t chunkSize;
		stream.read((char*)&chunkSize, sizeof chunkSize);
		if (!stream) return nullptr;


		Log::Debug(L"Data Length :" + ToString(chunkSize));

		track->data = vector<uint8_t>(chunkSize, 0);

		stream.read((char*)track->data.data(), track->data.size());
		if (!stream) return nullptr;

		return track;
	}



	void WaveTrack::FillMetadata() {
		metadata.maxVolume = CalculateMaxVolume(*this);
	}



	std::shared_ptr<WaveTrack> WaveTrack::LoadWaveFile(std::istream& stream) {

		if (!stream) {
			//Log::Error(L"LoadWaveFile: failed to open stream");
			Log::Warning(L"LoadWaveFile: failed to open stream");
			return NULL;
		}


		// Riff chunk
		char riffId[4];  // 'RIFF'
		uint32_t len;
		char riffType[4];  // 'WAVE'


		stream.read(riffId, sizeof riffId);
		stream.read((char*)&len, sizeof len);
		stream.read(riffType, sizeof riffType);


		optional<WaveFmtHeader> header;
		std::shared_ptr<WaveTrack> track;

		while (optional<ChunkId> chunkId = ReadChunkHeader(stream)) {
			if (chunkId == fmt_id) {
				header = WaveFmtHeader::ReadFrom(stream);
				if (!header) return nullptr;
			} else if (chunkId == data_id) {
				if (!header) {
					return nullptr;
				}

				track = ReadData(*header, stream);
				if (!track) return nullptr;
			} else {
				if (!SkipChunk(stream)) {
					return nullptr;
				}
			}
		}


		assert(!track || track->format.wBitsPerSample == 16);
		if (track && track->format.wBitsPerSample != 16) {
			return nullptr;
		}


		if (track) {
			PostProcessTrack(*track);
		}
		


		return track;

	}

	std::shared_ptr<WaveTrack> WaveTrack::LoadWaveFile(const wchar_t* fileName) {

		std::ifstream stream(fileName, std::ifstream::binary);

		return LoadWaveFile(stream);
	}



	static char chunk_id_riff[4] = { 'R', 'I', 'F', 'F' };
	static char chunk_id_wave[4] = { 'W', 'A', 'V', 'E' };
	static char chunk_id_fmt[4] = { 'f', 'm', 't', ' ' };
	static char chunk_id_data[4] = { 'd', 'a', 't', 'a' };

	bool WaveTrack::Save(const CString& fileName) {
		std::ofstream out(fileName, std::ofstream::binary);

		if (!out) {
			return false;
		}



		uint32_t headerChunkSize = sizeof(format);
		uint32_t dataChunkSize = data.size();
		uint32_t mainChunkSize = sizeof(chunk_id_wave) + headerChunkSize + dataChunkSize;


		out.write(chunk_id_riff, sizeof(chunk_id_riff));
		out.write((const char*)&mainChunkSize, sizeof(mainChunkSize));


		out.write((const char*)&chunk_id_wave, sizeof(chunk_id_wave));

		out.write((const char*)&chunk_id_fmt, sizeof(chunk_id_fmt));
		out.write((const char*)&headerChunkSize, sizeof(headerChunkSize));
		out.write((const char*)&format, sizeof(format));


		out.write(chunk_id_data, sizeof(chunk_id_data));
		out.write((const char*)&dataChunkSize, sizeof(dataChunkSize));
		out.write((const char*)data.data(), data.size());

		return true;
	}

}

