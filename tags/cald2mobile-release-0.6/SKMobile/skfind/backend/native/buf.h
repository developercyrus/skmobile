/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: buf.h,v 1.34.2.4 2005/03/16 13:58:24 bozo Exp $
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

#ifndef __BUF_H_
#define __BUF_H_


//
//  Compression Mode
//  --------------------------------------------------------------------------
enum CompressionMode
{
    CompressionNone = 0,
    CompressionZip = 1
};

//
//  Open Mode
//  --------------------------------------------------------------------------
enum OpenMode
{
    Standard = 0,
    Delayed,
    Transient,
    Mapped,
    Premapped
};

//
//  Catalogs for compressed files
//  --------------------------------------------------------------------------
typedef struct CATALOG
{
    PRUint32    offset;
    PRUint32    comp_offset;
}
CatalogRec, *CatalogPtr;



class SKPageFileFragment : public SKRefCount
{
public:
    SK_REFCOUNT_INTF(SKPageFileFragment)
    SK_REFCOUNT_INTF_CREATOR(SKPageFileFragment)(SKPageFile *pPagefile);

    SKPageFileFragment(SKPageFile *pPagefile);
    ~SKPageFileFragment();

    PRBool Contains(PRUint32 iOffset, PRUint32 iCount);

    SKERR LoadFromFile(PRFileDesc *pFd,
        PRUint32 iOffsetInFile, PRUint32 iSizeInFile,
        PRUint32 iUncompressedOffsetInFile, PRUint32 iUncompressedSizeInFile,
        CompressionMode compMode, const char *pszFileName,
        const char* pcKey, PRUint32 iKeySize);

    SKERR AppendFromFile(PRFileDesc *pFd,
        PRUint32 iOffsetInFile, PRUint32 iSizeInFile,
        const char *pszFileName, const char* pcKey, PRUint32 iKeySize);

    SKERR Unload(PRUint32 iMaxCacheSize);

    void Swap(SKPageFileFragment *pFragment);

    PRUint32    m_iBufferOffset;
    PRUint32    m_iBufferSize;

    PRUint32    m_iPhysicalSize;
    void *      m_pBuffer;

    skPtr<SKPageFile> m_pPageFile;
private:
    SKPageFileFragment();
};

//
//  Class SKPageFile
//  --------------------------------------------------------------------------
class SKPageFile : public SKFile
{
public:
    SK_REFCOUNT_INTF_DEFAULT(SKPageFile)

                        SKPageFile();
    virtual             ~SKPageFile();

    virtual SKERR       ConfigureItem(   char* szSection, char* szToken, 
                                         char* szValue);
    virtual SKERR       Check(void);

    // load
            PRBool      IsAvailable();
            SKERR       Load(PRUint32 lOffset, PRUint32 lSize,
                             SKPageFileFragment **ppStaticFragment = NULL);
            SKERR       LoadPage(PRUint32 lPageNum);
            SKERR       LoadRecord(PRUint32 lRecordNum,PRUint32 lRecordCount=1);
            SKERR       LoadStaticRecord(PRUint32 lRecordNum,PRUint32 lRecordCount, SKPageFileFragment **ppFragment);
            SKERR       Unload();

    // file statistics
    virtual PRUint32    GetPageCount(void);
    virtual PRUint32    GetRecordCount(void);
            PRUint32    GetFileSize(void) { return m_lFileSize; }

    // size of page in number of records
    // default is 256
    inline  PRUint32    GetPageSize(void) { return m_lPageSize; }
    inline  void        SetPageSize (PRUint32 lPageSize)
                        { m_lPageSize = lPageSize; }

    // name of file
    virtual SKERR       SetFileName(const char *pszFileName,
                                    const char *pszDefaultFileName = NULL);

    // encryption
            PRBool      GetEncrypted()
                                { return m_bEncrypted; }
            void        SetEncrypted(PRBool b) 
                                { m_bEncrypted = b; }
    
    // compression mode
            CompressionMode     GetCompressionMode (void)
                                { return m_iCompMode; }
            void                SetCompressionMode(CompressionMode iCompMode)
                                { m_iCompMode = iCompMode; }

    // buffer access
    virtual SKERR       GetRecordBuffer (PRUint32 lIndex, void **ppValue);
    virtual SKERR       GetRecord (PRUint32 lIndex, SKIRecord** ppIRecord);
    virtual void *      GetBufferPtr();
    virtual PRUint32    GetBufferSize()
            {
                SK_ASSERT(NULL != m_pActiveFragment);
                return m_pActiveFragment->m_iBufferSize;
            }
    virtual PRUint32    GetBufferOffset()
            {
                SK_ASSERT(NULL != m_pActiveFragment);
                return m_pActiveFragment->m_iBufferOffset;
            }

            PRFileDesc* GetFilePtr(void) { return m_fp; }

            SKFldCollection*    GetWeakFldCollection()
                                {
                                    SK_ASSERT(NULL != m_pFldCollection);
                                    return m_pFldCollection;
                                };

            void ReleaseFragment(SKPageFileFragment *pFragment);

    static  OpenMode StringToOpenMode(const char* szValue);

protected:
    // file layout
    PRUint32    m_lPageSize;

private:
            SKERR       FileOpen (void);
            void        FileClose (void);
            SKERR       FileAddRef (void);
            void        FileRelease (void);

    // init and terminate all
            SKERR       Init();
            SKERR       Terminate();

    // file management
    PRFileDesc* m_fp;
    PRUint32    m_lFileSize;
    PRUint32    m_lRecordCount;
    PRUint32    m_lRecordSize;
    PRUint32    m_lPageCount;
    OpenMode    m_iOpenMode;
    PRUint32    m_lRefFileCount;

    // buffer management
    skPtr<SKPageFileFragment>   m_pDefaultFragment;
    PRUint32                    m_iFragmentCount;
    SKPageFileFragment **       m_ppFragments;
    SKPageFileFragment *        m_pActiveFragment;

    PRUint32    m_lInternalOffset;
    PRUint32    m_lMaxCacheSize;

    // encryption
    PRBool      m_bEncrypted;
    
    // compression
    CompressionMode     m_iCompMode;
    CatalogPtr          m_pCatalog;
    PRUint32            m_lCatalogPages;
    char                *m_pszCatalogFileName;

            SKERR       LoadCatalog ();
            PRUint32    FindCompressedPage (PRUint32 lOffset);

    // temp var for configuration : stores the subtable file being
    // configured
    skPtr<SKFile>       m_pCurrentConfFile;

    skPtr<SKFldCollection>      m_pFldCollection;
    skPtr<SKRecordPool> m_pRecordPool;
};

//  Error codes
//  ------------------------------------------------------------------------------
#define err_skf_invalid                 200
#define err_skf_malloc                  201
#define err_skf_fopen                   202
#define err_skf_fread                   203
#define err_skf_fseek                   204

#define err_skf_nofile                  205
#define err_skf_nocatalog               206
#define err_skf_invalidof7              207
#define err_skf_invalidoption           208
#define err_skf_notda                   209
#define err_skf_notfound                210
#define err_skf_before                  211
#define err_skf_after                   212
#define err_skf_unsupported             213
#define err_skf_badtype                 214
#define err_skf_hasherror               215

#else /* !__BUF_H_ */
#error "Multiple inclusions of buf.h"
#endif /* !__BUF_H_ */

