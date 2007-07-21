// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

class CMainFrame : 
	public CFrameWindowImpl<CMainFrame>, 
	public CUpdateUI<CMainFrame>,
	public CMessageFilter, 
	public CIdleHandler,
	public CAppWindow<CMainFrame>
{
public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if (pMsg->message == WM_KEYDOWN )
		{
			UINT nRepCnt = pMsg->lParam & 0xFFFF;
			UINT nFlags = ((pMsg->lParam & 0xFFFF0000) >> 16);
			switch(pMsg->wParam)
			{
			case VK_RETURN :
				if(m_view.m_controller->OnReturnKey(nRepCnt, nFlags))
					return TRUE;
				break;
			case VK_UP :
				if(m_view.m_controller->OnUpKey(nRepCnt, nFlags))
					return TRUE;
				break;
			case VK_DOWN :
				if(m_view.m_controller->OnDownKey(nRepCnt, nFlags))
					return TRUE;
				break;
			case VK_LEFT :
				if(m_view.m_controller->OnLeftKey(nRepCnt, nFlags))
					return TRUE;
				break;
			case VK_RIGHT :
				if(m_view.m_controller->OnRightKey(nRepCnt, nFlags))
					return TRUE;
				break;
			}			
		}   

		if(CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
			return TRUE;

		return m_view.PreTranslateMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		return FALSE;
	}

	BEGIN_UPDATE_UI_MAP(CMainFrame)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainFrame)
		MSG_WM_CREATE	( OnCreate )
		// MSG_WM_DESTROY	( OnDestroy )
		// MSG_WM_KEYDOWN	( OnKeyDown )
		MSG_WM_COPYDATA	( m_view.m_controller->OnCopyData )
		// MSG_WM_ACTIVATE ( m_view.m_controller->OnActivate )
		COMMAND_ID_HANDLER(ID_ACTION,		m_view.m_controller->OnAction)
		COMMAND_ID_HANDLER(ID_APP_EXIT,		OnFileExit)
		COMMAND_ID_HANDLER(ID_APP_ABOUT,	OnAppAbout)
		COMMAND_ID_HANDLER(ID_BACK_LAST_APP,m_view.m_controller->OnBackLastApplication)
		COMMAND_ID_HANDLER(ID_SAVE_CURRENT_CONTENT,
											m_view.m_controller->OnSaveContentHtml)
		CHAIN_MSG_MAP(CAppWindow<CMainFrame>)
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	int OnCreate(LPCREATESTRUCT lpCreateStruct)
	{
		// initial controller
		m_view.setController(m_controllerFactory.create("Cald2"));
		m_view.m_controller->Init(this);

		CreateSimpleCEMenuBar();

		m_hWndClient = m_view.Create(m_hWnd);
		m_wndPick.MyCreate();

		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		m_mainMenu = (HMENU)SendMessage(m_hWndCECommandBar, SHCMBM_GETSUBMENU, (WPARAM)0, (LPARAM)ID_MENU);

		return 0;
	}



	LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		PostMessage(WM_CLOSE);
		return 0;
	}



	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CAboutDlg dlg;
		dlg.DoModal();
		return 0;
	}

/*
	void OnDestroy()
	{
		m_view.m_controller->Fini();
		SetMsgHandled(TRUE);

		// first remove all pages for avoiding exception.
		// m_wndContentTabView.RemoveAllPages();

		// ::PostQuitMessage(0);
	}


	LRESULT OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		UINT nState = (UINT)LOWORD(wParam);
		BOOL bMinimized = (BOOL)HIWORD(wParam);
		HWND wndOther = (HWND)lParam;

		switch(nState)
		{
		case WA_ACTIVE :
		case WA_CLICKACTIVE :
			installAppKey();
			break;
		case WA_INACTIVE :
			uninstallAppKey();
			break;
		}

		return CAppWindow<CMainFrame>::OnActivate(uMsg, wParam, lParam, bHandled);
	}



	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		switch(nChar)
		{
		case VK_APP1 :
			backLaskActiveWindow();
			break;
		case VK_APP2 :
			break;
		case VK_APP3 :
			break;
		case VK_APP4 :
			break;
		}
		int i = 0;
	}

*/


/*
	void installAppKey()
	{
		SHSetAppKeyWndAssoc(VK_APP1, *this);
		SHSetAppKeyWndAssoc(VK_APP2, *this);
		SHSetAppKeyWndAssoc(VK_APP3, *this);
		SHSetAppKeyWndAssoc(VK_APP4, *this);
	}



	void uninstallAppKey()
	{
		SHSetAppKeyWndAssoc(VK_APP1, NULL);
		SHSetAppKeyWndAssoc(VK_APP2, NULL);
		SHSetAppKeyWndAssoc(VK_APP3, NULL);
		SHSetAppKeyWndAssoc(VK_APP4, NULL);
	}

*/

	HMENU					m_mainMenu;

	CSKMobileView			m_view;

	CPickButton				m_wndPick;

	ControllerFactory		m_controllerFactory;

};
