#pragma once

#include "stdafx.h"
/**
TODO

magic number az elejere hogy nehezebb legyen elbaszni

*/

namespace TSPlugin {

	namespace Steganography {


		static inline int GetBit(int c, int bit) {
			return ((c >> bit) & 0x01);
		}

		// http://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit-in-c-c (2017.04.15)
		// remelem nem basztam el
		static inline void  SetBit(char& c, int bit, int value) {
			//const int x = value ? 1 : 0;
			assert(value == 1 || value == 0);
			c ^= (-value ^ c) & (1 << bit);
		}


		static void WriteSecret(short* data, size_t size, const char* secret, size_t secretLength) {
			size_t secretByteIndex = 0;
			int secretBitIndex = 0;
			size_t secretIndex = 0;

			if (secretLength * 8 > size) {
				ONCE(Log::Warning(L"Steganography::WriteSecret : secret too long "));
				return;
			}

			for (size_t dstIndex = 0; dstIndex < secretLength * 8; ++dstIndex) {
				data[dstIndex] ^= GetBit(secret[secretByteIndex], secretBitIndex);

				++secretBitIndex;

				if (secretBitIndex == 8) {
					secretBitIndex = 0;
					++secretByteIndex;
				}
			}
		}

		static void WriteSecret(short* data, size_t size, CStringA secret) {
			WriteSecret(data, size, (const char*)secret, secret.GetLength() + 1);
		}


		static CStringA ReadSecret(const short* data, size_t size) {
			std::vector<char> result;

			int secretBitIndex = 0;
			char currentSecretByte;
			bool success = false;

			for (size_t srcIndex = 0; srcIndex < size; ++srcIndex) {

				int value = GetBit(data[srcIndex], 0);
				SetBit(currentSecretByte, secretBitIndex, value);
				++secretBitIndex;

				if (secretBitIndex == 8) {
					secretBitIndex = 0;
					result.push_back(currentSecretByte);

					if (currentSecretByte == '\0') {
						success = true;
						break;
					}
				}
			}

			if (success) {
				return CStringA(result.data());
			} else {
				return CStringA("FAIL - THIS IS NOT THE ACTUAL MESSAGE");
			}
		}

		//static void WriteSecretBinary(short* data, size_t size, char* secret, size_t secretLength);

	}


}

