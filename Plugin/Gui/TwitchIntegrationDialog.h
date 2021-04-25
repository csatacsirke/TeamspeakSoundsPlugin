#pragma once


#include <Twitch/TwitchApi.h>
#include <Util/Util.h>

#include <concurrent_queue.h>

#include <thread>
#include <memory>


// TwitchIntegrationDialog dialog
namespace TSPlugin {


	class TwitchIntegrationDialog : public CDialogEx {
		DECLARE_DYNAMIC(TwitchIntegrationDialog)

		CString m_rewardId;
		CString m_rewardTitle;
		int m_rewardCost = -1;
		CString m_rewardMessageForUser;
	public:
		TwitchIntegrationDialog(shared_ptr<Twitch::TwitchState> twitchState, CWnd* pParent = nullptr);   // standard constructor
		virtual ~TwitchIntegrationDialog();

		BOOL OnInitDialog() override;

		// Dialog Data
#ifdef AFX_DESIGN_TIME
		enum { IDD = IDD_TWITCH_INTEGRATION_DIALOG };
#endif

		static const int WM_USER_GUI_CALLBACK = WM_USER;
		concurrency::concurrent_queue<std::function<void()>> guiTasks;
		std::jthread userLoginThread;
		std::jthread loadThread;
	public:
		void ExecuteOnGuiThread(std::function<void()>&& fn);


	protected:
		void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

		DECLARE_MESSAGE_MAP()
	public:
		afx_msg void OnLoginToTwitchButtonClicked();
		afx_msg void OnGuiCallback();
		afx_msg void OnLogoutButtonClicked();
		afx_msg void OnReloadButtonClicked();
		afx_msg void OnSaveButtonClicked();

	private:
		void RefreshStatusText();
		void CallRefreshStatusText();
		void LoadRewardsFromTwitch();
		void LoadRewardsFromTwitchInBackground();
		void SaveRewardsToTwitchInBackground();
		void LoginInBackground();
	private:
		std::shared_ptr<Twitch::TwitchState> twitchState;

	};

}
