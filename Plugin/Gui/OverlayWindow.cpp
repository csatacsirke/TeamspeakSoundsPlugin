#include "stdafx.h"
#include "OverlayWindow.h"

#include <Util/Config.h>

#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib")


namespace TSPlugin {

	using namespace Gdiplus;

	constexpr COLORREF TransparentKey = RGB(1, 1, 1);

	GdiPlusComponent::GdiPlusComponent() {
		// Initialize GDI+.
		GdiplusStartupInput gdiplusStartupInput;
		GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	}
	GdiPlusComponent::~GdiPlusComponent() {
		GdiplusShutdown(gdiplusToken);
	}


	BEGIN_MESSAGE_MAP(OverlayWindow, CDialogEx)
		ON_WM_PAINT()
		ON_WM_ERASEBKGND()
	END_MESSAGE_MAP()


	void OverlayWindow::PreSubclassWindow() {
		__super::PreSubclassWindow();
		ModifyStyle(WS_SYSMENU | WS_CAPTION, 0);
		ModifyStyleEx(0, WS_EX_TOOLWINDOW);
	}

	void OverlayWindow::DoDataExchange(CDataExchange* pDX) {
		__super::DoDataExchange(pDX);
	}

	BOOL OverlayWindow::OnInitDialog() {
		const BOOL result = __super::OnInitDialog();

		//const BYTE overlayBackgroundAlpha = (BYTE)Global::config.Get<int>(ConfigKeys::OverlayBackgroundAlpha).value_or(96);
		const BYTE overlayBackgroundAlpha = 255;
		SetLayeredWindowAttributes(TransparentKey, overlayBackgroundAlpha, LWA_COLORKEY | LWA_ALPHA);
		ShowWindow(SW_SHOWMAXIMIZED);
		
		return result;
	}


	static inline void DrawTextToContext(Graphics& graphics, const CString& text) {

		const REAL fontSize = Global::config.Get<REAL>(ConfigKeys::OverlayFontSize).value_or(18);

		FontFamily fontFamily(L"Times New Roman");
		Gdiplus::Font font(&fontFamily, fontSize, FontStyleRegular, UnitPixel);
		PointF pointF(10.0f, 20.0f);


		RectF boundingBox;
		graphics.MeasureString(text, -1, &font, pointF, &boundingBox);

		//SolidBrush backgroundBrush(Color(255, 0, 0, 0));
		//graphics.FillRectangle(&backgroundBrush, boundingBox);
		//graphics.DrawString(text, -1, &font, pointF, &brush);


		StringFormat stringFormat;
		GraphicsPath path;
		path.AddString(text, -1, &fontFamily, FontStyleRegular, fontSize, Gdiplus::Point(10, 10), &stringFormat);

		Pen pen(Color(0, 0, 0), fontSize / 4);
		graphics.DrawPath(&pen, &path);
		SolidBrush brush(Color(255, 255, 255, 255));
		graphics.FillPath(&brush, &path);
		
	}


	void OverlayWindow::PaintToBackbuffer(HDC dc) {
		Graphics graphics(dc);
		graphics.SetSmoothingMode(SmoothingMode::SmoothingModeHighQuality);

		CRect clientRect;
		GetClientRect(clientRect);

		Rect rect(0, 0, clientRect.Width(), clientRect.Height());
		
		const shared_ptr<const CString> infoData_guard = infoData;
		DrawTextToContext(graphics, *infoData_guard);

	}


	void OverlayWindow::OnPaint() {

		CPaintDC dc(this);
		CRect clientRect;

		GetClientRect(clientRect);

		BP_PAINTPARAMS params = { sizeof(params), BPPF_NOCLIP | BPPF_ERASE };
		HDC hdcMem;
		HPAINTBUFFER hpb = BeginBufferedPaint(dc.m_hDC, &clientRect, BPBF_TOPDOWNDIB, &params, &hdcMem);
		
		if (hpb) {
			CBrush brush;
			brush.CreateSolidBrush(TransparentKey);
			FillRect(hdcMem, &clientRect, brush);

			PaintToBackbuffer(hdcMem);

			EndBufferedPaint(hpb, TRUE);
		}
		

	}

	BOOL OverlayWindow::OnEraseBkgnd(CDC* dc) {
		dc->SetBkMode(TRANSPARENT);
		return TRUE;
	}

	void OverlayWindow::SetInfoData(const CString& newInfoData) {
		infoData = make_shared<CString>(newInfoData);
		Invalidate();
	}


} // namespace TSPlugin 


