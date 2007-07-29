/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: recordcomparator.h,v 1.4.2.4 2005/02/21 14:22:44 krys Exp $
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

#ifndef __SKF_RECORDCOMPARATOR_H_
#define __SKF_RECORDCOMPARATOR_H_

// SKIRecordComparator

#define SK_SKIRECORDCOMPARATOR_IID                                      \
{ 0x45f96720, 0x530a, 0x4893,                                           \
    { 0xa5, 0x5a, 0x69, 0x31, 0x37, 0x49, 0x8e, 0x05 } }

class SKAPI SKIRecordComparator : public SKRefCount
{
public:
    SK_REFCOUNT_INTF_IID(SKIRecordComparator, SK_SKIRECORDCOMPARATOR_IID)

    SKIRecordComparator();

    virtual SKERR CompareRecords(SKIRecord *pRecord1, SKIRecord *pRecord2,
                                 PRInt32 *piResult) = 0;

    virtual SKERR SetRecordSet(SKIRecordSet *pRecordSet);
    SKERR GetRecordComparatorID(SKBinary** ppStrID);
    SKERR GetRecordSet(SKIRecordSet **ppRecordSet);
    SKERR SetSortingOrder(skfSortingOrder sortingOrder);

protected:
    virtual SKERR   GetMyComparatorID(SKBinary** ppStrID) = 0;

    skPtr<SKIRecordSet> m_pRecordSet;
    skfSortingOrder m_sortingOrder;
};

SK_REFCOUNT_DECLARE_INTERFACE(SKIRecordComparator, SK_SKIRECORDCOMPARATOR_IID);

// SKRecordComparatorBaseField

#define SK_SKRECORDCOMPARATORBASEFIELD_IID                              \
{ 0xcae44174, 0xdfe2, 0x4e2f,                                           \
    { 0xbb, 0x77, 0x8d, 0xd6, 0x25, 0xc3, 0xb5, 0x31 } }

class SKAPI SKRecordComparatorBaseField : public SKIRecordComparator
{
public:
    SK_REFCOUNT_INTF_IID(SKRecordComparatorBaseField,
                         SK_SKRECORDCOMPARATORBASEFIELD_IID)

    SKRecordComparatorBaseField();
    ~SKRecordComparatorBaseField();

    // this enables to use a "second level" recordset
    SKERR SetOptionalLink(const char *pszLinkFieldName,
                          SKIRecordSet *pLinkRecordSet,
                          PRBool bNoLinkLookup);

    // this is mandatory and is the field we are interested in
    SKERR SetField(const char *pszFieldName);

    virtual SKERR SetRecordSet(SKIRecordSet *pRecordSet);

protected:
    virtual SKERR   GetMyComparatorID(SKBinary** ppStrID) = 0;
    SKERR           GetResolvedRecord(SKIRecord** ppRecord);

    char *              m_pszLinkFieldName;
    skPtr<SKIField>     m_pLinkField;
    skPtr<SKIRecordSet> m_pLinkRecordSet;
    PRBool              m_bNoLinkLookup;

    char *              m_pszFieldName;
    skPtr<SKIField>     m_pField;
};

SK_REFCOUNT_DECLARE_INTERFACE(SKRecordComparatorBaseField,
                              SK_SKRECORDCOMPARATORBASEFIELD_IID)

// SKRecordComparatorUNumField

#define SK_SKRECORDCOMPARATORUNUMFIELD_IID                              \
{ 0x63d50c8f, 0x12bb, 0x4e4d,                                           \
    { 0x89, 0x61, 0xb5, 0x5a, 0xa8, 0xa5, 0x46, 0x6c } }

class SKAPI SKRecordComparatorUNumField : public SKRecordComparatorBaseField
{
public:
    SK_REFCOUNT_INTF_DEFAULT(SKRecordComparatorUNumField)
    SK_REFCOUNT_INTF_IID(SKRecordComparatorUNumField,
                         SK_SKRECORDCOMPARATORUNUMFIELD_IID)

    virtual SKERR CompareRecords(SKIRecord *pRecord1, SKIRecord *pRecord2,
                                 PRInt32 *piResult);

protected:
    virtual SKERR       GetMyComparatorID(SKBinary** ppStrID);

private:
    SKERR GetValue(SKIRecord *pRecord, PRUint32 *piValue);
};

SK_REFCOUNT_DECLARE_INTERFACE(SKRecordComparatorUNumField,
                              SK_SKRECORDCOMPARATORUNUMFIELD_IID)

// SKRecordComparatorSNumField

#define SK_SKRECORDCOMPARATORSNUMFIELD_IID                              \
{ 0xf9b900e0, 0x5f38, 0x46f6,                                           \
    { 0xb9, 0x8b, 0x07, 0xcd, 0xab, 0xc7, 0x34, 0xd8 } }

class SKAPI SKRecordComparatorSNumField : public SKRecordComparatorBaseField
{
public:
    SK_REFCOUNT_INTF_DEFAULT(SKRecordComparatorSNumField)
    SK_REFCOUNT_INTF_IID(SKRecordComparatorSNumField,
                         SK_SKRECORDCOMPARATORSNUMFIELD_IID)

    virtual SKERR CompareRecords(SKIRecord *pRecord1, SKIRecord *pRecord2,
                                 PRInt32 *piResult);

protected:
    virtual SKERR       GetMyComparatorID(SKBinary** ppStrID);

private:
    SKERR GetValue(SKIRecord *pRecord, PRInt32 *piValue);
};

SK_REFCOUNT_DECLARE_INTERFACE(SKRecordComparatorSNumField,
                              SK_SKRECORDCOMPARATORSNUMFIELD_IID)

// SKRecordComparatorDataField

#define SK_SKRECORDCOMPARATORDATAFIELD_IID                              \
{ 0x09658b76, 0x5de7, 0x4c67,                                           \
    { 0x8f, 0x6b, 0x95, 0x0f, 0xd3, 0x8d, 0x69, 0x7e } }

class SKAPI SKRecordComparatorDataField : public SKRecordComparatorBaseField
{
public:
    SK_REFCOUNT_INTF_DEFAULT(SKRecordComparatorDataField)
    SK_REFCOUNT_INTF_IID(SKRecordComparatorDataField,
                         SK_SKRECORDCOMPARATORDATAFIELD_IID)

    virtual SKERR CompareRecords(SKIRecord *pRecord1, SKIRecord *pRecord2,
                                 PRInt32 *piResult);

protected:
    virtual SKERR       GetMyComparatorID(SKBinary** ppStrID);

private:
    SKERR GetValue(SKIRecord *pRecord, SKBinary **ppValue);
};

SK_REFCOUNT_DECLARE_INTERFACE(SKRecordComparatorDataField,
                              SK_SKRECORDCOMPARATORDATAFIELD_IID)
// SKRecordComparatorChain

#define SK_SKRECORDCOMPARATORCHAIN_IID                                  \
{ 0x13c6f1e3, 0xe175, 0x4bd3,                                           \
    { 0xb7, 0xb5, 0x75, 0xb0, 0xaf, 0x4c, 0x7a, 0x17 } }

class SKAPI SKRecordComparatorChain : public SKIRecordComparator
{
public:
    SK_REFCOUNT_INTF_DEFAULT(SKRecordComparatorChain)
    SK_REFCOUNT_INTF_IID(SKRecordComparatorChain,
                         SK_SKRECORDCOMPARATORCHAIN_IID)

    SKRecordComparatorChain();
    virtual ~SKRecordComparatorChain();

    SKERR AddComparator(SKIRecordComparator *pComparator);

    virtual SKERR CompareRecords(SKIRecord *pRecord1, SKIRecord *pRecord2,
                                 PRInt32 *piResult);

    virtual SKERR SetRecordSet(SKIRecordSet *pRecordSet);

protected:
    virtual SKERR       GetMyComparatorID(SKBinary** ppStrID);
    PRUint32 m_iComparatorCount;
    SKIRecordComparator **m_ppComparators;
};

SK_REFCOUNT_DECLARE_INTERFACE(SKRecordComparatorChain,
                              SK_SKRECORDCOMPARATORCHAIN_IID)

#else
#error "Multiple inclusions of recordcomparator.h"
#endif

