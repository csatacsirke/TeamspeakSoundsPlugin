#pragma once


class PatternDetector
{
	CString prefix;
	CString suffix;

	//CString buffer;
	std::deque<TCHAR> buffer;

	std::function<void(CString)> OnPatternFound;
public:
	PatternDetector(void);
	~PatternDetector(void);

	void SetPrefix(CString prefix);
	void SetSuffix(CString suffix);

	void SetOnPatternFoundListener(std::function<void(CString)>);

	void Add(const CString& input);
	void RemoveLast();
	void Clear();
	//void Add(const KBDLLHOOKSTRUCT& input);

	

};

