// Gui/SoundFolderSelector.cpp : implementation file
//

#include "stdafx.h"
#include "Gui/SoundFolderSelector.h"
#include "afxdialogex.h"


// SoundFolderSelector dialog

IMPLEMENT_DYNAMIC(SoundFolderSelector, CDialogEx)

SoundFolderSelector::SoundFolderSelector(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SOUND_FOLDER_SELECTOR, pParent)
{

}

SoundFolderSelector::~SoundFolderSelector()
{
}

void SoundFolderSelector::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FOLDER_PATH_EDIT, folderPathEdit);
}


BEGIN_MESSAGE_MAP(SoundFolderSelector, CDialogEx)
	ON_EN_CHANGE(IDC_FOLDER_PATH_EDIT, &SoundFolderSelector::OnEnChangeFolderPathEdit)
	ON_BN_CLICKED(IDOK, &SoundFolderSelector::OnBnClickedOk)
END_MESSAGE_MAP()


// SoundFolderSelector message handlers


void SoundFolderSelector::OnEnChangeFolderPathEdit() {
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}


void SoundFolderSelector::OnBnClickedOk() {
	CString folder;
	folderPathEdit.GetWindowText(folder);
	Global::config.Add(ConfigKey::SoundFolder, folder);
	Global::config.Save();
	CDialogEx::OnOK();
}
