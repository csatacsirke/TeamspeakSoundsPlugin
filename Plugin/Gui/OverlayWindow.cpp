#include "stdafx.h"
#include "OverlayWindow.h"

#include <Util/Config.h>

#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib")


namespace TSPlugin {

	using namespace Gdiplus;

	constexpr COLORREF TransparentKey = RGB(123, 53, 54);

	GdiPlusComponent::GdiPlusComponent() {
		// Initialize GDI+.
		GdiplusStartupInput gdiplusStartupInput;
		GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	}
	GdiPlusComponent::~GdiPlusComponent() {
		GdiplusShutdown(gdiplusToken);
	}


	//static weak_ptr<OverlayWindow> overlayWindowInstanceWeakRef;



	//const shared_ptr<OverlayWindow>& OverlayWindow::GetInstance() {
	//	auto overlayWindowInstance = overlayWindowInstanceWeakRef.lock();
	//	if (!overlayWindowInstance) {
	//		overlayWindowInstance = make_shared<OverlayWindow>();
	//		overlayWindowInstanceWeakRef = overlayWindowInstance;
	//		
	//		overlayWindowInstance->Create(IDD_OVERLAY_WINDOW);
	//		overlayWindowInstance->ShowWindow(SW_SHOW);
	//	}

	//	return overlayWindowInstance;
	//}

	BEGIN_MESSAGE_MAP(OverlayWindow, CDialogEx)
		ON_WM_PAINT()
		ON_WM_ERASEBKGND()
	END_MESSAGE_MAP()



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

	BOOL OverlayWindow::OnInitDialog() {
		const BOOL result = __super::OnInitDialog();


		const BYTE overlayBackgroundAlpha = (BYTE)Global::config.Get<int>(ConfigKeys::OverlayBackgroundAlpha).value_or(96);
		SetLayeredWindowAttributes(TransparentKey, overlayBackgroundAlpha, LWA_COLORKEY | LWA_ALPHA);
		ShowWindow(SW_SHOWMAXIMIZED);
		//SetLayeredWindowAttributes(TransparentKey, 125, LWA_COLORKEY);
		//SetLayeredWindowAttributes(0xffffffff, 125, LWA_ALPHA); 

		
		return result;
	}
	

	static inline void PaintBorder(Graphics& graphics, const Rect& clientRect) {
		
		//CPen borderPen;
		//borderPen.CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
		//dc.SelectObject(borderPen);
		//
		////dc.SetBkMode(TRANSPARENT);
		//dc.SelectStockObject(NULL_BRUSH);
		//dc.Rectangle(&clientRect);


	}

	static inline void DrawTextToContext(Graphics& graphics, const CString& text) {

		const REAL fontSize = Global::config.Get<REAL>(ConfigKeys::OverlayFontSize).value_or(18);

		SolidBrush brush(Color(255, 255, 255, 255));
		FontFamily fontFamily(L"Times New Roman");
		Gdiplus::Font font(&fontFamily, fontSize, FontStyleRegular, UnitPixel);
		PointF pointF(10.0f, 20.0f);

		RectF boundingBox;
		graphics.MeasureString(text, -1, &font, pointF, &boundingBox);

		SolidBrush backgroundBrush(Color(255, 0, 0, 0));
		graphics.FillRectangle(&backgroundBrush, boundingBox);
		graphics.DrawString(text, -1, &font, pointF, &brush);
	}

	void OverlayWindow::PaintToBackbuffer(HDC dc) {
		Graphics graphics(dc);

		//graphics.Clear(Color(0, 0, 0, 0));
		//graphics.Clear(Color(TransparentKey));


		CRect clientRect;
		GetClientRect(clientRect);

		Rect rect(0, 0, clientRect.Width(), clientRect.Height());
		//PaintBorder(graphics, rect);
		
		DrawTextToContext(graphics, infoData);

	}


	void OverlayWindow::OnPaint() {


		if (CDC* dc = GetDC()) {
			//const int graphicsState = dc->SaveDC();



			CRect clientRect;
			GetClientRect(clientRect);


			BP_PAINTPARAMS params = { sizeof(params), BPPF_NOCLIP | BPPF_ERASE };
			HDC hdcMem;
			HPAINTBUFFER hpb = BeginBufferedPaint(*dc, &clientRect, BPBF_TOPDOWNDIB, &params, &hdcMem);
			//CDC memDc;
			//memDc.Attach(hdcMem);

			if (hpb) {
				CBrush brush;
				brush.CreateSolidBrush(TransparentKey);
				FillRect(hdcMem, &clientRect, brush);

				PaintToBackbuffer(hdcMem);
				

				//int cxRow;
				//RGBQUAD *prgbBits;
				//if (SUCCEEDED(GetBufferedPaintBits(hpb, &prgbBits, &cxRow))) {
				//
				//	EraseToTransparent(hdcMem, clientRect);
				//	PaintBorder(hdcMem, clientRect);
				//}

				EndBufferedPaint(hpb, TRUE);
			}
			//memDc.Detach();

			//dc->RestoreDC(graphicsState);
			
			ReleaseDC(dc);
			ValidateRect(clientRect);
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

	void OverlayWindow::SetInfoData(const CString& newInfoData) {
		infoData = newInfoData;
		Invalidate();
	}


	//void OverlayWindow::OnDraw(CDC* pDC) {
	//	//DrawTextA()
	//	CRect rect = { 0, 0, 200, 200 };
	//	pDC->DrawText(L"asdf", 4, &rect, 0);
	//}

} // namespace TSPlugin 


