// Gui/SoundFolderSelector.cpp : implementation file
//

#include "stdafx.h"
#include "SoundFolderSelector.h"
#include "afxdialogex.h"


#include <Util\Config.h>


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
	ON_BN_CLICKED(IDC_BROWSE_BUTTON, &SoundFolderSelector::OnBnClickedBrowseButton)
END_MESSAGE_MAP()


// SoundFolderSelector message handlers

BOOL SoundFolderSelector::OnInitDialog() {
	BOOL result = CDialog::OnInitDialog();


	CString path = Global::config.Get(ConfigKey::SoundFolder, L"");
	this->folderPathEdit.SetWindowText(path);

	return result;
}

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

	if(folder.Right(1) != "\\" && folder.Right(1) != "/") {
		folder += "\\";
	}

	Global::config.Add(ConfigKey::SoundFolder, folder);
	Global::config.Save();


	CString path = Global::config.Get(ConfigKey::SoundFolder, L"");
	this->folderPathEdit.SetWindowText(path);

	CDialogEx::OnOK();
}


void SoundFolderSelector::OnBnClickedBrowseButton() {
	CFolderPickerDialog dialog;
	if(IDOK == dialog.DoModal()) {
		folderPathEdit.SetWindowText(dialog.GetFolderPath());
	}
}
