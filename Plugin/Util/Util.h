#pragma once

#include <string>


#include <pluginsdk\include\teamspeak/public_errors.h>
#include <pluginsdk\include\teamspeak/public_errors_rare.h>
#include <pluginsdk\include\teamspeak/public_definitions.h>
#include <pluginsdk\include\teamspeak/public_rare_definitions.h>
#include <pluginsdk\include\teamspeak/clientlib_publicdefinitions.h>


//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
CString GetLastErrorAsString();

CString ErrorToString(DWORD error);

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
	extern anyID myID;

	extern char appPath[PATH_BUFSIZE];
	extern char resourcesPath[PATH_BUFSIZE];
	extern char configPath[PATH_BUFSIZE];
	extern char pluginPath[PATH_BUFSIZE];

}


class Buffer : private std::unique_ptr<std::vector<byte>> {
public:
	Buffer() {
		std::unique_ptr<std::vector<byte>>::reset(new std::vector<byte>());
	}

	void Append(std::vector<byte>& other) {
		Append(other.data(), other.size());
	}

	void Append(void* data, size_t length) {
		size_t offset = (*this)->size();
		(*this)->resize(offset + length);
		memcpy((*this)->data() + offset, data, length);
	}

	void Truncate(size_t size, Buffer& overflow) {
		if((*this)->size() > size) {
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

	operator const byte* () {
		return (*this)->data();
	}

	void* Data() {
		return (*this)->data();
	}

	void Swap(std::vector<byte>& other) {
		(*this)->swap(other);
	}
};


// https://www.arclab.com/en/kb/cppmfc/convert-cstring-unicode-utf-16le-to-utf-8-and-reverse.html (2016.03.26)
CStringA ConvertUnicodeToUTF8(const CStringW& uni);
BOOL DirectoryExists(CString szPath);

template<typename T>
static inline size_t GetDataSizeInBytes(const typename std::vector<T>& vec) {
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
	if(x < lowerBound) return lowerBound;
	if(x > upperBound) return upperBound;
	return x;
}

// Rectified Linear Unit
template<typename T>
T Relu(T x) {
	if(x < 0) return 0;
	return x;
}


namespace Global {

	static CString TsErrorToString(unsigned int error) {
		char* errormsg;
		if(ts3Functions.getErrorMessage(error, &errormsg) == ERROR_ok) {
			CString result(errormsg);
			ts3Functions.freeMemory(errormsg);
			return result;
		} else {
			return CString("Unknown error");
		}
	}

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

static bool EqualsIgnoreCaseAndWhitespace(CString a, CString b) {
	a.MakeLower();
	b.MakeLower();

	a.Remove(L' ');
	b.Remove(L' ');

	a.Remove(L'\t');
	b.Remove(L'\t');

	return a == b;
}

inline CWnd* NoParent() {
	return NULL;
}

void ListFilesInDirectory(_Out_ std::vector<CString>& files, CString path, CString filter = L"");
CString FileNameFromPath(CString path);





#ifdef _WIN32
#define _strcpy(dest, destSize, src) strcpy_s(dest, destSize, src)
#define snprintf sprintf_s
#else
#define _strcpy(dest, destSize, src) { strncpy(dest, src, destSize-1); (dest)[destSize-1] = '\0'; }
#endif
