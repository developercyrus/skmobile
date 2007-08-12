/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: sktable.cpp,v 1.9.4.5 2005/02/21 14:22:44 krys Exp $
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

#include <skcore/skcore.h>

#include "sktable.h"

SK_REFCOUNT_IMPL_IID(SKIField, SK_SKIFIELD_IID, SKRefCount)

SK_REFCOUNT_IMPL_IID(SKIFldCollection, SK_SKIFLDCOLLECTION_IID, SKRefCount)

SK_REFCOUNT_IMPL_IID(SKIRecordSet, SK_SKIRECORDSET_IID, SKRefCount)

SK_REFCOUNT_IMPL_IID(SKIRecord, SK_SKIRECORD_IID, SKRefCount)

#define IsType(x)                               \
SKERR SKIField::Is ## x (PRBool * pb)           \
{                                               \
    skfFieldType ft = skfFTInvalid;             \
    SKERR err = GetFieldType(&ft);              \
    *pb = (ft == skfFT ## x);                   \
    return err;                                 \
}

IsType(Data);
IsType(UNum);
IsType(SNum);
IsType(Link);

typedef SKERR (*tComparator)(SKIRecord*, SKIField*, const void*, PRInt32*);

static SKERR DerivateLookups(SKIRecordSet* pRecordSet,
        skfLookupMode mode, const void* pValue, 
        PRUint32 iLastBeforeId, SKERR errLookup, PRUint32* piId,
        tComparator pFunc);

static SKERR TextComparator(SKIRecord* pRecord, SKIField* pField,
        const void* pValue, PRInt32 *piCmp)
{
    skPtr<SKBinary> pBin;
    SKERR err;
    err = pRecord->GetDataFieldValue(pField, pBin.already_AddRefed());
    if(err != noErr)
        return err;
    *piCmp = PL_strcmp((const char*) pBin->GetSharedData(),
                (const char*) pValue);
    return noErr;
}

static SKERR UNumComparator(SKIRecord* pRecord, SKIField* pField,
        const void* pValue, PRInt32 *piCmp)
{
    PRUint32 iValue;
    SKERR err;
    err = pRecord->GetUNumFieldValue(pField, &iValue);
    if(err != noErr)
        return err;
    *piCmp = iValue < (PRUint32) pValue;
    return noErr;
}

static SKERR SNumComparator(SKIRecord* pRecord, SKIField* pField,
        const void* pValue, PRInt32 *piCmp)
{
    PRInt32 iValue;
    SKERR err;
    err = pRecord->GetSNumFieldValue(pField, &iValue);
    if(err != noErr)
        return err;
    *piCmp = iValue < (PRInt32) pValue;
    return noErr;
}

SKERR SKIRecordSet::LookupText(const char * pszSearch, 
        skfLookupMode mode, PRUint32 *piId)
{
    if(mode == skflmEXACT || mode == skflmLASTBEFORE)
        return LookupTextImp(pszSearch, mode, piId);
    
    PRUint32 iLastBeforeId;
    SKERR errLookup = LookupTextImp(pszSearch, skflmLASTBEFORE, &iLastBeforeId);
    
    return DerivateLookups(this, mode, (const void*) pszSearch, iLastBeforeId,
            errLookup, piId, TextComparator);
}

SKERR SKIRecordSet::LookupUNum(PRUint32 iSearch, 
        skfLookupMode mode, PRUint32 *piId)
{
    if(mode == skflmEXACT || mode == skflmLASTBEFORE)
        return LookupUNumImp(iSearch, mode, piId);
    
    PRUint32 iLastBeforeId;
    SKERR errLookup = LookupUNumImp(iSearch, skflmLASTBEFORE, &iLastBeforeId);
    
    return DerivateLookups(this, mode, (const void*) iSearch, iLastBeforeId,
            errLookup, piId, UNumComparator);
}

SKERR SKIRecordSet::LookupSNum(PRInt32 iSearch, 
        skfLookupMode mode, PRUint32 *piId)
{
    if(mode == skflmEXACT || mode == skflmLASTBEFORE)
        return LookupSNumImp(iSearch, mode, piId);
    
    PRUint32 iLastBeforeId;
    SKERR errLookup = LookupSNumImp(iSearch, skflmLASTBEFORE, &iLastBeforeId);
    
    return DerivateLookups(this, mode, (const void*) iSearch, iLastBeforeId,
            errLookup, piId, SNumComparator);
}

SKERR DerivateLookups(SKIRecordSet* pRecordSet, skfLookupMode mode, 
        const void* pValue, PRUint32 iLastBeforeId, SKERR errLookup, 
        PRUint32 *piId, tComparator FuncComp)
{
    if(errLookup != noErr && errLookup != err_notfound)
        return errLookup;

    skPtr<SKIField> pField;
    SKERR err = pRecordSet->GetLookupField(pField.already_AddRefed());
    SK_ASSERT(err == noErr);
    PRUint32 iCount;
    err = pRecordSet->GetCount(&iCount);
    SK_ASSERT(err == noErr);
    // if target <= min(rs)
    if(errLookup == err_notfound)
    {
        if(mode == skflmEXACTORAFTER)
        {
            *piId = 0;
            return noErr;
        }
        skPtr<SKIRecord> pRecord;
        err = pRecordSet->GetRecord(0, pRecord.already_AddRefed());
        if(err != noErr)
            return err;
        PRInt32 iCmp;
        err = FuncComp(pRecord, pField, pValue, &iCmp);
        if(err != noErr)
            return err;
        // if target == min(rs)
        if(!iCmp)
        {
            // FIRSTAFTER or EXACTORBEFORE
            *piId = mode == skflmFIRSTAFTER ? 1 : 0;
            return noErr;
        }
        else // target < min(rs)
        {
            *piId = mode == skflmFIRSTAFTER ? 0 : (PRUint32) -1;
            return mode == skflmFIRSTAFTER ? noErr : err_notfound;
        }
    }
    // target > max(rs)
    if(iLastBeforeId == iCount - 1)
    {
        *piId = iLastBeforeId;
        return mode == skflmEXACTORBEFORE ? noErr
            : err_notfound; // skflmFIRSTAFTER || skflmEXACTORAFTER
    }
    // min < target <= max(rs)
    if(mode == skflmEXACTORAFTER)
    {
        *piId = iLastBeforeId + 1;
        return noErr;
    }
    else
    {
        // FIRSTAFTER or EXACTORBEFORE
        skPtr<SKIRecord> pRecord;
        err = pRecordSet->GetRecord(iLastBeforeId + 1, 
                pRecord.already_AddRefed());
        if(err != noErr)
            return err;
        PRInt32 iCmp;
        err = FuncComp(pRecord, pField, pValue, &iCmp);
        if(err != noErr)
            return err;
        if(!iCmp)
        {
            if(mode == skflmEXACTORBEFORE)
            {
                *piId = iLastBeforeId + 1;
                return noErr;
            }
            // skflmFIRSTAFTER
            if(iLastBeforeId + 2 == iCount)
            {
                *piId = (PRUint32) -1;
                return err_notfound;
            }
            *piId = iLastBeforeId + 2;
            return noErr;
        }
        else
        {
            *piId = iLastBeforeId + (mode == skflmFIRSTAFTER ? 1 : 0);
            return noErr;
        }

    }
}
