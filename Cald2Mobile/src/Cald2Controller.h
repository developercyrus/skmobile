
#pragma once

namespace mysk 
{

const TCHAR TAB_NAME_WORDLIST[] = _T("WordList");

const TCHAR TAB_NAME_RESULTS[] = _T("Results");

const TCHAR TAB_NAME_CONTENT[] = _T("Content");

enum QueryMode
{
	QUERY_MODE_INDEX_HWDLIST,
	QUERY_MODE_INDEX_PHRVERB,
	QUERY_MODE_INDEX_SENSE,
	QUERY_MODE_INDEX_FULLDEFT,
	QUERY_MODE_INDEX_FULLEXAMP,
	QUERY_MODE_INDEX_SIZE
};

struct QueryModeInfo
{
	const TCHAR * name;
	UINT		queryType;
	BOOL		disableDeflect;
};

QueryModeInfo QUERY_MODE_INFO[] = 
{
	{ _T("HeadWord"),	0x07,	FALSE },
	{ _T("Phrase"),		0x08,	FALSE },
	{ _T("Sense"),		0x30,	FALSE },
	{ _T("Defines"),	0x40,	TRUE  },
	{ _T("Examples"),	0x80,	TRUE  }
};

class History
{
public :

	History() : m_pos(NULL), m_maxHistory(0)
	{}

	void Add(CAtlString const& urlPath)
	{
		// delete from current pos to tail, then add a new tail
		if(!m_history.IsEmpty())
		{
			while(m_history.GetTailPosition() != m_pos)
			{
				m_history.RemoveTail();
			}
		}
		m_pos = m_history.AddTail(urlPath);
		while(m_maxHistory > 0 && m_history.GetCount() > m_maxHistory)
			m_history.RemoveHead();
	}

	CAtlString GetCurrent()
	{
		if(m_history.IsEmpty())
			return  _T("");
		return m_history.GetAt(m_pos);
	}

	CAtlString GetNext()
	{
		CAtlString const& value = m_history.GetNext(m_pos);
		if(NULL == m_pos)
		{
			// reach tail
			m_pos = m_history.GetTailPosition();
			return _T("");
		}		
		return m_history.GetAt(m_pos);
	}

	CAtlString GetPrev()
	{
		CAtlString const& value = m_history.GetPrev(m_pos);
		if(NULL == m_pos)
		{
			// reach head
			m_pos = m_history.GetHeadPosition();
			return _T("");
		}		
		return m_history.GetAt(m_pos);
	}

	void SetMaxHistory(UINT maxHistory)
	{
		m_maxHistory = maxHistory;
	}

protected :

	CAtlList<CAtlString>	m_history;

	POSITION				m_pos;

	UINT					m_maxHistory;
};



class Cald2Controller : public BasicController, public DictionaryService
{
public :

	typedef BasicController Base;



	Cald2Controller() : m_isFrist(TRUE), m_currentWordListBegin(0), m_wordListCount(0), m_currentResultsBegin(0)
	{				
	}



	virtual void Init(CMainFrame * mainFrame)
	{
		SK_TRACE(SK_LOG_DEBUG, "Cald2Controller::Init(CMainFrame * mainFrame)");

		Base::Init(mainFrame);

		m_wordListPageSize = mainFrame->m_setting.LoadIntValue("MYSK_WORDLIST_PAGE_SIZE", 15);
		m_resultsPageSize = mainFrame->m_setting.LoadIntValue("MYSK_RESULTS_PAGE_SIZE", 20);
		m_queryMode = mainFrame->m_setting.LoadIntValue("MYSK_QUERY_MODE", 0x07);
		m_zoomLevel = mainFrame->m_setting.LoadIntValue("MYSK_ZOOM_LEVEL", 2);

		DictionaryService::init(mainFrame->m_setting.GetRootPathCString());
	}



	virtual void Init(CSKMobileView * view) 
	{
		SK_TRACE(SK_LOG_DEBUG, "Cald2Controller::Init(CSKMobileView * view)");

		Base::Init(view);

		m_view->m_wndHtmlViewer.ZoomLevel(m_zoomLevel);
		
		CAtlString contentHtmlPath;
		int width = ::GetDeviceCaps(m_view->GetDC(), HORZRES) ;
		int height = ::GetDeviceCaps(m_view->GetDC(), VERTRES) ;
		int dpi = ::GetDeviceCaps(m_view->GetDC(), LOGPIXELSX) ;
		this->m_isVGA = (max(width, height) >= 640 && dpi == 96);
		SK_TRACE(SK_LOG_DEBUG, _T("width = %d, height = %d, dpi = %d, m_isVGA = %d"), 
			width, height, dpi, this->m_isVGA);

		if(this->m_isVGA)
		{
			contentHtmlPath.Format(_T("file://%s/cald2/cald2_VGA.html"), _RootPath);
		}
		else
		{
			contentHtmlPath.Format(_T("file://%s/cald2/cald2_QVGA.html"), _RootPath);
		}
		
		SK_TRACE(SK_LOG_DEBUG, _T("Navigate to %s"), contentHtmlPath);
		view->m_wndHtmlViewer.Navigate(contentHtmlPath);
	}



	virtual void Fini()
	{
		SK_TRACE(SK_LOG_DEBUG, "Cald2Controller::Fini");
	}



	virtual LRESULT OnSaveContentXml(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		SK_TRACE(SK_LOG_DEBUG, "Cald2Controller::OnSaveContentXml");

		CAtlString currentUrlPath = m_contentHistory.GetCurrent();
		SK_TRACE(SK_LOG_DEBUG, _T("currentUrlPath = %s"), currentUrlPath);

		if(currentUrlPath.IsEmpty())
		{
			CAtlString msg;
			msg.Format(_T("The content tab is empty!!!"), m_queryString);
			m_view->MessageBox(msg, _T("Cald2Mobile"), MB_OK | MB_ICONWARNING);
			return S_OK;
		}
		ContentData result;
		BOOL rc = loadContentData(currentUrlPath, result);
		if(rc)
		{
			SK_TRACE(SK_LOG_DEBUG, _T("content = %s"), result.content);

			std::string utf8 = CW2A(result.content, CP_UTF8);
			FILE * savedFile = _wfopen(_T("content.xml"), _T("w"));
			fwrite(utf8.c_str(), sizeof (char), utf8.size(), savedFile);
			fclose(savedFile);			
		}
		return S_OK;
	}



	virtual LRESULT OnDocumentComplete(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
	{
		SK_TRACE(SK_LOG_DEBUG, "Cald2Controller::OnDocumentComplete");

		Base::OnDocumentComplete(idCtrl, pnmh, bHandled);

		if(m_isFrist)
		{
			m_isFrist = FALSE;
			SK_TRACE(SK_LOG_DEBUG, "m_isFrist = FALSE");

			try
			{
				// first
				DictionaryService::wordListService = new SKWordListService("native:data/wordlist.skn");
				DictionaryService::queryService = new SKIndexQueryService(
					"data/index/complete.skn/index.cfi", 
					"native:data/complete.skn",
					"data/index/flex2stem.skn/index.cfi", 
					"native:data/index/flex2stem.skn");
				DictionaryService::fileSystemService = new FileSystemService;
				DictionaryService::fileSystemService->prepare("data/entry/filesystem.cff");
				DictionaryService::fileSystemService->prepare("data/extraexample/filesystem.cff");
				DictionaryService::fileSystemService->prepare("data/htmlpanel/filesystem.cff");
				DictionaryService::fileSystemService->prepare("data/pronus/filesystem.cff");
				DictionaryService::fileSystemService->prepare("data/pronuk/filesystem.cff");

				// add 1 tab
				Base::addTab(TAB_NAME_WORDLIST);

				// Add query modes
				for(UINT i = 0; i < QUERY_MODE_INDEX_SIZE; i++)
				{
					QueryModeInfo & info = QUERY_MODE_INFO[i];
					Base::addQueryMode(info.name);
				}
				Base::setQueryMode(m_queryMode);
				
				// initial XSL
				CAtlString xslPath;
				xslPath.Format(_T("%s/cald2/cald2.xsl"), _RootPath);
				SK_TRACE(SK_LOG_DEBUG, _T("xslPath = %s"), xslPath);

				CAtlString xslContent = loadFile(xslPath);
				Base::setStyleSheet(xslContent);

				// load word list
				m_wordListCount = DictionaryService::wordListService->getWordListCount();
				SK_TRACE(SK_LOG_DEBUG, _T("m_wordListCount = %d"), m_wordListCount);
				setWordListPage(0);
			}
			catch (truntime_error& e)
			{
				SK_TRACE(SK_LOG_INFO, _T("Failed to inital Cald2DictionaryController. error=%s"), 
					e.errorMsg().c_str());
			}
		}

		return S_OK;
	}



	virtual BOOL OnReturnKey(UINT nRepCnt, UINT nFlags)
	{
		SK_TRACE(SK_LOG_DEBUG, "Cald2Controller::OnReturnKey");

		if(m_view->m_wndContentTabView.IsChild(::GetFocus()))
		{
			int activeTab = m_view->m_wndContentTabView.GetActivePage();
			CAtlString title = m_view->m_wndContentTabView.GetPageTitle(activeTab);
			if(title == TAB_NAME_WORDLIST || title == TAB_NAME_RESULTS)
			{
				// maybe click a link
				SK_TRACE(SK_LOG_DEBUG, "maybe click a link");
				return FALSE;
			}
		}

		this->doAction();
		return TRUE;
	}



	virtual BOOL OnUpKey(UINT nRepCnt, UINT nFlags)
	{
		SK_TRACE(SK_LOG_DEBUG, "Cald2Controller::OnUpKey");

		HWND focusHwnd = ::GetFocus();
		if(m_view->m_wndInput.IsChild(focusHwnd) || m_view->m_wndInput == focusHwnd)
		{
			SK_TRACE(SK_LOG_DEBUG, "In input edit box.");

			// skip to m_wndContentTabView
			m_view->m_wndContentTabView.SetFocus();
			return TRUE;
		}
		else if(m_view->m_wndContentTabView.IsChild(focusHwnd))
		{
			int activeTab = m_view->m_wndContentTabView.GetActivePage();
			CAtlString title = m_view->m_wndContentTabView.GetPageTitle(activeTab);
			if(title == TAB_NAME_CONTENT)
			{
				SK_TRACE(SK_LOG_DEBUG, "In content tab.");
				Base::htmlPageUp();
				return TRUE;
			}
		}
		return FALSE;
	}



	virtual BOOL OnDownKey(UINT nRepCnt, UINT nFlags)
	{
		SK_TRACE(SK_LOG_DEBUG, "Cald2Controller::OnDownKey");

		HWND focusHwnd = ::GetFocus();
		if(m_view->m_wndInput.IsChild(focusHwnd) || m_view->m_wndInput == focusHwnd)
		{
			SK_TRACE(SK_LOG_DEBUG, "In input edit box.");

			// skip to m_wndContentTabView
			m_view->m_wndContentTabView.SetFocus();
			return TRUE;
		}
		else if(m_view->m_wndContentTabView.IsChild(focusHwnd))
		{
			int activeTab = m_view->m_wndContentTabView.GetActivePage();
			CAtlString title = m_view->m_wndContentTabView.GetPageTitle(activeTab);
			if(title == TAB_NAME_CONTENT)
			{
				SK_TRACE(SK_LOG_DEBUG, "In content tab.");
				Base::htmlPageDown();
				return TRUE;
			}
		}
		return FALSE;
	}



	virtual BOOL OnLeftKey(UINT nRepCnt, UINT nFlags)
	{
		SK_TRACE(SK_LOG_DEBUG, "Cald2Controller::OnLeftKey");

		HWND focusHwnd = ::GetFocus();
		if(m_view->m_wndInput.IsChild(focusHwnd) || m_view->m_wndInput == focusHwnd)
		{
			SK_TRACE(SK_LOG_DEBUG, "In input edit box.");
			// do nothing
			return FALSE;
		}
		else if(m_view->m_wndContentTabView.IsChild(focusHwnd))
		{
			int activeTab = m_view->m_wndContentTabView.GetActivePage();
			CAtlString title = m_view->m_wndContentTabView.GetPageTitle(activeTab);
			if(title == TAB_NAME_WORDLIST)
			{
				SK_TRACE(SK_LOG_DEBUG, "In wordlist tab.");

				// word list page view				
				this->setWordListPage(m_currentWordListBegin - m_wordListPageSize);
				return TRUE;
			}
			else if(title == TAB_NAME_RESULTS)
			{
				SK_TRACE(SK_LOG_DEBUG, "In results tab.");

				// result page view
				this->setResultPage(m_currentResultsBegin - m_resultsPageSize);
				return TRUE;
			}
			else if(title == TAB_NAME_CONTENT)
			{
				SK_TRACE(SK_LOG_DEBUG, "In content tab.");

				// navigate in history
				CAtlString prevUrlPath = m_contentHistory.GetPrev();
				if(!prevUrlPath.IsEmpty())
				{
					this->handleSetContentTab(prevUrlPath, FALSE);
				}
				return TRUE;
			}
			else
			{
				SK_TRACE(SK_LOG_DEBUG, "In other tab.");
				// goto prev tab
				Base::prevTab();
				return TRUE;
			}
		}
		return Base::OnLeftKey(nRepCnt, nFlags);
	}



	virtual BOOL OnRightKey(UINT nRepCnt, UINT nFlags)
	{
		SK_TRACE(SK_LOG_DEBUG, "Cald2Controller::OnRightKey");

		HWND focusHwnd = ::GetFocus();
		if(m_view->m_wndInput.IsChild(focusHwnd) || m_view->m_wndInput == focusHwnd)
		{
			SK_TRACE(SK_LOG_DEBUG, "In input edit box.");
			// do nothing
			return FALSE;
		}
		else if(m_view->m_wndContentTabView.IsChild(focusHwnd))
		{
			int activeTab = m_view->m_wndContentTabView.GetActivePage();
			CAtlString title = m_view->m_wndContentTabView.GetPageTitle(activeTab);
			if(title == TAB_NAME_WORDLIST)
			{
				SK_TRACE(SK_LOG_DEBUG, "In wordlist tab.");

				// word list page view
				this->setWordListPage(m_currentWordListBegin + m_wordListPageSize);
				return TRUE;
			}
			else if(title == TAB_NAME_RESULTS)
			{
				SK_TRACE(SK_LOG_DEBUG, "In results tab.");

				// result page view
				this->setResultPage(m_currentResultsBegin + m_resultsPageSize);
				return TRUE;
			}
			else if(title == TAB_NAME_CONTENT)
			{
				SK_TRACE(SK_LOG_DEBUG, "In content tab.");

				// navigate in history
				CAtlString nextUrlPath = m_contentHistory.GetNext();
				if(!nextUrlPath.IsEmpty())
				{
					this->handleSetContentTab(nextUrlPath, FALSE);
				}
				return TRUE;
			}
			else
			{
				SK_TRACE(SK_LOG_DEBUG, "In other tab.");

				// goto next tab
				Base::nextTab();
				return TRUE;
			}
		}
		return Base::OnRightKey(nRepCnt, nFlags);
	}



protected :

	struct ContentData
	{
		CAtlString content;
		BOOL isXml;
		CAtlString archor;
	};

	virtual void handleLookup(CAtlString const& queryString, UINT queryModes)
	{
		SK_TRACE(SK_LOG_DEBUG, _T("Cald2Controller::handleLookup(%s, %d)"), queryString, queryModes);

		try
		{
			PRUint8 types = 0;
			PRBool bUseDeflect = PR_TRUE;
			CAtlString message;
			message.AppendFormat(_T("Searching '%s' in "), queryString);
			for(int i = 0; i < QUERY_MODE_INDEX_SIZE; i++)
			{
				UINT mask = (0x01 << i);
				if(0 != (queryModes & mask))
				{
					QueryModeInfo queryModeInfo = QUERY_MODE_INFO[i];
					message.AppendFormat(_T("'%s' "), queryModeInfo.name);
					types |= queryModeInfo.queryType;
					if(queryModeInfo.disableDeflect)
						bUseDeflect = PR_FALSE;
				}
			}
			message.Append(_T( "modes..."));

			Base::setTabTextContent(TAB_NAME_RESULTS, message);

			SK_TRACE(SK_LOG_DEBUG, _T("queryString = %s, bUseFlex = true, types = %d, bUseDeflect = %d)"), 
				queryString, types, bUseDeflect);
			getQueryService()->query((TCHAR const*)queryString, true, types, bUseDeflect);

			UINT count = getQueryService()->getResultCount();
			SK_TRACE(SK_LOG_DEBUG, _T("ResultCount = %d"), count);
			if(0 < count)
			{
				setResultPage(0);
			} 
			else
			{
				CAtlString notFound;
				notFound.Format(_T("Failed to find '%s'"), m_queryString);
				Base::setTabTextContent(TAB_NAME_RESULTS, notFound);
			}
			
		}
		catch (truntime_error& e)
		{
			SK_TRACE(SK_LOG_INFO, _T("Failed to look up. queryString_=%s, error=%s"), 
				(TCHAR const*)queryString, e.errorMsg().c_str());
		}
		return;
	}



	virtual void handleCommand(CAtlString const& command, CSimpleArray<CAtlString> const& params)
	{
		SK_TRACE(SK_LOG_DEBUG, _T("Cald2Controller::handleCommand(%s)"), command);

		if(command == _T("setContentTab"))
		{
			// only one parameter 'path'
			CAtlString const& path = params[0];
			handleSetContentTab(path, TRUE);
		}
		else if(command == _T("playSound"))
		{
			// only one parameter 'path'
			CAtlString const& path = params[0];
			handlePlaySound(path);
		}
		else if(command == _T("playSoundEnded"))
		{
			// only one parameter 'path'
			CAtlString const& path = params[0];
			handlePlaySoundEnded(path);
		}
		else if(command == _T("newTab"))
		{
			// the first parameter is 'tabName', the second parameter is 'path'
			CAtlString const& tabName = params[0];
			CAtlString const& path = params[1];
			handleNewTab(tabName, path);
		}
		else if(command == _T("clickWordList"))
		{
			// only one parameter 'label'
			CAtlString const& label = params[0];
			handleClickWordList(label);
		}
	}



	virtual void updateWordList(CAtlString const& queryString)
	{
		SK_TRACE(SK_LOG_DEBUG, _T("Cald2Controller::updateWordList(%s)"), queryString);

		try
		{
			// get sort key
			CComVariant result;
			TCHAR const* params[] = { queryString };
			size_t paramsCount = sizeof(params) / sizeof(TCHAR const*);
			m_script.Run(_T("GetWordListSortKey"), &result, paramsCount, params);

			CComBSTR sortKey(result.bstrVal);
			CAtlString sortKeyString = sortKey;
			SK_TRACE(SK_LOG_DEBUG, _T("sortKeyString = %s"), sortKeyString);

			std::string utf8(CW2A(sortKeyString, CP_UTF8));
			UINT pos = getWordListService()->findWordListPosition(utf8);
			SK_TRACE(SK_LOG_DEBUG, _T("WordListPosition = %d"), pos);

			if(pos > 0 && pos < getWordListService()->getWordListCount())
			{
				UINT begin = pos - pos % m_wordListPageSize;
				setWordListPage(begin);
				CAtlString archor;
				archor.Format(_T("wordlist-%d"), pos);
				Base::htmlAnchor(archor);
				Base::selectAnchor(archor);
			}
		}
		catch (truntime_error& e)
		{
			SK_TRACE(SK_LOG_INFO, _T("Failed to update word list. queryString=%s, error=%s"), 
				queryString, e.errorMsg().c_str());
		}
	}



	virtual CAtlString formatContentHtml(CAtlString const& contentInnerHTML)
	{
		SK_TRACE(SK_LOG_DEBUG, "Cald2Controller::formatContentHtml");

		CAtlString result;
		result.Append(_T("<?xml version='1.0' encoding='utf-8'>\r\n"));
		result.Append(_T("<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Strict//EN' 'http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd'>"));
		result.Append(_T("<html xmlns='http://www.w3.org/1999/xhtml'>"));
		result.Append(_T("<head>"));
		if(this->m_isVGA)
			result.AppendFormat(_T("<link rel='stylesheet' type='text/css' href='%s/cald2/entry_VGA.css'/>"), _RootPath);
		else
			result.AppendFormat(_T("<link rel='stylesheet' type='text/css' href='%s/cald2/entry_QVGA.css'/>"), _RootPath);
		result.AppendFormat(_T("<link rel='stylesheet' type='text/css' href='%s/cald2/cle.css'/>"), _RootPath);
		result.AppendFormat(_T("<link rel='stylesheet' type='text/css' href='%s/cald2/colpanel.css'/>"), _RootPath);
		result.AppendFormat(_T("<link rel='stylesheet' type='text/css' href='%s/cald2/vforms.css'/>"), _RootPath);
		result.AppendFormat(_T("<link rel='stylesheet' type='text/css' href='%s/cald2/wfamily.css'/>"), _RootPath);
		result.Append(_T("</head>"));
		result.Append(_T("<body>"));
		result.Append(contentInnerHTML);
		result.Append(_T("</body>"));
		result.Append(_T("</html>"));

		return result;
	}



	BOOL handleSetContentTab(CAtlString const& urlPath, BOOL addToHistory)
	{
		SK_TRACE(SK_LOG_DEBUG, _T("Cald2Controller::handleSetContentTab(%s)"), urlPath);

		try
		{
			ContentData result;
			BOOL rc = loadContentData(urlPath, result);
			if(!rc)
			{
				SK_TRACE(SK_LOG_INFO, _T("Failed to load %s"), urlPath);
				return FALSE;
			}

			if(result.isXml)
				Base::setTabXmlContent(TAB_NAME_CONTENT, result.content);
			else
				Base::setTabHtmlContent(TAB_NAME_CONTENT, result.content);
			if(!result.archor.IsEmpty() && _T("0") != result.archor)
				Base::htmlAnchor(result.archor);
			if(addToHistory)
				m_contentHistory.Add(urlPath);
			return TRUE;
		}
		catch (truntime_error& e)
		{
			SK_TRACE(SK_LOG_INFO, _T("Failed to load content. urlPath = %s, cause = %s"),
				urlPath, e.errorMsg().c_str());
			return FALSE;
		}
	}



	void handlePlaySound(CAtlString const& urlPath)
	{
		SK_TRACE(SK_LOG_DEBUG, _T("Cald2Controller::handlePlaySound(%s)"), urlPath);

		try
		{
			int pos = urlPath.Find(_T(':'));
			CAtlString protocal = urlPath.Mid(0, pos);

			CAtlString soundFilePath;
			if(protocal == _T("sk"))
			{
				// skip '://fs/2.0/'
				CAtlString prefix = urlPath.Mid(pos, 10);
				if(prefix != _T("://fs/2.0/"))
				{
					// unsupported SK protocal
					SK_TRACE(SK_LOG_INFO, _T("unsupported SK protocal. urlPath = %s"), urlPath);
					return;
				}

				CAtlString fullPath = urlPath.Mid(pos + 10);

				// extract it
				char utf8Path[2048] = {0};
				int len = AtlUnicodeToUTF8(fullPath, fullPath.GetLength(), utf8Path, sizeof utf8Path);
				string utf8FullPath(utf8Path, len);
				pos = utf8FullPath.find('!');
				string filesystem = utf8FullPath.substr(0, pos);
				// skip '!/'
				string path = utf8FullPath.substr(pos + 2);

				// temp file path
				// skip /filesystem.cff
				string dir = filesystem.substr(0, filesystem.size() - 15);
				string utf8SoundFilePath;
				utf8SoundFilePath.append(m_mainFrame->m_setting.GetTempPathCString()).append("/").append(dir).append("/").append(path);
				soundFilePath = CA2W(utf8SoundFilePath.c_str(), CP_UTF8);

				SK_TRACE(SK_LOG_DEBUG, "filesystem = %s, path = %s, utf8SoundFilePath = %s", 
					filesystem.c_str(), path.c_str(), utf8SoundFilePath.c_str());
				getFileSystemService()->copyFile(filesystem, path, utf8SoundFilePath);
			} else if(protocal == _T("file"))
			{
				// skip '://'
				soundFilePath = urlPath.Mid(pos + 3);
			} else
			{
				// unsupported protocal
				SK_TRACE(SK_LOG_INFO, _T("unsupported protocal. urlPath = %s"), urlPath);
				return;
			}

			SK_TRACE(SK_LOG_DEBUG, _T("soundFilePath = %s"), soundFilePath);
			Base::playSound(soundFilePath);
		}
		catch (truntime_error& e)
		{
			SK_TRACE(SK_LOG_INFO, _T("Failed to play sound. urlPath = %s, cause = %s"),
				urlPath, e.errorMsg().c_str());
		}	
	}



	void handlePlaySoundEnded(CAtlString const& urlPath)
	{
		SK_TRACE(SK_LOG_DEBUG, _T("Cald2Controller::handlePlaySoundEnded(%s)"), urlPath);

		try
		{
			PR_Delete(CW2A(urlPath, CP_UTF8));
		}
		catch (truntime_error& e)
		{
			SK_TRACE(SK_LOG_INFO, "Failed to delete temp sound file. soundFilePath = %s, cause = %s",
				urlPath, e.errorMsg().c_str());
		}	
	}


	void handleNewTab(CAtlString const& tabName, CAtlString const& urlPath)
	{
		SK_TRACE(SK_LOG_DEBUG, _T("Cald2Controller::handleNewTab(%s, %s)"), tabName, urlPath);

		try
		{
			ContentData result;
			BOOL rc = loadContentData(urlPath, result);
			if(!rc)
			{
				SK_TRACE(SK_LOG_INFO, _T("Failed to load %s"), urlPath);
				return;
			}

			Base::addTab(tabName);
			if(result.isXml)
				Base::setTabXmlContent(tabName, result.content);
			else
				Base::setTabHtmlContent(tabName, result.content);
			if(!result.archor.IsEmpty() && _T("0") != result.archor)
				Base::htmlAnchor(result.archor);
		}
		catch (truntime_error& e)
		{
			SK_TRACE(SK_LOG_INFO, _T("Failed to new tab. tabName=  %s, urlPath = %s, cause = %s"),
				tabName, urlPath, e.errorMsg().c_str());
		}
	}



	void handleClickWordList(CAtlString const& label)
	{
		SK_TRACE(SK_LOG_DEBUG, _T("Cald2Controller::handleClickWordList(%s)"), label);

		this->doLookup(label);
	}



	void setResultPage(UINT begin)
	{
		SK_TRACE(SK_LOG_DEBUG, _T("Cald2Controller::setResultPage(%d)"), begin);

		UINT count = getQueryService()->getResultCount();
		try
		{
			if(begin >= count || begin < 0)
			{
				return;
			}

			m_currentResultsBegin = begin;
			UINT offset = min(count - m_currentResultsBegin, m_resultsPageSize);
			UINT end = begin + offset;

			ostringstream buf;
			buf << "<skshell root='" << m_mainFrame->m_setting.GetRootPathCString() 
				<< "' isVGA='" << m_isVGA << "'>"
				<< "<results xmlns:p='Edi'"
				<< "  total='" << count
				<< "' begin='" << begin
				<< "' end='" << end
				<< "'>";

			for(UINT i = begin; i < end; i++)
			{
				ResultItem item = getQueryService()->getResultItem(i);

				buf << "<resultItem>"
					<< "<entryId>" << item.entryId << "</entryId>"
					<< "<type>" << item.type << "</type>";

				if(item.contextId != "0" )
					buf << "<contextId>" << item.contextId << "</contextId>";

				buf	<< "<label>" << item.label << "</label>"
					<< "<clid>" << item.clid << "</clid>"
					<< "</resultItem>";
			}

			buf << "</results>"
				<< "</skshell>";

			string utf8 = buf.str();
			CAtlString utf16(CA2W(utf8.c_str(), CP_UTF8));

			Base::setTabXmlContent(TAB_NAME_RESULTS, utf16);

			if(0 == begin && 1 == offset)
			{
				SK_TRACE(SK_LOG_DEBUG, _T("only one result, directly display content"));

				// only one result, directly display content
				ResultItem item = getQueryService()->getResultItem(0);
				SK_TRACE(SK_LOG_DEBUG, "item.entryId = %d, item.type = %d, item.contextId = %s, item.clid = %d, item.label = %s", 
					item.entryId, item.type, item.contextId.c_str(), item.clid, item.label.c_str());

				CAtlString urlPath;
				urlPath.AppendFormat(_T("sk://fs/2.0/data/entry/filesystem.cff!/@%d"), item.entryId);
				if(item.contextId != "0" )
					urlPath.AppendFormat(_T("#%s"), CA2W(item.contextId.c_str(), CP_UTF8));
				this->handleSetContentTab(urlPath, TRUE);
			}
		}
		catch (truntime_error& e)
		{
			SK_TRACE(SK_LOG_INFO, _T("Failed to set result page. queryString_=%s, begin = %d, count = %d, error=%s"), 
				(TCHAR const*)Base::m_queryString, begin, count, e.errorMsg().c_str());
		}
		return;
	}



	void setWordListPage(UINT begin)
		{
		SK_TRACE(SK_LOG_DEBUG, _T("Cald2Controller::setWordListPage(%d)"), begin);

		try
		{
			if(begin >= m_wordListCount || begin < 0)
				return;

			m_currentWordListBegin = begin;
			UINT offset = min(m_wordListCount - m_currentWordListBegin, m_wordListPageSize);
			UINT end = begin + offset;
			ostringstream buf;
			buf << "<skshell root='" << m_mainFrame->m_setting.GetRootPathCString() 
				<< "' isVGA='" << m_isVGA << "'>"
				<< "<wordlist xmlns:p='Edi'"
				<< "  total='" << m_wordListCount
				<< "' begin='" << begin
				<< "' end='" << end
				<< "'>";

			for(UINT i = begin; i < end; i++)
			{
				WordListItem item = getWordListService()->getWordListItem(i);

				buf << "<wordListItem>"
					<< "<entryId>" << item.entryId << "</entryId>"
					<< "<label>" << item.label << "</label>"
					<< "</wordListItem>";
			}

			buf << "</wordlist>"
				<< "</skshell>";

			string utf8 = buf.str();
			CAtlString utf16(CA2W(utf8.c_str(), CP_UTF8));

			Base::setTabXmlContent(TAB_NAME_WORDLIST, utf16);
		}
		catch (truntime_error& e)
		{
			SK_TRACE(SK_LOG_INFO, _T("Failed to init word list. error=%s"), e.errorMsg().c_str());
		}
		return;
	}



	BOOL loadContentData(CAtlString const& urlPath, ContentData & result)
	{
		SK_TRACE(SK_LOG_DEBUG, _T("Cald2Controller::loadContentData(%s)"), urlPath);

		try
		{
			int pos = urlPath.Find(_T(':'));
			CAtlString protocal = urlPath.Mid(0, pos);

			if(protocal == _T("sk"))
			{
				// skip '://fs/2.0/'
				CAtlString prefix = urlPath.Mid(pos, 10);
				if(prefix != _T("://fs/2.0/"))
				{
					// unsupported SK protocal
					SK_TRACE(SK_LOG_INFO, _T("unsupported SK protocal. urlPath = %s"), urlPath);
					return FALSE;
				}

				CAtlString fullPath = urlPath.Mid(pos + 10);

				// extract it
				char utf8Path[2048] = {0};
				int len = AtlUnicodeToUTF8(fullPath, fullPath.GetLength(), utf8Path, sizeof utf8Path);
				string utf8FullPath(utf8Path, len);
				int filesystemSplit = utf8FullPath.find('!');
				int archorSplit = utf8FullPath.find('#');
				string filesystem = utf8FullPath.substr(0, filesystemSplit);
				// skip '!/'
				string path = utf8FullPath.substr(filesystemSplit + 2, archorSplit - filesystemSplit - 2);
				// skip '#'
				string utf8Archor = utf8FullPath.substr(archorSplit + 1);
				result.archor = CA2W(utf8Archor.c_str(), CP_UTF8);
				SK_TRACE(SK_LOG_DEBUG, "filesystem = %s, path = %s, archor = %s", 
					filesystem.c_str(), path.c_str(), utf8Archor.c_str());

				std::string contentData;
				getFileSystemService()->getTextFileData(filesystem, path, contentData);

				if(filesystem == "data/entry/filesystem.cff")
				{
					result.isXml = TRUE;
					ostringstream buf;
					buf << "<skshell root='" << m_mainFrame->m_setting.GetRootPathCString() 
						<< "' isVGA='" << m_isVGA << "'>"
						<< "<content>"
						<< contentData
						<< "</content>"
						<< "</skshell>";
					string utf8 = buf.str();

					result.content = CA2W(utf8.c_str(), CP_UTF8);
				}
				else
				{
					result.isXml = FALSE;
					result.content = CA2W(contentData.c_str(), CP_UTF8);
					// extract HTML Body
					int bodyBeg = result.content.Find(_T("<body"));
					if(-1 != bodyBeg)
						bodyBeg = result.content.Find(_T('>'), bodyBeg);
					int bodyEnd = result.content.Find(_T("</body>"));
					if(-1 != bodyBeg && -1 != bodyEnd)
					{
						result.content = result.content.Mid(bodyBeg + 1, bodyEnd - bodyBeg - 1);
					}
				}
			} else if(protocal == _T("file"))
			{
				// skip '://'
				CAtlString fullPath = urlPath.Mid(pos + 3);
				SK_TRACE(SK_LOG_DEBUG, _T("fullPath = %s"), fullPath);

				CAtlFile file;
				HRESULT rc = file.Create(fullPath, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);
				if(S_OK != rc)
				{
					SK_TRACE(SK_LOG_INFO, _T("File isn't existance. urlPath = %s"), urlPath);
					return FALSE;
				}
				ULONGLONG size = 0;
				file.GetSize(size);
				LPTSTR buffer = result.content.GetBufferSetLength((int)(size + 1));
				file.Read(buffer, (int)size);
			} else
			{
				// unsupported protocal
				SK_TRACE(SK_LOG_INFO, _T("unsupported protocal. urlPath = %s"), urlPath);
				return FALSE;
			}

			if(result.content.IsEmpty())
			{
				SK_TRACE(SK_LOG_INFO, _T("Failed to load file data. urlPath = %s"), urlPath);
				return FALSE;
			}

			SK_TRACE(SK_LOG_DEBUG, _T("result.isXml = %d, result.archor = %s, result.content : \n%s"), result.isXml, result.archor, result.content);
			return TRUE;
		}
		catch (truntime_error& e)
		{
			SK_TRACE(SK_LOG_INFO, _T("Failed to load content. urlPath = %s, cause = %s"),
				urlPath, e.errorMsg().c_str());
			return FALSE;
		}
	}



	static CAtlString fixWindowsFontBug(CAtlString const& text)
	{
		CAtlString modified = text;
		// U+02C8 MODIFIER LETTER VERTICAL LINE
		modified.Remove(0x02C8);
		// U+02C9 MODIFIER LETTER MACRON
		modified.Remove(0x02C9);
		// U+02CA MODIFIER LETTER ACUTE ACCENT
		modified.Remove(0x02CA);
		// U+02CB MODIFIER LETTER GRAVE ACCENT
		modified.Remove(0x02CB);
		// U+02CC MODIFIER LETTER LOW VERTICAL LINE
		modified.Remove(0x02CC);
		// U+F198 MODIFIER LETTER DOT VERTICAL BAR
		modified.Remove(0xF198);
		// U+F199 MODIFIER LETTER DOT SLASH 
		modified.Remove(0xF199);

		return modified;
	}



	static CAtlString loadFile(CAtlString const& path)
	{
		SK_TRACE(SK_LOG_DEBUG, _T("Cald2Controller::loadFile(%s)"), path);

		std::string content;

		FILE *stream;
		if( NULL == (stream = _wfopen( path, _T("r" ))) )
			THROW_RUNTIME_EXCEPTION(_T("Failed to open ") << (TCHAR const*)path);

		int  count = 0;
		char buffer[1024];
		while( !feof( stream ) )
		{
			count = fread( buffer, sizeof( char ), sizeof buffer, stream );
			if( ferror( stream ) )
			{
				fclose( stream );
				THROW_RUNTIME_EXCEPTION(_T("Failed to read ") << (TCHAR const*)path);
			}
			content.append(buffer, count);
		}
		fclose( stream );
		
		return CAtlString (CA2W(content.c_str(), CP_UTF8));
	}


	UINT			m_wordListPageSize;

	UINT			m_resultsPageSize;

	UINT			m_queryMode;

	UINT			m_zoomLevel;

	BOOL			m_isVGA;

	BOOL			m_isFrist;

	History			m_contentHistory;

	UINT			m_currentWordListBegin;

	UINT			m_wordListCount;

	UINT			m_currentResultsBegin;

};

}	// namespace mysk 
