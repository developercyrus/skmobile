/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: field.cpp,v 1.68.4.3 2005/02/21 14:22:39 krys Exp $
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

#include <skfind/skfind.h>

#include "stream.h"
#include "field.h"
#include "record.h"
#include "buf.h"

int fldSort(const void* fld1, const void* fld2);
int compare (void *p, void *q, PRBool bCompareText, PRInt16 iSize /*=-1*/);

static PRUint32 readValue8(const void *pBuffer);
static PRUint32 readValue16(const void *pBuffer);
static PRUint32 readValue24(const void *pBuffer);
static PRUint32 readValue32(const void *pBuffer);

static valueReader_t g_pfValueReaders[5] =
{
    NULL,
    readValue8,
    readValue16,
    readValue24,
    readValue32
};

//  ----------------------------------------------------------------------------
//
//  SKField
//
//  ----------------------------------------------------------------------------

SK_REFCOUNT_IMPL_DEFAULT(SKField)

SKField::SKField() 
{
    m_lPosition = 0;
    m_lOffset = 0;
    m_lSize = 0;
    m_eFieldType = SKFT_UNKNOWN;

    m_pszName = NULL;

    m_ppFiles = NULL;
    m_lFilesArraySize = 0;
}

SKField::~SKField()
{
    if(m_ppFiles)
        delete[] m_ppFiles;
    if(m_pszName)
        PL_strfree(m_pszName);
}

SKERR SKField::GetName(char **ppszName) const
{
    SK_ASSERT(NULL != m_pszName);
    if(m_pszName)
    {
        *ppszName = PL_strdup(m_pszName);
        return noErr;
    }
    else
    {
        *ppszName = NULL;
        return err_fld_invalid;
    }
}

SKERR SKField::GetLinkSubRecordSet(SKIRecordSet **ppRecordSet)
{
    SK_ASSERT(m_eFieldType == SKFT_LINK);
    if(m_eFieldType != SKFT_LINK)
        return err_failure;

    return m_pSubRecordSet.CopyTo(ppRecordSet);
}

SKERR SKField::SetOffset(PRUint32 offset)
{
    m_lOffset = offset;
    
    // special case of the composite field...
    if(IsACompositeFieldType(m_eFieldType))
        ComputeOffsets();

    return noErr;
}

SKERR SKField::SetName(const char * name)
{
    m_pszName = PL_strdup(name);
    return noErr;
}

SKERR SKField::SetType(FieldType t)
{
    m_eFieldType = t;
    if(t == SKFT_UNKNOWN)
        return SKError(err_fld_unknown, "[SKField::SetType] "
                "Can not manage UNKNOWN type...");
    if(t == SKFT_ULONG || t == SKFT_SLONG)
    {
        m_lSize = 4;
        return noErr;
    }
    if(t == SKFT_U24 || t == SKFT_S24)
    {
        m_lSize = 3;
        return noErr;
    }
    if(t == SKFT_USHORT || t == SKFT_SSHORT)
    {
        m_lSize = 2;
        return noErr;
    }
    if(t == SKFT_UBYTE || t == SKFT_SBYTE)
    {
        m_lSize = 1;
        return noErr;
    }
    if(t == SKFT_LINK)
    {
        m_lSize = 0;
        return noErr;
    }
    if(t == SKFT_DATA)
    {
        m_lSize = 0;
        return noErr;
    }
    return SKError(err_fld_unknown, "[SKField::SetType] "
            "Type size is unknown...");
}
    
SKERR SKField::SetType(const char * TypeName)
{
    FieldType t = TypeFromName(TypeName);
    if(t == SKFT_UNKNOWN)
        return SKError(err_fld_unknown, "[SKField::SetType] "
            "Do not know anything about type %s", TypeName);
    return SetType(t);
}

SKERR SKField::GetFieldType(skfFieldType* pType)
{
    switch(m_eFieldType)
    {
        case SKFT_UNKNOWN:
            *pType = skfFTInvalid;
        break;
        case SKFT_LINK:
            *pType = skfFTLink;
        break;
        case SKFT_DATA:
            *pType = skfFTData;
        break;
        case SKFT_SBYTE:
        case SKFT_SSHORT:
        case SKFT_SLONG:
        case SKFT_S24:
            *pType = skfFTSNum;
        break;
        case SKFT_UBYTE:
        case SKFT_USHORT:
        case SKFT_ULONG:
        case SKFT_U24:
            *pType = skfFTUNum;
        break;
    }
    return noErr;
}

SKERR SKField::IsLink(PRBool * bIsLink)
{
    if (m_eFieldType == SKFT_LINK) *bIsLink = PR_TRUE;
    else *bIsLink = PR_FALSE;
    return noErr;
}

SKERR SKField::IsData(PRBool * bIsData)
{
    if (m_eFieldType == SKFT_DATA) *bIsData = PR_TRUE;
    else *bIsData = PR_FALSE;
    return noErr;
}

SKERR SKField::IsUNum(PRBool * bIsUNum)
{
    *bIsUNum = IsAnUnsignedFieldType( m_eFieldType );
    return noErr;
}

SKERR SKField::IsSNum(PRBool * bIsSNum)
{
    *bIsSNum = IsASignedFieldType( m_eFieldType );
    return noErr;
}

SKERR SKField::Configure(char* szToken, char* szValue)
{
    // we have to be composite 
    if(!IsACompositeFieldType(m_eFieldType))
        return SKError(err_fld_conf,"[SKField::Configure] "
                "Configure called for %s field (%s=%s), which is not a "
                "composite field (type = %ld).", m_pszName, szToken, szValue,
                m_eFieldType);

    skPtr<SKField>* ppField = NULL;
    
    // select the subfield
    if(!PL_strcmp(szToken, "OFFSET")) ppField = &m_pFieldOffset;
    else if(!PL_strcmp(szToken, "COUNT")) ppField = &m_pFieldCount;
    else if(    (m_eFieldType == SKFT_DATA)
             && (!PL_strcmp(szToken, "PACKID"))) ppField = &m_pFieldPack;

    // check we know the key
    if(!ppField)
        return SKError(err_fld_conf,"[SKField::Configure] "
                "Configure called for %s field (%s=%s). Key is incorrect.", 
                m_pszName, szToken, szValue);

    // check the key is not yet done
    if(*ppField)
        return SKError(err_fld_conf,"[SKField::Configure] "
                "Configure called for %s field (%s=%s). "
                "Key given twice or more.", 
                m_pszName, szToken, szValue);
    
    // configure the subfield
    *(*ppField).already_AddRefed() = sk_CreateInstance(SKField)();
    if(!*ppField)
        return SKError(err_fld_malloc,"[SKField::Configure] Mem. Allocation");

    // set the type
    (*ppField)->SetType(szValue);

    // put a name to the subfield (usefull for debugging purpose)
    char * pcSubFieldName = (char*) 
          PR_Malloc(PL_strlen(szToken) + PL_strlen(m_pszName) + 2);
    if(!pcSubFieldName)
        return SKError(err_fld_malloc,"[SKField::Configure] Mem. Allocation"); 
    sprintf(pcSubFieldName, "%s,%s", m_pszName, szToken);
    (*ppField)->SetName(pcSubFieldName);
    PR_Free(pcSubFieldName);
    
    // compute size of record
    m_lSize = 0;
    if(m_pFieldOffset)
        m_lSize += m_pFieldOffset->GetSize();;
    if(m_pFieldCount)
        m_lSize += m_pFieldCount->GetSize();
    if(m_pFieldPack)
        m_lSize += m_pFieldPack->GetSize();

    // compute offsets
    ComputeOffsets();
    
    return noErr;
}

SKERR SKField::ConfigureComposite(PRUint32 lId, char *pszToken, char *pszValue)
{
    if(m_eFieldType == SKFT_DATA)
        return ConfigureFile(lId, pszToken, pszValue);
    else if(m_eFieldType == SKFT_LINK)
        return ConfigureLink(pszToken, pszValue);
    else
        return err_failure;
}

SKERR SKField::Check()
{
    if(m_eFieldType == SKFT_DATA)
    {
        PRUint32 i;
        for(i=0; i<m_lFilesArraySize; i++)
        {
            SKERR err = noErr;
            if(m_ppFiles[i])
                err = m_ppFiles[i]->Check();
            if(err != noErr)
                return err;
        }
    }
    return noErr;
}

SKERR SKField::ConfigureFile(PRUint32 lId, char *pszToken, char *pszValue)
{
    SK_ASSERT(m_eFieldType == SKFT_DATA);
    // see if the array is big enough
    if(lId >= m_lFilesArraySize)
    {
        // replace the array with another one pre-initialized
        skPtr<SKPageFile>* ppFiles = new skPtr<SKPageFile>[lId + 10];
        for(PRUint32 i = 0; i < m_lFilesArraySize; i++)
            ppFiles[i] = m_ppFiles[i];
        delete[] m_ppFiles;
        m_ppFiles = ppFiles;
        m_lFilesArraySize = lId + 10;
    }
    // create the file if needed
    if(m_ppFiles[lId] == NULL)
    {
        *m_ppFiles[lId].already_AddRefed() = sk_CreateInstance(SKPageFile)();
    }
    return m_ppFiles[lId]->ConfigureItem("", pszToken, pszValue);
}

SKERR SKField::ConfigureLink(char *pszToken, char *pszValue)
{
    SK_ASSERT(m_eFieldType == SKFT_LINK);
    if(!PL_strcmp(pszToken, "RSURL"))
    {
        SKERR err = noErr;
        SKFactoryGetSubRecordSet(pszValue, m_pSubRecordSet, err);
        return err;
    }
    else
        return SKError(err_fld_invalid, "[SKField::ConfigureLink] "
                       "unknown configuration token '%s'", pszToken);
}

SKERR SKField::GetUNumFieldValue(const void* pBuffer, PRUint32 *plValue) const
{
    *plValue = g_pfValueReaders[m_lSize]((char *)pBuffer + m_lOffset);
    return noErr;
}

SKERR SKField::GetSNumFieldValue(const void* pBuffer, PRInt32 *plValue) const
{
    *plValue = g_pfValueReaders[m_lSize]((char *)pBuffer + m_lOffset);
    return noErr;
}

SKERR SKField::GetDataFieldValue(const void * pBuffer,
                                 SKBinary** ppBinary,
                                 const void * pNextBuffer) const
{
    SKERR err = noErr;
    *ppBinary = NULL;

    if(!pBuffer)
        return SKError(err_fld_invalid,"[SKField::GetDataFieldValue] "
                "Invalid record");
 
    // get file
    skPtr<SKPageFile> pTda;
    err = GetFile(pBuffer, pTda.already_AddRefed());
    if(err != noErr || !pTda)
        return SKError(err_fld_invalid, "[SKField::GetDataFieldValue] "
                "package not initialized.");

    // get offset
    SK_ASSERT(NULL != m_pFieldOffset);

    PRUint32 lOffset = 0;
    err = m_pFieldOffset->GetUNumFieldValue(pBuffer, &lOffset);
    if(err != noErr)
        return err;

    // is there a length ?
    PRUint32 lCount = 0;
    if(m_pFieldCount)
    {
        m_pFieldCount->GetUNumFieldValue(pBuffer, &lCount);
    }
    else
    {
        if(pNextBuffer == 0)
        {
            // We have no info about where we are in the file. 
            // We can not retrieve the data.
            return SKError(err_fld_invalid, "[SKField::GetDataFieldValue] "
                    "No length info and no following record...");
        }

        // detect pack boundary
        if(m_pFieldPack && ((PRInt32)pNextBuffer != -1))
        {
            PRUint32 pack1, pack2;
            m_pFieldPack->GetUNumFieldValue(pBuffer, &pack1);
            m_pFieldPack->GetUNumFieldValue(pNextBuffer, &pack2);
            if(pack1 != pack2)
                pNextBuffer = (const void*)-1;
        }

        // is it the last record ?
        if( ((PRInt32) pNextBuffer) == -1 )
        {
            pTda->IsAvailable();
            // read all the end of the file
            lCount = pTda->GetFileSize() - lOffset;
        }
        else
        {
            // stop where next record begins.
            m_pFieldOffset->GetUNumFieldValue(pNextBuffer, &lCount);
            lCount-= lOffset;
        }
    }

    // read from TDA file
    if(lCount)
    {
        err = pTda->Load (lOffset, lCount);
        if(noErr != err)
		{
			pTda->Unload();
			return SKError(err,"[SKField::GetDataFieldValue] "
				"Failed to load TDA file. lOffset = %d, lCount = %d, error = %d",
				lOffset, lCount, err);
		}
        *ppBinary=sk_CreateInstance(SKBinary)(pTda->GetBufferPtr(),lCount);
		err = pTda->Unload();
    }

    return noErr;
}

SKERR SKField::GetStreamFieldValue(const void * pBuffer,
                                   SKIStream** ppStream,
                                   const void * pNextBuffer) const
{
    SKERR err = noErr;
    *ppStream = NULL;

    if(!pBuffer)
        return SKError(err_fld_invalid,"[SKField::GetStreamFieldValue] "
                "Invalid record");
 
    // get file
    skPtr<SKPageFile> pTda;
    err = GetFile(pBuffer, pTda.already_AddRefed());
    if(err != noErr || !pTda)
        return SKError(err_fld_invalid, "[SKField::GetStreamFieldValue] "
                "package not initialized.");

    // get offset
    SK_ASSERT(NULL != m_pFieldOffset);

    PRUint32 lOffset = 0;
    err = m_pFieldOffset->GetUNumFieldValue(pBuffer, &lOffset);
    if(err != noErr)
        return err;

    // is there a length ?
    PRUint32 lCount = 0;
    if(m_pFieldCount)
    {
        m_pFieldCount->GetUNumFieldValue(pBuffer, &lCount);
    }
    else
    {
        if(pNextBuffer == 0)
        {
            // We have no info about where we are in the file. 
            // We can not retrieve the data.
            return SKError(err_fld_invalid, "[SKField::GetStreamFieldValue] "
                    "No length info and no following record...");
        }

        // detect pack boundary
        if(m_pFieldPack && ((PRInt32)pNextBuffer != -1))
        {
            PRUint32 pack1, pack2;
            m_pFieldPack->GetUNumFieldValue(pBuffer, &pack1);
            m_pFieldPack->GetUNumFieldValue(pNextBuffer, &pack2);
            if(pack1 != pack2)
                pNextBuffer = (const void*)-1;
        }

        // is it the last record ?
        if( ((PRInt32) pNextBuffer) == -1 )
        {
            pTda->IsAvailable();
            // read all the end of the file
            lCount = pTda->GetFileSize() - lOffset;
        }
        else
        {
            // stop where next record begins.
            m_pFieldOffset->GetUNumFieldValue(pNextBuffer, &lCount);
            lCount-= lOffset;
        }
    }

    // create the stream
    *ppStream = sk_CreateInstance(SKStream)(pTda, lOffset, lCount);

    return noErr;
}

SKERR SKField::GetOffsetField(SKField** ppField)
{
    m_pFieldOffset.CopyTo(ppField);
    return noErr; 
}

SKERR SKField::GetCountField(SKField** ppField)
{
    m_pFieldCount.CopyTo(ppField);
    return noErr; 
}

SKERR SKField::GetPackField(SKField** ppField)
{ 
    m_pFieldPack.CopyTo(ppField);
    return noErr; 
}

SKERR SKField::GetFile(SKPageFile ** ppFile, PRUint32 lPackId /*= 0*/) const
{
    *ppFile = NULL;
    if(lPackId > m_lFilesArraySize)
        return SKError(err_fld_unknown, "[SKField::GetFile] "
                "File ID is incorrect.");
    *ppFile = m_ppFiles[lPackId];
    if(!*ppFile)
        return SKError(err_fld_unknown, "[SKField::GetFile] "
                "File ID is incorrect.");
    (*ppFile)->AddRef();
    return noErr;
}

SKERR SKField::GetFile(const void * pBuffer, SKPageFile** ppFile) const
{
    SK_ASSERT(m_eFieldType == SKFT_DATA);
    if(m_eFieldType != SKFT_DATA)
        return err_failure;
    PRUint32 pack = 0;
    if(m_pFieldPack)
        m_pFieldPack->GetUNumFieldValue(pBuffer, &pack);
    return GetFile(ppFile, pack);
}

void SKField::ComputeOffsets()
{
    if(!m_pFieldOffset) return;
    m_pFieldOffset->SetOffset(m_lOffset);
    
    if(m_pFieldCount)
    {
        PRUint32 lOffsetSize = m_pFieldOffset->GetSize();
        m_pFieldCount->SetOffset(m_lOffset + lOffsetSize);

        if(m_pFieldPack)
        {
            PRUint32 lCountSize = m_pFieldCount->GetSize();
            m_pFieldPack->SetOffset(m_lOffset + lOffsetSize + lCountSize);
        }
    }
    else if(m_pFieldPack)
    {
        PRUint32 lOffsetSize = m_pFieldOffset->GetSize();
        m_pFieldPack->SetOffset(m_lOffset + lOffsetSize);
    }
}

//  ----------------------------------------------------------------------------
//
//  SKFldCollection 
//
//  ----------------------------------------------------------------------------
SK_REFCOUNT_IMPL_DEFAULT(SKFldCollection);

SKFldCollection::SKFldCollection (void) 
{
    m_ppFields = NULL;
    m_lTotalFieldSize = 0;
    m_lRecordSize = 0;
    m_lFieldCount = 0;
    m_bSorted = PR_TRUE;
}

//  ----------------------------------------------------------------------------
//
//  SKFldCollection 
//
//  ----------------------------------------------------------------------------

SKFldCollection::~SKFldCollection (void) 
{
    if(m_ppFields)
        delete[] m_ppFields;
}

//  ----------------------------------------------------------------------------
//
//  Configure
//
//  ----------------------------------------------------------------------------

SKERR SKFldCollection::Configure (char* szToken, char* szValue) 
{
    SKERR err = noErr;

    // look if the token contains a ','
    char * pcComa = PL_strstr(szToken, ",");
    if(pcComa == NULL)
    {
        // adjust field array
        if (m_lFieldCount % 10 == 0)
        {
            skPtr<SKField>* ppFields = new skPtr<SKField>[m_lFieldCount + 10];
            if (!ppFields)
                return SKError(err_fld_malloc, "[SKFldCollection::Configure]"
                               " Failed to allocate %ld bytes",
                               (m_lFieldCount + 10) * sizeof(skPtr<SKField>));
            for(PRUint32 i = 0; i < m_lFieldCount; i++)
                ppFields[i] = m_ppFields[i];
            delete[] m_ppFields;
            m_ppFields = ppFields;
        }

        skPtr<SKField>& pfd = m_ppFields[m_lFieldCount++];
        *pfd.already_AddRefed() = sk_CreateInstance(SKField)();
        pfd->SetPosition(m_lFieldCount - 1);

        // handle field name
        err = SKFirstError(err, pfd->SetName(szToken));

        // handle type
        err = SKFirstError(err, pfd->SetType(szValue));
    
        // fill in the other fields
        err = SKFirstError(err, pfd->SetOffset(m_lRecordSize));
 
        m_lRecordSize += pfd->GetSize();

        Sort();

        if(err != noErr)
            SKError(err_not_handled,
                    "[SKFldCollection::Configure]"
                    " ignoring error on token '%s', value '%s'",
                    szToken, szValue);

        return noErr;
    }
    else
    {
        // we have a coma. we must configure a already existing field.
        // check that we already have the field we are interested in.
        *pcComa = '\0';
        skPtr<SKIField> pIField;
        err = GetField(szToken, pIField.already_AddRefed());
        if(err != noErr)
            return err;

        SKField * pField = (SKField*)(SKIField*)pIField;
        
        // the field is unknown
        if(!pField)
        {
            return SKError(err_fld_unknown,
                    "[SKPageFile::Configure] Trying to configure %s "
                    "which is not a known field", szToken);
        }
        // The field has been found, save its old size.
        PRUint32 oldFieldSize = pField->GetSize();

        // configure it
        err = pField->Configure(pcComa+1, szValue);
        if(err != noErr)
        {
            return err;
        }

        // adjust the record size
        PRUint32 lSize = pField->GetSize();
        m_lRecordSize = m_lRecordSize - oldFieldSize + lSize;

        return noErr;
    }
}

//  ----------------------------------------------------------------------------
//
//  Check
//
//  ----------------------------------------------------------------------------
SKERR SKFldCollection::Check()
{
    SKERR err;
    for(PRUint32 i = 0; i < m_lFieldCount; i++)
    {
        err = m_ppFields[i]->Check();
        if(err != noErr)
            return err;
    }
    return noErr;
}

//  ----------------------------------------------------------------------------
//
//  GetField
//
//  ----------------------------------------------------------------------------

inline 
SKERR SKFldCollection::GetField (const char* szField, SKIField** ppField) const
{
    *ppField = NULL;
    for (PRUint32 i = 0; i < m_lFieldCount; i++)
    {
        if(!PL_strcmp(m_ppFields[i]->GetSharedName(), szField))
        {
            *ppField = m_ppFields[i];
            (*ppField)->AddRef();
            return noErr;
        }
        
    }
    return SKError(err_fld_unknown, "[SKFldCollection::GetField] "
            "Field `%s' is unknown.", szField);;
}

SKERR SKFldCollection::GetFieldName(PRUint32 lIndex, 
                                    char **ppszName) const
{
    if(lIndex > m_lFieldCount)
        return SKError(err_fld_unknown, "[SKFldCollection::GetFieldName] "
                "Field #%ld is unknown.", lIndex);
    skPtr<SKIField> pField;
    SKERR err;
    err = GetField(lIndex, pField.already_AddRefed());
    return pField->GetName(ppszName);
}

//  ----------------------------------------------------------------------------
//
//  GetUNumFieldValue
//  returns the value of a given field into a unsigned long
//  szBuffer should point to the current "record"
//
//  ----------------------------------------------------------------------------
SKERR SKFldCollection::GetUNumFieldValue (const void* szBuffer, 
                                          const char* szField,
                                          PRUint32 *plValue) const
{
    // field info
    skPtr<SKIField> pfd;
    // found
    if (GetField(szField, pfd.already_AddRefed()) == noErr && pfd)
    {
        SKERR err =
            ((SKField*)(SKIField*)pfd)->GetUNumFieldValue(szBuffer, plValue);
        return err;
    }
    else
        return SKError(err_fld_unknown, "[SKFldCollection::GetUNumFieldValue ]"
                "Field `%s' unknown.", szField);
}
//  ----------------------------------------------------------------------------
//
//  GetSNumFieldValue
//  returns the value of a given field into a unsigned long
//  szBuffer should point to the current "record"
//
//  ----------------------------------------------------------------------------
SKERR SKFldCollection::GetSNumFieldValue (const void* szBuffer, 
                                          const char* szField,
                                          PRInt32 *plValue) const
{
    // field info
    skPtr<SKIField> pfd;
    // found
    if (GetField(szField, pfd.already_AddRefed()) == noErr && pfd)
    {
        SKERR err =
            ((SKField*)(SKIField*)pfd)->GetSNumFieldValue(szBuffer, plValue);
        return err;
    }
    else
        return SKError(err_fld_unknown, "[SKFldCollection::GetSNumFieldValue ]"
                "Field `%s' unknown.", szField);
}

int fldSort(const void* fld1, const void* fld2)
{
    const char * psz1 = (*(SKField**)fld1)->GetSharedName(); 
    const char * psz2 = (*(SKField**)fld2)->GetSharedName();
    return PL_strcmp(psz1, psz2);
}

inline void SKFldCollection::Sort(void) 
{
    qsort(m_ppFields, m_lFieldCount, sizeof(m_ppFields[0]), fldSort);
    m_bSorted = PR_TRUE;
}

#define SLONG sizeof(long)
int compare (void *p, void *q, PRBool bCompareText, PRInt16 iSize /*=-1*/)
{
    if (iSize < 0) 
    {
        // variable size text
        return PL_strcmp ((char *)p, (char *)q);   
    }
    else
    {
#ifdef IS_BIG_ENDIAN
        // on a big_endian machine, comparing numeric types
        // is the same as comparing strings so we use strcmp
        // which should be faster
        bCompareText = PR_TRUE;
#endif
        // fixed-size text or big-endian numeric 
        if (bCompareText)
            return PL_strncmp ((char *)p, (char *)q, iSize);
        else
        {
            // applies only to little-endian numeric
            unsigned long *lp = (unsigned long *)p, *lq = (unsigned long *)q;
            short x = iSize % SLONG;
            
            // all this is necessary if the size is not a multiple of 4
            if (x)
            {
                long lMask = ~(-1L << 8 * (SLONG - x));
                iSize -= x;
                lp = (unsigned long *)((char *)p + iSize);
                lq = (unsigned long *)((char *)q + iSize);
                if ((lMask & *lp) != (lMask & *lq))
                    return ((*lp & lMask) - (*lq & lMask));
            }
            
            // compare using longs
            for (;iSize > 0; lp-=1, lq-=1, iSize-=SLONG)
            {
                if (*lp != *lq)
                    return (*lp - *lq);
            }
        }
    }
    return (0);
}

//  ----------------------------------------------------------------------------
static PRUint32 readValue8(const void *pBuffer)
{
    SK_ASSERT(NULL != pBuffer);
    return *(PRUint8 *)pBuffer;
}

static PRUint32 readValue16(const void *pBuffer)
{
    SK_ASSERT(NULL != pBuffer);
#ifdef IS_BIG_ENDIAN
    PRUint16 iValue;
    ((PRUint8 *)&iValue)[1] = ((PRUint8 *)pBuffer)[0];
    ((PRUint8 *)&iValue)[0] = ((PRUint8 *)pBuffer)[1];
    return iValue;
#else
	PRUint16 iValue;
	((PRUint8 *)&iValue)[1] = ((PRUint8 *)pBuffer)[1];
	((PRUint8 *)&iValue)[0] = ((PRUint8 *)pBuffer)[0];
	return iValue;
//    return *(PRUint16 *)pBuffer;
#endif
}

static PRUint32 readValue24(const void *pBuffer)
{
    SK_ASSERT(NULL != pBuffer);
    PRUint32 iValue;
#ifdef IS_BIG_ENDIAN
    ((PRUint8 *)&iValue)[3] = ((PRUint8 *)pBuffer)[0];
    ((PRUint8 *)&iValue)[2] = ((PRUint8 *)pBuffer)[1];
    ((PRUint8 *)&iValue)[1] = ((PRUint8 *)pBuffer)[2];
    ((PRUint8 *)&iValue)[0] = 0;
#else
    ((PRUint8 *)&iValue)[3] = 0;
    ((PRUint8 *)&iValue)[2] = ((PRUint8 *)pBuffer)[2];
    ((PRUint8 *)&iValue)[1] = ((PRUint8 *)pBuffer)[1];
    ((PRUint8 *)&iValue)[0] = ((PRUint8 *)pBuffer)[0];
#endif
    return iValue;
}

static PRUint32 readValue32(const void *pBuffer)
{
    SK_ASSERT(NULL != pBuffer);
	PRUint32 iValue;
#ifdef IS_BIG_ENDIAN
    ((PRUint8 *)&iValue)[3] = ((PRUint8 *)pBuffer)[0];
    ((PRUint8 *)&iValue)[2] = ((PRUint8 *)pBuffer)[1];
    ((PRUint8 *)&iValue)[1] = ((PRUint8 *)pBuffer)[2];
    ((PRUint8 *)&iValue)[0] = ((PRUint8 *)pBuffer)[3];
    return iValue;
#else
	((PRUint8 *)&iValue)[3] = ((PRUint8 *)pBuffer)[3];
	((PRUint8 *)&iValue)[2] = ((PRUint8 *)pBuffer)[2];
	((PRUint8 *)&iValue)[1] = ((PRUint8 *)pBuffer)[1];
	((PRUint8 *)&iValue)[0] = ((PRUint8 *)pBuffer)[0];
#endif
	return iValue;
}

