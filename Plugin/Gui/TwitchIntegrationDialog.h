#pragma once


// TwitchIntegrationDialog dialog

class TwitchIntegrationDialog : public CDialogEx
{
	DECLARE_DYNAMIC(TwitchIntegrationDialog)

public:
	TwitchIntegrationDialog(CWnd* pParent = nullptr);   // standard constructor
	virtual ~TwitchIntegrationDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TWITCH_INTEGRATION_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
