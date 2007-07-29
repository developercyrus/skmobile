/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: unicodesimplifier.cpp,v 1.9.2.4 2005/02/28 14:03:52 krys Exp $
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

#include "../skcoreconfig.h"

#include "../machine.h"
#include "../error.h"
#include "../refcount.h"
#include "../skptr.h"

#include "../unichar.h"

#include "../simplifier.h"
#include "unicodesimplifier.h"

#include "unicode.cpp"

static inline PRUint32* CharLookup(PRUint32 lChar)
{
    PRUint32 lMin, lMax;
    lMin = 0;
    lMax = sizeof(g_piUnicode) / sizeof(PRUint32) / 3;
    if(g_piUnicode[0] == lChar)
        return g_piUnicode;
    while(lMax - lMin > 1)
    {
        PRUint32 lMid = (lMin + lMax) >> 1;
        if(g_piUnicode[lMid*3] == lChar)
            return &g_piUnicode[lMid*3];
        if(g_piUnicode[lMid*3] < lChar)
            lMin = lMid;
        else
            lMax = lMid;
    }
    return NULL;
}

inline PRUint32 skStringUnicodeSimplifier::SimpCharUniCase(PRUint32 lChar)
{
    PRUint32* pTable = CharLookup(lChar);
    if(!pTable)
        return lChar;
    return pTable[1];
}

inline PRUint32 skStringUnicodeSimplifier::SimpCharUniComp(PRUint32 lChar)
{
    PRUint32* pTable = CharLookup(lChar);
    if(!pTable)
        return lChar;
    return pTable[2];
}

inline PRUint32 skStringUnicodeSimplifier::SimpCharUniSpace(PRUint32 lChar)
{
    if(IsUnicodeSpace(lChar))
        return 0x20;
    return lChar;
}

SK_REFCOUNT_IMPL(skStringUnicodeSimplifier);
SK_REFCOUNT_IMPL_CREATOR(skStringUnicodeSimplifier)(const char* pszParam)
{
    return new skStringUnicodeSimplifier(pszParam);
}

PRInt32 compar(const void* a, const void* b)
{
    return (*(PRUint32*) a) - *((PRUint32*)b);
}

skStringUnicodeSimplifier::skStringUnicodeSimplifier(const char* pszParam)
    : m_pszParam(NULL), m_bSimpUniCase(PR_FALSE), m_bSimpUniComp(PR_FALSE),
    m_bSimpUniSpaces(PR_FALSE), m_bFrSloppyUppercaseMatch(PR_FALSE),
    m_iMapSize(0), m_iMapCount(0), m_pMap(NULL)
{
    m_pszParam = PL_strdup(pszParam);
    while(*pszParam)
    {
        if(!PL_strncmp(pszParam, "uni_case", 8))
            m_bSimpUniCase = PR_TRUE;
        if(!PL_strncmp(pszParam, "uni_comp", 8))
            m_bSimpUniComp = PR_TRUE;
        if(!PL_strncmp(pszParam, "uni_space", 9))
            m_bSimpUniSpaces = PR_TRUE;
        if(!PL_strncmp(pszParam, "fr_sloppy_uppercase_match", 25))
            m_bFrSloppyUppercaseMatch = PR_TRUE;
        if(!PL_strncmp(pszParam, "map:", 4))
        {
            pszParam += 4;
            while(*pszParam != ',' && *pszParam)
            {
                // Look for the :
                const char* pc = pszParam;
                while(*pc && *pc != ':') pc++;

                if(*pc == ':')
                {
                    pc++;
                    if (*pc)
                    {
                        PRUint32 iChar = strtoul(pszParam, NULL, 0);
                        // Eat delimiter
                        char cSep = *pc++;
                        const char* pcString = pc;
                        // Eat the string
                        while(*pc && *pc != cSep)
                            pc++;
                        if(m_iMapSize == m_iMapCount)
                        {
                            m_iMapSize = m_iMapSize ? m_iMapSize * 2 : 8;
                            m_pMap = (Mapping*) PR_Realloc(m_pMap,
                                m_iMapSize * sizeof(Mapping));
                        }
                        m_pMap[m_iMapCount].lChar = iChar;
                        m_pMap[m_iMapCount].pcString = PL_strndup(pcString,
                            pc - pcString);
                        m_iMapCount++;
                    }
                    pszParam = pc;
                    if(*pszParam)
                        pszParam++;
                }
            }
        }
        while(*pszParam != ',' && *pszParam)
            pszParam++;
        if(*pszParam == ',')
            pszParam++;
    }
    qsort(m_pMap, m_iMapCount, sizeof(Mapping), compar);
}

skStringUnicodeSimplifier::~skStringUnicodeSimplifier()
{
    if(m_pszParam)
        PL_strfree(m_pszParam);
    for(PRUint32 i = 0; i < m_iMapCount; i++)
        PL_strfree(m_pMap[i].pcString);
    if(m_pMap)
        PR_Free(m_pMap);
}

SKERR skStringUnicodeSimplifier::SimplifyFirstChar(
                        const char* pcIn, PRUint32 *piRead,
                        char* pcOut, PRUint32* piWritten)
{
    SKERR err;
    if(!*pcIn)
    {
        if(piRead)
            *piRead = 0;
        if(pcOut)
            *pcOut = 0;
        if(piWritten)
            *piWritten = 0;
        return noErr;
    }
    // Convert the first char to UCS4
    PRUint32 c;
    err = CharUTF8ToUCS4(&c, pcIn, piRead);
    if(err != noErr)
        return err;

    // Simplify
    if(m_bSimpUniCase)
        c = SimpCharUniCase(c);
    if(m_bSimpUniComp)
        c = SimpCharUniComp(c);
    if(m_bSimpUniSpaces)
        c = SimpCharUniSpace(c);

    char pcLocal[8];
    char *pcTmp = pcLocal;
    PRUint32 iLength = 8;
    pcTmp[0] = 0;

    if(m_pMap)
    {
        Mapping* value = (Mapping*) bsearch(&c, m_pMap, m_iMapCount,
                sizeof(Mapping), compar);
        if(value)
            pcTmp = value->pcString;
    }

    if(pcTmp != pcLocal) // we are remapped
        iLength = PL_strlen(pcTmp);
    else
    {
        err = CharUCS4ToUTF8(pcTmp, c, &iLength);
        if(err != noErr)
            return err;
    }

    if(piWritten && iLength > *piWritten)
        return err_memory;

    if(pcOut)
        for(PRUint32 i = 0; i < iLength; i++)
            pcOut[i] = pcTmp[i];

    if(piWritten)
        *piWritten = iLength;
    return noErr;
}

SKERR skStringUnicodeSimplifier::CompareFirstChar(
        const char* pc1, PRUint32* plLen1,
        const char* pc2, PRUint32* plLen2, PRInt32 *piCmp)
{
    if(m_bFrSloppyUppercaseMatch)
    {
        SKERR err;
        PRUint32 iL1, iL2;
        PRUint32 iC1, iC2;

        iL1 = 0;
        iL2 = 0;
        // Read UCS4 chars values
        err = CharUTF8ToUCS4(&iC1, pc1, &iL1);
        if(err != noErr)
            return err;

        err = CharUTF8ToUCS4(&iC2, pc2, &iL2);
        if(err != noErr)
            return err;

        // Find characters characteristics
        PRUint32* piInfo1 = CharLookup(iC1);
        PRUint32* piInfo2 = CharLookup(iC2);

        if(piInfo1 && piInfo2)
        {
            PRBool bStripped = PR_FALSE;
            if(    piInfo1[1] != piInfo1[0] /* char 1 is uppercase */
                && piInfo1[2] == piInfo1[0] /* char 1 has no accent */
                && piInfo2[2] != piInfo2[0] /* char 2 has accent */)
            {
                bStripped = 1;
                iC2 = SimpCharUniComp(iC2); // strip accent on 2
            }
            else
            if(    piInfo2[1] != piInfo2[0] /* char 2 is uppercase */
                && piInfo2[2] == piInfo2[0] /* char 2 has no accent */
                && piInfo1[2] != piInfo1[0] /* char 1 has accent */)
            {
                bStripped = 1;
                iC1 = SimpCharUniComp(iC1); // strip accent on 1
            }
            if(bStripped)
            {
                if(m_bSimpUniCase)
                {
                    iC1 = SimpCharUniCase(iC1);
                    iC2 = SimpCharUniCase(iC2);
                }
                if(piCmp)
                    *piCmp = iC1 - iC2;
                if(plLen1)
                    *plLen1 = iL1;
                if(plLen2)
                    *plLen2 = iL2;
                return noErr;
            }
        }
    }
    return skIStringSimplifier::CompareFirstChar(pc1,plLen1, pc2,plLen2, piCmp);
}

void skStringUnicodeSimplifier::ComputeIdentity()
{
    m_pcIdentity = PR_smprintf("simplifier:unicode:%s", m_pszParam);
}
