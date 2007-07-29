/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: filterhtml.cpp,v 1.11.4.3 2005/02/21 14:22:46 krys Exp $
 *
 * Authors: Marc Ariberti <ariberti @at@ idm .dot. fr>
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

#include "filterhtml.h"

SK_REFCOUNT_IMPL_DEFAULT(SKRecordFilterHtml)
SK_REFCOUNT_IMPL_IID(SKRecordFilterHtml, SK_SKRECORDFILTERHTML_IID,
                     SKIRecordFilter)

SKRecordFilterHtml::SKRecordFilterHtml() : m_pszStructureFieldName(NULL),
                                           m_bTitle(PR_TRUE),
                                           m_iTitleLevel(7)
{
}

SKRecordFilterHtml::~SKRecordFilterHtml()
{
    if(m_pszStructureFieldName)
        PL_strfree(m_pszStructureFieldName);
}

SKERR SKRecordFilterHtml::SetStructureFieldName(
                const char *pszStructureFieldName)
{
    if(!pszStructureFieldName)
        return err_invalid;
    m_pStructureField = NULL;

    if(m_pszStructureFieldName)
        PL_strfree(m_pszStructureFieldName);

    m_pszStructureFieldName = PL_strdup(pszStructureFieldName);
    if(!m_pszStructureFieldName)
        return err_memory;

    return noErr;
}

SKERR SKRecordFilterHtml::SetCheckMainTitle(PRBool bTitle)
{
    m_bTitle = bTitle;
    return noErr;
}

SKERR SKRecordFilterHtml::SetCheckTitleLevel(PRUint32 iTitleLevel)
{
    m_iTitleLevel = iTitleLevel;
    return noErr;
}

SKERR SKRecordFilterHtml::SetRecordSet(SKIRecordSet* pRecordSet)
{
    if(!m_pszStructureFieldName)
        return err_failure;

    SKERR err;

    skPtr<SKIFldCollection> pCol;
    err = pRecordSet->GetFldCollection(pCol.already_AddRefed());
    if(err != noErr)
        return err;

    err = pCol->GetField(m_pszStructureFieldName,
                         m_pStructureField.already_AddRefed());
    if(err != noErr)
        return err;

    return SKIRecordFilter::SetRecordSet(pRecordSet);
}

SKERR SKRecordFilterHtml::CheckRecord(SKIRecord *pRecord, PRBool *pbKeepIt)
{
    SKERR err;

    PRUint32 lValue;
    err = pRecord->GetUNumFieldValue(m_pStructureField, &lValue);
    if(err != noErr)
        return err;

    *pbKeepIt = PR_FALSE;

    // gets the 2-4 bits corresponding to the title level
    lValue = (lValue >> 2) & 7;

    if( m_bTitle && (lValue == 0))
    {
        *pbKeepIt = PR_TRUE;
    }
    if ( (lValue <= m_iTitleLevel) && (lValue != 0) )
    {
        *pbKeepIt = PR_TRUE;
    }
    return noErr;
}

