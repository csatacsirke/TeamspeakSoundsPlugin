#include "stdafx.h"
#include "OverlayWindow.h"

#include <gdiplus.h>


namespace TSPlugin {

	static shared_ptr<OverlayWindow> overlayWindowInstance = nullptr;



	const shared_ptr<OverlayWindow>& OverlayWindow::GetInstance() {
		if (!overlayWindowInstance) {
			overlayWindowInstance = make_shared<OverlayWindow>();
			
			const auto windowClass = AfxRegisterWndClass(0, 0, (HBRUSH)GetStockObject(NULL_BRUSH));
			overlayWindowInstance->CreateEx(WS_EX_TOPMOST, windowClass, L"OverlayWindow", WS_VISIBLE, CRect{0, 0, 400, 400}, nullptr, 0);
			//overlayWindowInstance->Create(IDD_OVERLAY_WINDOW);
			//overlayWindowInstance->ShowWindow(SW_SHOWMAXIMIZED);
			overlayWindowInstance->ShowWindow(SW_SHOW);
		}

		return overlayWindowInstance;
	}


	IMPLEMENT_DYNAMIC(OverlayWindow, CWnd)


	BEGIN_MESSAGE_MAP(OverlayWindow, OverlayWindow::ParentClass)
		ON_WM_PAINT()
		ON_WM_ERASEBKGND()
	END_MESSAGE_MAP()

	//OverlayWindow::OverlayWindow(CWnd* pParent /*=nullptr*/)
	//	: CDialogEx(IDD_OVERLAY_WINDOW, pParent) {
	//	// NULL
	//}

	//OverlayWindow::OverlayWindow(CWnd* pParent /*=nullptr*/)
	//	: CDialogEx(IDD_OVERLAY_WINDOW, pParent) {
	//	// NULL
	//}

	BOOL OverlayWindow::PreCreateWindow(CREATESTRUCT& cs) {
		cs.style &= ~(WS_SYSMENU | WS_CAPTION);
		//cs.dwExStyle |= WS_EX_LAYERED;

		//cs.dwExStyle &= ~(WS_EX_TOPMOST);

		const BOOL result = __super::PreCreateWindow(cs);

		//cs.style &= ~(WS_SYSMENU | WS_CAPTION);
		//cs.dwExStyle |= WS_EX_LAYERED;

		return result;
	}

	void OverlayWindow::DoDataExchange(CDataExchange* pDX) {
		__super::DoDataExchange(pDX);
	}

	//BOOL OverlayWindow::OnInitDialog() {
	//	const BOOL result = __super::OnInitDialog();
	//
	//	return result;
	//}

	void OverlayWindow::OnPaint() {
		if (CDC* dc = GetDC()) {

			CRect clientRect;
			GetClientRect(clientRect);


			const int graphicsState = dc->SaveDC();

			CPen borderPen;
			borderPen.CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
			dc->SelectObject(borderPen);

			//dc->SetBkMode(TRANSPARENT);
			dc->SelectStockObject(NULL_BRUSH);
			dc->Rectangle(&clientRect);


			dc->RestoreDC(graphicsState);

			ReleaseDC(dc);
		}
		

	}

	BOOL OverlayWindow::OnEraseBkgnd(CDC* dc) {
		dc->SetBkMode(TRANSPARENT);
		return TRUE;
		//CRect rect;
		//GetClientRect(&rect);
		//dc->SelectObject((HBRUSH)GetStockObject(NULL_BRUSH));
		//return dc->PatBlt(0, 0, rect.Width(), rect.Height(), PATCOPY);
/*
		CRect clientRect;
		GetClientRect(clientRect);

		dc->SetBkMode(TRANSPARENT);
		dc->SelectStockObject(NULL_BRUSH);
		dc->Rectangle(&clientRect);

		return TRUE;*/
	}

	void OverlayWindow::SetInfoData(const CStringA& infoData) {
		InfoData = infoData;
	}


	//void OverlayWindow::OnDraw(CDC* pDC) {
	//	//DrawTextA()
	//	CRect rect = { 0, 0, 200, 200 };
	//	pDC->DrawText(L"asdf", 4, &rect, 0);
	//}

} // namespace TSPlugin 

