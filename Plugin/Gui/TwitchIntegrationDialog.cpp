// Gui/TwitchIntegrationDialog.cpp : implementation file
//

#include "stdafx.h"
#include "TwitchIntegrationDialog.h"
#include "afxdialogex.h"
#include <Twitch/TwitchLogin.h>

// TwitchIntegrationDialog dialog

IMPLEMENT_DYNAMIC(TwitchIntegrationDialog, CDialogEx)

TwitchIntegrationDialog::TwitchIntegrationDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TWITCH_INTEGRATION_DIALOG, pParent)
{

}

TwitchIntegrationDialog::~TwitchIntegrationDialog()
{
}

void TwitchIntegrationDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(TwitchIntegrationDialog, CDialogEx)
END_MESSAGE_MAP()


// TwitchIntegrationDialog message handlers
