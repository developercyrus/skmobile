
#pragma once

class BasicController : public Controller
{
public :

	static const unsigned int MAX_HTML_TAB = 10;

	static const unsigned int INPUT_CHANGED_TIMEOUT = 1000;



	BasicController() : m_lastInputPannelStatus(FALSE), m_isSearching(FALSE), m_inputChangedTimer(0)
	{
		this->m_lock = PR_NewLock();
	}



	virtual ~BasicController()
	{
		PR_DestroyLock(this->m_lock);
	}



	virtual void Init(CMainFrame * mainFrame)
	{
		m_mainFrame = mainFrame;
	}



	virtual void Init(CSKMobileView * view)
	{
		m_view = view;
	}



	virtual void Fini()
	{
		// do nothing
	}



	virtual BOOL OnCopyData(CWindow wnd, PCOPYDATASTRUCT pCopyDataStruct)
	{
		if(NULL != wnd)
		{
			m_lastActiveWindows = wnd;

			::EnableMenuItem(m_mainFrame->m_mainMenu, ID_BACK_LAST_APP, MF_BYCOMMAND | MF_ENABLED);

			::SetForegroundWindow(*m_mainFrame);

			DWORD size = pCopyDataStruct->cbData;
			TCHAR * data = (TCHAR *)pCopyDataStruct->lpData;
			CAtlString pickWord(data, size);
			doLookup(pickWord);
		}
		return S_OK;
	}



	virtual LRESULT OnBackLastApplication(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		::EnableMenuItem(m_mainFrame->m_mainMenu, ID_BACK_LAST_APP, MF_BYCOMMAND | MF_GRAYED);

		::SetForegroundWindow(m_lastActiveWindows);
		return S_OK;
	}



	virtual LRESULT OnSaveContentHtml(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		// get tab content
		CComVariant result;
		TCHAR const* params[] = { m_selectedHtmlTabTitle };
		size_t paramsCount = sizeof(params) / sizeof(TCHAR const*);
		m_script.Run(_T("getTabContent"), &result, paramsCount, params);

		CComBSTR contentHtml(result.bstrVal);
		CAtlString contentHtmlString = contentHtml;
		CAtlString fullHtml = formatContentHtml(contentHtmlString);
		std::string utf8Html = CW2A(fullHtml, CP_UTF8);

		FILE * savedFile = _wfopen(_T("content.html"), _T("w"));
		fwrite(utf8Html.c_str(), sizeof (char), utf8Html.size(), savedFile);
		fclose(savedFile);
		return S_OK;
	}



	virtual LRESULT OnSaveContentXml(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		return S_OK;
	}



	virtual LRESULT OnInputChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		// wait the user stop to press key
		m_inputChangedTimer = m_view->SetTimer(ID_TIMER_INPUTCHANGED, INPUT_CHANGED_TIMEOUT);
		return S_OK;
	}



	virtual void OnTimer(UINT_PTR nIDEvent)
	{
		if(nIDEvent == m_inputChangedTimer)
		{
			m_view->KillTimer(ID_TIMER_INPUTCHANGED);
			CAtlString queryString;
			m_view->m_wndInput.GetWindowText(queryString);
			queryString.Trim();
			if(!queryString.IsEmpty() && queryString.GetLength() >= 2)
			{
				updateWordList(queryString);
				// the focus will be lost when the word list was updated
				m_view->m_wndInput.SetFocus();
				m_view->m_wndInput.SetSel(queryString.GetLength(), -1);
			}
		}
	}



	virtual LRESULT OnInputSetFocus(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		// save the old status of the input panel, we will restore it when the focus is killed.
		SIPINFO sipInfo = {0};
		sipInfo.cbSize = sizeof SIPINFO;
		::SipGetInfo(&sipInfo);
		m_lastInputPannelStatus = 0 != (sipInfo.fdwFlags & SIPF_ON);

		::SipShowIM(SIPF_ON);
		return S_OK;
	}



	virtual LRESULT OnInputKillFocus(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		::SipShowIM(m_lastInputPannelStatus ? SIPF_ON : SIPF_OFF);
		return S_OK;
	}



	virtual LRESULT OnContentTabChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		int tabId = pnmh->idFrom;
		if(-1 != tabId)
		{
			ATL::CString tabTitle = m_view->m_wndContentTabView.GetPageTitle(tabId);
			selectTab(tabTitle);
		}
		return S_OK;
	}



	virtual LRESULT OnDocumentComplete(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		BOOL isFirst = !m_script.isReady();
		CComPtr<IDispatch> pScriptEngine;
		m_view->m_wndHtmlViewer.GetScriptDispatch(&pScriptEngine);
		m_script.Set(pScriptEngine);
		
		if(isFirst)
		{
			m_view->m_wndInput.SetFocus();
		}

		return S_OK;
	}



	virtual LRESULT OnHotspot(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		if(m_isHandlingHotspot)
		{
			m_view->MessageBox(_T("Now A request has been handling, ignore the second request."), 
				_T("Cald2Mobile"), MB_OK | MB_ICONWARNING);
			return S_OK;
		}

		PR_Lock(m_lock);
		m_isHandlingHotspot = TRUE;
			
		NM_HTMLVIEWA * pnmHTMLView = (NM_HTMLVIEWA *)pnmh;
		CAtlString szHREFText = pnmHTMLView->szTarget;
		CAtlString szPostData = pnmHTMLView->szData;
		try
		{
			if(szHREFText.Left(10) == _T("skshell://"))
			{
				CAtlString command = szHREFText.Mid(10);

				CSimpleArray<CAtlString> params;
				TCHAR unescapePostDataBuf[2048] = {0};
				DWORD unescapePostDataLength = 0;
				int curPos= 0;
				CAtlString key  = szPostData.Tokenize(_T("="),curPos);
				CAtlString value;
				while (-1 != curPos)
				{
					value= szPostData.Tokenize(_T("&"),curPos);
					AtlUnescapeUrl(
						value, 
						unescapePostDataBuf, 
						&unescapePostDataLength, 
						sizeof unescapePostDataBuf / sizeof TCHAR);
					value = CAtlString(unescapePostDataBuf, unescapePostDataLength);
					// the 'AtlUnescapeUrl' cann't translate '+' to 'space'
					value.Replace(_T('+'), _T(' '));

					params.Add(value);

					key  = szPostData.Tokenize(_T("="),curPos);
				};

				this->handleCommand(command, params);
				// disable default handle
				m_isHandlingHotspot = FALSE;
				return 1;
			}
		}
		catch (truntime_error& e)
		{
			skConsoleLog::Log(_T("Failed to handle OnHotspot Event. href = %s, postData = %s, cause = %s"),
				(TCHAR const*)szHREFText, (TCHAR const*)szPostData, e.errorMsg().c_str());
		}
		m_isHandlingHotspot = FALSE;
		PR_Unlock(m_lock);

		return S_OK;
	}



	virtual LRESULT OnCloseTab(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		int uiIndex = m_view->m_wndContentTabView.GetActivePage();
		if(uiIndex >= 0)
		{
			CAtlString tabName = m_view->m_wndContentTabView.GetPageTitle(uiIndex);
			removeTab(tabName);
		}
		return S_OK;
	}



	virtual LRESULT OnQueryMode(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CRect buttonRect;
		m_view->m_wndQueryModeButton.GetWindowRect(&buttonRect);
		CPoint showPos = buttonRect.BottomRight();
		UINT menuId = m_view->m_queryModeMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, showPos.x, showPos.y, *m_view);
		if(0 != menuId) 
		{
			UINT mask = (0x01 << (menuId - ID_MENU_QUERYMODE));
			if(0 != (m_queryMode & mask))
			{
				m_view->m_queryModeMenu.CheckMenuItem(menuId, MF_BYCOMMAND| MF_UNCHECKED);
				m_queryMode &= ~mask;
			}
			else
			{
				m_view->m_queryModeMenu.CheckMenuItem(menuId, MF_BYCOMMAND| MF_CHECKED);
				m_queryMode |= mask;
			}
		}

		return S_OK;
	}



	virtual LRESULT OnAction(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		doAction();
		return S_OK;
	}



	virtual LRESULT OnLookup(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		doLookup();
		return S_OK;
	}



	virtual BOOL OnReturnKey(UINT nRepCnt, UINT nFlags)
	{
		this->doLookup();
		return FALSE;
	}



	virtual BOOL OnUpKey(UINT nRepCnt, UINT nFlags)
	{
		return FALSE;
	}



	virtual BOOL OnDownKey(UINT nRepCnt, UINT nFlags)
	{
		return FALSE;
	}



	virtual BOOL OnLeftKey(UINT nRepCnt, UINT nFlags)
	{
		return FALSE;
	}



	virtual BOOL OnRightKey(UINT nRepCnt, UINT nFlags)
	{
		return FALSE;
	}



protected :

	virtual void handleLookup(CAtlString const& queryString, UINT queryModes) = 0;

	virtual void handleCommand(CAtlString const& command, CSimpleArray<CAtlString> const& params) = 0;

	virtual void updateWordList(CAtlString const& queryString) = 0;

	virtual CAtlString formatContentHtml(CAtlString const& contentInnerHTML) = 0;



	void doAction()
	{
		HWND focusHwnd = GetFocus();
		if(m_view->m_wndInput.IsChild( focusHwnd )  || m_view->m_wndInput == focusHwnd)
		{
			this->doLookup();
		}
		else if(m_view->m_wndHtmlViewer.IsSelection()) 
		{
			CAtlString selection = this->getHtmlSelection();
			doLookup(selection);
		}
		else
		{
			m_view->m_wndInput.SetFocus();
		}
	}



	void doLookup()
	{
		if(m_isSearching)
		{
			CAtlString msg;
			msg.Format(_T("Now '%s' has been querying, ignore the second request."), m_queryString);
			m_view->MessageBox(msg, _T("Cald2Mobile"), MB_OK | MB_ICONWARNING);
			return;
		}
		
		PR_Lock(m_lock);
		m_isSearching = TRUE;

		m_view->m_wndInput.GetWindowText(m_queryString);
		m_queryString.Trim();
		if(m_queryString.IsEmpty())
		{
			m_isSearching = FALSE;
			return;
		}

		// stop update word list
		m_view->KillTimer(ID_TIMER_INPUTCHANGED);

		// Just only one query can be do.
		handleLookup(m_queryString, m_queryMode);
		m_isSearching = FALSE;
		PR_Unlock(m_lock);

		return;
	}



	void doLookup(CAtlString word)
	{
		word.Trim();
		if(word.IsEmpty())
			return;

		m_view->m_wndInput.SetWindowText(word);
		doLookup();
	}



	BOOL addQueryMode(CAtlString const& queryMode)
	{
		UINT index = m_queryModes.GetSize();
		m_queryModes.Add(queryMode);
		UINT id = ID_MENU_QUERYMODE + index;
		m_view->m_queryModeMenu.AppendMenu(MF_STRING, (UINT_PTR)id, queryMode);
		if(0 == index)
		{
			m_queryMode = 1;
			m_view->m_queryModeMenu.CheckMenuItem(index, MF_BYPOSITION | MF_CHECKED);

		}
		return TRUE;
	}



	BOOL setQueryMode(UINT queryMode)
	{
		UINT index = m_queryModes.GetSize();
		for(UINT i = 0; i < index; i++)
		{
			UINT mask = (0x01 << i);
			if(0 != (queryMode & mask))
			{
				m_view->m_queryModeMenu.CheckMenuItem(i, MF_BYPOSITION| MF_CHECKED);
			}
		}
		m_queryMode = queryMode;
		return TRUE;
	}



	BOOL nextTab()
	{
		if(1 == m_view->m_wndContentTabView.GetPageCount())
			return FALSE;

		int currentUiTabIndex = m_view->m_wndContentTabView.GetActivePage();
		int nextUiTabIndex = (currentUiTabIndex + 1) % m_view->m_wndContentTabView.GetPageCount();
		CAtlString nextTabName = m_view->m_wndContentTabView.GetPageTitle(nextUiTabIndex);
		selectTab(nextTabName);

		return TRUE;
	}



	BOOL prevTab()
	{
		if(1 == m_view->m_wndContentTabView.GetPageCount())
			return FALSE;

		int currentUiTabIndex = m_view->m_wndContentTabView.GetActivePage();
		int prevUiTabIndex = 0;
		if(0 == currentUiTabIndex)
			prevUiTabIndex = m_view->m_wndContentTabView.GetPageCount() - 1;
		else
			prevUiTabIndex = (currentUiTabIndex - 1) % m_view->m_wndContentTabView.GetPageCount();
		CAtlString prevTabName = m_view->m_wndContentTabView.GetPageTitle(prevUiTabIndex);
		selectTab(prevTabName);

		return TRUE;
	}



	BOOL addTab(CAtlString const& tabName)
	{
		for(int i = 0; i < MAX_HTML_TAB; i++)
		{
			if(this->m_tabIdMapping[i].IsEmpty())
			{
				// find a valid HTML tab.
				this->m_tabIdMapping[i] = tabName;
				m_view->m_wndContentTabView.AddPage(m_view->m_wndHtmlViewer, tabName);
				return TRUE;
			}
		}
		return FALSE;
		// THROW_RUNTIME_EXCEPTION(_T("All tab is busy, please first close some tab."));
	}



	BOOL removeTab(CAtlString const& tabName)
	{
		// clear HTML content
		this->setTabHtmlContent(tabName, _T(""));

		// remove HTML tab
		int htmlTabIndex = findHtmlTabIndex(tabName);
		if(-1 != htmlTabIndex)
		{
			this->m_tabIdMapping[htmlTabIndex] = _T("");
		}

		// remove UI tab
		int uiTabIndex = findUiTabIndex(tabName);
		if(-1 != uiTabIndex)
		{
			m_view->m_wndContentTabView.RemovePage(uiTabIndex);
		}

		return TRUE;
	}



	BOOL selectTab(CAtlString const& tabTitle)
	{
		if(!m_script.isReady())
			return FALSE;

		int htmlTabIndex = findHtmlTabIndex(tabTitle);
		int uiTabIndex = findUiTabIndex(tabTitle);
		if(-1 == htmlTabIndex || -1 == uiTabIndex)
			return FALSE;

		// select UI tab
		m_view->m_wndContentTabView.SetActivePage(uiTabIndex);

		// select HTML tab
		CAtlString htmlTabId;
		htmlTabId.Format(_T("%d"), htmlTabIndex);
		CComVariant result;
		TCHAR const* params[] = { htmlTabId };
		size_t paramsCount = sizeof(params) / sizeof(TCHAR const*);
		m_script.Run(_T("selectTab"), &result, paramsCount, params);

		m_selectedHtmlTabTitle = htmlTabId;

		return TRUE;
	}



	BOOL setTabTextContent(CAtlString const& tabName, CAtlString const& textContent)
	{
		// convert it to HTML
		CAtlString htmlContent;
		htmlContent.Format(_T("<pre>%s</pre>"), textContent);

		return setTabHtmlContent(tabName, htmlContent);
	}



	BOOL setTabHtmlContent(CAtlString const& tabName, CAtlString const& htmlContent)
	{
		if(!m_script.isReady())
			return FALSE;

		int htmlTabIndex = findHtmlTabIndex(tabName);
		int uiTabIndex = findUiTabIndex(tabName);
		if(-1 == htmlTabIndex && -1 == uiTabIndex)
		{
			if( addTab(tabName) )
			{
				htmlTabIndex = findHtmlTabIndex(tabName);
				uiTabIndex = findUiTabIndex(tabName);
			}
			else
				return FALSE;
		} 
		if(-1 == htmlTabIndex || -1 == uiTabIndex)
			return FALSE;

		// set content
		CAtlString htmlTabId;
		htmlTabId.Format(_T("%d"), htmlTabIndex);
		CComVariant result;
		TCHAR const* params[] = { htmlTabId, htmlContent };
		size_t paramsCount = sizeof(params) / sizeof(TCHAR const*);
		m_script.Run(_T("setContentHtml"), &result, paramsCount, params);

		// select tab
		m_selectedHtmlTabTitle = htmlTabId;
		m_view->m_wndContentTabView.SetActivePage(uiTabIndex);
		return TRUE;
	}



	BOOL setStyleSheet(CAtlString const& xslContent)
	{
		if(!m_script.isReady())
			return FALSE;

		CComVariant result;
		TCHAR const* params[] = { xslContent };
		size_t paramsCount = sizeof(params) / sizeof(TCHAR const*);
		m_script.Run(_T("setContentXsl"), &result, paramsCount, params);

		return TRUE;
	}



	BOOL setTabXmlContent(CAtlString const& tabName, CAtlString const& xmlContent)
	{
		if(!m_script.isReady())
			return FALSE;

		int htmlTabIndex = findHtmlTabIndex(tabName);
		int uiTabIndex = findUiTabIndex(tabName);
		if(-1 == htmlTabIndex && -1 == uiTabIndex)
		{
			if( addTab(tabName) )
			{
				htmlTabIndex = findHtmlTabIndex(tabName);
				uiTabIndex = findUiTabIndex(tabName);
			}
			else
				return FALSE;
		} 
		if(-1 == htmlTabIndex || -1 == uiTabIndex)
			return FALSE;

		// set content
		CAtlString htmlTabId;
		htmlTabId.Format(_T("%d"), htmlTabIndex);
		CComVariant result;
		TCHAR const* params[] = { htmlTabId, xmlContent };
		size_t paramsCount = sizeof(params) / sizeof(TCHAR const*);
		m_script.Run(_T("setContentXml"), &result, paramsCount, params);

		// select tab
		m_selectedHtmlTabTitle = htmlTabId;
		m_view->m_wndContentTabView.SetActivePage(uiTabIndex);
		return TRUE;
	}



	BOOL setTabXmlContent(CAtlString const& tabName,CAtlString const& xmlContent, CAtlString const& xslContent)
	{
		if(!m_script.isReady())
			return FALSE;

		int htmlTabIndex = findHtmlTabIndex(tabName);
		int uiTabIndex = findUiTabIndex(tabName);
		if(-1 == htmlTabIndex && -1 == uiTabIndex)
		{
			if( addTab(tabName) )
			{
				htmlTabIndex = findHtmlTabIndex(tabName);
				uiTabIndex = findUiTabIndex(tabName);
			}
			else
				return FALSE;
		} 
		if(-1 == htmlTabIndex || -1 == uiTabIndex)
			return FALSE;

		// set content
		CAtlString htmlTabId;
		htmlTabId.Format(_T("%d"), htmlTabIndex);
		CComVariant result;
		TCHAR const* params[] = { htmlTabId, xmlContent, xslContent };
		size_t paramsCount = sizeof(params) / sizeof(TCHAR const*);
		m_script.Run(_T("setContentXmlWithXsl"), &result, paramsCount, params);

		// select tab
		m_selectedHtmlTabTitle = htmlTabId;
		m_view->m_wndContentTabView.SetActivePage(uiTabIndex);
		return TRUE;
	}



	BOOL playSound(CAtlString const& soundPath)
	{
		if(!m_script.isReady())
			return FALSE;

		// set soundPath to WMP in the content HTML
		CComVariant result;
		TCHAR const* params[] = { soundPath };
		size_t paramsCount = sizeof(params) / sizeof(TCHAR const*);
		m_script.Run(_T("playSound"), &result, paramsCount, params);

		return TRUE;
	}



	BOOL selectAnchor(CAtlString const& anchor)
	{
		if(!m_script.isReady())
			return FALSE;

		CComVariant result;
		TCHAR const* params[] = { anchor };
		size_t paramsCount = sizeof(params) / sizeof(TCHAR const*);
		m_script.Run(_T("selectAnchor"), &result, paramsCount, params);

		return TRUE;
	}



	BOOL htmlPageUp()
	{
		m_view->m_wndHtmlViewer.SendMessageToDescendants(WM_VSCROLL, SB_PAGEUP, NULL);
		return TRUE;
	}



	BOOL htmlPageDown()
	{
		m_view->m_wndHtmlViewer.SendMessageToDescendants(WM_VSCROLL, SB_PAGEDOWN, NULL);
		return TRUE;
	}



	BOOL htmlAnchor(CAtlString const& anchor)
	{
		m_view->m_wndHtmlViewer.Anchor(anchor);
		return TRUE;
	}



	int findUiTabIndex(CAtlString const& title)
	{
		for(int i = 0 ; i < m_view->m_wndContentTabView.GetPageCount(); i++)
		{
			if(title == m_view->m_wndContentTabView.GetPageTitle(i))
			{
				return i;
			}
		}
		return -1;
	}



	int findHtmlTabIndex(CAtlString const& title)
	{
		for(int i = 0; i < MAX_HTML_TAB; i++)
		{
			if(this->m_tabIdMapping[i] == title)
			{
				// find the HTML tab.
				return i;
			}
		}
		return -1;
	}



	CAtlString getHtmlSelection()
	{
		CAtlString strClipboardText;
		LPSTREAM  stream = 0; // give us the output stream here
		DWORD rsd = 0; // required, can be checked with SUCCEEDED?...
		::SendMessage (m_view->m_wndHtmlViewer, DTM_COPYSELECTIONTONEWISTREAM, (WPARAM)&rsd, (LPARAM) &stream);
		if (stream) // got?
		{
			STATSTG stat = { 0 };
			stream->Stat (&stat, 0); // probably check for the S_OK code...

			if (LPBYTE  buf = (LPBYTE) LocalAlloc (LHND, (SIZE_T)stat.cbSize.QuadPart + 2))
			{
				ULONG ulNumChars;

				if (SUCCEEDED (stream->Read (buf, (ULONG)stat.cbSize.QuadPart,
					&ulNumChars)) && ulNumChars == stat.cbSize.QuadPart) // read whole
					strClipboardText = (LPCWSTR) buf; // our text here!

				GlobalFree (buf);
			}
			stream->Release ();
		} 
		return strClipboardText;
	}



	CMainFrame *		m_mainFrame;

	CSKMobileView *		m_view;

	HWND				m_lastActiveWindows;

	CScript				m_script;

	BOOL				m_lastInputPannelStatus;

	CAtlString			m_selectedHtmlTabTitle;

	CAtlString			m_tabIdMapping[MAX_HTML_TAB];

	CAtlString			m_queryString;

	UINT				m_queryMode;

	BOOL				m_isSearching;

	BOOL				m_isHandlingHotspot;

	CSimpleArray<CAtlString>	m_queryModes;

	UINT_PTR			m_inputChangedTimer;

	PRLock *			m_lock;

};