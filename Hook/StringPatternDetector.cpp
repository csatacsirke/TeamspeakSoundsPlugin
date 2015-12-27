#include "stdafx.h"
#include "StringPatternDetector.h"


StringPatternDetector::StringPatternDetector() 
{
}

void StringPatternDetector::SetOnPatternFoundListener(std::function<void(CString)> callback) {
	this->OnPatternFound = callback;
}

void StringPatternDetector::SetPrefix(std::vector<int> strPrefix){
	this->strPrefix = strPrefix;
}

void StringPatternDetector::SetSuffix(std::vector<int> strSuffix){
	this->strSuffix = strSuffix;
}

void StringPatternDetector::Add(const KBDLLHOOKSTRUCT& input){
	int vk = input.vkCode;

	if(vk == VK_ESCAPE){
		buffer.clear();
		return;
	}
	if(vk == VK_BACK){
		buffer.pop_back();
		return;
	}


	buffer.push_back(vk);
	if(!StartsWith(buffer, strPrefix)){
		buffer.clear();
		buffer.push_back(vk);
		return;
	}

	if(buffer.size() < strPrefix.size() + strSuffix.size() + 1){
		return; // tul kev�s bet�, nem lehet j�
	}
	if(EndsWith(buffer, strSuffix)){
		std::vector<int> result;
		for(int i=strPrefix.size(); i < buffer.size()-strSuffix.size(); ++i){
			result.push_back(buffer[i]);
		}
		if( OnPatternFound ) {
			OnPatternFound( BuildStringFromVKVector( result ) );
		}
		buffer.clear();
		return;
	}
	
	// esc
	// enter
}
// Az is egyez�snek sz�m�t ha a sz� tul kicsi
bool StringPatternDetector::StartsWith(const std::deque<int>& strWord, const std::vector<int>& strPrefix){
	for(int i=0; i < strPrefix.size() && i < strWord.size(); ++i){
		if(strWord[i] != strPrefix[i]) return false;
	}
	return true;

}


bool StringPatternDetector::EndsWith(const std::deque<int>& strWord, const std::vector<int>& strSuffix){
	if(strWord.size() < strSuffix.size()){
		return false; // tul kev�s bet�, nem lehet j�
	}

	for(int i=strWord.size()-strSuffix.size(); i < strWord.size(); ++i){
		if(strWord[i] != strSuffix[i-strWord.size()+strSuffix.size()]) return false;
	}
	return true;
}


 CString StringPatternDetector::BuildStringFromVKVector(std::vector<int>& vvk){
	 CString ret;

	 for(int i=0; i < vvk.size(); ++i){
		 TCHAR c;
		 // ha nem bet�
		 if(vvk[i] < VK_LETTER_A || vvk[i] > VK_LETTER_Z){
			switch(vvk[i]){
			case VK_SPACE:
				c = _T(' ');
				break;
			default:
				continue;
			}
			// ha sz�m
			if(vvk[i] > VK_NUMBER_0 && vvk[i] < VK_NUMBER_9){
				c = _T('0') + vvk[i] - VK_NUMBER_0;
			}
		 // ha bet�
		 } else {
			 c = _T('a') + vvk[i] - VK_LETTER_A;
		 }
		 ret.AppendChar(c);
		 
	 }
	 return ret;
 }
