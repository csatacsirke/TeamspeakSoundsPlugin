// Gui/SettingsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Gui/SettingsDialog.h"
#include "afxdialogex.h"


#include "Config.h"
// SettingsDialog dialog

IMPLEMENT_DYNAMIC(SettingsDialog, CDialogEx)

SettingsDialog::SettingsDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SETTINGS_DIALOG, pParent)
{

}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

class Button : public CButton {
public:
	std::function<void()> OnClick;
protected:
	
	//BOOL OnButtonClicked() override {

	LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) override {
		
		if(message == WM_LBUTTONDOWN) {
			if(OnClick) {
				OnClick();
			}
			return TRUE;
		}
		return CButton::WindowProc(message, wParam, lParam);
	}
	
};

class HotkeyButton : public Button {
	CStringA key;
public:

	HotkeyButton(CStringA key) {
		this->key = key;
	}
	void UpdateCaption() {
		CStringA value = GetHotkey(key);
		if(value != "") {
			value = "(" + value + ")";
		}

		CString text = CString("Stop" + value);
		SetWindowText(text);
	}
};

//void SettingsDialog::AddButton(CString caption, CWnd* parent, std::function<void()> onClick) {
//	
//	Button* button = new Button;
//	button->Create(caption, WS_VISIBLE | WS_CHILD, CRect(pos, pos + size), parent, 0);
//	button->OnClick = onClick;
//	
//	pos.y += 40;
//	children.push_back(std::unique_ptr<CWnd>(button));
//}

void SettingsDialog::AddHotkeyButton(CString caption, CStringA key) {
	CWnd* parent = GetDlgItem(IDC_HOTKEYS_GROUP);

	HotkeyButton* button = new HotkeyButton(key);
	button->Create(caption, WS_VISIBLE | WS_CHILD, CRect(pos, pos + size), parent, 0);
	button->OnClick = [=] {
		Global::ts3Functions.requestHotkeyInputDialog(Global::pluginID, key, 0, qParentWidget);
		button->UpdateCaption();
	};
	button->UpdateCaption();

	pos.y += 40;
	children.push_back(std::unique_ptr<CWnd>(button));
	
	CRect rect;
	parent->GetWindowRect(&rect);
	
}

BOOL SettingsDialog::OnInitDialog() {

	


	AddHotkeyButton(L"Stop", Hotkey::STOP);
	AddHotkeyButton(L"Replay", Hotkey::REPLAY);
	AddHotkeyButton(L"Play queued", Hotkey::PLAY_QUEUED);


	//CWnd* group = GetDlgItem(IDC_HOTKEYS_GROUP);

	//

	//AddButton(CString("Stop" + GetHotkey(Hotkey::STOP)), group, [&] {
	//	//MessageBoxA(0, "lel", 0, 0);
	//	Global::ts3Functions.requestHotkeyInputDialog(Global::pluginID, Hotkey::STOP, 0, qParentWidget);
	//});
	//
	//AddButton(CString("Replay"), group, [&] {
	//	//MessageBoxA(0, "lel", 0, 0);
	//	Global::ts3Functions.requestHotkeyInputDialog(Global::pluginID, Hotkey::REPLAY, 0, qParentWidget);
	//});

	//AddButton(CString("Play queued"), group, [&] {
	//	//MessageBoxA(0, "lel", 0, 0);
	//	Global::ts3Functions.requestHotkeyInputDialog(Global::pluginID, Hotkey::PLAY_QUEUED, 0, qParentWidget);
	//});

	

	//CButton* button1 = new CButton;
	//button1->Create(L"Stop", WS_VISIBLE| WS_CHILD, CRect(20, 20, 200, 20), group, 0);
	//
	//CButton* button2 = new CButton;
	//button2->Create(L"Stop", WS_VISIBLE | WS_CHILD, CRect(20, 20, 200, 20), group, 0);

	return 0;
}


BEGIN_MESSAGE_MAP(SettingsDialog, CDialogEx)
END_MESSAGE_MAP()


// SettingsDialog message handlers
