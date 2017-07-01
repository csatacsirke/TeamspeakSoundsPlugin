// Gui/NetworkDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Gui/NetworkDialog.h"
#include "afxdialogex.h"


// NetworkDialog dialog

IMPLEMENT_DYNAMIC(NetworkDialog, CDialogEx)

NetworkDialog::NetworkDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG2, pParent)
{

}

NetworkDialog::~NetworkDialog()
{
}

void NetworkDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(NetworkDialog, CDialogEx)
END_MESSAGE_MAP()


// NetworkDialog message handlers


BOOL NetworkDialog::OnInitDialog() {
	CDialogEx::OnInitDialog();

	CWnd* listArea = GetDlgItem(IDC_LIST_BOX);
	scrollView.reset(new CDlgScrollable(listArea));


	CRect rect(0, 0, 200, 30);

	CButton* button1 = new CButton;
	button1->Create(L"lofasz", WS_VISIBLE | WS_CHILD, rect, this, 0);

	CButton* button2 = new CButton;
	button2->Create(L"lofasz 2 ", WS_VISIBLE | WS_CHILD, rect, this, 0);

	scrollView->AddChild(button1);
	scrollView->AddChild(button2);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}
