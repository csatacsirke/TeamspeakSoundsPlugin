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
}

BOOL TwitchIntegrationDialog::OnInitDialog() {
	const BOOL ret = __super::OnInitDialog();
	LoadRewardsFromTwitchInBackground();
	RefreshStatusText();
	return ret;
}

void TwitchIntegrationDialog::ExecuteOnGuiThread(std::function<void()>&& fn) {
	guiTasks.push(std::move(fn));
	PostMessage(WM_USER_GUI_CALLBACK);
}

void TwitchIntegrationDialog::DoDataExchange(CDataExchange* pDX)
{	

	DDX_Text(pDX, IDC_REWARD_ID_EDIT, m_rewardId);
	DDX_Text(pDX, IDC_REWARD_TITLE_EDIT, m_rewardTitle);
	DDX_Text(pDX, IDC_MESSAGE_FOR_USER_EDIT, m_rewardMessageForUser);
	DDX_Text(pDX, IDC_COST_EDIT, m_rewardCost);
	

	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(TwitchIntegrationDialog, CDialogEx)
	ON_BN_CLICKED(IDC_LOGIN_TO_TWITCH_BUTTON, &TwitchIntegrationDialog::OnLoginToTwitchButtonClicked)
	ON_BN_CLICKED(IDC_LOGOUT_BUTTON, &TwitchIntegrationDialog::OnLogoutButtonClicked)
	ON_MESSAGE_VOID(WM_USER_GUI_CALLBACK, OnGuiCallback)
	ON_BN_CLICKED(IDC_RELOAD_BUTTON, &TwitchIntegrationDialog::OnReloadButtonClicked)
	ON_BN_CLICKED(IDC_SAVE_BUTTON, &TwitchIntegrationDialog::OnSaveButtonClicked)
END_MESSAGE_MAP()


// TwitchIntegrationDialog message handlers


void TwitchIntegrationDialog::OnLoginToTwitchButtonClicked() {

	LoginInBackground();
	
}

void TwitchIntegrationDialog::OnGuiCallback() {
	function<void()> fn;
	while (guiTasks.try_pop(fn)) {
		fn();
	}
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

void TwitchIntegrationDialog::CallRefreshStatusText() {
	ExecuteOnGuiThread([this] {
		RefreshStatusText();
	});
}

void TwitchIntegrationDialog::LoadRewardsFromTwitch() try {

	auto optRewards = Twitch::GetRewards(*twitchState);
	if (!optRewards) {
		return;
	}

	auto& rewards = *optRewards;

	if (rewards["data"].size() == 0) {
		Twitch::CreateReward(*twitchState);

		// re-poll, not elegant, but gets the job done
		optRewards = Twitch::GetRewards(*twitchState);
	}

	if (!optRewards) {
		return;
	}

	if (rewards["data"].size() == 0) {
		ASSERT(0);
		return;
	}

	const auto rewardJson = rewards["data"][0];

	const std::string rewardId = rewardJson["id"];
	const std::string rewardTitle = rewardJson["title"];
	const int rewardCost = rewardJson["cost"];
	const std::string rewardMessageForUser = rewardJson["prompt"];

	ExecuteOnGuiThread([=, this] {
		m_rewardId = Utf8ToCString(rewardId.c_str());
		m_rewardTitle = Utf8ToCString(rewardTitle.c_str());
		m_rewardCost = rewardCost;
		m_rewardMessageForUser = Utf8ToCString(rewardMessageForUser.c_str());
		UpdateData(FALSE);
	});

} catch (nlohmann::json::exception) {
	ASSERT(0);
	// void
}

void TwitchIntegrationDialog::LoadRewardsFromTwitchInBackground() {
	if (loadThread.joinable()) {
		loadThread.join();
	}

	loadThread = jthread([this] {
		LoadRewardsFromTwitch();
	});
}


void TwitchIntegrationDialog::SaveRewardsToTwitchInBackground() {
	if (loadThread.joinable()) {
		loadThread.join();
	}

	UpdateData();
	Twitch::Json paramsToUpdate;
	paramsToUpdate["title"] = ConvertUnicodeToUTF8(m_rewardTitle);
	paramsToUpdate["cost"] = m_rewardCost;
	paramsToUpdate["prompt"] = ConvertUnicodeToUTF8(m_rewardMessageForUser);
	const CString rewardId = m_rewardId;

	loadThread = jthread([=, this] {
		const bool didUpdate = Twitch::UpdateReward(*twitchState, rewardId, paramsToUpdate);
		ExecuteOnGuiThread([didUpdate, this] {
			const wchar_t* text = didUpdate ? L"Update successful." : L"Failed to save changes to Twitch";
			const int nType = didUpdate ? MB_ICONINFORMATION : MB_ICONERROR;
			MessageBoxW(text, L"Reward update status", nType);
		});
	});
}

void TwitchIntegrationDialog::LoginInBackground() {
	if (userLoginThread.joinable()) {
		userLoginThread.request_stop();
		userLoginThread.join();
	}

	userLoginThread = std::jthread([&](std::stop_token stoken) {

		if (twitchState->session.Copy().GetLength() == 0) {
			if (!Twitch::StartUserLogin(*twitchState)) {
				CallRefreshStatusText();
				return;
			}
		}


		CallRefreshStatusText();

		while (true) {
			if (Twitch::PollAccessToken(*twitchState)) {
				CallRefreshStatusText();
				LoadRewardsFromTwitch();
				break;
			}

			std::mutex mutex;
			std::unique_lock lock(mutex);
			std::condition_variable_any().wait_for(lock, stoken, 3s,
				[&stoken] { return false; });

			if (stoken.stop_requested()) {
				break;
			}
		}
	});

}


void TwitchIntegrationDialog::OnLogoutButtonClicked() {
	twitchState->accessToken = L"";
	twitchState->session = L"";
	RefreshStatusText();
}


void TwitchIntegrationDialog::OnReloadButtonClicked() {
	LoadRewardsFromTwitchInBackground();
}


void TwitchIntegrationDialog::OnSaveButtonClicked() {
	SaveRewardsToTwitchInBackground();
}
