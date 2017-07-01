#pragma once
#include "DlgScrollable.h"


// NetworkDialog dialog

class NetworkDialog : public CDialogEx
{
	DECLARE_DYNAMIC(NetworkDialog)

public:
	NetworkDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~NetworkDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG2 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	std::unique_ptr<CDlgScrollable> scrollView;
public:
	virtual BOOL OnInitDialog();
};
