#pragma once


#include <Twitch/TwitchApi.h>
#include <Util/Util.h>

#include <thread>
#include <memory>

// TwitchIntegrationDialog dialog
namespace TSPlugin {


	class TwitchIntegrationDialog : public CDialogEx {
		DECLARE_DYNAMIC(TwitchIntegrationDialog)

	public:
		TwitchIntegrationDialog(shared_ptr<Twitch::TwitchState> twitchState, CWnd* pParent = nullptr);   // standard constructor
		virtual ~TwitchIntegrationDialog();

		BOOL OnInitDialog() override;

		// Dialog Data
#ifdef AFX_DESIGN_TIME
		enum { IDD = IDD_TWITCH_INTEGRATION_DIALOG };
#endif

		static const int WM_USER_REFRESH_DIALOG = WM_USER;

	protected:
		void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

		DECLARE_MESSAGE_MAP()
	public:
		afx_msg void OnLoginToTwitchButtonClicked();
		afx_msg void OnRefreshDialog();

	private:
		void RefreshStatusText();
	private:
		std::shared_ptr<Twitch::TwitchState> twitchState;

		timer_killer userLoginThreadState;
		std::thread userLoginThread;
	public:
		afx_msg void OnLogoutButtonClicked();
	};

}
