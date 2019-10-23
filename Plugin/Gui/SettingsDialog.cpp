// Gui/SettingsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "SettingsDialog.h"
#include "SoundFolderSelector.h"
#include "PresetHotkeyGroup.h"
#include "afxdialogex.h"


#include <Util\Config.h>


namespace TSPlugin {

	// SettingsDialog dialog

	IMPLEMENT_DYNAMIC(SettingsDialog, CDialogEx)

		SettingsDialog::SettingsDialog(CWnd* pParent /*=NULL*/)
		: CDialogEx(IDD_SETTINGS_DIALOG, pParent) {

	}

	SettingsDialog::~SettingsDialog() {
	}



	BOOL SettingsDialog::OnInitDialog() {
		CDialogEx::OnInitDialog();

		scrollView.reset(new CDlgScrollable(&hotkeysGroup));


		AddHotkeyButton(L"Stop", Hotkey::STOP);
		AddHotkeyButton(L"Replay", Hotkey::REPLAY);
		AddHotkeyButton(L"Play queued", Hotkey::PLAY_QUEUED);
		AddHotkeyButton(L"Play random", Hotkey::PLAY_RANDOM);

		//for(int i = 0; i < soundHotkeyCount; ++i) {
		//	CStringA hotkey;
		//	hotkey.Format(Hotkey::PLAY_PRESET_TEMPLATE, i);
		//
		//	CString title;
		//	title.Format(L"Play sound #%d", i);
		//
		//	AddPresetHotkeyButton(title, hotkey);
		//}

			// amig ki nem találok valamit ennek a végén kell lennie mert csak... 
		CRect rect;
		hotkeysGroup.GetClientRect(rect);
		rect.DeflateRect(20, 20, 20, 20);
		scrollView->MoveWindow(rect);

		return 0;
	}

	void SettingsDialog::DoDataExchange(CDataExchange* pDX) {
		CDialogEx::DoDataExchange(pDX);
		DDX_Control(pDX, IDC_DEAULT_SOUND_FOLDER_EDIT, defaultSoundfolderEdit);
		DDX_Control(pDX, IDC_HOTKEYS_GROUP, hotkeysGroup);

		defaultSoundfolderEdit.SetWindowText(Global::config.TryGet(ConfigKeys::SoundFolder).value_or(L"<unset>"));
	}

	void SettingsDialog::AddButton(CString caption, std::function<void()> onClick) {

		Button* button = new Button;
		button->Create(caption, WS_VISIBLE | WS_CHILD, CRect(CPoint(), size), this, 0);
		button->OnClick = onClick;

		scrollView->AddChild(button);
		children.push_back(std::unique_ptr<CWnd>(button));
	}



	void SettingsDialog::AddHotkeyButton(CString caption, CStringA key) {

		HotkeyButton* button = new HotkeyButton(key, caption);
		button->Create(caption, WS_VISIBLE | WS_CHILD, CRect(CPoint(), size), this, 0);
		button->OnClick = [=] {
			Global::ts3Functions.requestHotkeyInputDialog(Global::pluginID, key, 0, qParentWidget);
			button->UpdateCaption();
		};
		button->UpdateCaption();

		scrollView->AddChild(button);
		children.push_back(std::unique_ptr<CWnd>(button));

	}


	void SettingsDialog::AddPresetHotkeyButton(CString caption, CStringA key) {
		PresetHotkeyGroup* group = new PresetHotkeyGroup(key, caption, qParentWidget, this);
		scrollView->AddChild(group);
		children.push_back(std::unique_ptr<CWnd>(group));
	}


	BEGIN_MESSAGE_MAP(SettingsDialog, CDialogEx)
		ON_BN_CLICKED(IDC_EDIT_BUTTON, &SettingsDialog::OnBnClickedEditButton)
	END_MESSAGE_MAP()


	// SettingsDialog message handlers


	void SettingsDialog::OnBnClickedEditButton() {
		SoundFolderSelector dialog;
		dialog.DoModal();
		//AddButton(L"Set default sound folder", [] {
		//	
		//});
	}

}
