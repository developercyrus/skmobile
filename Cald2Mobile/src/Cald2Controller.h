
#pragma once

#include "skfind/backend/native/native.h"

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



class Cald2Controller : public BasicController
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

		m_queryMode = mainFrame->m_setting.LoadIntValue("MYSK_QUERY_MODE", 0x07);
		m_zoomLevel = mainFrame->m_setting.LoadIntValue("MYSK_ZOOM_LEVEL", 2);

		m_dictionaryService.init(mainFrame->m_setting.GetRootPathCString());
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

			// first
			SKERR err = loadDictionaryService();
			if(noErr != err)
			{
				SK_TRACE(SK_LOG_INFO, _T("Failed to inital Cald2DictionaryController. error=%d"), err);
				return S_OK;				
			}

			// add 1 tab
			Base::addTab(TAB_NAME_WORDLIST);

			// Add query modes
			for(UINT i = 0; i < QUERY_MODE_INDEX_SIZE; i++)
			{
				QueryModeInfo & info = QUERY_MODE_INFO[i];
				Base::addQueryMode(info.name);
			}
			Base::setQueryMode(m_queryMode);

			//read page sizes
			m_wordListPageSize = Base::getWordListPageSize();
			m_resultsPageSize = Base::getResultsPageSize();
			
			// initial XSL
			CAtlString xslPath;
			xslPath.Format(_T("%s/cald2/cald2.xsl"), _RootPath);
			SK_TRACE(SK_LOG_DEBUG, _T("xslPath = %s"), xslPath);

			CAtlString xslContent;
			err = loadFile(xslPath, xslContent);
			if(noErr != err)
			{
				CAtlString msg;
				msg.Format(_T("Failed to load xsl file %s"), xslPath);
				error(err, _T("OnDocumentComplete"), msg);
				return S_OK;
			}
			Base::setStyleSheet(xslContent);

			// load word list
			m_wordListCount = m_dictionaryService.getWordListService()->getWordListCount();
			SK_TRACE(SK_LOG_DEBUG, _T("m_wordListCount = %d"), m_wordListCount);
			setWordListPage(0);
		}

		return S_OK;
	}



	virtual BOOL OnReturnKey(UINT nRepCnt, UINT nFlags)
	{
		SK_TRACE(SK_LOG_DEBUG, "Cald2Controller::OnReturnKey");

		if(m_view->m_wndContentTabView.IsChild(::GetFocus()))
		{
			return FALSE;
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

				Base::htmlPageUp();
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

				Base::htmlPageDown();
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
			// simulate shift + tab
			keybd_event(VK_SHIFT, 0, KEYEVENTF_SILENT, 0);
			keybd_event(VK_TAB, 0, KEYEVENTF_SILENT, 0);
			keybd_event(VK_TAB, 0, KEYEVENTF_SILENT | KEYEVENTF_KEYUP, 0);
			keybd_event(VK_SHIFT, 0, KEYEVENTF_SILENT | KEYEVENTF_KEYUP, 0);
			return TRUE;
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
			// simulate tab
			keybd_event(VK_TAB, 0, KEYEVENTF_SILENT, 0);
			keybd_event(VK_TAB, 0, KEYEVENTF_SILENT | KEYEVENTF_KEYUP, 0);
			return TRUE;
		}
		return Base::OnRightKey(nRepCnt, nFlags);
	}



	virtual BOOL OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
	{
		SK_TRACE(SK_LOG_DEBUG, "Cald2Controller::OnChar(%d, %d, %d)", nChar, nRepCnt, nFlags);

		HWND focusHwnd = GetFocus();
		if(m_view->m_wndContentTabView.IsChild(focusHwnd))
		{
			int activeTab = m_view->m_wndContentTabView.GetActivePage();
			CAtlString title = m_view->m_wndContentTabView.GetPageTitle(activeTab);
			if(title == TAB_NAME_CONTENT)
			{
				switch(nChar)
				{
				case '.':
					{
						// navigate in history
						CAtlString nextUrlPath = m_contentHistory.GetNext();
						if(!nextUrlPath.IsEmpty())
						{
							this->handleSetContentTab(nextUrlPath, FALSE);
						}
						return TRUE;
					}
				case ',':
					{
						// navigate in history
						CAtlString prevUrlPath = m_contentHistory.GetPrev();
						if(!prevUrlPath.IsEmpty())
						{
							this->handleSetContentTab(prevUrlPath, FALSE);
						}
						return TRUE;
					}
				}
			}
		}
		
		return Base::OnChar(nChar, nRepCnt, nFlags);
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

		SKERR err = noErr;

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
		err = m_dictionaryService.getQueryService()->query((TCHAR const*)queryString, true, types, bUseDeflect);
		if(noErr != err)
		{
			SK_TRACE(SK_LOG_DEBUG, _T("Failed to query '%s'. error = %d"), queryString, err); 

			// prompt user
			CAtlString msg;
			msg.Format(_T("Failed to query '%s'. error = %d."), queryString, err);
			Base::setTabTextContent(TAB_NAME_RESULTS, msg);

			if(err_skf_fseek == err)
			{
				// reopen the dictionary service
				reloadDictionaryService();
			}
			return;
		}

		UINT count = m_dictionaryService.getQueryService()->getResultCount();
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

		SKERR err = noErr;
		// get sort key
		CComVariant result;
		TCHAR const* params[] = { queryString };
		size_t paramsCount = sizeof(params) / sizeof(TCHAR const*);
		m_script.Run(_T("GetWordListSortKey"), &result, paramsCount, params);

		CComBSTR sortKey(result.bstrVal);
		CAtlString sortKeyString = sortKey;
		SK_TRACE(SK_LOG_DEBUG, _T("sortKeyString = %s"), sortKeyString);

		std::string utf8(CW2A(sortKeyString, CP_UTF8));
		UINT pos = 0;
		err = m_dictionaryService.getWordListService()->findWordListPosition(utf8, &pos);
		if(err != noErr)
		{
			if(err_skf_fseek == err)
				reloadDictionaryService();
			else
				error(err, _T("updateWordList"), _T("Failed to update word list."));
			return;
		}
		
		SK_TRACE(SK_LOG_DEBUG, _T("WordListPosition = %d"), pos);
		if(pos > 0 && pos < m_dictionaryService.getWordListService()->getWordListCount())
		{
			UINT begin = pos - pos % m_wordListPageSize;
			setWordListPage(begin);
			CAtlString archor;
			archor.Format(_T("wordlist-%d"), pos);
			Base::htmlAnchor(archor);
			Base::selectAnchor(archor);
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



	void handlePlaySound(CAtlString const& urlPath)
	{
		SK_TRACE(SK_LOG_DEBUG, _T("Cald2Controller::handlePlaySound(%s)"), urlPath);

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
			SKERR err = m_dictionaryService.getFileSystemService()->copyFile(filesystem, path, utf8SoundFilePath);
			switch(err)
			{
			case noErr:
				// do nothing
				break;
			case err_notfound:
				{
					CAtlString msg;
					msg.Format(_T("Can't find the %s folder. Do you install the sound data?"), 
						CA2W(filesystem.c_str()));
					error(err, _T("handlePlaySound"), msg);
					return;
				}
			case err_skf_fseek :
				reloadDictionaryService();
				return;
			default :
				{
					CAtlString msg;
					msg.Format(_T("Failed to play sound. urlPath = %s"), urlPath);
					error(err, _T("handlePlaySound"), msg);
					return;
				}
			}
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



	void handlePlaySoundEnded(CAtlString const& urlPath)
	{
		SK_TRACE(SK_LOG_DEBUG, _T("Cald2Controller::handlePlaySoundEnded(%s)"), urlPath);

		PR_Delete(CW2A(urlPath, CP_UTF8));
	}


	void handleNewTab(CAtlString const& tabName, CAtlString const& urlPath)
	{
		SK_TRACE(SK_LOG_DEBUG, _T("Cald2Controller::handleNewTab(%s, %s)"), tabName, urlPath);

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



	void handleClickWordList(CAtlString const& label)
	{
		SK_TRACE(SK_LOG_DEBUG, _T("Cald2Controller::handleClickWordList(%s)"), label);

		this->doLookup(label);
	}



	void setResultPage(UINT begin)
	{
		SK_TRACE(SK_LOG_DEBUG, _T("Cald2Controller::setResultPage(%d)"), begin);

		UINT count = m_dictionaryService.getQueryService()->getResultCount();
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
			ResultItem item;
			SKERR err = m_dictionaryService.getQueryService()->getResultItem(i, &item);
			if(err != noErr)
			{
				CAtlString msg;
				msg.Format(_T("Failed to get NO. %d result item."), i);
				error(err, _T("setResultPage"), msg);
				return;
			}

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
			ResultItem item;
			SKERR err =  m_dictionaryService.getQueryService()->getResultItem(0, &item);
			if(err != noErr)
			{
				error(err, _T("setResultPage"), _T("Failed to get the first result item."));
				return;
			}

			SK_TRACE(SK_LOG_DEBUG, "item.entryId = %d, item.type = %d, item.contextId = %s, item.clid = %d, item.label = %s", 
				item.entryId, item.type, item.contextId.c_str(), item.clid, item.label.c_str());

			CAtlString urlPath;
			urlPath.AppendFormat(_T("sk://fs/2.0/data/entry/filesystem.cff!/@%d"), item.entryId);
			if(item.contextId != "0" )
				urlPath.AppendFormat(_T("#%s"), CA2W(item.contextId.c_str(), CP_UTF8));
			this->handleSetContentTab(urlPath, TRUE);
		}
		return;
	}



	void setWordListPage(UINT begin)
	{
		SK_TRACE(SK_LOG_DEBUG, _T("Cald2Controller::setWordListPage(%d)"), begin);

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
			WordListItem item;
			SKERR err = m_dictionaryService.getWordListService()->getWordListItem(i, &item);
			if(err != noErr)
			{
				CAtlString msg;
				msg.Format(_T("Failed to get NO. %d result item."), i);
				error(err, _T("setWordListPage"), msg);
				return;
			}

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
		return;
	}



	BOOL loadContentData(CAtlString const& urlPath, ContentData & result)
	{
		SK_TRACE(SK_LOG_DEBUG, _T("Cald2Controller::loadContentData(%s)"), urlPath);

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
			SKERR err = m_dictionaryService.getFileSystemService()->getTextFileData(filesystem, path, contentData);
			switch(err)
			{
			case noErr:
				// do nothing
				break;
			case err_notfound:
				{
					CAtlString msg;
					msg.Format(_T("Can't find the %s folder. Do you install this data?"), 
						CA2W(filesystem.c_str()));
					error(err, _T("loadContentData"), msg);
					return FALSE;
				}
			case err_skf_fseek :
				reloadDictionaryService();
				return FALSE;
			default :
				{
					CAtlString msg;
					msg.Format(_T("Failed to load content. urlPath = %s, error = %d"), urlPath, err);
					error(err, _T("loadContentData"), msg);
					return FALSE;
				}
			}

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



	SKERR loadDictionaryService()
	{
		SKERR err = noErr;

		SKWordListService * wordlistService = new SKWordListService;
		err = wordlistService->init("native:data/wordlist.skn");
		if(noErr != err)
		{
			CAtlString msg;
			msg.Format(_T("Failed to load wordlist data from %s"), _T("data/wordlist.skn"));
			error(err, _T("loadDictionaryService"), msg);
			return err;
		}
		m_dictionaryService.setWordListService(wordlistService);

		SKIndexQueryService * queryService = new SKIndexQueryService;
		err = queryService->init(
			"data/index/complete.skn/index.cfi", 
			"native:data/complete.skn",
			"data/index/flex2stem.skn/index.cfi", 
			"native:data/index/flex2stem.skn");
		if(noErr != err)
		{
			CAtlString msg;
			msg.Format(_T("Failed to init index service from %s, %s, %s, %s"), 
				_T("data/index/complete.skn/index.cfi"),
				_T("data/complete.skn"),
				_T("data/index/flex2stem.skn/index.cfi"),
				_T("data/index/flex2stem.skn"));
			error(err, _T("loadDictionaryService"), msg);
			return err;
		}
		m_dictionaryService.setQueryService(queryService);

		m_dictionaryService.setFileSystemService(new FileSystemService);

		return noErr;
	}



	void reloadDictionaryService()
	{
		SK_TRACE(SK_LOG_INFO, "checkDictionaryService : The SD/CF card is removed.");

		CAtlString msg;
		msg.Format(_T("FileDesc is invalidation. Maybe the SD/CF card was removed. Please check the data files of Cald2Mobile. The dictionary engine will be initialled again."));
		m_view->MessageBox(msg, _T("Cald2Mobile"), MB_OK | MB_ICONWARNING);

		SKERR err = loadDictionaryService();
		if(noErr == err)
		{
			msg.Format(_T("The dictionary engine is initialled.\n You can work normally."));
			m_view->MessageBox(msg, _T("Cald2Mobile"), MB_OK | MB_ICONINFORMATION);
		}
		else
		{
			msg.Format(_T("Failed to initial the dictionary engine.\n Maybe you should restart the program."));
			m_view->MessageBox(msg, _T("Cald2Mobile"), MB_OK | MB_ICONWARNING);
		}
	}



	void error(SKERR err, CAtlString const& functionName, CAtlString const& message)
	{
		SK_TRACE(SK_LOG_INFO, _T("%s : error = %d. (%s)"), functionName, err, message);

		CAtlString prompt;
		prompt.Format(_T("%s\nerror = %d"), message, err);
		m_view->MessageBox(prompt, _T("Cald2Mobile"), MB_OK | MB_ICONWARNING);
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



	static SKERR loadFile(CAtlString const& path, CAtlString & result)
	{
		SK_TRACE(SK_LOG_DEBUG, _T("Cald2Controller::loadFile(%s)"), path);

		std::string content;

		FILE *stream;
		if( NULL == (stream = _wfopen( path, _T("r" ))) )
		{
			SK_TRACE(SK_LOG_INFO, _T("Failed to open %s"), path);
			return err_notfound;
		}

		int  count = 0;
		char buffer[1024];
		while( !feof( stream ) )
		{
			count = fread( buffer, sizeof( char ), sizeof buffer, stream );
			if( ferror( stream ) )
			{
				fclose( stream );
				SK_TRACE(SK_LOG_INFO, _T("Failed to read %s"), path);
				return err_skf_fread;
			}
			content.append(buffer, count);
		}
		fclose( stream );
		
		result = CA2W(content.c_str(), CP_UTF8);
		return noErr;
	}

	DictionaryService	m_dictionaryService;

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
