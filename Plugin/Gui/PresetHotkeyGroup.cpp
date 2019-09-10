// Gui\PresetHotkeyGroup.cpp : implementation file
//

#include "stdafx.h"
#include "PresetHotkeyGroup.h"
#include "afxdialogex.h"


#include <Util\Config.h>



namespace TSPlugin {

	// PresetHotkeyGroup dialog

	IMPLEMENT_DYNAMIC(PresetHotkeyGroup, CDialogEx)

		PresetHotkeyGroup::PresetHotkeyGroup(CStringA key, CString caption, void* qParentWidget, CWnd* pParent /*=NULL*/)
		: CDialogEx(IDD_PRESET_HOTKEY_GROUP, pParent) {
		this->key = key;
		this->caption = caption;
		this->qParentWidget = qParentWidget;

		Create(IDD_PRESET_HOTKEY_GROUP, pParent);
	}

	PresetHotkeyGroup::~PresetHotkeyGroup() {
	}

	BOOL PresetHotkeyGroup::OnInitDialog() {
		CDialogEx::OnInitDialog();

		const CPoint buttonSize = CPoint(200, 20);
		hotkeyButton.reset(new HotkeyButton(key, caption));
		hotkeyButton->Create(caption, WS_VISIBLE | WS_CHILD, CRect(CPoint(), buttonSize), this, 0);

		CRect buttonRect;
		hotkeyButton->GetWindowRect(buttonRect);
		hotkeyButton->OnClick = [&] {
			Global::ts3Functions.requestHotkeyInputDialog(Global::pluginID, key, 0, qParentWidget);
			hotkeyButton->UpdateCaption();
		};
		hotkeyButton->UpdateCaption();
		this->ScreenToClient(buttonRect);

		CPoint editOffset(buttonRect.right + 20, buttonRect.top);
		const CPoint editSize = CPoint(300, 20);

		pathEdit.Create(WS_VISIBLE | WS_CHILD, CRect(editOffset, editOffset + editSize), this, 0);
		pathEdit.EnableWindow(FALSE);
		pathEdit.SetWindowText(Global::config.Get(CString(key)));

		CRect editRect;
		pathEdit.GetWindowRect(editRect);
		this->ScreenToClient(editRect);
		CPoint buttonOffset(editRect.right + 20, editRect.top);
		const CPoint browseButtonSize = CPoint(100, 20);
		browseButton.Create(L"Browse", WS_VISIBLE | WS_CHILD, CRect(buttonOffset, buttonOffset + browseButtonSize), this, 0);
		browseButton.OnClick = [=] {
			CFileDialog dialog(TRUE);
			if (IDOK == dialog.DoModal()) {
				CString path = dialog.GetPathName();

				Global::config.Add(CString(key), path);
				Global::config.Save();

				pathEdit.SetWindowText(Global::config.Get(CString(key)));
			}
		};

		ResizeToContent();
		return 0;
	}


	void PresetHotkeyGroup::ResizeToContent() {
		int x = 0;
		int y = 0;
		for (CWnd *pWnd = GetWindow(GW_CHILD); pWnd != NULL; pWnd = pWnd->GetNextWindow(GW_HWNDNEXT)) {
			//Insert your code here. pWnd is a pointer to control window.
			CRect rect;
			pWnd->GetWindowRect(rect);

			y = max(rect.bottom, y);
			x = max(rect.right, x);
		}

		CRect rect;
		GetWindowRect(rect);
		rect.right = x;
		rect.bottom = y;
		MoveWindow(rect);
	}

	void PresetHotkeyGroup::DoDataExchange(CDataExchange* pDX) {
		CDialogEx::DoDataExchange(pDX);
	}


	BEGIN_MESSAGE_MAP(PresetHotkeyGroup, CDialogEx)
	END_MESSAGE_MAP()


	// PresetHotkeyGroup message handlers
}
