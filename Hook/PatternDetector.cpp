#include "stdafx.h"
#include "PatternDetector.h"


PatternDetector::PatternDetector(void)
{
}


PatternDetector::~PatternDetector(void)
{
}

void PatternDetector::SetOnPatternFoundListener(std::function<void(CString)> callback) {
	this->OnPatternFound = callback;
}

void PatternDetector::SetPrefix(CString prefix) {
	this->prefix = prefix;
}

void PatternDetector::SetSuffix(CString suffix) {
	this->suffix = suffix;
}

void PatternDetector::RemoveLast() {
	buffer.pop_back();
}

void PatternDetector::Clear() {
	buffer.clear();
}

//void PatternDetector::Add(const KBDLLHOOKSTRUCT& input) {
void PatternDetector::Add(const CString& input) {
	
	ASSERT( input.GetLength() <= 1);
	for (int i = 0; i < input.GetLength(); i++) {
		buffer.push_back(input[i]);
	}


	
}