#pragma once


namespace TSPlugin {

	// PresetHotkeyGroup dialog

	class PresetHotkeyGroup : public CDialogEx {
		DECLARE_DYNAMIC(PresetHotkeyGroup)

		CStringA key;
		CString caption;
		void* qParentWidget;
	protected:
		std::unique_ptr<HotkeyButton> hotkeyButton;
		CEdit pathEdit;
		Button browseButton;

	public:
		PresetHotkeyGroup(CStringA key, CString caption, void* qParentWidget, CWnd* pParent);   // standard constructor
		virtual ~PresetHotkeyGroup();

		// Dialog Data
#ifdef AFX_DESIGN_TIME
		enum { IDD = IDD_PRESET_HOTKEY_GROUP };
#endif

	protected:
		void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
		BOOL OnInitDialog() override;
		void ResizeToContent();

		DECLARE_MESSAGE_MAP()


	};

}
