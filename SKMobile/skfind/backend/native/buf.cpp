/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: buf.cpp,v 1.62.2.9 2005/03/16 16:27:13 krys Exp $
 *
 * Authors: W.P. Dauchy
 *          Mathieu Poumeyrol <poumeyrol @at@ idm .dot. fr>
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

#ifdef HAVE_CONFIG_H
#include "skbuildconfig.h"
#endif

#include "../../../skzlib/zlib.h"

#include "stream.h"
#include "field.h"
#include "record.h"
#include "buf.h"
#include "table.h"
#include "lc.h"

#define MIN_LOAD_BYTES 1024
#define POOL_SIZE 10

#ifdef  SK_USE_EXTERNAL_PROTECTION
extern "C" unsigned long SKFactoryCryptKey( const unsigned char** pcKey, 
                                            unsigned long *plLength);
#endif

SK_REFCOUNT_IMPL(SKPageFileFragment)
SK_REFCOUNT_IMPL_CREATOR(SKPageFileFragment)(SKPageFile *pPagefile)
{
    return new SKPageFileFragment(pPagefile);
}

SKPageFileFragment::SKPageFileFragment(SKPageFile *pPageFile)
{
    m_pPageFile = pPageFile;

    m_iBufferOffset = 0;
    m_iBufferSize = 0;
    m_iPhysicalSize = 0;
    m_pBuffer = NULL;
}

SKPageFileFragment::~SKPageFileFragment()
{
    if(m_pPageFile)
        m_pPageFile->ReleaseFragment(this);
    if(m_pBuffer)
        PR_Free(m_pBuffer);
}

PRBool SKPageFileFragment::Contains(PRUint32 iOffset, PRUint32 iCount)
{
    if(    (m_iBufferOffset <= iOffset)
        && ((m_iBufferOffset + m_iBufferSize) >= (iOffset + iCount)))
    {
        return PR_TRUE;
    }
    return PR_FALSE;
}

SKERR SKPageFileFragment::LoadFromFile(PRFileDesc *pFd,
        PRUint32 iOffsetInFile, PRUint32 iSizeInFile,
        PRUint32 iUncompressedOffsetInFile, PRUint32 iUncompressedSizeInFile,
        CompressionMode compMode, const char *pszFileName, const char* pcKey,
        PRUint32 iKeySize)
{
    /*
    printf("LoadFromFile(%p, %u, %u, %u, %u, %s) to %p;\n",
    pFd, iOffsetInFile, iSizeInFile,
    iUncompressedOffsetInFile, iUncompressedSizeInFile, pszFileName,
    this);
    */
    // allocate more memory if needed
    // this is the data buffer, not the compressed one
    if(!m_pBuffer || (iUncompressedSizeInFile > m_iPhysicalSize))
    {
        m_pBuffer = PR_Realloc(m_pBuffer, iUncompressedSizeInFile);
        SK_ASSERT(NULL != m_pBuffer);
        if(!m_pBuffer)
        {
            m_iPhysicalSize = 0;
            return SKError(err_memory,
                           "[SKPageFileFragment::LoadFromFile] "
                           "Failed to allocate %lu bytes",
                           iUncompressedSizeInFile);
        }
        m_iPhysicalSize = iUncompressedSizeInFile;
    }

    // where will we read ?
    void *pBuffer = NULL;
    if(compMode != CompressionNone)
    {
        // need a temp buffer
        pBuffer = PR_Malloc(iSizeInFile);
        if(!pBuffer)
        {
            return SKError(err_memory,
                           "[SKPageFileFragment::LoadFromFile] "
                           "Failed to allocate a temp buffer of %ld bytes",
                           iSizeInFile);
        }
    }
    else
    {
        pBuffer = m_pBuffer;
    }

    // set file position
    if(PR_Seek(pFd, iOffsetInFile, PR_SEEK_SET) == -1)
    {
        if(pBuffer && (pBuffer != m_pBuffer))
            PR_Free(pBuffer);
        return SKError(err_skf_fseek,
                       "[SKPageFileFragment::LoadFromFile] "
                        "Could not move internal file marker");
    }

    // reads in the buffer
    if(PR_Read(pFd, pBuffer, iSizeInFile) != (PRInt32) iSizeInFile)
    {
        if(pBuffer && (pBuffer != m_pBuffer))
            PR_Free(pBuffer);
        return SKError(err_skf_fread,
                       "[SKPageFileFragment::LoadFromFile] "
                       "Could not read %ld bytes at %ld.",
                       iSizeInFile, iOffsetInFile);
    }

    if(iKeySize)
        for(PRUint32 i = 0; i < iSizeInFile; i++)
            ((char*)pBuffer)[i] ^= pcKey[(iOffsetInFile + i)%iKeySize];

    if(compMode != CompressionNone)
    {
        // unzip the buffer
        uncompress((unsigned char *)m_pBuffer,
                   (unsigned long *)&iUncompressedSizeInFile,   // dest
                   (unsigned char *)pBuffer,
                   (unsigned long)iSizeInFile);                 // source
        // release compressed buffer
        PR_Free(pBuffer);
    }

    m_iBufferOffset = iUncompressedOffsetInFile;
    m_iBufferSize = iUncompressedSizeInFile;

    return noErr;
}

SKERR SKPageFileFragment::AppendFromFile(PRFileDesc *pFd,
        PRUint32 iOffsetInFile, PRUint32 iSizeInFile,
        const char *pszFileName, const char* pcKey, PRUint32 iKeySize)
{
    /*
    printf("AppendFromFile(%p, %u, %u, %s) to %p;\n",
    pFd, iOffsetInFile, iSizeInFile,
    pszFileName, this);
    */

    SK_ASSERT(iOffsetInFile >= m_iBufferOffset);
    SK_ASSERT(iOffsetInFile <= m_iBufferOffset + m_iBufferSize);

    // Cut the already loaded data
    if(iOffsetInFile < m_iBufferOffset + m_iBufferSize)
    {
        PRUint32 iDiff = m_iBufferOffset + m_iBufferSize - iOffsetInFile;
        iOffsetInFile += iDiff;
        iSizeInFile -= iDiff;
    }

    SK_ASSERT(iOffsetInFile >= m_iBufferOffset);
    SK_ASSERT(iOffsetInFile <= m_iBufferOffset + m_iBufferSize);

    if(m_iBufferSize + iSizeInFile > m_iPhysicalSize)
    {
        void *p = PR_Realloc(m_pBuffer, m_iBufferSize + iSizeInFile);
        SK_ASSERT(NULL != p);
        if(!p)
            return err_memory;
        m_pBuffer = p;
        m_iPhysicalSize = m_iBufferSize + iSizeInFile;
    }

    // set file position
    if(PR_Seek(pFd, iOffsetInFile, PR_SEEK_SET) == -1)
    {
        return SKError(err_skf_fseek,
                       "[SKPageFileFragment::AppendFromFile] "
                        "Could not move internal file marker");
    }

    // reads in the buffer
    if(PR_Read(pFd, (char *)m_pBuffer + m_iBufferSize, iSizeInFile)
                    != (PRInt32) iSizeInFile)
    {
        return SKError(err_skf_fread,
                       "[SKPageFileFragment::AppendFromFile] "
                       "Could not read %ld bytes at %ld.",
                       iSizeInFile, iOffsetInFile);
    }

    if(iKeySize)
        for(PRUint32 i = 0; i < iSizeInFile; i++)
            ((char*)m_pBuffer)[i + m_iBufferSize] ^=
                pcKey[(iOffsetInFile + i)%iKeySize];

    m_iBufferSize += iSizeInFile;

    return noErr;
}

SKERR SKPageFileFragment::Unload(PRUint32 iMaxCacheSize)
{
    if(m_pBuffer && (m_iBufferSize > iMaxCacheSize))
    {
        PR_Free(m_pBuffer);
        m_pBuffer = NULL;
        m_iBufferOffset = 0;
        m_iBufferSize = 0;
        m_iPhysicalSize = 0;
    }
    return noErr;
}

#define _swapvar(_tmp, _var)    \
    _tmp = pFragment->_var;     \
    pFragment->_var = _var;     \
    _var = _tmp;

void SKPageFileFragment::Swap(SKPageFileFragment *pFragment)
{
    PRUint32 i;
    _swapvar(i, m_iBufferOffset);
    _swapvar(i, m_iBufferSize);
    _swapvar(i, m_iPhysicalSize);

    void *p;
    _swapvar(p, m_pBuffer);
}

#undef _swapvar



SK_REFCOUNT_IMPL_DEFAULT(SKPageFile);

OpenMode SKPageFile::StringToOpenMode(const char* szValue)
{
    if(!PL_strcmp (szValue, "STANDARD"))
        return Standard;
    if(!PL_strcmp (szValue, "TRANSIENT"))
        return Transient;
    if(!PL_strcmp (szValue, "DELAYED"))
        return Delayed;
    if(!PL_strcmp (szValue, "MAPPED"))
        return Mapped;
    if(!PL_strcmp (szValue, "PREMAPPED"))
        return Premapped;
    SKError(err_failure, 
            "[(buf.cpp):StringToOpenMode] Unknown Openmode : `%s'", szValue);
    return Standard;
}
// -------------------------------------------------------------------
//
//    SKPageFile
//
// -------------------------------------------------------------------
SKPageFile::SKPageFile()
{
    Init();
}

// -------------------------------------------------------------------
//
//    ~SKPageFile
//
// -------------------------------------------------------------------
SKPageFile::~SKPageFile()
{
    Terminate();
}

// -------------------------------------------------------------------
//
//    Init
//
// -------------------------------------------------------------------
SKERR SKPageFile::Init()
{
    *m_pFldCollection.already_AddRefed() = sk_CreateInstance(SKFldCollection)();
    m_fp = NULL;
    m_lFileSize = 0;
    m_lRecordCount = INVALID_VALUE;
    m_lRecordSize = 0;
    m_lPageCount = INVALID_VALUE;
    m_lRefFileCount = 0;

    *m_pDefaultFragment.already_AddRefed() =
        sk_CreateInstance(SKPageFileFragment)(NULL);

    SKERR err;
    SKEnvir *pEnv = NULL;
    err = SKEnvir::GetEnvir(&pEnv);
    if(err != noErr)
        return err;
    char* pcOpenMode = NULL;
    pEnv->GetValue(SKF_BE_NATIVE_DEFAULT_OPENMODE_ENVIR_VAR, &pcOpenMode);
    if(pcOpenMode && *pcOpenMode)
        m_iOpenMode = StringToOpenMode(pcOpenMode);
    else
        m_iOpenMode = Standard;
    PL_strfree(pcOpenMode);

    m_ppFragments = (SKPageFileFragment **)
            PR_Malloc(sizeof(SKPageFileFragment *));
    m_ppFragments[0] = m_pDefaultFragment;
    m_iFragmentCount = 1;
    if(!m_ppFragments)
        m_iFragmentCount = 0;

    m_lInternalOffset = 0;
    m_lMaxCacheSize = 0; // (PRUint32)-1;

    m_iCompMode = CompressionNone;
    m_pCatalog = NULL;
    m_lCatalogPages = 0;
    m_lPageSize = 256;
    m_pszCatalogFileName = NULL;

    m_bEncrypted = PR_FALSE;

    if(!m_pFldCollection || !m_pDefaultFragment || !m_ppFragments)
        return err_memory;

    return noErr;
}

// -------------------------------------------------------------------
//
//    Terminate
//
// -------------------------------------------------------------------
SKERR SKPageFile::Terminate()
{
    SK_ASSERT(m_iFragmentCount == 1);
    SK_ASSERT(m_ppFragments[0] == m_pDefaultFragment);
    if(m_iFragmentCount != 1)
        return err_failure;

    m_iFragmentCount = 0;
    PR_Free(m_ppFragments);
    m_ppFragments = NULL;

    FileClose();

    m_pDefaultFragment = NULL;

    if (m_pCatalog)
        PR_Free (m_pCatalog);

    m_pCurrentConfFile = NULL;

    if(m_pszCatalogFileName)
        PL_strfree(m_pszCatalogFileName);

    return noErr;
}

// -------------------------------------------------------------------
//
//    FileAddref
//
// -------------------------------------------------------------------
inline SKERR SKPageFile::FileAddRef (void)
{
    m_lRefFileCount++;

    // a shortcut
    if(m_fp)
        return noErr;

    return FileOpen();
}

// -------------------------------------------------------------------
//
//    FileRelease
//
// -------------------------------------------------------------------
inline void SKPageFile::FileRelease (void)
{
    m_lRefFileCount--;
    if(m_lRefFileCount == 0 && 
            ( m_iOpenMode == Transient || m_iOpenMode == Premapped ))
        FileClose();
}

// -------------------------------------------------------------------
//
//    Open
//
// -------------------------------------------------------------------
SKERR SKPageFile::FileOpen (void)
{
    SKERR err;
    // do not open if it is already done
    if(m_fp)
        return noErr;

    SK_ASSERT(NULL != m_pszFileName);
    // open new file
    if (m_pszFileName)
    {
        // open file
        m_fp = skPR_Open (m_pszFileName, PR_RDONLY, 0);
        if (!m_fp)
            return SKError(err_skf_fopen, "[SKPageFile::SetFileName] "
                    "Could not open %s", m_pszFileName);

        // do we need to load the catalog ?
        if (m_pszCatalogFileName && !m_pCatalog)
            err = LoadCatalog();
        else if (!m_lFileSize)
        {
            // get file sizea
            PRFileInfo info;
            PRStatus status = PR_GetOpenFileInfo(m_fp, &info);
            if( status != PR_SUCCESS )
                return SKError(err_skf_fseek, "[SKPageFile::SetFileName] "
                               "Could not get size for %s", m_pszFileName);
            m_lFileSize = info.size;
            m_lRecordCount = INVALID_VALUE;

            // compute the page count to avoid reopening the file
            GetPageCount();
        }
    }
    return noErr;
}

// -------------------------------------------------------------------
//
//    FileClose
//
// -------------------------------------------------------------------
void SKPageFile::FileClose (void)
{
    if (m_fp)
    {
        PR_Close (m_fp);
    }
    m_fp = NULL;
}

// -------------------------------------------------------------------
//
//    SetFileName
//
// -------------------------------------------------------------------
SKERR SKPageFile::SetFileName(const char *pszFileName,
                              const char *pszDefaultFileName)
{
    // close file & buffer, etc.
    Terminate();

    // reinit
    SKERR err = Init();
    if(err != noErr)
        return err;

    // inherited
    err = SKFile::SetFileName(pszFileName, pszDefaultFileName);

    if (err != noErr)
        return err;

    return noErr;
}

// -------------------------------------------------------------------
//
//    IsAvailable
//
// -------------------------------------------------------------------

PRBool SKPageFile::IsAvailable ()
{
    if(m_iOpenMode == Premapped)
        return noErr;

    SKERR err = FileAddRef();
    FileRelease();
    return err == noErr;
}

// -------------------------------------------------------------------
//
//    GetRecordCount
//
// -------------------------------------------------------------------
PRUint32 SKPageFile::GetRecordCount (void)
{
    if(m_lRecordCount == INVALID_VALUE)
    {
        if(!m_lFileSize)
            FileOpen();
        if(m_lRecordSize)
            m_lRecordCount = m_lFileSize / m_lRecordSize;
    }

    return m_lRecordCount;
}

// -------------------------------------------------------------------
//
//    GetPageCount
//
// -------------------------------------------------------------------
PRUint32 SKPageFile::GetPageCount (void)
{
    if(m_lPageCount == INVALID_VALUE)
    {
        // Make sure the file has been opened.
        FileOpen();
        if (m_lRecordSize > 0 && m_lPageSize > 0)
            m_lPageCount = (((m_lFileSize / m_lRecordSize) + m_lPageSize - 1)
                            / m_lPageSize);
    }

    return m_lPageCount;
}

// -------------------------------------------------------------------
//
//    Load
//
// -------------------------------------------------------------------
SKERR SKPageFile::Load(PRUint32 lOffset, PRUint32 lNeededSize,
                       SKPageFileFragment **ppStaticFragment)
{
    /*
    printf("SKPageFile::Load(lOffset=%u lNeededSize=%u)\n", 
            lOffset, lNeededSize);
    */
    PRUint32    lInternalOffset = 0;
    PRUint32    iOffsetInFile = 0;
    PRUint32    iSizeInFile = 0;
    PRUint32    iUncompressedOffsetInFile = 0;
    PRUint32    iUncompressedSizeInFile = 0;

    SKERR       err;

    SK_ASSERT(lNeededSize >= 0);
    
    // invalid parameters
    if (lNeededSize < 0 || lOffset < 0)
        return SKError(err_skf_invalid, "[SKPageFile::Load] "
                "Invalid arguments (offset=%ld, size=%ld)", lOffset,
                lNeededSize);
 
    SKPageFileFragment *pLoadF = NULL;
    SKPageFileFragment *pAppendF = NULL;

    // 1/ If ppStaticFragment is NULL then we will look for an existing
    // fragment which contains the wanted data. If none is found then we will
    // load the wanted data in m_pDefaultFragment.
    // 2/ If ppStaticFragment is not NULL then we know that none of the
    // existing fragments contains the wanted data (checked by
    // LoadStaticRecord). If the wanted data can be appended to the last
    // fragment of the list then we will do it. If it cannot be appended then
    // we will load the wanted data in a newly allocated fragment.
    if(!ppStaticFragment)
    {
        for(PRUint32 i = 0; i < m_iFragmentCount; ++i)
        {
            SK_ASSERT(NULL != m_ppFragments[i]);
            // check whether to data is part of the current buffer
            if(m_ppFragments[i]->Contains(lOffset, lNeededSize))
            {
                m_pActiveFragment = m_ppFragments[i];
                m_lInternalOffset = lOffset - m_ppFragments[i]->m_iBufferOffset;
                return noErr;
            }
        }
        pLoadF = m_pDefaultFragment;
    }
    else
    {
        // Only TDAs can be compressed.
        SK_ASSERT(m_iCompMode == CompressionNone);
        if(m_iFragmentCount > 1)
        {
            pAppendF = m_ppFragments[m_iFragmentCount - 1];
            if((lOffset < pAppendF->m_iBufferOffset) ||
               (lOffset > pAppendF->m_iBufferOffset + pAppendF->m_iBufferSize))
                pAppendF = NULL;
        }
        if(!pLoadF && !pAppendF)
        {
            pLoadF = sk_CreateInstance(SKPageFileFragment)(this);
            SK_ASSERT(NULL != pLoadF);
            if(!pLoadF)
                return err_memory;

            m_ppFragments = (SKPageFileFragment **)
                PR_Realloc(m_ppFragments,
                    (m_iFragmentCount + 1) * sizeof(SKPageFileFragment *));
            SK_ASSERT(NULL != m_ppFragments);

            m_ppFragments[m_iFragmentCount++] = pLoadF;
        }
        if(pLoadF)
            *ppStaticFragment = pLoadF;
        else if(pAppendF)
            *ppStaticFragment = pAppendF;
        if(*ppStaticFragment)
            (*ppStaticFragment)->AddRef();
    }

    SK_ASSERT(pLoadF || pAppendF);

    err = FileAddRef();
    // can not open file
    if (err != noErr)
    {
        FileRelease();
        return SKError(err_skf_nofile, "[SKPageFile::Load] "
                "the file %s is not open", m_pszFileName);
    }

    // must translate the offset if the file is compressed
    if (m_iCompMode != CompressionNone)
    {
        SK_ASSERT(NULL != pLoadF);

        // last page is not reachable
        PRUint32 lPageNum = FindCompressedPage (lOffset);
        if (lPageNum < 0 || lPageNum >= m_lCatalogPages - 1)
        {
            FileRelease();
            return SKError(err_skf_invalidof7, "[SKPageFile::Load] "
                    "Invalid non-compressed offset %ld", lOffset);
        }

        // get the offset & size of the compressed page
        iOffsetInFile = m_pCatalog[lPageNum].comp_offset;
        iSizeInFile = m_pCatalog[lPageNum + 1].comp_offset
            - m_pCatalog[lPageNum].comp_offset;

        // the permanent buffer need to hold the entire page
        iUncompressedOffsetInFile = m_pCatalog[lPageNum].offset;
        iUncompressedSizeInFile = m_pCatalog[lPageNum+1].offset
            - m_pCatalog[lPageNum].offset;

        lInternalOffset = lOffset - iUncompressedOffsetInFile;
    }
    else
    {
        if(pLoadF)
        {
            lInternalOffset = 0;
            iOffsetInFile = iUncompressedOffsetInFile = lOffset;
            iSizeInFile = lNeededSize;
            // avoid to load less than MIN_LOAD_BYTES bytes
            if(iSizeInFile < MIN_LOAD_BYTES)
            {
                if(lOffset + MIN_LOAD_BYTES < m_lFileSize)
                    iSizeInFile = MIN_LOAD_BYTES;
                else
                    iSizeInFile = m_lFileSize - lOffset;
            }

            iUncompressedSizeInFile = iSizeInFile;
        }
        else
        {
            SK_ASSERT(NULL != pAppendF);
        }
    }

    PRUint32 lKeySize = 0;
    const char* pcKey = "";
#ifdef  SK_USE_EXTERNAL_PROTECTION
    SKFactoryCryptKey((const unsigned char**) &pcKey,
                      (long unsigned *)&lKeySize);
#endif

    if(pLoadF)
    {
        m_pActiveFragment = pLoadF;
        m_lInternalOffset = lInternalOffset;

        err = pLoadF->LoadFromFile(m_fp, iOffsetInFile, iSizeInFile,
            iUncompressedOffsetInFile, iUncompressedSizeInFile, m_iCompMode, 
            m_pszFileName, pcKey, lKeySize);
    }
    else
    {
        SK_ASSERT(NULL != pAppendF);
        
        m_pActiveFragment = pAppendF;
        m_lInternalOffset = lOffset - pAppendF->m_iBufferOffset;

        err = pAppendF->AppendFromFile(m_fp, lOffset, lNeededSize,
            m_pszFileName, pcKey, lKeySize);
    }
    
    FileRelease();
    if(err != noErr)
        return err;

    return noErr;
}

// -------------------------------------------------------------------
//
//    LoadPage
//
// -------------------------------------------------------------------
SKERR SKPageFile::LoadPage (PRUint32 lPageNum)
{
    // out of bounds
    if (lPageNum < 0 || lPageNum >= GetPageCount())
    {
        FileRelease();
        return SKError(err_skf_invalid, "[SKPageFile::LoadPage] "
                "Invalid arguments");
    }

    // last page may not contain as many records
    PRUint32 lSize;
    if (lPageNum < GetPageCount() - 1)
        // read an extra record on the next page
        lSize = m_lPageSize + 1;
    else
    {
        // ...unless it's the last page
        lSize = GetRecordCount() % m_lPageSize;
        if( GetRecordCount() && (GetRecordCount() % m_lPageSize == 0) )
            lSize = m_lPageSize;
    }

    // load records
    return LoadRecord (lPageNum * m_lPageSize, lSize);
}

// -------------------------------------------------------------------
//
//    LoadRecord
//
// -------------------------------------------------------------------
SKERR SKPageFile::LoadRecord (PRUint32 lRecordNum, PRUint32 lRecordCount /*=1*/)
{
    // out of bounds
    if (lRecordNum < 0 || (lRecordNum + lRecordCount) > GetRecordCount())
        return SKError(err_skf_invalid, "[SKPageFile::LoadRecord] "
                "Invalid arguments (lRecordNum=%d, lRecordCount=%d, GetRecordCount()=%d", lRecordNum, lRecordCount, GetRecordCount());

    // bad file type
    if (m_lRecordSize <= 0)
        return SKError(err_skf_badtype, "[SKPageFile::LoadRecord] "
                "No fixed-length records in file %s", m_pszFileName);

    // load buffer
    return Load(lRecordNum * m_lRecordSize, lRecordCount * m_lRecordSize);
}

SKERR SKPageFile::LoadStaticRecord(PRUint32 lRecordNum, PRUint32 lRecordCount,
                                   SKPageFileFragment **ppFragment)
{
    // out of bounds
    if (lRecordNum < 0 || (lRecordNum + lRecordCount) > GetRecordCount())
        return SKError(err_skf_invalid, "[SKPageFile::LoadStaticRecord] "
                "Invalid arguments (lRecordNum=%d, lRecordCount=%d, GetRecordCount()=%d", lRecordNum, lRecordCount, GetRecordCount());

    for(PRUint32 i = 0; i < m_iFragmentCount; ++i)
    {
        SK_ASSERT(NULL != m_ppFragments[i]);
        // check whether to data is part of the current buffer
        if(m_ppFragments[i]->Contains(lRecordNum * m_lRecordSize,
                                      lRecordCount * m_lRecordSize))
        {
            if(    (m_ppFragments[i] == m_pDefaultFragment)
                && (m_iOpenMode != Premapped))
            {
                *ppFragment = sk_CreateInstance(SKPageFileFragment)(this);
                SK_ASSERT(NULL != *ppFragment);
                if(!*ppFragment)
                    return err_memory;

                m_ppFragments = (SKPageFileFragment **)
                    PR_Realloc(m_ppFragments,
                        (m_iFragmentCount + 1) * sizeof(SKPageFileFragment *));
                SK_ASSERT(NULL != m_ppFragments);

                m_ppFragments[m_iFragmentCount++] = *ppFragment;

                m_pDefaultFragment->Swap(*ppFragment);

                return noErr;
            }
            else
            {
                m_ppFragments[i]->AddRef();
                *ppFragment = m_ppFragments[i];
                return noErr;
            }
        }
    }

    PRUint32 iDiff = lRecordNum % m_lPageSize;
    lRecordNum = lRecordNum - iDiff;
    lRecordCount = lRecordCount + iDiff + m_lPageSize;
    lRecordCount = lRecordCount - lRecordCount % m_lPageSize + 1;
    if(lRecordNum + lRecordCount > GetRecordCount())
        lRecordCount = GetRecordCount() - lRecordNum;
    SK_ASSERT((lRecordNum + lRecordCount) <= GetRecordCount());

    // bad file type
    if (m_lRecordSize <= 0)
        return SKError(err_skf_badtype, "[SKPageFile::LoadStaticRecord] "
                "No fixed-length records in file %s", m_pszFileName);

    // load buffer
    SKERR err = Load(lRecordNum * m_lRecordSize, lRecordCount * m_lRecordSize,
                ppFragment);
    SK_ASSERT((*ppFragment)->Contains(lRecordNum * m_lRecordSize,
                                      lRecordCount * m_lRecordSize));
    return err;
}

// -------------------------------------------------------------------
//
//    Unload
//
// -------------------------------------------------------------------
SKERR SKPageFile::Unload()
{
    return m_pDefaultFragment->Unload(m_lMaxCacheSize);
}

// -------------------------------------------------------------------
//
//    GetRecordBuffer
//
// -------------------------------------------------------------------
SKERR SKPageFile::GetRecordBuffer(PRUint32 lIndex, void **ppValue)
{
    SK_ASSERT(lIndex < GetRecordCount());
    if(lIndex >= GetRecordCount())
        return SKError(err_failure, "[SKPageFile::GetRecordBuffer] "
                       "record out of bounds for file %s", m_pszFileName);

    *ppValue = NULL;

    SKERR err = LoadPage(lIndex / m_lPageSize);

    if(err == noErr)
    {
        *ppValue = ((char *)m_pActiveFragment->m_pBuffer + m_lInternalOffset
            + m_lRecordSize * (lIndex % m_lPageSize));
        return noErr;
    }

    return err;
}

// -------------------------------------------------------------------
//
//    GetRecord
//
// -------------------------------------------------------------------
SKERR SKPageFile::GetRecord(PRUint32 lIndex, SKIRecord** ppIRecord)
{
    if(lIndex >= GetRecordCount())
        return SKError(err_failure, "[SKPageFile::GetRecord] "
                       "record out of bounds for file %s", m_pszFileName);

    PRUint32 div = lIndex / m_lPageSize;
    SKERR err = LoadPage(div);
    SK_ASSERT(err == noErr);
    if(err != noErr)
        return err;

    SKRecord *pRec;
    if(m_pActiveFragment != m_pDefaultFragment)
    {
        err = m_pRecordPool->GetRecord(&pRec,
                (char *)m_pActiveFragment->m_pBuffer + m_lInternalOffset +
                (lIndex - m_lPageSize * div) * m_lRecordSize,
                PR_TRUE);
        if(err == noErr)
            err = pRec->SetFragment(m_pActiveFragment);
    }
    else
    {
        err = m_pRecordPool->GetRecord(&pRec,
                (char *)m_pActiveFragment->m_pBuffer + m_lInternalOffset +
                (lIndex - m_lPageSize * div) * m_lRecordSize,
                PR_TRUE);
    }
    if(err != noErr)
        return err;

    pRec->SetId(lIndex);

    *ppIRecord = pRec;

    return noErr;
}

void *SKPageFile::GetBufferPtr()
{
    return m_pActiveFragment->m_pBuffer ?
        (char *)m_pActiveFragment->m_pBuffer + m_lInternalOffset :
        NULL;
}

// -------------------------------------------------------------------
//
//    Configure
//
// -------------------------------------------------------------------
SKERR SKPageFile::ConfigureItem (char* szSection, char* szToken, char* szValue)
{
#ifdef DEBUG_CONFIG
    fprintf(stderr, "SKPageFile:: [%s] %s=%s\n", szSection, szToken, szValue);
#endif
    if(!szToken || !szValue) return noErr;
    // compression mode
    if (!PL_strcmp (szToken, "COMPRESSION"))
    {
        sk_upper(szValue);
        if (!PL_strcmp (szValue, "ZIP"))
        {
            m_iCompMode = CompressionZip;
            return noErr;
        }
    }
    // compression catalog
    else if (!PL_strcmp (szToken, "CATALOG"))
    {
        return FindFileInEnvirPath(szValue, &m_pszCatalogFileName);
    }
    // page size
    else if (!PL_strcmp (szToken, "PAGESIZE"))
    {
        m_lPageSize = atol(szValue);
        return noErr;
    }
    // max cache size
    else if (!PL_strcmp (szToken, "MAXCACHESIZE"))
    {
        m_lMaxCacheSize = atol(szValue);
        return noErr;
    }
    // alternates loading mode
    else if (!PL_strcmp (szToken, "OPENMODE"))
    {
        sk_upper (szValue);
        m_iOpenMode = StringToOpenMode(szValue);
        if(m_iOpenMode == Standard)
            return err_invalid;
        return noErr;
    }
    else if (!PL_strcmp (szToken, "CRYPT"))
    {
        m_bEncrypted = !PL_strcmp(szValue, "YES");
        return noErr;
    }
    // fields
    else if (*szToken == FIELD_ESCAPE_CHAR)
    {
        return m_pFldCollection->Configure(szToken + 1, szValue);
    }
    // default
    return SKFile::ConfigureItem(szSection, szToken, szValue);
}

//  -------------------------------------------------------------------
//
//    Check
//
//  -------------------------------------------------------------------
SKERR SKPageFile::Check()
{
    SKERR err = noErr;
    if(m_pFldCollection)
    {
        m_lRecordSize = m_pFldCollection->GetRecordSize();
        *m_pRecordPool.already_AddRefed() =
            sk_CreateInstance(SKRecordPool)(POOL_SIZE, m_lRecordSize);
        if(!m_pRecordPool)
            return err_memory;

        err = m_pFldCollection->Check();
        if(err != noErr)
            return err;
    }
    
    // read the automap limit size from the envir
    SKEnvir *pEnv = NULL;
    err = SKEnvir::GetEnvir(&pEnv);
    if(err != noErr)
        return err;
    char* pcAutomapLimit = NULL;
    PRUint32 lAutomapLimit;
    pEnv->GetValue(SKF_BE_NATIVE_AUTOMAPLIMIT_ENVIR_VAR, &pcAutomapLimit);
    lAutomapLimit = atol(pcAutomapLimit);
    if(!lAutomapLimit)
        lAutomapLimit = 1024;
    PL_strfree(pcAutomapLimit);

    err = FileAddRef();

    // we have not find the file, but for some modes it may be ok
    if(err != noErr)
        return (m_iOpenMode == Transient || m_iOpenMode == Delayed 
                || m_iOpenMode == Mapped ) ? noErr : err;
   
    if( (m_iOpenMode == Premapped || m_lFileSize < lAutomapLimit)
        && m_iCompMode == CompressionNone)
    {
        err = Load(0, m_lFileSize, NULL);
        if(err != noErr)
        {
            FileRelease();
            return err;
        }
    }

    FileRelease();

    return noErr;
}

// -------------------------------------------------------------------
//
//    LoadCatalog
//
// -------------------------------------------------------------------
SKERR SKPageFile::LoadCatalog ()
{
    PRFileDesc* f;
    PRUint32 lSize;

    if (m_iCompMode == CompressionNone)
        return SKError(err_skf_invalid, "[SKPageFile::LoadCatalog] Invalid arguments");

    // open it
    f = skPR_Open (m_pszCatalogFileName, PR_RDONLY, 0);
    if (!f)
        return SKError(err_skf_fopen, "[SKPageFile::LoadCatalog] Could not open file %s", m_pszCatalogFileName);

    // compute the size of the file
	PRFileInfo info;
    PRStatus status = PR_GetOpenFileInfo(f, &info);
    if( status != PR_SUCCESS )
      return err_failure;
    
    lSize = info.size;

    // allocate room for an extra record
    m_pCatalog = (CatalogPtr) PR_Malloc (lSize + sizeof(CatalogRec));
    if (!m_pCatalog)
    {
        PR_Close (f);
        return SKError(err_skf_malloc, "[SKPageFile::LoadCatalog] Failed to allocate %ld bytes", lSize + sizeof(CatalogRec));
    }

    // load the catalog file starting at offset 1
    if (PR_Read (f, m_pCatalog + 1, lSize) != (PRInt32) lSize)
    {
        PR_Close (f);
        return SKError(err_skf_fread, "[SKPageFile::LoadCatalog] Could not read %ld bytes from file %s", lSize, m_pszFileName);
    }

    PR_Close (f);

    // the file contains the sizes of the pages
    // before and after compression
    // instead we need the compressed and deflated offsets
    // of the beginning of each page
    //
    // the last record gives the size of the file
    m_lCatalogPages = 1 + (lSize / sizeof(CatalogRec));

    // first page is always 0
    m_pCatalog[0].offset = 0;
    m_pCatalog[0].comp_offset = 0;

    for (PRUint32 i=1; i<m_lCatalogPages; i++)
    {
#ifdef IS_BIG_ENDIAN
        REVERSE_32(m_pCatalog[i].offset);
        REVERSE_32(m_pCatalog[i].comp_offset);
#endif
        m_pCatalog[i].offset += m_pCatalog[i-1].offset;
        m_pCatalog[i].comp_offset += m_pCatalog[i-1].comp_offset;
    }

    m_lFileSize = m_pCatalog[m_lCatalogPages - 1].offset;

    // done
    return noErr;
}

// -------------------------------------------------------------------
//
//    FindCompressedPage
//
// -------------------------------------------------------------------
PRUint32 SKPageFile::FindCompressedPage (PRUint32 lOffset)
{
    if (m_pCatalog && m_lCatalogPages > 0)
    {
        PRInt32 pos, inf = -1, sup = m_lCatalogPages;

        while (sup > inf + 1)
        {
            pos = (inf + sup) / 2;
            if (m_pCatalog[pos].offset == lOffset)
                return pos;
            else if (m_pCatalog[pos].offset > lOffset)
                sup = pos;
            else
                inf = pos;
        }
        return inf;
    }
    else
        return INVALID_VALUE;
}

void SKPageFile::ReleaseFragment(SKPageFileFragment *pFragment)
{
    SK_ASSERT(pFragment != m_pDefaultFragment);
    PRUint32 i;
    for(i = 0; i < m_iFragmentCount; ++i)
    {
        if(m_ppFragments[i] == pFragment)
            break;
    }
    SK_ASSERT(i < m_iFragmentCount);
    if(m_pActiveFragment == pFragment)
    {
        m_pDefaultFragment->Swap(pFragment);
        m_pActiveFragment = m_pDefaultFragment;
        m_lInternalOffset = 0;
    }
    for(; i + 1 < m_iFragmentCount; ++i)
        m_ppFragments[i] = m_ppFragments[i + 1];
    SK_ASSERT(i + 1 == m_iFragmentCount);
    m_iFragmentCount--;
}

