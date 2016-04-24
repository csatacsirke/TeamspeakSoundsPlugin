#pragma once

#include <afxwin.h>
#include <functional>
#include <Util\Util.h>

class Button : public CButton {
public:
	std::function<void()> OnClick;
protected:


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
	CString caption;
	CStringA key;
public:

	HotkeyButton(CStringA key, CString caption) {
		this->key = key;
		this->caption = caption;
	}
	void UpdateCaption() {
		CStringA value = GetHotkey(key);
		if(value != "") {
			value = "(" + value + ")";
		} else {
			value = "(undefined)";
		}

		CString text = caption + CString(value);
		SetWindowText(text);
	}
};



