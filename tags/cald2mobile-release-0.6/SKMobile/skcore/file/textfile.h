/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: textfile.h,v 1.6.2.2 2005/02/17 15:29:21 krys Exp $
 *
 * Authors: Alexis Seigneurin <seigneurin @at@ idm .dot. fr>
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

#ifndef __SKC_TEXTFILE_H_
#define __SKC_TEXTFILE_H_

class SKAPI SKConfigurationDelegate
{
public:
    virtual SKERR ConfigureItem(char* pszSection,
                                char* pszName,
                                char* pcsValue) = 0;
};

class SKAPI SKTextFile : public SKFile
{
public:
    SKTextFile();
    ~SKTextFile();

    virtual SKERR SetFileName(const char *pszFileName,
                              const char *pszDefaultFileName = NULL);

    virtual SKERR SetFileDesc(PRFileDesc *pFd);

    PRBool IsReady() const
    {
        return m_bReady;
    }

    PRUint32 GetFilePosition();

    PRUint32 GetFileSize();

    SKERR GetLine(char **ppszLine);

    SKERR ParseConfiguration();

    virtual SKERR ConfigureItem(char* pszSection, char* pszName, char* pszValue);

    void SetConfigurationDelegate(SKConfigurationDelegate *pCD)
    {
        m_pDelegate = pCD;
    }

private:
    SKERR PostInitialize();

    PRBool m_bReady;
    PRBool m_bFinished;
    PRFileDesc *m_pFileDesc;

    char *m_pszBuffer;          // our internal buffer
    PRUint32 m_iBufferSize;     // its size (as in malloc)
    PRUint32 m_iDeadLength;     // the length of it which has already been 
                                // returned
    PRUint32 m_iReadLength;     // the number of bytes in the buffer
    PRUint32 m_iFileSize;       // file size in bytes

    SKConfigurationDelegate* m_pDelegate;
};

#else
#error "Multiple inclusions of textfile.h"
#endif

