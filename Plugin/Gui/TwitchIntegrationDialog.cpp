// Gui/TwitchIntegrationDialog.cpp : implementation file
//

#include "stdafx.h"
#include "TwitchIntegrationDialog.h"
#include "afxdialogex.h"


using namespace TSPlugin;

// TwitchIntegrationDialog dialog

IMPLEMENT_DYNAMIC(TwitchIntegrationDialog, CDialogEx)

TwitchIntegrationDialog::TwitchIntegrationDialog(shared_ptr<Twitch::TwitchState> twitchState, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TWITCH_INTEGRATION_DIALOG, pParent), twitchState(twitchState)
{

}

TwitchIntegrationDialog::~TwitchIntegrationDialog()
{
	if (userLoginThread.joinable()) {
		userLoginThreadState.kill();
		userLoginThread.join();
		userLoginThreadState.reset();
	}
}

BOOL TwitchIntegrationDialog::OnInitDialog() {
	const BOOL ret = __super::OnInitDialog();
	RefreshStatusText();
	return ret;
}

void TwitchIntegrationDialog::DoDataExchange(CDataExchange* pDX)
{	
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(TwitchIntegrationDialog, CDialogEx)
	ON_BN_CLICKED(IDC_LOGIN_TO_TWITCH_BUTTON, &TwitchIntegrationDialog::OnLoginToTwitchButtonClicked)
	ON_MESSAGE_VOID(WM_USER_REFRESH_DIALOG, OnRefreshDialog)
	ON_BN_CLICKED(IDC_LOGOUT_BUTTON, &TwitchIntegrationDialog::OnLogoutButtonClicked)
END_MESSAGE_MAP()


// TwitchIntegrationDialog message handlers


void TwitchIntegrationDialog::OnLoginToTwitchButtonClicked() {

	if (userLoginThread.joinable()) {
		userLoginThreadState.kill();
		userLoginThread.join();
		userLoginThreadState.reset();
	}

	userLoginThread = std::thread([&] {
		if (!Twitch::StartUserLogin(*twitchState)) {
			PostMessage(WM_USER_REFRESH_DIALOG);
			return;
		}

		PostMessage(WM_USER_REFRESH_DIALOG);

		while (true) {
			if (Twitch::PollAccessToken(*twitchState)) {
				PostMessage(WM_USER_REFRESH_DIALOG);
				break;
			}

			if (!userLoginThreadState.wait_for(3s)) {
				break;
			}
		}


	});
	
}

void TwitchIntegrationDialog::OnRefreshDialog() {
	RefreshStatusText();
}

void TwitchIntegrationDialog::RefreshStatusText() {
	const CString session = twitchState->session;
	const CString accessToken = twitchState->accessToken;
	CString statusText = L"Not logged in.";
	if (accessToken.GetLength() > 0) {
		statusText = L"Logged in.";
	} else if (session.GetLength() > 0) {
		statusText = L"Waiting for server.";
	}
	SetDlgItemTextW(IDC_LOGIN_STATUS_LABEL, statusText);
}


void TwitchIntegrationDialog::OnLogoutButtonClicked() {
	twitchState->accessToken = L"";
	twitchState->session = L"";
	RefreshStatusText();
}
