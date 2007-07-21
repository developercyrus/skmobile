/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: recordfilter.h,v 1.2.4.4 2005/02/21 14:22:44 krys Exp $
 *
 * Authors: Arnaud de Bossoreille de Ribou <debossoreille @at@ idm .dot. fr>
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

#ifndef __SKF_RECORDFILTER_H_
#define __SKF_RECORDFILTER_H_

// SKIRecordFilter

#define SK_SKIRECORDFILTER_IID                                          \
{ 0x890d318d, 0xf0b7, 0x4e99,                                           \
    { 0xa9, 0xac, 0x60, 0x53, 0xf3, 0x28, 0x80, 0x28 } }

class SKAPI SKIRecordFilter : public SKRefCount
{
public:
    SK_REFCOUNT_INTF_IID(SKIRecordFilter, SK_SKIRECORDFILTER_IID)

    virtual SKERR Reset();
    virtual SKERR CheckRecord(SKIRecord *pRecord, PRBool* pbKeepIt) = 0;

    virtual SKERR SetRecordSet(SKIRecordSet *pRecordSet);
    SKERR GetRecordSet(SKIRecordSet **ppRecordSet);

protected:
    skPtr<SKIRecordSet> m_pRecordSet;
};

SK_REFCOUNT_DECLARE_INTERFACE(SKIRecordFilter, SK_SKIRECORDFILTER_IID)

// SKRecordFilterJoinRSOnUNum

#define SK_SKRECORDFILTERJOINRSONUNUM_IID                               \
{ 0xc84d2518, 0x90f0, 0x490a,                                           \
    { 0x88, 0x25, 0x9f, 0xab, 0x85, 0xc8, 0xb7, 0x0a } }

class SKAPI SKRecordFilterJoinRSOnUNum : public SKIRecordFilter
{
public:
    SK_REFCOUNT_INTF_DEFAULT(SKRecordFilterJoinRSOnUNum)
    SK_REFCOUNT_INTF_IID(SKRecordFilterJoinRSOnUNum,
                         SK_SKRECORDFILTERJOINRSONUNUM_IID)

    SKRecordFilterJoinRSOnUNum();
    ~SKRecordFilterJoinRSOnUNum();

    SKERR SetFieldName(const char *pszFieldName);

    SKERR SetFilterData(SKCursor *pFilterCursor,
                        SKIRecordSet *pFilterRecordSet,
                        const char *pszFilterFieldName);

    virtual SKERR SetRecordSet(SKIRecordSet *pRecordSet);

    virtual SKERR Reset();
    virtual SKERR CheckRecord(SKIRecord *pRecord, PRBool* pbKeepIt);

private:
    char                *m_pszFieldName;
    skPtr<SKIField>     m_pField;

    skPtr<SKCursor>     m_pFilterCursor;
    skPtr<SKIRecordSet> m_pFilterRecordSet;
    skPtr<SKIField>     m_pFilterField;
    PRUint32            m_iLastRank;
};

SK_REFCOUNT_DECLARE_INTERFACE(SKRecordFilterJoinRSOnUNum,
                              SK_SKRECORDFILTERJOINRSONUNUM_IID)

// SKRecordFilterUNumBitField

struct BitFieldRule {
    PRBool bAction;
    PRUint32 iBitShift;
    PRUint32 iBitMask;
    PRUint32 iMin;
    PRUint32 iMax;
};

#define SK_SKRECORDFILTERUNUMBITFIELD_IID                               \
{ 0xf5586dcc, 0xc076, 0x407d,                                           \
    { 0x90, 0x1a, 0x47, 0x45, 0x15, 0x59, 0x2f, 0x15 } }

class SKAPI SKRecordFilterUNumBitField : public SKIRecordFilter
{
public:
    SK_REFCOUNT_INTF_DEFAULT(SKRecordFilterUNumBitField)
    SK_REFCOUNT_INTF_IID(SKRecordFilterUNumBitField,
                         SK_SKRECORDFILTERUNUMBITFIELD_IID)

    SKRecordFilterUNumBitField();
    ~SKRecordFilterUNumBitField();

    // this enables to use a "second level" recordset
    SKERR SetOptionalLink(const char *pszLinkFieldName,
                          SKIRecordSet *pLinkRecordSet,
                          PRBool bNoLinkLookup);

    // this is mandatory and is the field we are interested in
    SKERR SetField(const char *pszFieldName);

    SKERR Clear();
    SKERR SetDefaultPolicy(PRBool bPolicy);

    SKERR AddRule(PRBool bAction, PRUint32 iBitShift, PRUint32 iBitMask,
                  PRUint32 iMinValue, PRUint32 iMaxValue);
    SKERR AddAcceptedValue(PRUint32 value);
    SKERR AddAcceptedRange(PRUint32 min, PRUint32 max);
    SKERR AddRejectedValue(PRUint32 value);
    SKERR AddRejectedRange(PRUint32 min, PRUint32 max);

    virtual SKERR CheckRecord(SKIRecord *pRecord, PRBool *pbKeepIt);

    virtual SKERR SetRecordSet(SKIRecordSet *pRecordSet);

private:
    char *              m_pszLinkFieldName;
    skPtr<SKIField>     m_pLinkField;
    skPtr<SKIRecordSet> m_pLinkRecordSet;
    PRBool              m_bNoLinkLookup;

    char *              m_pszFieldName;
    skPtr<SKIField>     m_pField;

    PRBool              m_bDefaultPolicy;
    PRUint32            m_iRuleSize;
    PRUint32            m_iRuleCount;
    BitFieldRule *      m_pRules;
};

SK_REFCOUNT_DECLARE_INTERFACE(SKRecordFilterUNumBitField,
                              SK_SKRECORDFILTERUNUMBITFIELD_IID)

#define SK_SKFRECORDFILTERUNUMINCURSOR_IID                              \
{ 0x343ef156, 0xd5b3, 0x4a44,                                           \
    { 0xa1, 0x0f, 0xc8, 0x0b, 0x67, 0xf1, 0x31, 0xb8 } }

class SKAPI skfRecordFilterUNumInCursor : public SKIRecordFilter
{
public:
    SK_REFCOUNT_INTF_DEFAULT(skfRecordFilterUNumInCursor)
    SK_REFCOUNT_INTF_IID(skfRecordFilterUNumInCursor,
                         SK_SKFRECORDFILTERUNUMINCURSOR_IID)

    skfRecordFilterUNumInCursor() {};
    ~skfRecordFilterUNumInCursor() {};

    SKERR SetField(SKIField* pField) { m_pField = pField; return noErr; }
    SKERR SetCursor(SKCursor* pCursor);
    
    virtual SKERR CheckRecord(SKIRecord *pRecord, PRBool *pbKeepIt);
private:
    skPtr<SKIField>     m_pField;
    skPtr<SKCursor>     m_pCursor;
};

SK_REFCOUNT_DECLARE_INTERFACE(skfRecordFilterUNumInCursor,
                              SK_SKFRECORDFILTERUNUMINCURSOR_IID)

#else
#error "Multiple inclusions of recordfilter.h"
#endif

