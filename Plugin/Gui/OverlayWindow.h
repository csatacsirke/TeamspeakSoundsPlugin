#pragma once
#include <afxwin.h>


namespace TSPlugin {

	class BufferedPaintComponent {
	public:
		BufferedPaintComponent() {
			::BufferedPaintInit();
		}
		~BufferedPaintComponent() {
			::BufferedPaintUnInit();
		}
	};

	class GdiPlusComponent {
		ULONG_PTR gdiplusToken;
	public:
		GdiPlusComponent();
		~GdiPlusComponent();
	};

	class OverlayWindow : public CDialogEx {
		shared_ptr<const CString> interfaceText = make_shared<CString>();

	public:
		BufferedPaintComponent bufferedPaintComponent;
		GdiPlusComponent gdiPlusComponent;

		void SetInterfaceText(const CString& newInfoData);

	public:

		// Dialog Data
#ifdef AFX_DESIGN_TIME
		enum { IDD = IDD_OVERLAY_WINDOW };
#endif

	protected:
		void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
		BOOL OnInitDialog() override;

		//BOOL PreCreateWindow(CREATESTRUCT& cs) override;
		void PreSubclassWindow() override;
		void PaintTextToBackbuffer(HDC dc, const CString& text);


		afx_msg void OnPaint();
		afx_msg BOOL OnEraseBkgnd(CDC*);
		

		DECLARE_MESSAGE_MAP()


	};

} // namespace TSPlugin 

