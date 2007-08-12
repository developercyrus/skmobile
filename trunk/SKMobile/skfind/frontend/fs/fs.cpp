/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: fs.cpp,v 1.39.2.4 2005/03/16 11:13:39 krys Exp $
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

#include "fs.h"

PRBool IsStringsEqual(const char* p1Start,
                     const char* p1End,
                     const char* p2)
{
    // p1 is [p1Start ; p1End[ , not NULL-terminated.
    // p2 is a standard psz.

    // no, we are *not* doing a mere PL_strncmp
    while(p1Start < p1End)
    {
        if(*p1Start != *p2)
            return PR_FALSE;

        p1Start++;
        p2++;
    }

    return ((*p2) == 0);
}

// -------------------------------------------------------------------------
// SKFileSystem
//

SK_REFCOUNT_IMPL_DEFAULT(SKFileSystem)
SK_REFCOUNT_IMPL_IID(SKFileSystem, SK_SKFILESYSTEM_IID, SKTextFile)

SKFileSystem::SKFileSystem()
{
}

SKERR SKFileSystem::SetFileName(const char *pszFileName,
                                const char *pszDefaultFileName)
{
    SKERR err;

    err = SKTextFile::SetFileName(pszFileName, pszDefaultFileName);
    if(err != noErr)
        return err;

    err = ParseConfiguration();
    if(err != noErr)
        return err;

    if(!m_pDirRecordSet)
        return SKError(err_invalid, "[SKFileSystem::SetFileName]"
                       " %s has no directory recordset", m_pszFileName);

    if(    !m_pDirNameField
        || !m_pDirParentField
        || !m_pDirChildrenField
        || !m_pDirChildrenSubField
        || !m_pDirFilesField
        || !m_pDirFilesSubField)
        return SKError(err_invalid, "[SKFileSystem::SetFileName]"
                       " %s has a badly configured directory recordset",
                       m_pszFileName);

    if(!m_pFileRecordSet)
        return SKError(err_invalid, "[SKFileSystem::SetFileName]"
                       " %s has no file recordset", m_pszFileName);

    if(    !m_pFileNameField
        || !m_pFileParentField
        || !m_pFileTitleField
        || !m_pFileContentField)
        return SKError(err_invalid, "[SKFileSystem::SetFileName]"
                       " %s has a badly configured file recordset",
                       m_pszFileName);

    return noErr;
}

SKERR SKFileSystem::GetRecordSetField(SKIRecordSet *pRecordSet,
                                      const char *pszFieldName,
                                      SKIField **ppField)
{
    *ppField = NULL;

    if(!pRecordSet)
        return SKError(err_invalid, "[SKFileSystem::GetRecordSetField]"
                       " The recordset hasn't been loaded yet.");

    SKERR err;

    skPtr<SKIFldCollection> pCol;
    err = pRecordSet->GetFldCollection(pCol.already_AddRefed());
    if(err != noErr)
        return err;

    err = pCol->GetField(pszFieldName, ppField);

    return err;
}

SKERR SKFileSystem::GetFieldSubField(SKIField *pField,
                                     const char *pszSubFieldName,
                                     SKIField **ppSubField)
{
    *ppSubField = NULL;

    if(!pField)
        return SKError(err_invalid, "[SKFileSystem::GetFieldSubField]"
                       " The field hasn't been configured yet.");

    SKERR err;

    skPtr<SKIRecordSet> pRS;
    err = pField->GetLinkSubRecordSet(pRS.already_AddRefed());
    if(err != noErr)
        return SKError(err, "[SKFileSystem::GetFieldSubField]"
                       " The sub-recordset hasn't been loaded yet.");

    err = GetRecordSetField(pRS, pszSubFieldName, ppSubField);

    return err;
}

SKERR SKFileSystem::ConfigureItem(char* pszSection, char* pszToken, char* pszValue)
{
    if(!pszToken || !pszSection)
        return noErr;

    SKERR err;

    if(!PL_strcmp(pszSection,"DIRTABLE"))
    {
#define _TABLE m_pDirRecordSet
        if(!PL_strcmp(pszToken, "RSURL"))
        {
            SKFactoryGetSubRecordSet(pszValue, _TABLE, err);
            if(err != noErr)
                return err;

            return noErr;
        }
        else if(!PL_strcmp(pszToken, "NAMEFIELD"))
        {
            return GetRecordSetField(_TABLE, pszValue,
                                     m_pDirNameField.already_AddRefed());
        }
        else if(!PL_strcmp(pszToken, "PARENTFIELD"))
        {
            return GetRecordSetField(_TABLE, pszValue,
                                     m_pDirParentField.already_AddRefed());
        }
        else if(!PL_strcmp(pszToken, "CHILDRENFIELD"))
        {
            return GetRecordSetField(_TABLE, pszValue,
                                     m_pDirChildrenField.already_AddRefed());
        }
        else if(!PL_strcmp(pszToken, "CHILDRENSUBFIELD"))
        {
            return GetFieldSubField(m_pDirChildrenField, pszValue,
                                    m_pDirChildrenSubField.already_AddRefed());
        }
        else if(!PL_strcmp(pszToken, "FILESFIELD"))
        {
            return GetRecordSetField(_TABLE, pszValue,
                                     m_pDirFilesField.already_AddRefed());
        }
        else if(!PL_strcmp(pszToken, "FILESSUBFIELD"))
        {
            return GetFieldSubField(m_pDirFilesField, pszValue,
                                    m_pDirFilesSubField.already_AddRefed());
        }
#undef _TABLE
    }
    else if(!PL_strcmp(pszSection,"FILETABLE"))
    {
#define _TABLE m_pFileRecordSet
        if(!PL_strcmp(pszToken, "RSURL"))
        {
            SKFactoryGetSubRecordSet(pszValue, _TABLE, err);
            if(err != noErr)
                return err;

            return noErr;
        }
        else if(!PL_strcmp(pszToken, "NAMEFIELD"))
        {
            return GetRecordSetField(_TABLE, pszValue,
                                     m_pFileNameField.already_AddRefed());
        }
        else if(!PL_strcmp(pszToken, "PARENTFIELD"))
        {
            return GetRecordSetField(_TABLE, pszValue,
                                     m_pFileParentField.already_AddRefed());
        }
        else if(!PL_strcmp(pszToken, "TITLEFIELD"))
        {
            return GetRecordSetField(_TABLE, pszValue,
                                     m_pFileTitleField.already_AddRefed());
        }
        else if(!PL_strcmp(pszToken, "CONTENTFIELD"))
        {
            return GetRecordSetField(_TABLE, pszValue,
                                     m_pFileContentField.already_AddRefed());
        }
#undef _TABLE
    }

    return err_not_handled;
}

SKERR SKFileSystem::GetRootDir(SKFSDirectory** ppRootDir)
{
    return GetDir((PRUint32)0, ppRootDir);
}

SKERR SKFileSystem::GetDir(const char* pszName, SKFSDirectory** ppDir)
{
    skPtr<SKFSDirectory> pDir;
    SKERR err = GetRootDir(pDir.already_AddRefed());
    if(err != noErr)
        return err;

    if(pszName[0] == '\0')
        return pDir.CopyTo(ppDir);

    return pDir->GetDir(pszName, ppDir);
}

SKERR SKFileSystem::GetFile(const char* pszName, SKFSFile** ppFile)
{
    if(pszName[0] == '\0')
        return err_fs_invalid;

    skPtr<SKFSDirectory> pDir;
    SKERR err = GetRootDir(pDir.already_AddRefed());
    if(err != noErr)
        return err;

    return pDir->GetFile(pszName, ppFile);
}

SKERR SKFileSystem::GetObject(const char* pszName, SKFSObject** ppObject)
{
    skPtr<SKFSDirectory> pDir;
    SKERR err = GetRootDir(pDir.already_AddRefed());
    if(err != noErr)
        return err;

    if(pszName[0] == '\0')
        return pDir.CopyTo((SKFSDirectory**)ppObject);

    return pDir->GetObject(pszName, ppObject);
}

SKERR SKFileSystem::GetDir(PRUint32 lIdx, SKFSDirectory** ppDir)
{
    skPtr<SKIRecord> pRecord;
    SKERR err = m_pDirRecordSet->GetRecord(lIdx, pRecord.already_AddRefed());
    SK_ASSERT(err == noErr);
    if(err != noErr)
        return err;

    *ppDir = sk_CreateInstance(SKFSDirectory)(this, pRecord);
    return (*ppDir)->Init();
}

SKERR SKFileSystem::GetFile(PRUint32 lIdx, SKFSFile** ppFile)
{
    skPtr<SKIRecord> pRecord;
    SKERR err = m_pFileRecordSet->GetRecord(lIdx, pRecord.already_AddRefed());
    SK_ASSERT(err == noErr);
    if(err != noErr)
        return err;

    *ppFile = sk_CreateInstance(SKFSFile)(this, pRecord);
    return (*ppFile)->Init();
}

// -------------------------------------------------------------------------
// SKFSObject
//

SK_REFCOUNT_IMPL_IID(SKFSObject, SK_SKFSOBJECT_IID, SKRefCount)

SKFSObject::SKFSObject(SKFileSystem* pFileSystem, SKIRecord* pRecord)
{
    SK_ASSERT(NULL != pRecord);
    m_pFileSystem = pFileSystem;
    m_pRecord = pRecord;
    m_pszName = NULL;
    m_pszPath = NULL;
}

SKFSObject::~SKFSObject()
{
   if(m_pszName) 
       PL_strfree(m_pszName);
}

SKERR SKFSObject::GetLocation(SKFSDirectory **ppDir)
{
    SKERR err = FetchParent();
    if(err == noErr)
        return m_pParentDir.CopyTo(ppDir);
    return err;
}

SKERR SKFSObject::GetParentDir(SKFSDirectory **ppDir)
{
    m_pParentDir.CopyTo(ppDir);
    return noErr;
}

SKERR SKFSObject::GetFSObjectId(PRUint32 *piId)
{
    return m_pRecord->GetId(piId);
}

SKERR SKFSObject::FetchParents()
{
    SKERR err = FetchParent();
    if((err == noErr) && m_pParentDir)
        err = m_pParentDir->FetchParents();
    return err;
}

void SKFSObject::ComputePath()
{
    if(m_pszName && !m_pszPath)
    {
        PRInt32 len = -1;
        skPtr<SKFSDirectory> pParent = m_pParentDir;
        skPtr<SKFSDirectory> pNext;
        while(pParent)
        {
            if(pParent->GetSharedName())
                len += 1 + PL_strlen(pParent->GetSharedName());
            else
                return;
            pParent->GetParentDir(pNext.already_AddRefed());
            pParent = pNext;
        }

        if (len == -1)
        {
            m_pszPath = PL_strdup("");
            return;
        }

        m_pszPath = (char*)PR_Malloc((len + 1) * sizeof(char));
    
        if(m_pszPath)
        {
            PRInt32 pos = len;
            pParent = m_pParentDir;
            while(pos >= 0)
            {
                PRUint32 l = PL_strlen(pParent->GetSharedName());
                PL_strcpy(m_pszPath + pos - l, pParent->GetSharedName());
                SK_ASSERT(m_pszPath[pos] == 0);
                m_pszPath[pos] = '/';
                pos -= l + 1;
                pParent->GetParentDir(pNext.already_AddRefed());
                pParent = pNext;
            }
            m_pszPath[len] = '\0';
        }
    }
}

// -------------------------------------------------------------------------
// SKFSDirectory
//

SK_REFCOUNT_IMPL(SKFSDirectory);
SK_REFCOUNT_IMPL_CREATOR(SKFSDirectory)(SKFileSystem* pFileSystem,
                                        SKIRecord* pRecord)
{
    return new SKFSDirectory(pFileSystem, pRecord);
}
SK_REFCOUNT_IMPL_IID(SKFSDirectory, SK_SKFSDIRECTORY_IID, SKFSObject)

SKERR SKFSDirectory::FetchInfo()
{
    skPtr<SKBinary> pBinary;
    SKERR err = m_pRecord->GetDataFieldValue(m_pFileSystem->m_pDirNameField,
                                             pBinary.already_AddRefed());
    SK_ASSERT(err == noErr);
    if(err != noErr)
        return err;
    SK_ASSERT(NULL != pBinary);
    SK_ASSERT(NULL != pBinary->GetSharedData());
    m_pszName = PL_strdup((char*)pBinary->GetSharedData());
    SK_ASSERT(NULL != m_pszName);
    SK_ASSERT(m_pszName[pBinary->GetSize() - 1] == '\0');

	return noErr;
}

SKERR SKFSDirectory::FetchParent()
{
    if(!m_pParentDir)
    {
        PRUint32 lId;
        SKERR err = m_pRecord->GetId(&lId);
        if(err != noErr)
            return err;

        if(lId == 0)
        {
            m_pParentDir = NULL;
            return noErr;
        }

        err = m_pRecord->GetUNumFieldValue(m_pFileSystem->m_pDirParentField,
                                           &lId);
        if(err != noErr)
            return err;

        err = m_pFileSystem->GetDir(lId, m_pParentDir.already_AddRefed());
        if(err != noErr)
            return err;
    }

    return noErr;
}

SKERR SKFSDirectory::GetDirCount(PRUint32 *plCount) const
{
    skPtr<SKIRecordSet> pChildren;
    SKERR err = m_pRecord->GetLinkFieldValue(m_pFileSystem->m_pDirChildrenField,
                                             pChildren.already_AddRefed());
    if(err != noErr)
        return err;

    err = pChildren->GetCount(plCount);

    return err;
}

SKERR SKFSDirectory::GetDir(PRUint32 lRank, SKFSDirectory** ppDir) const
{
    *ppDir = NULL;
    skPtr<SKIRecordSet> pChildren;
    SKERR err = m_pRecord->GetLinkFieldValue(m_pFileSystem->m_pDirChildrenField,
                                             pChildren.already_AddRefed());
    if(err != noErr)
        return err;

    skPtr<SKIRecord> pIRecord;
    err = pChildren->GetRecord(lRank, pIRecord.already_AddRefed());
    if(err != noErr)
        return err;

    PRUint32 lDirId;
    err= pIRecord->GetUNumFieldValue(m_pFileSystem->m_pDirChildrenSubField,
                                     &lDirId);
    if(err != noErr)
        return err;

    err = m_pFileSystem->GetDir(lDirId, ppDir);

    return err;
}

SKERR SKFSDirectory::GetFileCount(PRUint32 *plCount) const
{
    skPtr<SKIRecordSet> pChildren;
    SKERR err = m_pRecord->GetLinkFieldValue(m_pFileSystem->m_pDirFilesField,
                                             pChildren.already_AddRefed());
    if(err != noErr)
        return err;

    err = pChildren->GetCount(plCount);

    return err;
}

SKERR SKFSDirectory::GetFile(PRUint32 lRank, SKFSFile** ppFile) const
{
    *ppFile = NULL;
    skPtr<SKIRecordSet> pChildren;
    SKERR err = m_pRecord->GetLinkFieldValue(m_pFileSystem->m_pDirFilesField,
                                             pChildren.already_AddRefed());
    if(err != noErr)
        return err;

    skPtr<SKIRecord> pIRecord;
    err = pChildren->GetRecord(lRank, pIRecord.already_AddRefed());
    if(err != noErr)
        return err;

    PRUint32 lFileId;
    err= pIRecord->GetUNumFieldValue(m_pFileSystem->m_pDirFilesSubField,
                                     &lFileId);
    if(err != noErr)
        return err;

    err = m_pFileSystem->GetFile(lFileId, ppFile);

    return err;
}

SKERR SKFSDirectory::GetDir(const char* pszName, SKFSDirectory** ppDir)
{
    while(*pszName == '/')
        ++pszName;
    char * pszEnd = PL_strchr(pszName, '/');
    SKERR err;

    if(pszEnd)
    {
        char * pszStart = pszEnd + 1;
        while(*pszStart == '/')
            ++pszStart;
        if(IsStringsEqual(pszName, pszEnd, "."))
        {
            if(*pszStart == '\0')
            {
                AddRef();
                *ppDir = this;
                return noErr;
            }
            else
            {
                return GetDir(pszStart, ppDir);
            }
        }
        if(IsStringsEqual(pszName, pszEnd, ".."))
        {
            err = FetchParent();
            if(err != noErr)
                return err;
            if(*pszStart == '\0')
                return m_pParentDir.CopyTo(ppDir);
            else
                return m_pParentDir->GetDir(pszStart, ppDir);
        }

        skPtr<SKFSDirectory> pDir;
        PRUint32 ulCount;
        err = GetDirCount(&ulCount);
        if(err != noErr)
            return err;

        for(PRUint32 i = 0; i < ulCount; ++i)
        {
            err = GetDir(i, pDir.already_AddRefed());
            if(err != noErr)
                return err;

            if(IsStringsEqual(pszName, pszEnd, pDir->GetSharedName()))
            {
                if(*pszStart == '\0')
                    return pDir.CopyTo(ppDir);
                else
                    return pDir->GetDir(pszStart, ppDir);
            }
        }
    }
    else
    {
        if(!PL_strcmp(pszName, "."))
        {
            AddRef();
            *ppDir = this;
            return noErr;
        }
        if(!PL_strcmp(pszName, ".."))
        {
            err = FetchParent();
            if(err != noErr)
                return err;
            return m_pParentDir.CopyTo(ppDir);
        }

        skPtr<SKFSDirectory> pDir;
        PRUint32 ulCount;
        err = GetDirCount(&ulCount);
        if(err != noErr)
            return err;

        for(PRUint32 i = 0; i < ulCount; ++i)
        {
            err = GetDir(i, pDir.already_AddRefed());
            if(err != noErr)
                return err;
            if(!PL_strcmp(pDir->GetSharedName(), pszName))
                return pDir.CopyTo(ppDir);
        }
    }

    return err_fs_invalid;
}

SKERR SKFSDirectory::GetFile(const char* pszName, SKFSFile** ppFile)
{
    while(*pszName == '/')
        ++pszName;
    char * pszEnd = PL_strrchr(pszName, '/');
    char * pszStart;
    skPtr<SKFSDirectory> pDir(this);
    SKERR err;

    if(pszEnd)
    {
        pszStart = pszEnd + 1;
        while(*pszStart == '/')
            ++pszStart;

        if(*pszStart == '\0')
            return err_fs_invalid;

        char* psz = PL_strdup(pszName);
        SK_ASSERT(NULL != psz);
        if(!psz)
            return err_fs_malloc;

        psz[pszEnd - pszName] = '\0';

        err = GetDir(psz, pDir.already_AddRefed());
        PL_strfree(psz);
        if(err != noErr)
            return err_fs_invalid;
    }
    else
    {
        pszStart = (char*)pszName;
    }

    if(!pDir)
        return err_fs_invalid;

    skPtr<SKFSFile> pFile;
    PRUint32 ulCount;
    err = pDir->GetFileCount(&ulCount);
    if(err != noErr)
        return err;

    for(PRUint32 i = 0; i < ulCount; ++i)
    {
        err = pDir->GetFile(i, pFile.already_AddRefed());
        if(err != noErr)
            return err;
        if(!PL_strcmp(pFile->GetSharedName(), pszStart))
            return pFile.CopyTo(ppFile);
    }

    return err_fs_invalid;
}

SKERR SKFSDirectory::GetObject(const char* pszName, SKFSObject** ppObject)
{
    while(*pszName == '/')
        ++pszName;
    char * pszEnd = PL_strrchr(pszName, '/');
    char * pszStart;
    skPtr<SKFSDirectory> pDir(this);
    SKERR err;

    if(pszEnd)
    {
        pszStart = pszEnd + 1;
        while(*pszStart == '/')
            ++pszStart;

        char* psz = PL_strdup(pszName);
        SK_ASSERT(NULL != psz);
        if(!psz)
            return err_fs_malloc;

        psz[pszEnd - pszName] = '\0';

        err = GetDir(psz, pDir.already_AddRefed());
        if(err != noErr)
            return err_fs_invalid;
    }
    else
    {
        pszStart = (char*)pszName;
    }

    if(!pDir)
        return err_fs_invalid;

    if(*pszStart == '\0')
    {
        return pDir.CopyTo((SKFSDirectory**)ppObject);
    }
    else
    {
        skPtr<SKFSDirectory> pResultDir;
        err = pDir->GetDir(pszStart, pResultDir.already_AddRefed());
        if(err == noErr)
            return pResultDir.CopyTo((SKFSDirectory**)ppObject);;

        return pDir->GetFile(pszStart, (SKFSFile**)ppObject);
    }
}

// -------------------------------------------------------------------------
// SKFSFile
//

SK_REFCOUNT_IMPL(SKFSFile);
SK_REFCOUNT_IMPL_CREATOR(SKFSFile)(SKFileSystem* pFileSystem,
                                   SKIRecord* pRecord)
{
    return new SKFSFile(pFileSystem, pRecord);
}
SK_REFCOUNT_IMPL_IID(SKFSFile, SK_SKFSFILE_IID, SKFSFile)


SKERR SKFSFile::GetData(SKBinary** ppBinary)
{
    return m_pRecord->GetDataFieldValue(m_pFileSystem->m_pFileContentField,
                                        ppBinary);
}

SKERR SKFSFile::GetTitle(SKBinary** ppBinary)
{
    return m_pRecord->GetDataFieldValue(m_pFileSystem->m_pFileTitleField,
                                        ppBinary);
}

SKERR SKFSFile::FetchInfo()
{
    skPtr<SKBinary> pBinary;
    SKERR err = m_pRecord->GetDataFieldValue(m_pFileSystem->m_pFileNameField,
                                             pBinary.already_AddRefed());
    SK_ASSERT(err == noErr);
    if(err != noErr)
	{
		m_pszName = NULL;
        return err;
	}
    SK_ASSERT(NULL != pBinary);
    SK_ASSERT(NULL != pBinary->GetSharedData());
    m_pszName = PL_strdup((char*)pBinary->GetSharedData());
    SK_ASSERT(NULL != m_pszName);
    SK_ASSERT(m_pszName[pBinary->GetSize() - 1] == '\0');

	return noErr;
}

SKERR SKFSFile::FetchParent()
{
    if(!m_pParentDir)
    {
        PRUint32 lId;
        SKERR err =
            m_pRecord->GetUNumFieldValue(m_pFileSystem->m_pFileParentField,
                                         &lId);
        if(err != noErr)
            return err;

        err = m_pFileSystem->GetDir(lId, m_pParentDir.already_AddRefed());
        if(err != noErr)
            return err;
    }

    return noErr;
}

// Unused constructors

SKFSObject::SKFSObject()
{
    fprintf(stderr, "This message must NOT appear ! This is a bug !"
                    "(SKFSObject) \n");
    SK_ASSERT(PR_FALSE);
}

SKFSDirectory::SKFSDirectory() : SKFSObject(NULL, NULL)
{
    fprintf(stderr, "This message must NOT appear ! This is a bug !"
                    "(SKFSDirectory) \n");
    SK_ASSERT(PR_FALSE);
}

SKFSFile::SKFSFile() : SKFSObject(NULL, NULL)
{
    fprintf(stderr, "This message must NOT appear ! This is a bug !"
                    "(SKFSFile) \n");
    SK_ASSERT(PR_FALSE);
};

