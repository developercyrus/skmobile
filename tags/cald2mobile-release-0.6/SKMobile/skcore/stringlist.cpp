/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: stringlist.cpp,v 1.5.4.3 2005/02/17 15:29:20 krys Exp $
 *
 * Authors: Christopher Gautier <krys @at@ idm .dot. fr>
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

#include "skcoreconfig.h"

#include "machine.h"
#include "error.h"

#include "refcount.h"

#include "file/skfopen.h"
#include "file/file.h"

#include "stringlist.h"

static int g_sInsensitiveCompareArrayWords(const void *p1, const void *p2)
// use this function to compare strings stored in a char** array
{
    return PL_strcasecmp(*(char**)p1, *(char**)p2);
}

SK_REFCOUNT_IMPL_DEFAULT(SKStringList);

SKStringList::SKStringList()
{
    m_iNbWords = 0;
    m_pWordList = NULL;
    m_pszWordString = NULL;
    m_bInitialized = PR_FALSE;
}

SKStringList::~SKStringList()
{
    if(m_pWordList)
        delete[] m_pWordList;
    if(m_pszWordString)
        delete[] m_pszWordString;
}

PRUint32 SKStringList::GetCount()
{
    return m_iNbWords;
}

SKERR SKStringList::SetFileName(const char *pszFileName,
                                const char *pszDefaultFileName)
{
    m_bInitialized = PR_FALSE;

    if(m_pWordList)
    {
        delete[] m_pWordList;
        m_pWordList = NULL;
    }
    if(m_pszWordString)
    {
        delete[] m_pszWordString;
        m_pszWordString = NULL;
    }

    SKERR err = SKFile::SetFileName(pszFileName);
    if(err != noErr)
        return err;

    PRFileInfo info;
    if(PR_GetFileInfo(GetSharedFileName(), &info) != PR_SUCCESS)
        return err_failure;

    PRFileDesc *f = skPR_Open(GetSharedFileName(), PR_RDONLY, 0);
    if(!f)
        return err_notfound;

    if (info.size == 0)
    {
        PR_Close(f);
        // there is no stopwords, m_bInitialized is set to PR_FALSE
        return noErr;
    }

    PRUint32 iSize = info.size + 1;
    m_pszWordString = new char[iSize];
    if(!m_pszWordString)
    {
        PR_Close(f);
        return err_memory;
    }

    if(PR_Read(f, m_pszWordString, info.size) != info.size)
    {
        PR_Close(f);
        return err_failure;
    }

    PR_Close(f);

    m_pszWordString[iSize- 1]= 0;

    // count the numbers of words. Do not count empty lines ('\n') as words
    PRUint32 iScanString = 0;
    for (; iScanString < iSize- 1; iScanString++)
        if (m_pszWordString[iScanString]== '\n')
            if (iScanString!= 0 && m_pszWordString[iScanString- 1]!= '\n')
                m_iNbWords++;

    if (m_pszWordString[iSize- 2]!= '\n')
        m_iNbWords++;

    m_pWordList = new char*[m_iNbWords];
    if(!m_pWordList)
    {
        PR_Close(f);
        return err_memory;
    }

    // save a pointer to the starting character of each word
    char* pszTokenStart = m_pszWordString;
    PRUint32 iWordIndex = 0;
    for (iScanString = 0; iScanString < iSize - 1; iScanString++)
        if (m_pszWordString[iScanString]== '\n')
        {
            if (iScanString!= 0 && m_pszWordString[iScanString - 1]!= 0)
            {
                m_pszWordString[iScanString] = 0;
                m_pWordList[iWordIndex++] = pszTokenStart;
                pszTokenStart = m_pszWordString + iScanString + 1;
            }
            else
            {
                m_pszWordString[iScanString]= 0;
                pszTokenStart++;
            }
        }

    // sort the list so that we can do binary searches
    qsort(m_pWordList, m_iNbWords, sizeof(char*), &g_sInsensitiveCompareArrayWords);

    m_bInitialized = PR_TRUE;

    return noErr;
}


SKERR SKStringList::SetListWithOneWord(const char *pszWord)
{
    m_bInitialized = PR_FALSE;

    if(m_pWordList)
    {
        delete[] m_pWordList;
        m_pWordList = NULL;
    }
    if(m_pszWordString)
    {
        delete[] m_pszWordString;
        m_pszWordString = NULL;
    }

    m_pszWordString = new char[PL_strlen(pszWord)+ 1];
    if(!m_pszWordString)
    {
        return err_memory;
    }
    m_pWordList = new char*[1];
    if(!m_pWordList)
    {
        return err_memory;
    }

    m_iNbWords = 1;
    memcpy (m_pszWordString, pszWord, PL_strlen(pszWord)+ 1);
    m_pWordList[0] = m_pszWordString;
    m_bInitialized = PR_TRUE;

    return noErr;
}


SKERR SKStringList::IsPresent(char* pszUTF8Word, PRBool *pbResult)
{
    *pbResult = PR_FALSE;

    if(!m_bInitialized || !m_iNbWords)
        return noErr;

    if(!PL_strcmp(m_pWordList[0], pszUTF8Word) ||
       !PL_strcmp(m_pWordList[m_iNbWords - 1], pszUTF8Word))
    {
        *pbResult = PR_TRUE;
        return noErr;
    }

    PRUint32 iMin = 0;
    PRUint32 iMax = m_iNbWords - 1;
    while(iMax - iMin > 1)
    {
        PRUint32 iMid = (iMin + iMax) >> 1;
        PRInt32  iCompare = PL_strcasecmp(m_pWordList[iMid], pszUTF8Word);

        if(iCompare == 0)
        {
            *pbResult = PR_TRUE;
            return noErr;
        } else
        if(iCompare < 0) iMin = iMid; else iMax = iMid;
    }

    *pbResult = PR_FALSE;
    return noErr;
}
