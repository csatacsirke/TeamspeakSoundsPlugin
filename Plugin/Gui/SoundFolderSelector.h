#pragma once
#include "afxwin.h"


// SoundFolderSelector dialog

class SoundFolderSelector : public CDialogEx
{
	DECLARE_DYNAMIC(SoundFolderSelector)

public:
	SoundFolderSelector(CWnd* pParent = NULL);   // standard constructor
	virtual ~SoundFolderSelector();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SOUND_FOLDER_SELECTOR };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeFolderPathEdit();
	CEdit folderPathEdit;
	afx_msg void OnBnClickedOk();
};
