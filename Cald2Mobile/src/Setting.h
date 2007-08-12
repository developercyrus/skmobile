
#pragma once

class Setting 
{
public :

	void Load()
	{
		m_rootPathW = _RootPath;
		m_rootPathA = CW2A(_RootPath, CP_UTF8);

		TCHAR tempPath[1024] = {0};
		::GetTempPath(sizeof tempPath, tempPath);
		m_tempPathW = tempPath;
		m_tempPathW += _T("Cald2Mobile");
		m_tempPathA = CW2A(m_tempPathW, CP_UTF8);

		SKEnvir::GetEnvir(&m_envir);
		SKERR err = m_envir->Init(m_rootPathA);
		if(err != noErr)
		{
			SK_TRACE(SK_LOG_INFO, _T("Failed to inital SKEnvir. error=%d"), err);
			return;
		}
		m_iniPath.Format(_T("%s/sk.ini"), _RootPath);
	}



	char const* GetRootPathCString()
	{
		return this->m_rootPathA;
	}



	TCHAR const* GetRootPathWString()
	{
		return this->m_rootPathW;
	}



	char const* GetTempPathCString()
	{
		return this->m_tempPathA;
	}



	TCHAR const* GetTempPathWString()
	{
		return this->m_tempPathW;
	}



	char const* LoadStringValue(char const* key, char const* defaultValue = "")
	{
		char * value = NULL;
		m_envir->GetValue(key, &value);
		if(NULL == value || 0 == strlen(value))
			return defaultValue;
		else
			return value;
	}



	UINT LoadIntValue(char const* key, UINT defaultValue = 0)
	{
		char * value = NULL;
		m_envir->GetValue(key, &value);
		if(NULL == value || 0 == strlen(value))
			return defaultValue;
		else
			return atoi(value);
	}



	BOOL LoadBoolValue(char const* key, BOOL defaultValue = FALSE)
	{
		char * value = NULL;
		m_envir->GetValue(key, &value);
		if(NULL == value || 0 == strlen(value))
			return defaultValue;
		else
		{
			if(0 == strcmp("0", value))
				return FALSE;
			if(0 == _stricmp("false", value))
				return FALSE;
			return TRUE;
		}
	}



protected :

	SKEnvir *	m_envir;

	CAtlString	m_iniPath;

	CAtlStringW m_rootPathW;

	CAtlStringA m_rootPathA;

	CAtlStringW m_tempPathW;

	CAtlStringA m_tempPathA;
};
