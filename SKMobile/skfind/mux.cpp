/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: mux.cpp,v 1.4.2.4 2005/02/21 14:22:44 krys Exp $
 *
 * Authors: Arnaud de Bossoreille de Ribou <debossoreille @at@ idm .dot. fr>
 *          Mathieu Poumeyrol <poumeyrol @at@ idm .dot. fr>
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

#include <skcore/skcore.h>

#include "sktable.h"

#include "cursor.h"
#include "cursorcomparator.h"
#include "recordcomparator.h"

#include "mux.h"

SKMux::SKMux()
{
    m_ppiData = NULL;
    m_piCount = NULL;
    m_piPosition = NULL;
    m_piInputData = NULL;
    m_piSortedIndexes = NULL;
    m_piFinalData = NULL;
    m_pAncillary = NULL;
    m_pbKept = NULL;
}

SKMux::~SKMux()
{
    if(m_ppiData)
        delete[] m_ppiData;
    if(m_piCount)
        delete[] m_piCount;
    if(m_piPosition)
        delete[] m_piPosition;
    if(m_piInputData)
        delete[] m_piInputData;
    if(m_piSortedIndexes)
        delete[] m_piSortedIndexes;
    if(m_piFinalData)
        delete[] m_piFinalData;
    if(m_pbKept)
        delete[] m_pbKept;
    if(m_pAncillary)
        PR_Free(m_pAncillary);
}

SKERR SKMux::MuxCursors(SKCursor **ppCursors, PRUint32 iCount,
                        SKICursorComparator *pComparator,
                        PRBool *pbInterrupt)
{
    SKERR err = noErr;
    if(!iCount)
        return noErr;
    skPtr<SKCursor> pTmp;
    for(PRUint32 i = 0; i<iCount; i++)
    {
        if(i==0)
        {
            err = ppCursors[0]->Clone(pTmp.already_AddRefed());
            if(err != noErr)
                return err;
            err = pTmp->ComputeBitmapForm();
            if(err != noErr)
                return err;
        }
        else
        {
            err = pTmp->Merge(ppCursors[i], skfopOR);
            if(err != noErr)
                return err;
        }
        if(pbInterrupt && *pbInterrupt)
            return err_interrupted;
    }
    err = pTmp->GetCount(&m_iFinalCount);
    if(err != noErr)
        return err;
    m_piFinalData = (PRUint32*) PR_Malloc(m_iFinalCount * sizeof(PRUint32));
    if(!m_piFinalData)
        return err_memory;
    memcpy(m_piFinalData, pTmp->GetSharedCursorDataRead(), 
            m_iFinalCount * sizeof(PRUint32));
    return noErr;
}


SKERR SKMux::MuxCursorsWithAncillary(
        SKCursor **ppCursors, PRUint32 iWidth,
        PRUint32 lAncillaryItemSize,
        SKIMuxAncillaryCallback* pCallback,
        SKICursorComparator *pComparator,
        PRBool *pbInterrupt)
{
    if(iWidth == 0)
    {
        m_iFinalCount = 0;
        return noErr;
    }
    else if(iWidth == 1)
    {
        SKERR err = ppCursors[0]->GetCount(&m_iFinalCount);
        if(err != noErr)
        {
            m_iFinalCount = 0;
            return err;
        }
        else if(!m_iFinalCount)
            return noErr;

        m_piFinalData = new PRUint32[m_iFinalCount];
        if(!m_piFinalData)
        {
            m_iFinalCount = 0;
            return err_memory;
        }
        m_pAncillary = (void*) PR_Malloc(m_iFinalCount * lAncillaryItemSize);
        if(!m_pAncillary)
        {
            m_iFinalCount = 0;
            return err_memory;
        }

        err = ppCursors[0]->ComputeCursorForm();
        if (err != noErr)
            return err;
        const PRUint32 *piData = ppCursors[0]->GetSharedCursorDataRead();
        SK_ASSERT(NULL != piData);
        if(!piData)
            return err_failure;

        memcpy(m_piFinalData, piData, m_iFinalCount * sizeof(PRUint32));
        PRBool bTrue = PR_TRUE;
        if(lAncillaryItemSize && pCallback)
        {
            for(PRUint32 i = 0; i<m_iFinalCount; i++)
            {
                err = pCallback->Compute(m_piFinalData[i], &i, &bTrue,
                        (char*)m_pAncillary + i*lAncillaryItemSize);
                if(err != noErr)
                    return err;
            }
        }
        return noErr;
    }

    SK_ASSERT(iWidth >= 2);

    // 1/ Preparation
    m_ppiData = new const PRUint32 *[iWidth];
    m_piCount = new PRUint32[iWidth];
    m_piPosition = new PRUint32[iWidth];
    m_piInputData = new PRUint32[iWidth];
    m_piSortedIndexes = new PRUint32[iWidth];
    m_pbKept = new PRBool[iWidth];
    if(    !m_ppiData || !m_piCount | !m_piPosition || !m_piInputData
        || !m_piSortedIndexes)
        return err_memory;

    SKERR err;

    m_iFinalCount = 0;

    for(PRUint32 i = 0; i < iWidth; ++i)
    {
        err = ppCursors[i]->ComputeCursorForm();
        if (err != noErr)
            return err;
        m_ppiData[i] = ppCursors[i]->GetSharedCursorDataRead();
        err = ppCursors[i]->GetCount(m_piCount + i);
        if(err != noErr)
            return err;
        if(m_piCount[i] && !m_ppiData[i])
            return err_failure;
        m_piPosition[i] = 0;

        m_iFinalCount += m_piCount[i];
    }

    m_piFinalData = new PRUint32[m_iFinalCount];
    if(!m_piFinalData)
        return err_memory;

    m_pAncillary = (void*) PR_Malloc(m_iFinalCount * lAncillaryItemSize);
    if(!m_pAncillary)
    {
        m_iFinalCount = 0;
        return err_memory;
    }

    m_iFinalCount = 0;

    m_iSortedWidth = 0;

    for(PRUint32 j = 0; j < iWidth; ++j)
    {
        if(m_piPosition[j] < m_piCount[j])
        {
            if(pComparator)
            {
                err = InsertIndexWithComparator(j, pComparator);
                if(err != noErr)
                    return err;
            }
            else
                InsertIndexWithoutComparator(j);
        }
        if(pbInterrupt && *pbInterrupt)
            return err_interrupted;
    }

    // 2/ Merge
    while(m_iSortedWidth)
    {
        // Check interruption flag
        if(pbInterrupt && *pbInterrupt)
            return err_interrupted;

        PRUint32 iIndex = m_piSortedIndexes[--m_iSortedWidth];
        SK_ASSERT(iIndex < iWidth);

        m_piFinalData[m_iFinalCount] = m_piInputData[iIndex];
        if(lAncillaryItemSize && pCallback)
        {
            for(PRUint32 k = 0; k<iWidth; k++)
                m_pbKept[k] = m_piInputData[k] == m_piInputData[iIndex];
            err = pCallback->Compute(m_piInputData[iIndex],
                    m_piPosition, m_pbKept,
                    (char*) m_pAncillary + m_iFinalCount*lAncillaryItemSize);
            if(err != noErr)
                return err;
        }
        m_iFinalCount++;

        if(m_piPosition[iIndex] < m_piCount[iIndex])
        {
            if(pComparator)
            {
                err = InsertIndexWithComparator(iIndex, pComparator);
                if(err != noErr)
                    return err;
            }
            else
                InsertIndexWithoutComparator(iIndex);
        }
    }

    return noErr;
}

SKERR SKMux::RetrieveDataWithAncillary(PRUint32 *piCount, PRUint32 **ppiData, 
        void** ppAncillaryData)
{
    SK_ASSERT(NULL != piCount);
    SK_ASSERT(NULL != ppiData);

    if(m_iFinalCount && !m_piFinalData)
        return err_failure;

    *piCount = m_iFinalCount;
    *ppiData = m_piFinalData;
    if(ppAncillaryData)
        *ppAncillaryData = m_pAncillary;
    else
        PR_Free(m_pAncillary);

    m_piFinalData = NULL;
    m_iFinalCount = 0;
    m_pAncillary = NULL;

    return noErr;
}

void SKMux::InsertIndexWithoutComparator(PRUint32 iIndex)
{
    SK_ASSERT(m_piPosition[iIndex] < m_piCount[iIndex]);
    m_piInputData[iIndex] = m_ppiData[iIndex][m_piPosition[iIndex]++];

    PRUint32 iInf = (PRUint32)-1;
    PRUint32 iSup = m_iSortedWidth;
    PRUint32 iIndexValue = m_piInputData[iIndex];
    while(iSup > iInf + 1)
    {
        PRUint32 iPos = (iSup + iInf) >> 1;
        PRUint32 iPosValue = m_piInputData[m_piSortedIndexes[iPos]];
        if(iPosValue == iIndexValue)
        {
            if(m_piPosition[iIndex] >= m_piCount[iIndex])
                return;

            iIndexValue = m_piInputData[iIndex]
                        = m_ppiData[iIndex][m_piPosition[iIndex]++];
            iInf = (PRUint32)-1;
            iSup = iPos + 1;
        }
        else if(iPosValue < iIndexValue)
            iSup = iPos;
        else
            iInf = iPos;
    }

    SK_ASSERT(iInf + 1 == iSup);

    memmove(m_piSortedIndexes + iSup + 1,
            m_piSortedIndexes + iSup,
            (m_iSortedWidth - iSup) * sizeof(PRUint32));

    m_piSortedIndexes[iSup] = iIndex;

    m_iSortedWidth++;

    return;
}

SKERR SKMux::InsertIndexWithComparator(PRUint32 iIndex,
                                             SKICursorComparator *pComparator)
{
    SK_ASSERT(m_piPosition[iIndex] < m_piCount[iIndex]);
    m_piInputData[iIndex] = m_ppiData[iIndex][m_piPosition[iIndex]++];

    PRUint32 iInf = (PRUint32)-1;
    PRUint32 iSup = m_iSortedWidth;
    while(iSup > iInf + 1)
    {
        PRUint32 iPos = (iSup + iInf) >> 1;
        PRInt32 iCmp = 0;
        SKERR err = pComparator->CompareRanks(
                        m_piInputData[m_piSortedIndexes[iPos]],
                        m_piInputData[iIndex],
                        &iCmp);
        if(err != noErr)
            return err;

        if(iCmp == 0)
        {
            if(m_piPosition[iIndex] >= m_piCount[iIndex])
                return noErr;

            m_piInputData[iIndex] = m_ppiData[iIndex][m_piPosition[iIndex]++];
            iInf = (PRUint32)-1;
            iSup = iPos + 1;
        }
        else if(iCmp < 0)
            iSup = iPos;
        else
            iInf = iPos;
    }

    SK_ASSERT(iInf + 1 == iSup);

    memmove(m_piSortedIndexes + iSup + 1,
            m_piSortedIndexes + iSup,
            (m_iSortedWidth - iSup) * sizeof(PRUint32));

    m_piSortedIndexes[iSup] = iIndex;

    m_iSortedWidth++;

    return noErr;
}

