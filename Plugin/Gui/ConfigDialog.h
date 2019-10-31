#pragma once

#include <Util/Config.h>


namespace TSPlugin {

	// ConfigDialog dialog

	class ConfigDialog : public CDialogEx {
		

	private:

		DECLARE_DYNAMIC(ConfigDialog)

	public:
		ConfigDialog(const ConfigDictionary& configDictionary, CWnd* pParent = nullptr);   // standard constructor
		virtual ~ConfigDialog();

		// Dialog Data
#ifdef AFX_DESIGN_TIME
		enum { IDD = IDD_CONFIG_DIALOG };
#endif

	protected:
		void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
		BOOL OnInitDialog() override;

		DECLARE_MESSAGE_MAP()
	public:
		CListCtrl ConfigListControl;
		ConfigDictionary m_configDictionary;
	};

} // namespace TSPlugin
