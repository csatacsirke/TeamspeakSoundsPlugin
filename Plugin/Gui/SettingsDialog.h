#pragma once
#include "afxwin.h"

#include "Gui/DlgScrollable.h"

// SettingsDialog dialog


namespace TSPlugin {

	class SettingsDialog : public CDialogEx {
		const CPoint size = CPoint(200, 20);
	private:
		std::vector<std::unique_ptr<CWnd>> children;
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
		void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
		BOOL OnInitDialog() override;

		afx_msg void OnBnClickedEditButton();


		DECLARE_DYNAMIC(SettingsDialog)

		DECLARE_MESSAGE_MAP()
	private:
		void AddPresetHotkeyButton(CString caption, CStringA key);
		void AddHotkeyButton(CString caption, CStringA key);
		void AddButton(CString caption, std::function<void()> onClick);



		CEdit defaultSoundfolderEdit;
		CStatic hotkeysGroup;
		std::unique_ptr<CDlgScrollable> scrollView;
		//std::unique_ptr<CFormView> scrollView;
	};


}