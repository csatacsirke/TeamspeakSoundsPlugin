#pragma once
#include <afxwin.h>


namespace TSPlugin {

	class BufferedPaintComponent {
	public:
		BufferedPaintComponent() {
			BufferedPaintInit();
		}
		~BufferedPaintComponent() {
			BufferedPaintUnInit();
		}
	};

	class GdiPlusComponent {
		ULONG_PTR gdiplusToken;
	public:
		GdiPlusComponent();
		~GdiPlusComponent();
	};

	class OverlayWindow : public CDialogEx {
		CString infoData;

	public:
		BufferedPaintComponent bufferedPaintComponent;
		GdiPlusComponent gdiPlusComponent;


		//static const shared_ptr<OverlayWindow>& GetInstance();

		void SetInfoData(const CString& newInfoData);



	public:

		// Dialog Data
#ifdef AFX_DESIGN_TIME
		enum { IDD = IDD_OVERLAY_WINDOW };
#endif

	protected:
		void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
		BOOL OnInitDialog() override;

		BOOL PreCreateWindow(CREATESTRUCT& cs) override;
		void PaintToBackbuffer(HDC dc);


		afx_msg void OnPaint();
		afx_msg BOOL OnEraseBkgnd(CDC*);
		

		DECLARE_MESSAGE_MAP()


	};

} // namespace TSPlugin 

