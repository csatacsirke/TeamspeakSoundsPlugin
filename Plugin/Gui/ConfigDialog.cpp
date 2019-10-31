// ConfigDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ConfigDialog.h"
#include "afxdialogex.h"

namespace TSPlugin {

	// ConfigDialog dialog

	IMPLEMENT_DYNAMIC(ConfigDialog, CDialogEx)

	ConfigDialog::ConfigDialog(const ConfigDictionary& configDictionary, CWnd* pParent /*=nullptr*/)
		: CDialogEx(IDD_CONFIG_DIALOG, pParent), m_configDictionary(configDictionary)
	{
		// NULL
	}

	ConfigDialog::~ConfigDialog() {
	}

	void ConfigDialog::DoDataExchange(CDataExchange* pDX) {
		CDialogEx::DoDataExchange(pDX);
		DDX_Control(pDX, IDC_CONFIG_LIST, ConfigListControl);
	}

	BOOL ConfigDialog::OnInitDialog() {
		const BOOL result = __super::OnInitDialog();



		return result;
	}


	BEGIN_MESSAGE_MAP(ConfigDialog, CDialogEx)
	END_MESSAGE_MAP()


	// ConfigDialog message handlers

} // namespace TSPlugin

