
#pragma once

class CPickButton : public CWindowImpl< CPickButton >
{
public :

	typedef CWindowImpl< CPickButton > base;

	const static int WIDTH	= 16;
	const static int HEIGHT = 16;

	const static int ID_TIMER		= 1000;
	const static int TIMER_INTERVAL = 250;

	CImageList	m_ImageList;
	BOOL		m_BMouseDown;
	CPoint		m_cPointMouseDown;
	CPoint		m_cPointBtnPos;
	HWND		m_hMouseDownActiveWindow;
	HWND		m_hMouseDownActiveWindowRecent;

	CPickButton()
	{
		m_BMouseDown = FALSE;
		m_hMouseDownActiveWindow = NULL;
		m_hMouseDownActiveWindowRecent = NULL;
	}

	virtual ~CPickButton()
	{
		DestroyWindow();
	}

	void MyCreate()
	{
		HICON icon = AtlLoadIconImage(IDI_PICK_BUTTON, LR_DEFAULTCOLOR, 16, 16);
		BOOL rc = m_ImageList.Create(WIDTH, HEIGHT, ILC_COLOR, 1, 0);
		m_ImageList.AddIcon(icon);
		CWindow wndTaskBar = findTaskBar();
		if(NULL != wndTaskBar)
		{
			CRect taskBarRect;
			wndTaskBar.GetWindowRect(&taskBarRect);
			CRect rect(
				taskBarRect.CenterPoint().x - WIDTH / 2, 
				taskBarRect.CenterPoint().y - HEIGHT / 2, 
				taskBarRect.CenterPoint().x + WIDTH / 2, 
				taskBarRect.CenterPoint().y + HEIGHT / 2 );
			this->Create(wndTaskBar, rect, _T("PickWord1"), WS_CHILD|WS_VISIBLE);
			SetTimer(ID_TIMER, TIMER_INTERVAL);
		}
	}

	BOOL DestroyWindow() 
	{
		// TODO: Add your specialized code here and/or call the base class
		KillTimer(ID_TIMER);

		m_hMouseDownActiveWindow = NULL;
		m_hMouseDownActiveWindowRecent = NULL;
		return base::DestroyWindow();
	}

	HWND findTaskBar()
	{
		HWND taskbar = FindWindow(_T("HHTaskBar"), NULL);
		if(NULL == taskbar)
		{
			taskbar = FindWindow(_T("CeHHTaskBar"), NULL);
		}
		return taskbar;
	}

	HWND findMainFrame()
	{
		CAtlString className;
		className.LoadString(IDR_MAINFRAME);
		return FindWindow(className, NULL);
	}

	BOOL SizeToImage()
	{
		ATLASSERT(::IsWindow(m_hWnd) && m_ImageList.m_hImageList != NULL);
		int cx = 0;
		int cy = 0;
		if(!m_ImageList.GetIconSize(cx, cy))
			return FALSE;
		return ResizeClient(cx, cy);
	}

	void DoPaint(CDCHandle dc)
	{
		ATLASSERT(m_ImageList.m_hImageList != NULL);   // image list must be set

		// draw the button image
		m_ImageList.Draw(dc, 0, 0, 0, ILD_NORMAL);
	}

	void DoPickWord()
	{
		HWND mainFrame = findMainFrame();
		if(mainFrame == m_hMouseDownActiveWindow)
		{
			// MainFrame is active, we will back.
			::SetForegroundWindow(m_hMouseDownActiveWindowRecent);
			return;
		}

		if(FALSE == CopySelectionToClipboard(m_hMouseDownActiveWindow))
			return;

		// have something in the Clipboard
		CAtlString theWord;

		if(!OpenClipboard()) return ;
		
		HANDLE hText = GetClipboardData(CF_UNICODETEXT);
		LPTSTR lpText  = (LPTSTR)LocalLock(hText);
		theWord = lpText;
		LocalUnlock(hText);
		theWord.TrimLeft(L" \r\n\t");
		theWord.TrimRight(L" \r\n\t");
		
		if(theWord.GetLength() > 0)
		{
			COPYDATASTRUCT cd = { NULL, sizeof(TCHAR) * (theWord.GetLength() + 1), (LPVOID)(LPCTSTR)theWord };
			::SendMessage(mainFrame, WM_COPYDATA, (WPARAM)m_hMouseDownActiveWindow, (LPARAM)&cd);
			::EmptyClipboard();
		}		
		CloseClipboard();
	}

	BOOL CopySelectionToClipboard(HWND currentWindow)
	{
		if(::IsClipboardFormatAvailable(CF_UNICODETEXT))
		{
			// User have select something in clipboard.
			return TRUE;
		}

		// try ID_EDIT_COPY
		::SendMessage( currentWindow, WM_COMMAND, ID_EDIT_COPY, 0);
		if(::IsClipboardFormatAvailable(CF_UNICODETEXT))
			return TRUE;

		// try WM_COPY
		::SendMessage( currentWindow, WM_COPY, 0, 0);
		if(::IsClipboardFormatAvailable(CF_UNICODETEXT))
			return TRUE;

		// try magic message for PIE in mobile 5.0, whose class name is 'PIEHTML'
		::SendMessage( currentWindow, WM_COMMAND, 0x55F2, 0);
		if(::IsClipboardFormatAvailable(CF_UNICODETEXT))
			return TRUE;

		// try magic message for PIE in PocketPC 2002
		::SendMessage( currentWindow, WM_COMMAND, 0x139E, 0);
		if(::IsClipboardFormatAvailable(CF_UNICODETEXT))
			return TRUE;

		// try magic message for PIE in PocketPC 2000
		::SendMessage( currentWindow, WM_COMMAND, 0x156BA, 0);
		if(::IsClipboardFormatAvailable(CF_UNICODETEXT))
			return TRUE;

		// try magic message for Opera 8.6
		::SendMessage( currentWindow, WM_COMMAND, 0x9CD8, 0);
		if(::IsClipboardFormatAvailable(CF_UNICODETEXT))
			return TRUE;

		// enum all children
		HWND hWndFirstChild = ::GetWindow( currentWindow, GW_CHILD );
		if(NULL != hWndFirstChild)
		{
			if(CopySelectionToClipboard(hWndFirstChild))
				return TRUE;

			for(HWND hWndTemp = ::GetWindow( hWndFirstChild, GW_HWNDNEXT ); 
				NULL != hWndTemp && hWndFirstChild != hWndTemp; 
				hWndTemp = ::GetWindow( hWndTemp, GW_HWNDNEXT ))
			{
				if(CopySelectionToClipboard(hWndTemp))
					return TRUE;
			}
		}
		return FALSE;
	}

	BEGIN_MSG_MAP_EX( CPickButton )
		MSG_WM_PAINT		( OnPaint )
		MSG_WM_LBUTTONDOWN	( OnLButtonDown )
		MSG_WM_LBUTTONUP	( OnLButtonUp )
		MSG_WM_TIMER		( OnTimer )
	END_MSG_MAP()

	void OnPaint(HDC hdc)
	{
		if(hdc != NULL)
		{
			DoPaint(hdc);
		}
		else
		{
			CPaintDC dc(m_hWnd);
			DoPaint(dc.m_hDC);
		}
	}

	void OnLButtonDown(UINT wParam, CPoint point)
	{
		KillTimer(ID_TIMER);
		m_BMouseDown=TRUE;
		SetCapture();
		m_cPointMouseDown=point;
	}

	void OnLButtonUp(UINT wParam, CPoint point)
	{
		CRect rect;
		GetWindowRect(&rect);

		if(m_BMouseDown && (
			abs( point.x - m_cPointMouseDown.x ) > 4||
			abs( point.y - m_cPointMouseDown.y ) > 4))
		{
			CRect rectParent;
			GetParent().GetWindowRect(&rectParent);
			m_cPointBtnPos.x = point.x - m_cPointMouseDown.x + rect.left - rectParent.left;
			m_cPointBtnPos.y = point.y - m_cPointMouseDown.y + rect.top - rectParent.top;
			if(m_cPointBtnPos.y < 0)
				m_cPointBtnPos.y = 0;
			if(m_cPointBtnPos.y > rectParent.Height() - HEIGHT)
				m_cPointBtnPos.y = rectParent.Height() - HEIGHT;
			MoveWindow( m_cPointBtnPos.x, m_cPointBtnPos.y, WIDTH, HEIGHT);
		}

		ReleaseCapture();
		if(m_BMouseDown && point.x >= 0 && point.x <= WIDTH && point.y >= 0 && point.y <= HEIGHT)
		{
			DoPickWord();
		}
		m_BMouseDown = FALSE;
		SetTimer(ID_TIMER, TIMER_INTERVAL);
	}

	void OnTimer(UINT_PTR nIDEvent)
	{
		if(ID_TIMER == nIDEvent)
		{
			HWND current = ::GetForegroundWindow();
			if(current != m_hMouseDownActiveWindow)
			{
				m_hMouseDownActiveWindowRecent = m_hMouseDownActiveWindow;
				m_hMouseDownActiveWindow = current;
			}
		}
	}
};