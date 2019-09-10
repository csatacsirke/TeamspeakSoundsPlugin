// TsVersionFinder.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"


#include <Windows.h>
#include <tchar.h>

#include <iostream>
#include <functional>

using namespace std;

int wmain(int argc, wchar_t* argv[])
{
    
	if (argc < 1) {
		cerr << "Missing parameter: path" << endl;
		return -1;
	}

	wchar_t* dllPath = argv[1];

	HMODULE hDll = LoadLibraryW(dllPath);

	if (!hDll) {
		cerr << "Failed to load dll" << endl;
		return -1;
	}

	function<const char*()> ts3plugin_version = (const char*(*)(void))GetProcAddress(hDll, "ts3plugin_version");
	if (!ts3plugin_version) {
		cerr << "GetProcAddress failed" << endl;
		return -1;
	}

	cout << ts3plugin_version();

	return 0;
}

