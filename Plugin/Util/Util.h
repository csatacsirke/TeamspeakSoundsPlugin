#pragma once

#include <string>
#include <queue>
#include <thread>
#include <map>
#include <optional>

#include <pluginsdk\include\teamspeak/public_errors.h>
#include <pluginsdk\include\teamspeak/public_errors_rare.h>
#include <pluginsdk\include\teamspeak/public_definitions.h>
#include <pluginsdk\include\teamspeak/public_rare_definitions.h>
#include <pluginsdk\include\teamspeak/clientlib_publicdefinitions.h>


#include <KeyboardHook/KeyboardHookUtils.h>

#include "Log.h"

namespace TSPlugin {

	using namespace KeyboardHook;

	//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
	CString GetLastErrorAsString();

	CString ErrorToString(DWORD error);

	std::vector<CString> GetPossibleFiles(const CString & inputString);
	enum TryGetSoundsDirectoryOptions { None, AskGui };
	std::optional<CString> TryGetSoundsDirectory(TryGetSoundsDirectoryOptions options = None);
	std::optional<CString> TryGetLikelyFileName(const CString& inputString);


	struct Exception {
		CString errorMessage;
		DWORD errorCode = 0;
		Exception() {};
		Exception(CStringW msg) {
			errorMessage = msg;
		}
		Exception(CStringA msg) {
			errorMessage = msg;
		}
		Exception(DWORD errorCode) {
			this->errorCode = errorCode;
			ASSERT(0);
			errorMessage = ErrorToString(errorCode);
		}

	};

	class Finally {
		std::function<void()> func;
	public:
		//Finally(const std::function<void()>& func) { 
		//
		//}

		Finally(const std::function<void()>&& _func) {
			func = _func;
		}

		~Finally() {
			if (func) {
				func();
			}

		}
	};

	template<typename ... Args>
	std::string string_format(const std::string& format, Args ... args) {
		size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
		std::unique_ptr<char[]> buf(new char[size]);
		snprintf(buf.get(), size, format.c_str(), args ...);
		return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
	}

	template<typename ... Args>
	CString FormatString(const CString& format, Args ... args) {
		CString result;
		result.Format(format, args...); 
		return result;
	}


#define ONCE(X) {static bool first = true; if(first) { first = false; X; }}


#define COMMAND_BUFSIZE 128
#define INFODATA_BUFSIZE 128
#define SERVERINFO_BUFSIZE 256
#define CHANNELINFO_BUFSIZE 512
#define RETURNCODE_BUFSIZE 128

#define PATH_BUFSIZE 512

	/*The client lib works at 48Khz internally.
	It is therefore advisable to use the same for your project */
#define PLAYBACK_FREQUENCY 48000
#define PLAYBACK_CHANNELS 2



	namespace Global {
		extern struct TS3Functions ts3Functions;
		extern uint64 connection;
		extern char* pluginID;
		//extern anyID myID;

		extern char appPath[PATH_BUFSIZE];
		extern char resourcesPath[PATH_BUFSIZE];
		extern char configPath[PATH_BUFSIZE];
		extern char pluginPath[PATH_BUFSIZE];

	}


	class Buffer : private std::unique_ptr<std::vector<uint8_t>> {
	public:
		Buffer() {
			std::unique_ptr<std::vector<uint8_t>>::reset(new std::vector<uint8_t>());
		}

		void Append(std::vector<uint8_t>& other) {
			Append(other.data(), other.size());
		}

		void Append(void* data, size_t length) {
			size_t offset = (*this)->size();
			(*this)->resize(offset + length);
			memcpy((*this)->data() + offset, data, length);
		}

		void Truncate(size_t size, Buffer& overflow) {
			if ((*this)->size() > size) {
				size_t overflowSize = (*this)->size() - size;
				overflow.Append(this->get()->data() + size, overflowSize);
				this->get()->resize(size);
			}
		}

		size_t Size() {
			return (*this)->size();
		}

		operator void* () {
			return (*this)->data();
		}

		operator const uint8_t* () {
			return (*this)->data();
		}

		void* Data() {
			return (*this)->data();
		}

		void Swap(std::vector<uint8_t>& other) {
			(*this)->swap(other);
		}
	};


	// https://www.arclab.com/en/kb/cppmfc/convert-cstring-unicode-utf-16le-to-utf-8-and-reverse.html (2016.03.26)
	CStringA ConvertUnicodeToUTF8(const CStringW& uni);
	CStringW Utf8ToCString(const CStringA& utf8Str);

	BOOL DirectoryExists(CString szPath);

	template<typename T>
	static inline size_t GetDataSizeInBytes(const std::vector<T>& vec) {
		return sizeof(T) * vec.size();
	}



	template<typename T>
	CString ToString(T value) {
		return CString(std::to_string(value).c_str());
	}

	//
	//template<>
	//CString ToString<const char*>(const char* value) {
	//	return CString(value);
	//}

	//
	//template<>
	//CString ToString<char*>(char* value) {
	//	return CString(value);
	//}
	//
	//template<>
	//CString ToString<const char*>(const char* value) {
	//	return CString(value);
	//}
	//
	//template<typename T>
	//CString operator+(const CString& str, T value) {
	//	return str + ToString(value);
	//}
	//
	//template<typename T>
	//CString operator+(T value, const CString& str) {
	//	return ToString(value) + str;
	//}

	template<typename T>
	inline T Clamp(T x, T lowerBound, T upperBound) {
		if (x < lowerBound) return lowerBound;
		if (x > upperBound) return upperBound;
		return x;
	}

	// Rectified Linear Unit
	template<typename T>
	T Relu(T x) {
		if (x < 0) return 0;
		return x;
	}


	namespace Ts {
		using namespace Global;

		static inline void CheckAndLogError(UINT error) {
			if (error != ERROR_ok) {
				CString strError = ErrorToString(error);
				Log::Warning(strError);
				assert(0 && strError);
			}
		}

		static inline CString ErrorToString(unsigned int error) {
			char* errormsg;
			if (ts3Functions.getErrorMessage(error, &errormsg) == ERROR_ok) {
				CString result(errormsg);
				ts3Functions.freeMemory(errormsg);
				return result;
			} else {
				return CString("Unknown error");
			}
		}

		static inline CStringA GetPreProcessorConfigValue(CStringA key) {
			char* ptr;
			CStringA result;
			unsigned int error = ts3Functions.getPreProcessorConfigValue(Global::connection, key, &ptr);
			CheckAndLogError(error);
			if (error == ERROR_ok) {
				result = ptr;
				ts3Functions.freeMemory(ptr);
			}
			return result;
		}

		static inline void SetPreProcessorConfigValue(CStringA key, CStringA value) {
			unsigned int error = ts3Functions.setPreProcessorConfigValue(Global::connection, key, value);
			CheckAndLogError(error);
		}

		static inline void SetClientSelfVariableAsInt(size_t key, int value) {
			UINT error = ts3Functions.setClientSelfVariableAsInt(Global::connection, key, value);
			CheckAndLogError(error);
		}

		static inline int GetClientSelfVariableAsInt(size_t key) {
			int value;
			UINT error = ts3Functions.getClientSelfVariableAsInt(Global::connection, key, &value);
			CheckAndLogError(error);
			return value;
		}


		static const char* VoiceActivation = "vad";
		static const char* True = "true";
		static const char* False = "false";





	}


	//
	//template<typename T>
	//CString operator+(const wchar_t* str, T value) {
	//	return CString(str) + ToString(T);
	//}
	//
	//template<typename T>
	//CString operator+(T value, const wchar_t* str) {
	//	return ToString(T) + CString(str);
	//}
	//
	//
	//template<typename T>
	//CString operator+(const char* str, T value) {
	//	return CString(str) + ToString(T);
	//}
	//
	//template<typename T>
	//CString operator+(T value, const char* str) {
	//	return ToString(T) + CString(str);
	//}
	//



	CStringA GetHotkey(CStringA key);

	static bool StartsWith(const CString& str, const CString what) {
		return str.Left(what.GetLength()) == what;
	}

	//static bool EqualsIgnoreCaseAndWhitespace(CString a, CString b) {
	//	a.MakeLower();
	//	b.MakeLower();
	//
	//	a.Remove(L' ');
	//	b.Remove(L' ');
	//
	//	a.Remove(L'\t');
	//	b.Remove(L'\t');
	//
	//	return a == b;
	//}

	inline CString MakeComparable(const CString& str) {
		CString result = str;
		result.MakeLower();
		result.Remove(L' ');
		result.Remove(L'\t');

		result.Replace(L'ú', L'u');
		result.Replace(L'ű', L'u');
		result.Replace(L'ó', L'o');
		result.Replace(L'ő', L'o');
		result.Replace(L'á', L'a');
		result.Replace(L'é', L'e');
		result.Replace(L'í', L'i');

		return result;
	}

	static inline CWnd* NoParent() {
		return NULL;
	}



	static inline void CreateConsole() {
		AllocConsole();
		FILE* pCout;
		freopen_s(&pCout, "CONOUT$", "w", stdout);
		std::cout.clear();
		std::wcout.clear();

	}

	//void ListFilesInDirectory(_Out_ std::vector<CString>& files, CString path, CString filter = L"");
	std::vector<CString> ListFilesInDirectory(CString path, CString filter = L"");
	std::vector<CString> SortFilesByModificationDate(const std::vector<CString>& filesNames);

	CString FileNameFromPath(CString path);
	CString PickRandomFile(CString path);




#ifdef _WIN32
#define _strcpy(dest, destSize, src) strcpy_s(dest, destSize, src)
#define snprintf sprintf_s
#else
#define _strcpy(dest, destSize, src) { strncpy(dest, src, destSize-1); (dest)[destSize-1] = '\0'; }
#endif

} // namespace TSPlugin