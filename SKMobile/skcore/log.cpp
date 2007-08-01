/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: log.cpp,v 1.1.2.5 2005/02/17 15:29:20 krys Exp $
 *
 * Authors: Colin Delacroix <colin @at@ zoy .dot. org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Alternatively you can contact IDM <skcontact @at@ idm> for other license
 * contracts concerning parts of the code owned by IDM.
 *
 *****************************************************************************/
/* END LICENSE */

#include <nspr/nspr.h>
#include <nspr/plstr.h>
#include <nspr/prlock.h>

#include <time.h>

#include "machine.h"
#include "error.h"
#include "log.h"
#include "envir/envir.h"

#define SK_LOG_FILE						"SK_LOG_FILE"
#define SK_LOG_DEBUG_ENVIR_VAR			"SK_LOG_DEBUG"

PRLock* skConsoleLog::m_pLock = NULL;
char const* skConsoleLog::m_logFile = NULL;
PRBool skConsoleLog::m_isEnableDebug= PR_FALSE;

void skConsoleLog::Init()
{
    if(m_pLock)
        return;
    m_pLock = PR_NewLock();

	// remove exist logfile
	m_logFile = getLogFile();
	if(m_logFile)
	{
		PR_Delete(m_logFile);
	}
}

void skConsoleLog::Log(const char* pszFormat, ...)
{
    va_list argp;
    va_start(argp, pszFormat);
    
    vLog(pszFormat, argp);

    va_end(argp);
}

void skConsoleLog::DebugLog(const char* file, size_t line, const char* pszFormat, ...)
{
    char* msg;
    va_list argp;
    va_start(argp, pszFormat);
    
	msg = PR_smprintf("[skdebug] [%s:%d] - %s", file, line, pszFormat);

    vLog(msg, argp);

    PR_smprintf_free(msg);
    va_end(argp);
}

void skConsoleLog::vLog(const char *pszFormat, va_list ap)
{
#ifdef XP_MAC
    Str255 str255Log;

    // don't have CopyCStringToPascal here, need a bit of magic
    PR_vsnprintf((char*)(str255Log + 1), 254, pszFormat, ap);
    str255Log[0] = PL_strlen( (const char*)(str255Log + 1) );
    DebugStr( str255Log );
#else
    if(!m_pLock)
        return;
    PR_Lock(m_pLock);
	if(m_logFile)
    {
        FILE * f = fopen(m_logFile, "a");
        if(f)
		{
            vfprintf(f, pszFormat, ap);
		}
        fclose(f);
    }
    PR_Unlock(m_pLock);
#endif
}

void skConsoleLog::Log(const wchar_t* pszFormat, ...)
{
	va_list argp;
	va_start(argp, pszFormat);

	vLog(pszFormat, argp);

	va_end(argp);
}

void skConsoleLog::DebugLog(const char* file, size_t line, const wchar_t* pszFormat, ...)
{
	char* msg;
	va_list argp;

	if(m_isEnableDebug)
	{
		va_start(argp, pszFormat);

		msg = PR_smprintf("[skdebug] [%s:%d] - %s", file, line, pszFormat);

		vLog(msg, argp);

		PR_smprintf_free(msg);
		va_end(argp);
	}
}

void skConsoleLog::vLog(const wchar_t *pszFormat, va_list ap)
{
#ifdef XP_MAC
	Str255 str255Log;

	// don't have CopyCStringToPascal here, need a bit of magic
	PR_vsnprintf((char*)(str255Log + 1), 254, pszFormat, ap);
	str255Log[0] = PL_strlen( (const char*)(str255Log + 1) );
	DebugStr( str255Log );
#else
	if(!m_pLock)
		return;
	PR_Lock(m_pLock);

	if(m_logFile)
	{
		FILE * f = fopen(m_logFile, "a");
		if(f)
		{			
			vfwprintf(f, pszFormat, ap);
		}
		fclose(f);
	}
	PR_Unlock(m_pLock);
#endif
}

/*
void skConsoleLog::GetLog(char** pcLog)
{
    *pcLog = NULL;
    if(!m_pLock)
        return;
    PR_Lock(m_pLock);
    *pcLog = PL_strdup(m_pcLog);
    PR_Unlock(m_pLock);
}

void skConsoleLog::ClearLog()
{
    if(!m_pLock)
        return;
    PR_Lock(m_pLock);
    if(m_pcLog)
    {
        PL_strfree(m_pcLog);
        m_pcLog = NULL;
    }
    PR_Unlock(m_pLock);
}
*/
const char * skConsoleLog::getLogFile()
{
	SKEnvir *pEnvir = NULL;
	SKERR err = SKEnvir::GetEnvir(&pEnvir);
	if(err != noErr)
		return NULL;

	char* pcFile = NULL;
	err = pEnvir->GetValue(SK_LOG_FILE, &pcFile);
	if(err != noErr)
		return NULL;

	if(NULL == pcFile || 0 == strlen(pcFile) )
	{
		pcFile = "sk.log";
	}

	if(pcFile[0] != '/' && pcFile[0] != '\\')
	{
		// It's a relative path, convert it to absolute path.
		char* skHome = NULL;
		err = pEnvir->GetValue(SK_HOME_ENVIR_VAR, &skHome);
		if(err != noErr)
			return NULL;
		
		size_t len = strlen(skHome) + strlen(pcFile) + 2;
		char * fullPath = (char *)PR_Malloc(len);
		sprintf(fullPath, "%s/%s", skHome, pcFile);
		pcFile = fullPath;
	}
	else
	{
		pcFile = PL_strdup(pcFile);
	}

	// get enable debug flag
	char* enableDebugString = NULL;
	err = pEnvir->GetValue(SK_LOG_DEBUG_ENVIR_VAR, &enableDebugString);
	if(err != noErr)
		return NULL;

	if(NULL != enableDebugString && (
		0 == PL_strcmp("1", enableDebugString) || 
		0 == PL_strcasecmp("true", enableDebugString) ||
		0 == PL_strcasecmp("yes", enableDebugString) ))
	{
		m_isEnableDebug = PR_TRUE;
	}
	else
	{
		m_isEnableDebug = PR_FALSE;
	}
	
	return pcFile;
}

void skConsoleLog::trace(PRBool isDebug, const char* file, size_t line, const char * pszFormat, ...)
{
/*
	time_t timer = time(NULL);
	struct tm* localtimer = localtime(&timer);
	char timeString[64] = {0};
	sprintf(timeString, "%d-%d %d:%d:%d", 
		(localtimer->tm_mon + 1),
		localtimer->tm_mday,
		localtimer->tm_hour,
		localtimer->tm_min,
		localtimer->tm_sec);
*/	
	if(isDebug)
	{
		if(m_isEnableDebug)
		{
			// Log("\n[%s] [DEBUG] [%s:%d] - ", timeString, file, line);
			Log("\n[DEBUG] [%s:%d] - ", file, line);
			va_list argp;
			va_start(argp, pszFormat);
			vLog(pszFormat, argp);
			va_end(argp);
		}
	}
	else
	{
		Log("\n[INFO]  [%s:%d] - ", file, line);
		va_list argp;
		va_start(argp, pszFormat);
		vLog(pszFormat, argp);
		va_end(argp);
	}
}

void skConsoleLog::trace(PRBool isDebug, const char* file, size_t line, const wchar_t * pszFormat, ...)
{
	if(isDebug)
	{
		if(m_isEnableDebug)
		{
			Log("\n[DEBUG] [%s:%d] - ", file, line);
			va_list argp;
			va_start(argp, pszFormat);
			vLog(pszFormat, argp);
			va_end(argp);
		}
	}
	else
	{
		Log("\n[INFO]  [%s:%d] - ", file, line);
		va_list argp;
		va_start(argp, pszFormat);

		vLog(pszFormat, argp);

		va_end(argp);
	}
}

