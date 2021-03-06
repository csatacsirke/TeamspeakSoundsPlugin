#pragma once


// InputObserverDialog dialog

class InputObserverDialog : public CDialogEx
{
	DECLARE_DYNAMIC(InputObserverDialog)

public:
	InputObserverDialog(CWnd* pParent = nullptr);   // standard constructor
	virtual ~InputObserverDialog();
	
public:
	void CreateIfNecessary();
	void SetFiles(const std::vector<CString>& files);
	void SetSelectedIndex(int selectedIndex);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_INPUT_OBSERVER_DIALOG };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CListBox possibleFilesListBox;
};
