#pragma once
#include <afxwin.h>


namespace TSPlugin {
	//class OverlayWindow : public CDialogEx {
	class OverlayWindow : public CWnd {
		CStringA InfoData;

	private:

		

		DECLARE_DYNAMIC(OverlayWindow)

	public:

		typedef CWnd ParentClass;
		

		//OverlayWindow(CWnd* pParent = nullptr);   // standard constructor
		

		static const shared_ptr<OverlayWindow>& GetInstance();

		void SetInfoData(const CStringA& infoData);



	public:

		// Dialog Data
#ifdef AFX_DESIGN_TIME
		enum { IDD = IDD_OVERLAY_WINDOW };
#endif

	protected:
		void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
		//BOOL OnInitDialog() override;

		BOOL PreCreateWindow(CREATESTRUCT& cs) override;



		afx_msg void OnPaint();
		afx_msg BOOL OnEraseBkgnd(CDC*);
		

		DECLARE_MESSAGE_MAP()


	};

	//class OverlayWindow : public CDialogEx {
	//	
	//	CStringA InfoData;


	//public:
	//	static const shared_ptr<OverlayWindow>& GetInstance();

	//public:

	//	void SetInfoData(const CStringA& infoData);

	//protected:
	//	
	//	//void OnDraw(CDC* pDC) override;
	//};

} // namespace TSPlugin 

