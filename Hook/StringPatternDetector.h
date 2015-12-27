#pragma once

#include <deque>
#include <vector>
#include "util.h"


const int VK_LETTER_A = 0x41;
const int VK_LETTER_Z = 0x5A;

const int VK_NUMBER_0 = 0x30;
const int VK_NUMBER_9 = 0x39;
	
class StringPatternDetector
{
	
	std::function<void(CString)> OnPatternFound;

	std::vector<int> strPrefix;
	std::vector<int> strSuffix;
	std::deque<int> buffer;
	static bool StartsWith(const std::deque<int>& strWord, const std::vector<int>& strPrefix);
	static bool EndsWith(const std::deque<int>& strWord, const std::vector<int>& strSuffix);
	bool EndsWith();
public:
	StringPatternDetector();
	
	void SetOnPatternFoundListener(std::function<void(CString)> callback);


	void SetPrefix(std::vector<int> strPrefix);
	void SetSuffix(std::vector<int> strSuffix);
	void Add(const KBDLLHOOKSTRUCT& input);



	static CString BuildStringFromVKVector(std::vector<int>& strFoundString);

};

