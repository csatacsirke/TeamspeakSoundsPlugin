#pragma once


// SettingsDialog dialog

class SettingsDialog : public CDialogEx
{
private:
	std::vector<std::unique_ptr<CWnd>> children;
	CPoint pos = CPoint(20, 20);
	const CPoint size = CPoint(200, 20);
protected:
	//CScrollView container;
public:
	void* qParentWidget = NULL;

public:
	SettingsDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~SettingsDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTINGS_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	BOOL OnInitDialog() override;
	void AddHotkeyButton(CString caption, CStringA key);
	//void AddButton(CString caption, CWnd* parent, std::function<void()> onClick);


	DECLARE_DYNAMIC(SettingsDialog)

	DECLARE_MESSAGE_MAP()
};
