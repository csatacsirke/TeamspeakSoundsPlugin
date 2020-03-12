#pragma once

#include <Util/Config.h>


namespace TSPlugin {

	// ConfigDialog dialog

	class ConfigDialog : public CDialogEx {
	private:
		CListCtrl configListControl;
		ConfigDictionary m_configDictionary;
		map<int, CString> listIndexToKeyMapping;

	private:

		DECLARE_DYNAMIC(ConfigDialog)

	public:
		ConfigDialog(const ConfigDictionary& configDictionary, CWnd* pParent = nullptr);   // standard constructor
		virtual ~ConfigDialog();


		const ConfigDictionary& GetEntries() const { return m_configDictionary; }



		// Dialog Data
#ifdef AFX_DESIGN_TIME
		enum { IDD = IDD_CONFIG_DIALOG };
#endif

	protected:
		void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
		BOOL OnInitDialog() override;


		afx_msg void OnEdit(NMHDR* pNotifyStruct, LRESULT* result);

		DECLARE_MESSAGE_MAP()

	private:
	
	};

} // namespace TSPlugin
