#pragma once
#include "afxwin.h"


// AudioProcessorDialog dialog

class AudioProcessorDialog : public CDialogEx
{
	DECLARE_DYNAMIC(AudioProcessorDialog)

public:
	AudioProcessorDialog(class AudioProcessor& audioProcessor, CWnd* pParent = NULL);   // standard constructor
	virtual ~AudioProcessorDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	//BOOL OnInitDialog() override;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedEnableCheckbox();

private:
	class AudioProcessor& audioProcessor;
public:
	CButton enableCheckbox;
	CEdit windowSizeMillisecsEdit;
	afx_msg void OnBnClickedButton1();
};
