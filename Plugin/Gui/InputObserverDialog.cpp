// InputObserverDialog.cpp : implementation file
//

#include "stdafx.h"
#include "InputObserverDialog.h"
#include "afxdialogex.h"


// InputObserverDialog dialog

IMPLEMENT_DYNAMIC(InputObserverDialog, CDialogEx)

InputObserverDialog::InputObserverDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_INPUT_OBSERVER_DIALOG, pParent)
{
	__super::Create(IDD_INPUT_OBSERVER_DIALOG, pParent);
}

InputObserverDialog::~InputObserverDialog()
{
}

void InputObserverDialog::CreateIfNecessary() {
	if (!__super::GetSafeHwnd()) {
		__super::Create(IDD_INPUT_OBSERVER_DIALOG);
	}
}

void InputObserverDialog::SetFiles(const std::vector<CString>& files) {
	possibleFilesListBox.ResetContent();
	for (const CString& file : files) {
		//possibleFilesListBox.AddString(file);
		possibleFilesListBox.InsertString(-1, file);
	}
	
}

void InputObserverDialog::SetSelectedIndex(int selectedIndex) {
	possibleFilesListBox.SetCurSel(selectedIndex);
}

void InputObserverDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, possibleFilesListBox);
}


BEGIN_MESSAGE_MAP(InputObserverDialog, CDialogEx)
END_MESSAGE_MAP()


// InputObserverDialog message handlers
