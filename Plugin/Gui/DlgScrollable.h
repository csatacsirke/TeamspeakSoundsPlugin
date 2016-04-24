#if !defined(AFX_DLGSCROLLABLE_H__3586FDB6_FC0D_4FF7_BA09_D86692F0006A__INCLUDED_)
#define AFX_DLGSCROLLABLE_H__3586FDB6_FC0D_4FF7_BA09_D86692F0006A__INCLUDED_

// Innen loptam: (2016.04.24)
// http ://www.codeproject.com/Articles/4087/How-to-implement-a-scrollable-child-dialog

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgScrollable.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgScrollable dialog

class CDlgScrollable : public CDialog
{
// Construction
public:
	void EndDrag();
	CDlgScrollable(CWnd* pParent = NULL);   // standard constructor

	//HCURSOR m_hCursor1, m_hCursor2;

	// dialog size as you see in the resource view (original size)
	CRect	m_rcOriginalRect;

	// dragging
	BOOL	m_bDragging;
	CPoint	m_ptDragPoint;

	// actual scroll position
	int		m_nScrollPos;

	// actual dialog height
	int		m_nCurHeight;

// Dialog Data
	//{{AFX_DATA(CDlgScrollable)
	enum { IDD = IDD_SCROLLABLE };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgScrollable)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgScrollable)
	virtual BOOL OnInitDialog();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnParentNotify(UINT message,LPARAM lParam);
	
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	void AddChild(CWnd* wnd);
private:
	int CalculateBottomPos();
	void OnNewChild(HWND child);
	void UpdateScrollView();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSCROLLABLE_H__3586FDB6_FC0D_4FF7_BA09_D86692F0006A__INCLUDED_)
