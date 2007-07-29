/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: hiliter.h,v 1.3.8.5 2005/02/21 14:22:44 krys Exp $
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

#ifndef __HILITER_H_
#define __HILITER_H_

class SKAPI SKHiliter : public SKIFilter
{
public:
    SK_REFCOUNT_INTF_DEFAULT(SKHiliter)
    SKHiliter()
    {
        m_pValues = NULL;
        m_lValuesCount = 0;
        m_pszStartFrom = m_pszStartTo = m_pszStopFrom = m_pszStopTo = NULL;
        m_iStartOccPos = m_iStopOccPos = -1;
        m_lStartFromLen = m_lStartToLen = m_lStopFromLen = m_lStopToLen = 0;
    }
    virtual ~SKHiliter();

    SKERR SetValues(const PRUint32* pValues, PRUint32 lValuesCount);
    SKERR SetStartStrings(const char* pszStartFrom, const char* pszStartTo,
                          PRInt32 iStartOccPos);
    SKERR SetStopStrings(const char* pszStopFrom, const char* pszStopTo,
                         PRInt32 iStopOccPos);

    virtual SKERR FilterData(const void* pData, PRUint32 lSize);

private:
    PRUint32*   m_pValues;
    PRUint32    m_lValuesCount;

    char*       m_pszStartFrom;
    char*       m_pszStartTo;
    PRInt32     m_iStartOccPos;
    char*       m_pszStopFrom;
    char*       m_pszStopTo;
    PRInt32     m_iStopOccPos;

    PRUint32    m_lStartFromLen;
    PRUint32    m_lStartToLen;
    PRUint32    m_lStopFromLen;
    PRUint32    m_lStopToLen;
};

SKERR SKAPI skFilterHilite(SKBinary *pContent, SKCursor *pPositions,
                           const char *pwStartTag, const char *pwStopTag,
                           SKBinary **ppTo);

#else // __HILITER_H_
#error "Multiple inclusions of hiliter.h"
#endif // __HILITER_H_

