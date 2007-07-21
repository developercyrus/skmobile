/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: lc.cpp,v 1.25.2.5 2005/02/22 09:03:04 bozo Exp $
 *
 * Authors: W.P. Dauchy
 *          Mathieu Poumeyrol <poumeyrol @at@ idm .dot. fr>
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

#include <skfind/skfind.h>

#include "lc.h"
#include "field.h"
#include "record.h"
#include "buf.h"
#include "table.h"

//  -------------------------------------------------------------------
//
//  macros
//
//  -------------------------------------------------------------------

// this macros compares cleanly two PRUint32
#define COMPARE_LONG(a,b,type) ((((type)(a))==((type)(b)))?0:(((type)(a))<((type)(b))?-1:1))

#define COMPARE_LUTYPE(a, b, lutype)                                        \
        (PRInt32) ( ((lutype) == STRING) ? PL_strcmp((char*)(a), (char*)(b))\
                    : ((lutype) == SINT) ?                                  \
                        COMPARE_LONG((a), (b), PRInt32) :                   \
                        COMPARE_LONG((a), (b), PRUint32) )

#define COMPARE_FIRST(key,page,lutype)                                      \
            COMPARE_LUTYPE((lutype == STRING) ? (PRUint32) (key)            \
                                        : *((PRUint32*) (key)),             \
                (lutype) == STRING                                          \
                    ? (PRUint32) m_Pages[(page)].first_key                  \
                    : m_Pages[(page)].first_num,                            \
                (lutype))

#define COMPARE_LAST(key,page,lutype)                                       \
            COMPARE_LUTYPE((lutype == STRING) ? (PRUint32) (key)            \
                                        : *((PRUint32*) (key)),             \
                (lutype) == STRING                                          \
                    ? (PRUint32) m_Pages[(page)].last_key                   \
                    : m_Pages[(page)].last_num,                             \
                (lutype))


// If we use mozilla system stubs, we always get \r for newlines
#define EOL '\n'

SK_REFCOUNT_IMPL_DEFAULT(SKLCFile)

//  -------------------------------------------------------------------
//
//  SKLCFile
//
//  -------------------------------------------------------------------
SKLCFile::SKLCFile()
{
    m_lPageCount = 0;
    m_iHeaderLines = 0;
    m_Pages = NULL;
    m_szBuffer = NULL;
    m_bLoaded = PR_FALSE;
}

//  -------------------------------------------------------------------
//
//  ~SKLCFile
//
//  -------------------------------------------------------------------
SKLCFile::~SKLCFile(void)
{
    if (m_Pages)
        PR_Free (m_Pages);
    if (m_szBuffer)
        PR_Free (m_szBuffer);
}

//  -------------------------------------------------------------------
//
//  Load
//
//  -------------------------------------------------------------------
SKERR SKLCFile::Load(void)
{
    if(m_bLoaded)
        return noErr;

    PRFileDesc*         f;
    unsigned long       lSize;
    long                lLineCount = 0;
    short               iFieldNum = 0;
    LcPageRec           p = {0};
    PRBool              bEOL = PR_FALSE;

    // open SKLCFile file
    f = skPR_Open( m_pszFileName, PR_RDONLY, 0 );
    
    if (!f)
        return SKError(err_lc_fopen, "[SKLCFile::Load] Could not open file %s", m_pszFileName);

    // get file size
	PRFileInfo info;
    PRStatus status = PR_GetOpenFileInfo(f, &info);
    if( status != PR_SUCCESS )
      return err_failure;

    lSize = info.size;

    // allocate buffer to hold file content
    m_szBuffer = (char *) PR_Malloc(lSize + 1);
    if (!m_szBuffer)
    {
        PR_Close(f);
        return SKError(err_lc_malloc, "[SKLCFile::Load] Failed to allocate %ld bytes", lSize);
    }

    // read file content
    PR_Read (f, m_szBuffer, lSize);
    PR_Close (f);
    m_szBuffer[lSize] = 0;

    // walk down the buffer and have
    // each page point to the min & max values
    for (char* s=m_szBuffer, *t=s; *s; s++)
    {
        // end-of-line flag
        bEOL = PR_FALSE;

        // tabs separate fields
        if (*s == '\t' || *s == EOL)
        {
            switch(iFieldNum++)
            {
            case 0:
                p.first_key = t;
                p.first_num = strtoul(t, NULL, 10);
                break;
            case 1:
                p.last_key = t;
                p.last_num = strtoul(t, NULL, 10);
                break;
            case 2:
                p.offset = strtoul(t, NULL, 10);
                break;
            case 3:
                p.size = strtoul(t, NULL, 10);
                break;
            }

            // terminate the field
            bEOL = (*s == EOL);
            *s = 0;
            t = s + 1;
        }

        if (bEOL)
        {
            // skip header lines
            if (++lLineCount >= m_iHeaderLines)
            {
                // allocate memory every N pages
                if (m_lPageCount % 10 == 0)
                {
                    lSize = (10 + m_lPageCount) * sizeof(LcPageRec);
                    m_Pages = (LcPagePtr) PR_Realloc (m_Pages, lSize);
                    if (!m_Pages)
                        return SKError(err_lc_malloc, "[SKLCFile::Load] Failed to allocate %ld bytes", lSize);
                }

                // copy page content
                memcpy (&m_Pages[m_lPageCount++], &p, sizeof(LcPageRec));
                memset (&p, 0, sizeof(LcPageRec));
                iFieldNum = 0;
            }
        }
    }

    m_bLoaded = PR_TRUE;

    return noErr;
}

//  -------------------------------------------------------------------
//
//  Lookup
//
//  -------------------------------------------------------------------

// *** HIGHLY CRITICAL CODE ***
// It looks for the LC page that may contain the search record.
//
// If there a page matches the request then lPageNum contains its number
// and noErr is returned.
//
// If there is no match page then it returns err_lc_notfound and
// lPageNum is the page containing the last record before
// ((PRUint32)-1 if none).
//
// If another error is returned then is is a critical system failure
// and lPageNum has an unknown value.

SKERR SKLCFile::Lookup(const char* pszKey, LookupType eLookup,
                       PRUint32 *plPageNum)
{
    PRUint32 ulPos, ulInf, ulSup;
    // lCmp is set to 1 so that the lookup work even if there is no pages.
    long lCmp = 1;

    if(!pszKey || (eLookup == STRING && !*pszKey))
        return SKError(err_lc_invalid, "[SKLCFile::Lookup] Invalid arguments");

    // load the file on first time through
    if(!m_bLoaded)
    {
        SKERR err = Load();
        if(err != noErr)
            return err;
    }
    SK_ASSERT(m_bLoaded);
    if(!m_Pages)
        return err_lc_notfound;

    // recherche dans la SKLCFile le plus grand mot
    // inferieur ou egal au mot cherche
    ulInf = ulPos = (PRUint32)-1;
    ulSup = m_lPageCount;
    while(ulSup > ulInf + 1)
    {
        ulPos = (ulInf + ulSup) / 2;
        lCmp = COMPARE_FIRST(pszKey, ulPos, eLookup);
        if(lCmp == 0)
            break;
        else if(lCmp > 0)
            ulInf = ulPos;
        else
            ulSup = ulPos;
    }
//    printf("ulInf = %u, ulSup = %u\n", ulInf, ulSup);

    if(lCmp < 0)
    {
        --ulPos;
        if(ulPos == (PRUint32)-1)
        {
            *plPageNum = ulPos;
            return err_lc_notfound;
        }
        else if(COMPARE_LAST(pszKey, ulPos, eLookup) > 0)
        {
            *plPageNum = ulPos;
            return err_lc_notfound;
        }
    }
    else if(lCmp > 0)
    {
        if(COMPARE_LAST(pszKey, ulPos, eLookup) > 0)
        {
            *plPageNum = ulPos;
            return err_lc_notfound;
        }
    }

    if(ulPos >= m_lPageCount)
        return err_lc_notfound;

    *plPageNum = ulPos;
    return noErr;
}

//  -------------------------------------------------------------------
//
//  Configure
//
//  -------------------------------------------------------------------
SKERR SKLCFile::ConfigureItem(char* szSection, char* szToken, char* szValue)
{
    // old format had an extra line at the top
    if (szToken && !PL_strcmp(szToken, "OLDFORMAT"))
    {
        m_iHeaderLines = MAKE_BOOL(sk_upper(szValue)) ? 1 : 0;
        return noErr;
    }

    return SKFile::ConfigureItem(szSection, szToken, szValue);
}

SKERR SKLCFile::Check()
{
    SKEnvir *pEnv = NULL;
    SKERR err = SKEnvir::GetEnvir(&pEnv);
    if(err != noErr)
        return err;

    char* pcOpenMode = NULL;
    err = pEnv->GetValue(SKF_BE_NATIVE_DEFAULT_OPENMODE_ENVIR_VAR, &pcOpenMode);
    if(err != noErr)
        return err;
    OpenMode iOpenMode = Standard;
    if(pcOpenMode && *pcOpenMode)
        iOpenMode = SKPageFile::StringToOpenMode(pcOpenMode);
    if(pcOpenMode)
        PL_strfree(pcOpenMode);

    if(iOpenMode == Premapped)
        return Load();
    else
        return noErr;
}

