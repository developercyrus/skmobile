/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: log.h,v 1.1.4.4 2005/02/17 15:29:20 krys Exp $
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

#ifndef __SKC_LOG_H_
#define __SKC_LOG_H_

#define SK_TRACE  skConsoleLog::trace

#define SK_LOG_INFO  PR_FALSE, __FILE__, __LINE__

#define SK_LOG_DEBUG  PR_TRUE, __FILE__, __LINE__

class SKAPI skConsoleLog
{
public:
    static void Init();

    static void Log(const char* pszFormat, ...);
    static void vLog(const char *pszFormat, va_list ap);
    static void DebugLog(const char* file, size_t line, const char* pszFormat, ...);

	static void Log(const wchar_t * pszFormat, ...);
	static void vLog(const wchar_t *pszFormat, va_list ap);
	static void DebugLog(const char* file, size_t line, const wchar_t * pszFormat, ...);

	static void trace(PRBool isDebug, const char* file, size_t line, const char * pszFormat, ...);
	static void trace(PRBool isDebug, const char* file, size_t line, const wchar_t * pszFormat, ...);

private:

    skConsoleLog() {};
    
	~skConsoleLog() {};

	static const char * getLogFile();

    static PRLock*  m_pLock;

	static char const*	m_logFile;

	static PRBool	m_isEnableDebug;
};

#endif // __SKC_LOG_H_

