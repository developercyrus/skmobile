/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: inifile.cpp,v 1.4.4.3 2005/02/17 15:29:21 krys Exp $
 *
 * Authors: Mathieu Poumeyrol <poumeyrol @at@ idm .dot. fr>
 *          Arnaud de Bossoreille de Ribou <debossoreille @at@ idm .dot. fr>
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

#include "../skcoreconfig.h"

#include "../machine.h"
#include "../error.h"

#include "../refcount.h"
#include "../skptr.h"

#include "skfopen.h"
#include "file.h"
#include "textfile.h"
#include "inifile.h"

//  -------------------------------------------------------------------
//
//  GetINILine
//
//  reads [token = value] lines from a windows INI file
//  returns the section, token, value in uppercase
//  skip blank lines
//  strips C++ comments
//  returns false on EOF
//
//  -------------------------------------------------------------------

PRBool IniParser::readIniLine(char** pszSection, char** pszToken,
                              char** pszVal, PRUint32* piLine)
{
    char *p, *q, *r;

    // check for invalid parameters
    if(!m_pTextFile || !pszSection || !pszToken || !pszVal)
        return false;

    // initialization
    *pszSection = &m_pszCurrentSection[0];
    *pszToken = NULL;
    *pszVal = NULL;

    char *pszLine = NULL;
    SKERR err = noErr;

    while(err == noErr)
    {
        err = m_pTextFile->GetLine(&pszLine);
        if((err != noErr) || !pszLine)
            break;
        // increment line number if applicable
        if(piLine != NULL)
            ++*piLine;

        // strip comments
        p = PL_strstr(pszLine, "//");
        if(p)
            *p = 0;

        // right trim
        for(p = pszLine + PL_strlen(pszLine); p >= pszLine && *p <= 0x20; p--)
            *p = 0;

        // left trim
        for(p = pszLine; *p && *p <= 0x20; p++) {}

        // section
        if (*p == '[')
        {
            q = PL_strchr(p, ']');
            if (q)
                *q = 0;
            p++;
            PL_strncpy(m_pszCurrentSection, p, sizeof(m_pszCurrentSection) - 1);
            m_pszCurrentSection[sizeof(m_pszCurrentSection) - 1] = '\0';
            break;
        }

        // look for token = val pairs
        q = PL_strchr(p, '=');
        if (!q)
        {
            // skip ill-formed lines
            continue;
        }

        // right trim
        for (r = q ; r >= p && (*r <= 0x20 || *r == '=') ; r--)
            *r = 0;

        // found token
        *pszToken = p;

        // left trim
        for (q++; *q <= 0x20; q++)
        {
            if (!*q)
                break;
        }
/*
        if (!*q)
        {
            // skip ill-formed lines
            SKError(err_invalid, "[IniParser] invalid line: %s", pszLine);
            continue;
        }
*/
        // found value
        *pszVal = q;
        break;
    }

    // true on success
    return (err == noErr) && pszLine;
}

