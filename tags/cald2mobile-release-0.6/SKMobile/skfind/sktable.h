/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: sktable.h,v 1.58.2.9 2005/03/16 13:58:24 bozo Exp $
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

#ifndef __SKTABLE_H_
#define __SKTABLE_H_

enum skfLookupMode
{
    /* These two are the basics ones. */
    skflmEXACT          = 0,
    skflmLASTBEFORE     = 1,
    /* These ones are derived. */
    skflmFIRSTAFTER     = 2,
    skflmEXACTORBEFORE  = 3,
    skflmEXACTORAFTER   = 4
};

enum skfOperator
{
    skfopNONE           = 0,
    // OR, AND, EXCEPT, NEAR, NEARBEFORE _MUST_ have the values 1, 2, 3 and 4
    skfopOR             = 1,
    skfopAND            = 2,
    skfopEXCEPT         = 3,
    skfopNEAR           = 4,
    skfopNEARBEFORE     = 5,
    skfopSEQAND         = 6,
    skfopAPPEND         = 7
};

#define NB_OPERATORS 5

enum skfSortingOrder
{
    skfsoIncreasing     = 1,
    skfsoDecreasing     = -1
};

enum skfFieldType
{
    skfFTInvalid = 0,
    skfFTData,
    skfFTUNum,
    skfFTSNum,
    skfFTLink
};

class SKIRecordSet;

#define SK_SKIFIELD_IID                                                 \
{ 0x364e2fd7, 0x9995, 0x426d,                                           \
    { 0xba, 0xd7, 0xe3, 0x5f, 0xbd, 0x5a, 0xea, 0x1c } }

class SKAPI SKIField : public SKRefCount
{
public:
    SK_REFCOUNT_INTF_IID(SKIField, SK_SKIFIELD_IID)

    virtual SKERR   GetName(char **ppszName) const = 0;
    virtual SKERR   IsData(PRBool * pbIsData);
    virtual SKERR   IsUNum(PRBool * pbIsUNum);
    virtual SKERR   IsSNum(PRBool * pbIsSNum);
    virtual SKERR   IsLink(PRBool * pbIsLink);
    virtual SKERR   GetFieldType(skfFieldType* pFieldType) = 0;
    virtual SKERR   GetLinkSubRecordSet(SKIRecordSet **ppRecordSet) = 0;
};

SK_REFCOUNT_DECLARE_INTERFACE(SKIField, SK_SKIFIELD_IID)

#define SK_SKIFLDCOLLECTION_IID                                         \
{ 0x4c423b76, 0x2afd, 0x4b14,                                           \
    { 0x9c, 0xf8, 0x2e, 0x45, 0x61, 0x4c, 0x36, 0xc5 } }

class SKAPI SKIFldCollection : public SKRefCount
{
public:
    SK_REFCOUNT_INTF_IID(SKIFldCollection, SK_SKIFLDCOLLECTION_IID)

    virtual SKERR   GetField(const char* szField, SKIField** ppField) const = 0;
            SKERR   GetFieldByName(const char* szField, SKIField** ppField)
                { return GetField(szField, ppField); }
    virtual SKERR   GetFieldCount(PRUint32 *piCount) const = 0;
    virtual SKERR   GetField(PRUint32 index, SKIField** ppField) const = 0;
            SKERR   GetFieldByPosition(PRUint32 index, SKIField** ppField)
                { return GetField(index, ppField); }
    virtual SKERR   GetFieldName(PRUint32 index, char **ppszName) const = 0;
};

SK_REFCOUNT_DECLARE_INTERFACE(SKIFldCollection, SK_SKIFLDCOLLECTION_IID)

class SKAPI SKCursor;
class SKAPI SKIStream;

class SKAPI SKIRecord;
class SKAPI SKIRecordFilter;
class SKAPI SKIRecordComparator;

#define SK_SKIRECORDSET_IID                                             \
{ 0xc0b2968e, 0xcedf, 0x47a4,                                           \
    { 0x8d, 0xc6, 0x72, 0xfd, 0x15, 0xdb, 0xda, 0x67 } }

class SKAPI SKIRecordSet : public SKRefCount
{
public:
    SK_REFCOUNT_INTF_IID(SKIRecordSet, SK_SKIRECORDSET_IID);

    virtual SKERR   GetCount(PRUint32 *piCount) const = 0;
    virtual SKERR   GetRecord(PRUint32 id, SKIRecord** pRecord) = 0;
    virtual SKERR   GetFldCollection(SKIFldCollection** pFldCol) =0;
    virtual SKERR   InitCursorRS(PRUint32 iOffset,
                                 PRUint32 iCount,
                                 SKIRecordSet* pRecordSet,
                                 SKRefCount* pFragment)
                                        { return err_not_implemented; }
    virtual SKERR   LookupText(const char * pszSearch,
                               skfLookupMode mode, PRUint32 *piId);
    virtual SKERR   LookupTextImp(const char * pszSearch,
                               skfLookupMode mode, PRUint32 *piId) = 0;
    virtual SKERR   LookupUNum(PRUint32 lNum,
                               skfLookupMode mode, PRUint32 *piId);
    virtual SKERR   LookupUNumImp(PRUint32 lNum,
                               skfLookupMode mode, PRUint32 *piId) = 0;
    virtual SKERR   LookupSNum(PRInt32 lNum,
                               skfLookupMode mode, PRUint32 *piId);
    virtual SKERR   LookupSNumImp(PRInt32 lNum,
                               skfLookupMode mode, PRUint32 *piId) = 0;

    virtual SKERR   GetLookupField(SKIField ** ppField)
                    { *ppField = NULL; return err_not_implemented; }

    virtual SKERR   GetSubRecordSet(PRUint32 lOffset, PRUint32 lCount,
                                    SKIRecordSet** ppIRecordSet) = 0;

    virtual SKERR   ExtractCursor(SKIField* pIField, PRUint32 iOffset,
                                  PRUint32 iCount, SKCursor** ppCursor) = 0;

    virtual SKERR   Filter(SKIRecordFilter *pFilter) = 0;
    virtual SKERR   Merge(SKIRecordSet *pRecordSet,
                          skfOperator oper,
                          SKIRecordComparator *pComparator,
                          PRBool bConsiderRank) = 0;
    virtual SKERR   Sort(SKIRecordComparator *pComparator,
                         PRBool bConsiderRank) = 0;

    virtual SKERR   FilterToNew(SKIRecordFilter *pFilter,
                                SKIRecordSet **ppNewRecordSet) = 0;
    virtual SKERR   MergeToNew(SKIRecordSet *pRecordSet, skfOperator oper,
                               SKIRecordComparator *pComparator,
                               PRBool bConsiderRank,
                               SKIRecordSet **ppNewRecordSet) = 0;
    virtual SKERR   SortToNew(SKIRecordComparator *pComparator,
                              PRBool bConsiderRank,
                              SKIRecordSet **ppNewRecordSet) = 0;
};

SK_REFCOUNT_DECLARE_INTERFACE(SKIRecordSet, SK_SKIRECORDSET_IID);

#define SK_SKIRECORD_IID                                                \
{ 0xd4fc758a, 0x3338, 0x4017,                                           \
    { 0x8b, 0xf1, 0x3c, 0xaa, 0xec, 0x7e, 0xcf, 0x23 } }

class SKAPI SKIRecord : public SKRefCount
{
public:
    SK_REFCOUNT_INTF_IID(SKIRecord, SK_SKIRECORD_IID);

    virtual SKERR GetId(PRUint32 *piId) = 0;
    virtual SKERR GetUNumFieldValue(SKIField* pIField,
                                    PRUint32  *pValue) = 0;
    virtual SKERR GetSNumFieldValue(SKIField* pIField,
                                    PRInt32 *pValue) = 0;
    virtual SKERR GetDataFieldValue(SKIField* pIField,
                                    SKBinary** ppBinary) = 0;
    virtual SKERR GetStreamFieldValue(SKIField* pIField,
                                      SKIStream** ppStream) = 0;
    virtual SKERR GetLinkFieldCount(SKIField* pIField,
                                    PRUint32* piCount) = 0;
    virtual SKERR GetLinkFieldValue(SKIField* pIField,
                                    SKIRecordSet** recordset) = 0;

    virtual SKERR GetFldCollection(SKIFldCollection** pCol) = 0;
};

SK_REFCOUNT_DECLARE_INTERFACE(SKIRecord, SK_SKIRECORD_IID)

class SKAPI SKIStream : public SKRefCount
{
public:
    virtual SKERR Read(PRUint32 iCount, void* pDst, PRUint32 *pRead) = 0;
    virtual SKERR Seek(PRUint32 iPosition) { return err_not_implemented; };
};

class SKAPI SKIFilter : public SKBinary
{
public:
    virtual SKERR FilterData(const void* pData, PRUint32 lSize) = 0;
};

// The bucket isn't well designed at the moment.
// One day it'll work.

/*
#define skfBucketGetRecordSet(_pszRSURL, _pRS, _err)                    \
{                                                                       \
    skPtr<skfBucket> pBucket;                                           \
    _err = noErr;                                                       \
    _err = skfBucket::GetBucket(pBucket.already_AddRefed());            \
    if(_err == noErr)                                                   \
    {                                                                   \
        _err = pBucket->GetRecordSet(_pszRSURL,                         \
                                     _pRS.already_AddRefed());          \
    }                                                                   \
}
*/

#define SKFactoryGetSubRecordSet(_pszPath, _pRS, _err)                      \
{                                                                           \
    SKFactory *pFactory = NULL;                                             \
    _err = SKFactory::GetFactory(&pFactory);                                \
    if(_err == noErr)                                                       \
    {                                                                       \
        char *pszComponent = (char*)PR_Malloc(PL_strlen(_pszPath) + 11);    \
        PL_strcpy(pszComponent, "recordset:");                              \
        PL_strcpy(pszComponent + 10, _pszPath);                             \
        skPtr<SKRefCount> _pRC;                                             \
        _err = pFactory->CreateInstance(pszComponent,                       \
                                        _pRC.already_AddRefed());           \
        PR_Free(pszComponent);                                              \
        if(_err == noErr)                                                   \
        {                                                                   \
            _pRS.AssignSafeIID(_pRC);                                       \
            if(!_pRS)                                                       \
                _err = err_failure;                                         \
        }                                                                   \
    }                                                                       \
}

#define SKFactoryGetRecordSet(_pszPath, _pRS, _err)                         \
{                                                                           \
    SKFactory *pFactory = NULL;                                             \
    _err = SKFactory::GetFactory(&pFactory);                                \
    if(_err == noErr)                                                       \
    {                                                                       \
        SKEnvir *pEnvir = NULL;                                             \
        _err = SKEnvir::GetEnvir(&pEnvir);                                  \
        if(_err == noErr)                                                   \
        {                                                                   \
            pEnvir->Lock();                                                 \
            SKFactoryGetSubRecordSet(_pszPath, _pRS, _err);                 \
            pEnvir->Unlock();                                               \
        }                                                                   \
    }                                                                       \
}

#else
#error "Multiple inclusions of sktable.h"
#endif

