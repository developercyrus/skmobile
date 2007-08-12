// SKMobileView.h : interface of the CSKMobileView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

class CSKMobileView : 
	public CDialogImpl<CSKMobileView>,
	public CDialogResize<CSKMobileView>,
	public CWinDataExchange<CSKMobileView>
{
public:

	enum { IDD = IDD_SKMOBILE_FORM };

	typedef  CDialogImpl<CSKMobileView> BaseDialog;


	CSKMobileView()
	{}



	BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CWindow::IsDialogMessage(pMsg);
	}



	BEGIN_MSG_MAP(CSKMobileView)

		MESSAGE_HANDLER(WM_INITDIALOG,	OnInitDialog)
		// MESSAGE_HANDLER(WM_DESTROY,		OnDestroy)

		COMMAND_HANDLER(IDC_INPUT, EN_CHANGE,		m_controller->OnInputChanged)
		MSG_WM_TIMER(								m_controller->OnTimer)
		COMMAND_HANDLER(IDC_INPUT, EN_SETFOCUS,		m_controller->OnInputSetFocus)
		COMMAND_HANDLER(IDC_INPUT, EN_KILLFOCUS,	m_controller->OnInputKillFocus)

		NOTIFY_CODE_HANDLER( TBVN_PAGEACTIVATED,	m_controller->OnContentTabChanged)
		NOTIFY_CODE_HANDLER( NM_DOCUMENTCOMPLETE,	m_controller->OnDocumentComplete)
		NOTIFY_CODE_HANDLER( NM_HOTSPOT,			m_controller->OnHotspot)

		COMMAND_HANDLER(IDC_CLOSE_TAB,	BN_CLICKED,	m_controller->OnCloseTab)
		COMMAND_HANDLER(IDC_TYPES,		BN_CLICKED,	m_controller->OnQueryMode)

		COMMAND_HANDLER(IDC_LOOKUP,		BN_CLICKED,	m_controller->OnSimulateKey)
		COMMAND_HANDLER(IDC_UP,			BN_CLICKED,	m_controller->OnSimulateKey)
		COMMAND_HANDLER(IDC_DOWN,		BN_CLICKED,	m_controller->OnSimulateKey)
		COMMAND_HANDLER(IDC_LEFT,		BN_CLICKED,	m_controller->OnSimulateKey)
		COMMAND_HANDLER(IDC_RIGHT,		BN_CLICKED,	m_controller->OnSimulateKey)

		// CHAIN_MSG_MAP(BaseDialog)
		CHAIN_MSG_MAP(CDialogResize<CSKMobileView>)
		// DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()



	BEGIN_DLGRESIZE_MAP(CSKMobileView)
		DLGRESIZE_CONTROL(IDC_INPUT,	DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_LOOKUP,	DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_UP,		DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_DOWN,		DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_LEFT,		DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_RIGHT,	DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_TYPES,	DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CLOSE_TAB,DLSZ_MOVE_X)
		DLGRESIZE_CONTROL(IDC_CONTENT,	DLSZ_SIZE_X | DLSZ_SIZE_Y)
	END_DLGRESIZE_MAP()



	BEGIN_DDX_MAP(CSKMobileView)
		DDX_CONTROL_HANDLE(IDC_CONTENT,		m_wndContentPlaceHolder)
		DDX_CONTROL_HANDLE(IDC_INPUT,		m_wndInput)
		DDX_CONTROL_HANDLE(IDC_LOOKUP,		m_wndLookupButton)
		DDX_CONTROL_HANDLE(IDC_TYPES,		m_wndQueryModeButton)
		DDX_CONTROL_HANDLE(IDC_CLOSE_TAB,	m_wndCloseTabButton)
	END_DDX_MAP()



	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		// First DDX call, hooks up variables to controls.
		DoDataExchange(false);

		DlgResize_Init(FALSE);
		bHandled = FALSE;

		CRect resultRect;
		m_wndContentPlaceHolder.GetWindowRect(&resultRect);
		this->ScreenToClient(&resultRect); 
		m_wndContentPlaceHolder.DestroyWindow();
		m_wndContentTabView.Create(*this, resultRect, NULL, (WS_CHILD | WS_VISIBLE ), 0, IDC_CONTENT);
		// because we add same hwnd for each tab, so don't destroy it when remove.
		m_wndContentTabView.m_bDestroyPageOnRemove = false;

		m_wndHtmlViewer.Create(m_wndContentTabView, NULL, NULL, (WS_CHILD | WS_VISIBLE ));
		m_wndHtmlViewer.EnableClearType(TRUE);
		m_wndHtmlViewer.EnableScripting(TRUE);
		m_wndHtmlViewer.EnableContextMenu(FALSE);
		m_wndHtmlViewer.EnableShrink(TRUE);

		m_queryModeMenu.CreatePopupMenu();

		this->m_controller->Init(this);

		return TRUE;
	}



	void setController(Controller * controller)
	{
		this->m_controller.Attach(controller);
	}

/*
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_controller->Fini();
		// first remove all pages for avoiding exception.
		// m_wndContentTabView.RemoveAllPages();

		// ::PostQuitMessage(0);
		bHandled = FALSE;
		return S_OK;
	}
*/


	CStatic						m_wndContentPlaceHolder;

	CBottomTabView				m_wndContentTabView;

	CHtmlCtrl					m_wndHtmlViewer;

	CEdit						m_wndInput;

	CButton						m_wndLookupButton;

	CButton						m_wndQueryModeButton;

	CButton						m_wndCloseTabButton;

	CFont						m_font;

	CAutoPtr<Controller>		m_controller;

	CMenu						m_queryModeMenu;

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)	
};
