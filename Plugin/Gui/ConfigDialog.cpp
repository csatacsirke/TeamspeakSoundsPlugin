// ConfigDialog.cpp : implementation file
//

#include "stdafx.h"
#include "ConfigDialog.h"
#include "afxdialogex.h"

namespace TSPlugin {

	// ConfigDialog dialog

	IMPLEMENT_DYNAMIC(ConfigDialog, CDialogEx)


	BEGIN_MESSAGE_MAP(ConfigDialog, CDialogEx)
		ON_NOTIFY(LVN_ENDLABELEDIT, IDC_CONFIG_LIST, OnEdit)
	END_MESSAGE_MAP()

	ConfigDialog::ConfigDialog(const ConfigDictionary& configDictionary, CWnd* pParent /*=nullptr*/)
		: CDialogEx(IDD_CONFIG_DIALOG, pParent), m_configDictionary(configDictionary)
	{
		// NULL
	}

	ConfigDialog::~ConfigDialog() {
	}

	void ConfigDialog::DoDataExchange(CDataExchange* pDX) {
		CDialogEx::DoDataExchange(pDX);
		DDX_Control(pDX, IDC_CONFIG_LIST, configListControl);
	}

	BOOL ConfigDialog::OnInitDialog() {
		const BOOL result = __super::OnInitDialog();

		LVCOLUMN lvColumn;

		lvColumn.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
		lvColumn.fmt = LVCFMT_LEFT;
		lvColumn.cx = 250;
		lvColumn.pszText = L"Value";
		configListControl.InsertColumn(0, &lvColumn);

		lvColumn.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
		lvColumn.fmt = LVCFMT_LEFT;
		lvColumn.cx = 200;
		lvColumn.pszText = L"Key";
		
		configListControl.InsertColumn(1, &lvColumn);


		
		int nItem = 0;

		for (auto& pair : m_configDictionary) {
			LVITEM lvItem;
			lvItem.mask = LVIF_TEXT;
			lvItem.iItem = nItem;
			lvItem.iSubItem = 0;

			
			nItem = configListControl.InsertItem(&lvItem);

			configListControl.SetItemText(nItem, 0, pair.second);
			configListControl.SetItemText(nItem, 1, pair.first);

			listIndexToKeyMapping[nItem] = pair.first;
			++nItem;

		}

		return result;
	}

	void ConfigDialog::OnEdit(NMHDR* pNotifyStruct, LRESULT* result) {
		NMLVDISPINFO* notification = (NMLVDISPINFO*)pNotifyStruct;

		const int nItem = notification->item.iItem;
		//const CString newValue = configListControl.GetItemText(nItem, 0);
		const CString newValue = notification->item.pszText;
		const CString& key = listIndexToKeyMapping[nItem];

		m_configDictionary[key] = newValue;

		*result = TRUE;
	}



	// ConfigDialog message handlers

} // namespace TSPlugin

