/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: tokenize.cpp,v 1.3.2.3 2005/02/17 15:29:20 krys Exp $
 *
 * Authors: Mathieu Poumeyrol <poumeyrol @at@ idm .dot. fr>
 *          Arnaud de Bossoreille de Ribou <debossoreille @at@ idm .dot. fr>
 *          Alexis Seigneurin <seigneurin @at@ idm .dot. fr>
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
#include "skptr.h"

#include "envir/envir.h"
#include "file/skfopen.h"
#include "file/file.h"

#include "stringlist.h"
#include "integerlist.h"

#include "unichar.h"
#include "tokenize.h"

//#define DEBUG_TOKENIZER

SK_REFCOUNT_IMPL_DEFAULT(skTokenize)

skTokenize::skTokenize()
{
    m_pHardSepList = NULL;
    m_pSoftSepList = NULL;
    m_pSoftSepExcList = NULL;
    m_bInitialized = PR_FALSE;
}

skTokenize::~skTokenize()
{
    m_pHardSepList = NULL;
    m_pSoftSepList = NULL;
    m_pSoftSepExcList = NULL;
    m_bInitialized = PR_FALSE;
}

SKERR skTokenize::Init( SKIntegerList* pHardSepList,
                        SKIntegerList* pSoftSepList,
                        SKStringList* pSoftSepExcList,
                        PRBool bEntityAware)
{
    SKERR err = noErr;
    if(pHardSepList)
       m_pHardSepList = pHardSepList;
    else
    {
        *m_pHardSepList.already_AddRefed() = sk_CreateInstance(SKIntegerList)();
        if(!m_pHardSepList)
            return err_memory;

        err = m_pHardSepList->SetListFromUTF8String(
                        "!#()+,./:;<>^_`[]{|}~- \n\r\t\xc2\xa0");
        if(err != noErr)
            return err;
    }
    if(pSoftSepList)
        m_pSoftSepList = pSoftSepList;
    else
    {
        *m_pSoftSepList.already_AddRefed() = sk_CreateInstance(SKIntegerList)();
        if(!m_pSoftSepList)
            return err_memory;

        err = m_pSoftSepList->SetListFromUTF8String("'");
        if(err != noErr)
            return err;
    }
    if(pSoftSepExcList)
        m_pSoftSepExcList = pSoftSepExcList;
    else
    {
        *m_pSoftSepExcList.already_AddRefed()
            = sk_CreateInstance(SKStringList)();
        if(!m_pSoftSepExcList)
            return err_memory;
        err = m_pSoftSepExcList->SetListWithOneWord("aujourd'hui");
        if(err != noErr)
            return err;
    }

    m_bEntityAware = bEntityAware;
    m_bInitialized = PR_TRUE;
    return noErr;
}

SKERR skTokenize::NextEndOfSpace(   PRUint32* piUCS4String,
                                    PRUint32 iMaxLength,
                                    PRUint32 *piResult)
{
    PRUint32 iLength = 0;

    if (!m_pSoftSepExcList || !m_pSoftSepExcList->GetCount())
    {
        // Faster scan
        PRBool bResultH = PR_FALSE;
        PRBool bResultS = PR_FALSE;
        while( (!iMaxLength || iLength < iMaxLength)
                && piUCS4String[iLength]
                && m_pHardSepList->IsPresent(piUCS4String[iLength],&bResultH)
                    == noErr
                && m_pSoftSepList->IsPresent(piUCS4String[iLength],&bResultS)
                    == noErr
                && (bResultH || bResultS) )
            iLength++;
    }
    else
    {
        PRUint32 iScanPos = (PRUint32) -1;
        PRBool bException;

#define _CheckException(iPosStart, iPosEnd, bException)                     \
{                                                                           \
    char* p;                                                                \
    UCS4LenToNewUTF8(&p, piUCS4String + iPosStart, iPosEnd - iPosStart);    \
    SKERR err = m_pSoftSepExcList->IsPresent(p, &bException);               \
    PR_Free (p);                                                            \
    if (err != noErr)                                                       \
        bException = PR_FALSE;                                              \
}

        while((!iMaxLength || iLength < iMaxLength) && piUCS4String[iLength])
        {
            PRBool bH, bS;
            SKERR err = m_pHardSepList->IsPresent(piUCS4String[iLength], &bH);
            if (err != noErr)
                break;

            if (iScanPos == (PRUint32) -1)
            {
                if (!bH)
                {
                    if ((err = m_pSoftSepList->IsPresent(piUCS4String[iLength],
                        &bS)) != noErr)
                        break;

                    if (bS)
                        // We may be in the following case :
                        // `HHHHSSHHH' where H's are hard separators,
                        // S's are soft separators, and `SSS' is an exception.
                        // In that case, we want to stop at the first S.
                        iScanPos = iLength;
                    else
                        // not a separator : leaving the loop
                        break;
                }
            }
            else
            {
                if (bH)
                {
                    // iLength points to last S.
                    _CheckException (iScanPos, iLength, bException);
                    if (bException)
                    {
                        // `SSS' is an exception, the next word starts at
                        // the first S.
                        iLength = iScanPos;
                        iScanPos = (PRUint32) -1;
                        break;
                    }

                    // `SSS' is not an exception : ignore the sequence and
                    // continue the scanning
                    iScanPos = (PRUint32) -1;
                }
                else
                {
                    if ((err = m_pSoftSepList->IsPresent(piUCS4String[iLength],
                        &bS)) != noErr)
                        break;

                    if (!bS)
                    {
                        // Not a separator. Before leaving the loop,
                        // find out the start of the next word.
                        _CheckException (iScanPos, iLength, bException);
                        if (bException)
                            // The next word starts at the first S.
                            iLength = iScanPos;
                            // (else, it starts at the current character)

                        iScanPos = (PRUint32) -1;
                        break;
                    }
                }
            }

            iLength++;
        }

        if (iScanPos != (PRUint32) -1)
        {
            // We end up here if the sequence of separators ended with :
            // `HHHHSSS', so we still need to check for exceptions.
            _CheckException (iScanPos, iLength, bException);
            if (bException)
                iLength = iScanPos;
        }

#undef _CheckException
    }

    *piResult = iLength;
    return noErr;
}

SKERR skTokenize::NextEndOfWord(PRUint32* piUCS4String, PRUint32 iMaxLength,
                                PRUint32 *piResult)
{
    PRUint32 iLength = 0;
    PRUint32* piFirstSoftSep = NULL;

    char* pUTF8String;

    while( (!iMaxLength || iLength < iMaxLength) && piUCS4String[iLength] )
    {
        PRBool bResult;
        SKERR err;

#ifdef DEBUG_TOKENIZER
        char* pcTmp;
        UCS4LenToNewUTF8(&pcTmp, piUCS4String+iLength, iMaxLength-iLength);
        if(PL_strlen(pcTmp) > 20)
            pcTmp[19] = 0;
        printf("tokenize: %s\n", pcTmp);
        UTF8Free(pcTmp);
#endif
        // Try to keep entities together
        if(m_bEntityAware && piUCS4String[iLength] == '&')
        {
            PRUint32 iLength2 = iLength;
            while(iLength2 < iMaxLength && iLength2 - iLength < 8
                    && piUCS4String[iLength2] != ';'
                    && piUCS4String[iLength2] != 0) iLength2++;
            if(iLength2 < iMaxLength && piUCS4String[iLength2] == ';')
                iLength = iLength2;
        }
        else
        {
            if((err = m_pHardSepList->IsPresent(piUCS4String[iLength],&bResult))
                != noErr)
                    return err;
            if(bResult)
                break;

            PRBool bHasSoft;
            if((err = m_pSoftSepList->IsPresent(piUCS4String[iLength],
                            &bHasSoft))
                != noErr)
                    return err;

            if(bHasSoft)
            {
                if (piFirstSoftSep + 1 == piUCS4String + iLength)
                {
                    // sequence of soft separators
                    // read them all, and do not check for exceptions
                    // at this point

                    while( (!iMaxLength || iLength < iMaxLength) &&
                        piUCS4String[iLength] )
                    {
                        err = m_pSoftSepList->IsPresent(piUCS4String[iLength],
                            &bHasSoft);
                        if (err != noErr)
                            return err;
                        if (!bHasSoft)
                            break;
                        iLength++;
                    }
                    continue;
                }
                else
                {
                    if(piFirstSoftSep)
                        break;

                    piFirstSoftSep = piUCS4String + iLength;
                }
            }
        }
        iLength++;
    }

    if(piFirstSoftSep)
    {
        UCS4LenToNewUTF8(&pUTF8String, piUCS4String, iLength);

        PRBool bException;
        SKERR err;
        if((err = m_pSoftSepExcList->IsPresent(pUTF8String, &bException)) != noErr)
            return err;

        if (!bException)
        // cut at first soft separator
            iLength = piFirstSoftSep - piUCS4String;
        // else cut at second soft separator
        PR_Free (pUTF8String);
    }

    *piResult = iLength;
    return noErr;
}
