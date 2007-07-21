/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: inifile.h,v 1.4.2.2 2005/02/17 15:29:21 krys Exp $
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

#ifndef __SKC_INIFILE_H_
#define __SKC_INIFILE_H_

class SKAPI IniParser
{
public:
    IniParser(SKTextFile *pTextFile)
    {
        SK_ASSERT(NULL != pTextFile);
        m_pTextFile = pTextFile;
        m_pszCurrentSection[0] = '\0';
    }
    ~IniParser() {};

    PRBool readIniLine(char** pszSection, char** pszToken, char** pszVal,
                       PRUint32* piLine);

private:
    skPtr<SKTextFile> m_pTextFile;
    char m_pszCurrentSection[256];
};

#else // __SKC_INIFILE_H_
#error "Multiple inclusions of inifile.h"
#endif // __SKC_INIFILE_H_

