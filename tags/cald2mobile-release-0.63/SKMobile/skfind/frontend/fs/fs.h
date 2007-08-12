/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: fs.h,v 1.30.2.3 2005/02/21 14:22:45 krys Exp $
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

#ifndef __FS_H_
#define __FS_H_

#define err_fs_invalid                  3000
#define err_fs_malloc                   3001

class SKFSObject;
class SKFSFile;
class SKFSDirectory;

#define SK_SKFILESYSTEM_IID                                             \
{ 0xe9d2b1f1, 0xca0d, 0x455d,                                           \
    { 0xb5, 0x8f, 0x57, 0x96, 0xd2, 0xce, 0x3f, 0x4e } }

class SKAPI SKFileSystem : public SKTextFile
{
friend class SKFSDirectory;
friend class SKFSFile;
public:
    SK_REFCOUNT_INTF_DEFAULT(SKFileSystem)
    SK_REFCOUNT_INTF_IID(SKFileSystem, SK_SKFILESYSTEM_IID)

    SKFileSystem();
    
    // -------------------------------------------------------------------
    // configuration interface
    // it could change when a table manager will be here to handle table
    // creation

    // for file configuration 
    virtual SKERR           SetFileName(const char* pszFileName,
                                        const char *pszDefaultFileName = NULL);
    
    // for specific options...
    virtual SKERR           ConfigureItem(  char* szSection, char* szToken, 
                                            char* szValue);
 
    // -------------------------------------------------------------------
    // user interface
 
            SKERR           GetRootDir(SKFSDirectory** ppRootDir);
            SKERR           GetDir(const char* pszName, SKFSDirectory** ppDir);
            SKERR           GetDirByPath(   const char* pszName, 
                                            SKFSDirectory** ppDir)
                                        { return GetDir(pszName, ppDir);}

            SKERR           GetFile(const char* pszName, SKFSFile** ppFile);
            SKERR           GetFileByPath(  const char* pszName, 
                                            SKFSFile** ppFile)
                                        { return GetFile(pszName, ppFile);}
            
            SKERR           GetObject(  const char* pszName,
                                        SKFSObject** ppObject);
            SKERR           GetObjectByPath(    const char* pszName, 
                                                SKFSObject** ppObject)
                                        { return GetObject(pszName, ppObject);}

            SKERR           GetDir(PRUint32 lIdx, SKFSDirectory** ppDir);
            SKERR           GetFile(PRUint32 lIdx, SKFSFile** ppFile);

private:
    SKERR GetRecordSetField(SKIRecordSet *pRecordSet,
                            const char *pszFieldName,
                            SKIField **ppField);
    SKERR GetFieldSubField(SKIField *pField,
                           const char *pszSubFieldName,
                           SKIField **ppSubField);

    skPtr<SKIRecordSet>     m_pDirRecordSet;

    skPtr<SKIField>         m_pDirNameField;
    skPtr<SKIField>         m_pDirParentField;

    skPtr<SKIField>         m_pDirChildrenField;
    skPtr<SKIField>         m_pDirChildrenSubField;

    skPtr<SKIField>         m_pDirFilesField;
    skPtr<SKIField>         m_pDirFilesSubField;

    skPtr<SKIRecordSet>     m_pFileRecordSet;

    skPtr<SKIField>         m_pFileNameField;
    skPtr<SKIField>         m_pFileParentField;

    skPtr<SKIField>         m_pFileTitleField;
    skPtr<SKIField>         m_pFileContentField;
};

SK_REFCOUNT_DECLARE_INTERFACE(SKFileSystem, SK_SKFILESYSTEM_IID)

#define SK_SKFSOBJECT_IID                                               \
{ 0x312c2bfe, 0x93c4, 0x4118,                                           \
    { 0x88, 0x4f, 0xe3, 0x6d, 0x3e, 0x14, 0x05, 0x1f } }

class SKAPI SKFSObject : public SKRefCount
{
public:
    SK_REFCOUNT_INTF_IID(SKFSObject, SK_SKFSOBJECT_IID)

    SKFSObject(SKFileSystem* pFileSystem, SKIRecord* pRecord);
    virtual ~SKFSObject();

            const char*     GetSharedName() const { return m_pszName; };
            SKERR           GetName(char** ppcName) const
                                { *ppcName = PL_strdup(GetSharedName()); 
                                    return noErr; }
            const char*     GetSharedPath()
                            {
                                if(!m_pszPath)
                                {
                                    SKERR err = FetchParents();
                                    SK_ASSERT(err == noErr);
                                    if(err == noErr)
                                        ComputePath();
                                }
                                return m_pszPath;
                            };
            SKERR           GetPath(char** ppcPath)
                                { *ppcPath = PL_strdup(GetSharedPath()); 
                                    return noErr; }
            SKERR           GetLocation(SKFSDirectory **ppDir);

            SKERR           GetParentDir(SKFSDirectory **ppDir);

            SKERR           GetFSObjectId(PRUint32 *piId);

    virtual PRPackedBool    SharedIsFile() const = 0;
    virtual SKERR           IsFile(PRBool* pBool) const
                                { *pBool = SharedIsFile(); return noErr; }

            SKERR           FetchParents();
protected:
    virtual SKERR           FetchParent() = 0;
            void            ComputePath();

    skPtr<SKFileSystem>     m_pFileSystem;
    skPtr<SKIRecord>        m_pRecord;
    skPtr<SKFSDirectory>    m_pParentDir;

    char*                   m_pszName;
    char*                   m_pszPath;
public:
    // This is needed by the skPtr class for a dark reason. This
    // constructor must NOT be used/called/anythingelse. -- bozo
    SKFSObject();
};

SK_REFCOUNT_DECLARE_INTERFACE(SKFSObject, SK_SKFSOBJECT_IID)

#define SK_SKFSDIRECTORY_IID                                            \
{ 0x30653ab1, 0x3845, 0x497d,                                           \
    { 0x8c, 0xb7, 0x1b, 0xdc, 0xe5, 0x17, 0x67, 0xf1 } }

class SKAPI SKFSDirectory : public SKFSObject
{
public:
    SK_REFCOUNT_INTF(SKFSDirectory)
    SK_REFCOUNT_INTF_CREATOR(SKFSDirectory)(SKFileSystem* pFileSystem,
                                            SKIRecord* pRecord);
    SK_REFCOUNT_INTF_IID(SKFSDirectory, SK_SKFSDIRECTORY_IID)

    // configuration interface 
    SKFSDirectory(SKFileSystem* pFileSystem,
                  SKIRecord* pRecord) : SKFSObject(pFileSystem, pRecord)
        { };
    virtual ~SKFSDirectory() {};

	SKERR			Init() { return FetchInfo(); }
    // user interface
            SKERR           GetDirCount(PRUint32 *plCount) const;
            SKERR           GetDir(PRUint32 lRank, SKFSDirectory** ppDir) const;
            SKERR           GetFileCount(PRUint32 *plCount) const;
            SKERR           GetFile(PRUint32 lRank, SKFSFile** ppFile) const;

            SKERR           GetDir(const char* pszName, SKFSDirectory** ppDir);
            SKERR           GetDirByPath(   const char* pszName, 
                                            SKFSDirectory** ppDir)
                                { return GetDir(pszName, ppDir); }
            SKERR           GetFile(const char* pszName, SKFSFile** ppFile);
            SKERR           GetFileByPath(  const char* pszName, 
                                            SKFSFile** ppFile)
                                { return GetFile(pszName, ppFile); }
            SKERR           GetObject(const char* pszName,
                                      SKFSObject** ppObject);
            SKERR           GetObjectByPath(const char* pszName, 
                                            SKFSObject** ppObject)
                                { return GetObject(pszName, ppObject); }

    virtual PRPackedBool    SharedIsFile() const { return PR_FALSE; }

protected:
            SKERR           FetchInfo();
    virtual SKERR           FetchParent();
private:
    // This is needed by the skPtr class for a dark reason. This
    // constructor must NOT be used/called/anythingelse. -- bozo
    SKFSDirectory();
};

SK_REFCOUNT_DECLARE_INTERFACE(SKFSDirectory, SK_SKFSDIRECTORY_IID)

#define SK_SKFSFILE_IID                                                 \
{ 0xbe9c3845, 0x7d9f, 0x4fb3,                                           \
    { 0x8a, 0x4e, 0xfc, 0xfc, 0xea, 0x1b, 0x3b, 0xc9 } }

class SKAPI SKFSFile : public SKFSObject
{
public:
    SK_REFCOUNT_INTF(SKFSFile)
    SK_REFCOUNT_INTF_CREATOR(SKFSFile)(SKFileSystem* pFileSystem,
                                       SKIRecord* pRecord);
    SK_REFCOUNT_INTF_IID(SKFSFile, SK_SKFSFILE_IID)

    SKFSFile(SKFileSystem* pFileSystem,
             SKIRecord* pRecord) : SKFSObject(pFileSystem, pRecord)
        {};
    virtual ~SKFSFile() {};

			SKERR			Init() { return FetchInfo(); }

    // user interface
            SKERR           GetData(SKBinary** ppBinary);
            SKERR           GetTitle(SKBinary** ppBinary);

    virtual PRPackedBool    SharedIsFile() const { return PR_TRUE; }

protected:
            SKERR           FetchInfo();
    virtual SKERR           FetchParent();
private:
    // This is needed by the skPtr class for a dark reason. This
    // constructor must NOT be used/called/anythingelse. -- bozo
    SKFSFile();
};

SK_REFCOUNT_DECLARE_INTERFACE(SKFSFile, SK_SKFSFILE_IID)

#define err_fs_fopen         702

#endif

