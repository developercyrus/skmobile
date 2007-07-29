/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: factory.cpp,v 1.24.2.9 2005/02/17 15:29:21 krys Exp $
 *
 * Authors: Mathieu Poumeyrol <poumeyrol @at@ idm .dot. fr>
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

#ifdef HAVE_CONFIG_H
#include "skcore/skbuildconfig.h"
#endif

#include <nspr/nspr.h>
#include <nspr/plstr.h>
#include <nspr/plhash.h>

#include "../machine.h"
#include "../error.h"
#include "../log.h"
#include "../refcount.h"
#include "../skptr.h"
#include "../envir/envir.h"
#include "../factory/factory.h"
#include "../simplifier.h"
#include "../unicode/unicodesimplifier.h"

#include "skfind/skfind.h"
#include "skfind/backend/native/stream.h"
#include "skfind/backend/native/field.h"
#include "skfind/backend/native/record.h"
#include "skfind/backend/native/buf.h"
#include "skfind/backend/native/lc.h"
#include "skfind/backend/native/table.h"
#include "skfind/backend/cursor/cursorrs.h"

SK_COMPONENT_DEFAULT_CREATOR(SKCursorRecordSet);

#ifdef WIN32
#include "windows.h"
// #include <sys/types.h>
// #include <sys/stat.h>
#include <stdio.h>
static PRBool CheckDriveType(const char * pathName, PRBool bIncludeRemote);
static char findLogicalDrive(const char * pszVolumeName, PRBool bIncludeRemote);

#elif defined(LINUX)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <mntent.h>

#include "linux/iso_fs.h"

static PRBool checkVolumeName(const char * device, const char * volume_label);
#elif defined(SK_MACOSX)
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif

#ifdef SK_USE_EXTERNAL_PROTECTION
extern "C" unsigned long SKFactoryExternalProtection(const char* pszUrl);
extern "C" unsigned long SKFactoryExternalProtectionLogout();
#endif

//============================================================================
// Globals
//============================================================================

// string used to match [lib]xxx_rtldg.[so.x.x|dll|dylib...]
#define SK_RTLD_GLOBAL_SUFFIX "_rtldg."

PLHashAllocOps SKFactory::m_sAllocOps =
    { SKFactory::AllocTable, SKFactory::FreeTable,
      SKFactory::AllocEntry, SKFactory::FreeEntry };

static const char* s_ppcszValidDllExtensions[] = {
#ifdef XP_PC
    ".dll",     /* Windows */
#endif
#ifdef XP_UNIX
    ".so",      /* Unix */
    ".dylib",   /* Mac OS X */
#endif
#ifdef XP_MAC
    ".shlb",    /* Mac OS 9 */
#endif
#ifdef SK_MACOSX
    ".dylib",   /* Mac OS X */
#endif
    NULL
};

#ifdef XP_MAC
static const char* s_pcszValidDllPrefix = "sk";
#endif

#ifdef SK_MACOSX
#define NEW_FLIMIT 1024
// moved from mac/sysresource.c
void RaiseMacResourceLimit()
{
    struct rlimit lim;
    lim.rlim_cur = (rlim_t)NEW_FLIMIT;
    lim.rlim_max = (rlim_t)NEW_FLIMIT;
    setrlimit( RLIMIT_NOFILE, &lim );
}
#endif

//============================================================================
// SKFactory
//============================================================================

SKFactory::SKFactory()
{
    m_pHash = NULL;
    m_bIsProtected = PR_FALSE;
    m_bIsTerminated = PR_FALSE;
}

//============================================================================
// ~SKFactory
//============================================================================

SKFactory::~SKFactory()
{
}

//============================================================================
// Terminate
//============================================================================

SKERR SKFactory::Terminate()
{
    m_bIsTerminated = PR_TRUE;
#ifdef SK_USE_EXTERNAL_PROTECTION
    if (m_bIsProtected)
    {
        m_bIsProtected = PR_FALSE;
        return SKFactoryExternalProtectionLogout();
    }
#endif
    return noErr;
}

//----------------------------------------------------------------------------
// Init
//----------------------------------------------------------------------------

SKERR SKFactory::Init()
{
    if (m_bIsTerminated)
         return SKError(err_fac_terminated,"[SKFactory] Terminated");

    skConsoleLog::Init();

#ifdef SK_USE_EXTERNAL_PROTECTION
    SKERR err = SKFactoryExternalProtection(NULL);
    if(err != noErr)
        return err;
    m_bIsProtected = PR_TRUE;
#endif

    m_pHash = PL_NewHashTable(32, PL_HashString,
                              PL_CompareStrings, PL_CompareValues,
                              &m_sAllocOps, this);
    return OnInit();
}

//----------------------------------------------------------------------------
// Reactivate
//----------------------------------------------------------------------------

SKERR SKFactory::Reactivate()
{
    if (!m_bIsTerminated)
         return noErr;

#ifdef SK_USE_EXTERNAL_PROTECTION
    SKERR err = SKFactoryExternalProtection(NULL);
    if(err != noErr)
        return err;
    m_bIsProtected = PR_TRUE;
#endif

    return noErr;
}

//----------------------------------------------------------------------------
// HasComponent
//----------------------------------------------------------------------------

SKERR SKFactory::HasComponent(const char *pszComponent, PRBool *pbResult)
{
    if (m_bIsTerminated)
         return SKError(err_fac_terminated,"[SKFactory] Terminated");

    if (!m_pHash)
         return SKError(err_fac_not_initialised,
                        "[SKFactory::HasComponent] Factory not initialized");

    SK_ASSERT(pszComponent && pbResult);
    if(!pszComponent || !pbResult)
        return err_invalid;

    *pbResult = PR_FALSE;

    char *pszEnd = PL_strchr(pszComponent, ':');
    if(!pszEnd)
        return err_failure;
    char *pszKey = PL_strndup(pszComponent, pszEnd - pszComponent);
    void *pData = PL_HashTableLookup(m_pHash, pszKey);
    PL_strfree(pszKey);
    if(pData)
        return RealHasComponent(pData, pszEnd + 1, pbResult);
    else
    {
        if(pbResult)
            *pbResult = PR_FALSE;
        return noErr;
    }
}

//----------------------------------------------------------------------------
// CreateInstance
//----------------------------------------------------------------------------

SKERR SKFactory::CreateInstance(const char *pszParam, SKRefCount **ppInstance)
{
    if (m_bIsTerminated)
         return SKError(err_fac_terminated,"[SKFactory] Terminated");

    if (!m_pHash)
         return SKError(err_fac_not_initialised,
                        "[SKFactory::CreateInstance] Factory not initialized");

    SK_ASSERT(NULL != pszParam);
    if(!pszParam)
        return err_failure;

    char *pszEnd = PL_strchr(pszParam, ':');
    if(!pszEnd)
        return err_failure;
    char *pszKey = PL_strndup(pszParam, pszEnd - pszParam);
    void *pData = PL_HashTableLookup(m_pHash, pszKey);
    SKERR err;
    if(pData)
        err = RealCreateInstance(pData, pszEnd + 1, ppInstance);
    else
        err = SKError(err_notfound,
                       "[SKFactory::CreateInstance] component '%s' not found",
                       pszKey);
    PL_strfree(pszKey);
    return err;
}

//----------------------------------------------------------------------------
// AllocTable
//----------------------------------------------------------------------------

void * PR_CALLBACK SKFactory::AllocTable(void *pool, PRSize size)
{
    return PR_MALLOC(size);
}

//----------------------------------------------------------------------------
// FreeTable
//----------------------------------------------------------------------------

void PR_CALLBACK SKFactory::FreeTable(void *pool, void *item)
{
    PR_Free(item);
}

//----------------------------------------------------------------------------
// AllocEntry
//----------------------------------------------------------------------------

PLHashEntry * PR_CALLBACK SKFactory::AllocEntry(void *pool, const void *key)
{
    return PR_NEW(PLHashEntry);
}

//----------------------------------------------------------------------------
// AllocTable
//----------------------------------------------------------------------------

void PR_CALLBACK SKFactory::FreeEntry(void *pool, PLHashEntry *he, PRUintn flag)
{
    if(flag == HT_FREE_ENTRY)
    {
        PL_strfree((char*)he->key);
        ((SKFactory*)pool)->DeleteValue(he->value);
        PR_Free(he);
    }
    else if(flag == HT_FREE_VALUE)
    {
        PL_strfree((char*)he->key);
        ((SKFactory*)pool)->DeleteValue(he->value);
    }
}

//----------------------------------------------------------------------------
// AddDisk
//----------------------------------------------------------------------------

SKERR SKFactory::AddDisk(const char * pszVolumeName, PRBool bIncludeRemote)
{
    SKERR err = noErr;
    SKEnvir *pEnv = NULL;
    SKEnvir::GetEnvir(&pEnv);
#ifdef WINCE
	// do nothing
#elif defined(WIN32)
    char pszPath[] = "A:\\;";
    char driveLetter = findLogicalDrive(pszVolumeName, bIncludeRemote);
    if( driveLetter == 0 )
        return SKError(err_notfound, "[SKFactory::AddDisk] "
                "Volume : '%s' not found", pszVolumeName);

    pszPath[0] = driveLetter;

    pEnv->PrependToValue("PATH", pszPath );

#elif defined(LINUX)
    FILE * mtab;
    struct mntent * entry;
    char * device;
    char * fs;
    char * tmp;

    mtab = setmntent("/etc/mtab", "r");
    while ((entry = getmntent(mtab)) != NULL)
    {
        device = NULL;
        if (!PL_strcmp(entry->mnt_type, "supermount"))
        {
            fs = hasmntopt(entry, "fs");
            if (fs != NULL)
            {
                while (*fs++ != '=') fs++;
                tmp = fs;
                while (*tmp != ',') tmp++; *tmp = 0;
                if (!PL_strcmp(fs, "iso9660"))
                {
                    device  = hasmntopt(entry, "dev");
                    if (device != NULL)
                    {
                        while (*device++ != '=');
                        tmp = device;
                        while (*tmp != ',') tmp++; *tmp = 0;
            //skConsoleLog::DebugLog("Device = %s, fs = %s\n", device, fs);
                    }
                }
                else if (!PL_strcmp(fs, "auto"))
                {
                    // if the filesystem is "auto", checks if the mount point
                    // contains CD or DVD
                    char * mnt_dir = entry->mnt_dir;
                    if ((PL_strstr(mnt_dir, "/cd") != NULL) ||
                        (PL_strstr(mnt_dir, "/dvd") != NULL) )
                    {
                        device  = hasmntopt(entry, "dev");
                        if (device != NULL)
                        {
                            while (*device++ != '=');
                            tmp = device;
                            while (*tmp != ',') tmp++; *tmp = 0;
            //skConsoleLog::DebugLog("Device = %s, fs = %s\n", device, fs);
                        }
                    }
                }
            }
        }
        else if (!PL_strcmp(entry->mnt_type, "subfs"))
        {
            fs = hasmntopt(entry, "fs");
            if (fs != NULL)
            {
                while (*fs++ != '=') fs++;
                tmp = fs;
                while (*tmp != ',') tmp++; *tmp = 0;
                if (!PL_strcmp(fs, "cdfss"))
                {
                    device = entry->mnt_fsname;
                }
            }
        }
        else if (!PL_strcmp(entry->mnt_type, "iso9660"))
        {
            device = entry->mnt_fsname;
        }
        if (device != NULL)
        {
            if (checkVolumeName(device, pszVolumeName))
            {
                break;
            }
        }
    }
    endmntent(mtab);

    if (entry != NULL)
    {
        skConsoleLog::Log("Found CD ?s?at : %s",
                pszVolumeName, entry->mnt_dir);
    }
    else
    {
        return SKError(err_notfound, "[SKFactory::AddDisk] "
                "Volume : '%s' not found", pszVolumeName);
    }

    pEnv->PrependToValue("PATH", ";");
    pEnv->PrependToValue("PATH", entry->mnt_dir );

#elif defined(XP_MAC)
    PRUint32 uiLength = PL_strlen(pszVolumeName);
    char* pszTemp = (char *)PR_Malloc((uiLength+3) * sizeof(char));
    pszTemp[0] = '/';
    PL_strcpy( pszTemp + 1, pszVolumeName );
    pszTemp[uiLength + 1] = '/';
    pszTemp[uiLength + 2] = '\0';
    if( PR_Access(pszTemp, PR_ACCESS_EXISTS) == PR_SUCCESS )
    {
        //skConsoleLog::Log( "Found CD ?s?at : %s", pszVolumeName, pszTemp );
        pEnv->PrependToValue("PATH", ";");
        pEnv->PrependToValue("PATH", pszVolumeName );
        pEnv->PrependToValue("PATH", "/");
    }
    else
        err = SKError(err_notfound, "[SKFactory::AddDisk] "
                      "Volume : '%s' not found", pszVolumeName);

	PL_strfree( pszTemp );
#elif defined(SK_MACOSX)
#define MAC_VOLUMEPATH_PREFIX "/Volumes/"
    // CD: discussion
    // On Mac OS 10.3, the startup volume is correctly aliased in the /Volumes
    // mount dir. That was not the case in 10.2 but we don't care, since we are
    // not going to call addDisk on the startup volume.
    // If we ever want to, we will have to test the volume name against the
    // startup volume name and just add "/" in that case
    PRUint32 uiLength = PL_strlen(pszVolumeName) + sizeof(MAC_VOLUMEPATH_PREFIX);
    char* pszTemp = (char *)PR_Malloc( uiLength * sizeof(char) );
    PL_strcpy( pszTemp, MAC_VOLUMEPATH_PREFIX );
    PL_strcat( pszTemp, pszVolumeName );
    if( PR_Access(pszTemp, PR_ACCESS_EXISTS) == PR_SUCCESS )
    {
        // skConsoleLog::Log( "Found CD '%s' at : %s", pszVolumeName, pszTemp );
        pEnv->PrependToValue("PATH", ";");
        pEnv->PrependToValue("PATH", pszTemp );
    }
    else
        err = SKError(err_notfound, "[SKFactory::AddDisk] "
                      "Volume : '%s' not found", pszVolumeName);

	PL_strfree( pszTemp );
#else
#error "SKFactory::AddDisk does nothing on your platform. Add code here."
#endif

    return err;
}

//----------------------------------------------------------------------------
// UrlDecode
//----------------------------------------------------------------------------

SKERR SKFactory::UrlDecode(char*s)
{
    char* pc = s;
    for( ; *s; s++)
    {
        //+ to spaces
        if(*s=='+')
            *pc = ' ';
        else if(*s=='%')
        {
            // check that we still have 2 cars to read
            if(s[1] && s[2])
            {
                // now do they look like valid hex digits ?
                if (   ((s[1]>='0'&&s[1]<='9')
                            || (s[1]>='a' && s[1]<='f')
                            || (s[1]>='A' && s[1]<='F'))
                        && ((s[2]>='0'&&s[2]<='9')
                            || (s[2]>='a' && s[2]<='f')
                            || (s[2]>='A' && s[2]<='F')) )
                {
                    char tmp[3];
                    tmp[2] = '\0';
                    tmp[0] = s[1];
                    tmp[1] = s[2];
                    // do the actual conversion
                    *pc = (char) strtol(tmp, NULL, 16);
                }
                else
                {
                    *pc = '?';
                    return SKError(err_invalid, "[SKFactory::UrlDecode] "
                                                "bad URL format");
                }
                // we have to eat to more caracters
                s += 2;
            }
            else
            {
                // we do not have enough to eat, so we are at the end of
                // the buffer
                *pc = '?';
                pc[1] = '\0';
                return SKError(err_invalid, "[SKFactory::UrlDecode] "
                                            "bad URL format");
            }
        }
        else
            *pc = *s;
        pc++;
    }
    *pc ='\0';
    return noErr;
}

//============================================================================
// SKSubFactory
//============================================================================

SKSubFactory::~SKSubFactory()
{
	if(m_pHash)
		PL_HashTableDestroy(m_pHash);
}

//----------------------------------------------------------------------------
// OnInit
//----------------------------------------------------------------------------

SKERR SKSubFactory::OnInit()
{
	return noErr;
}

//----------------------------------------------------------------------------
// RealHasComponent
//----------------------------------------------------------------------------

SKERR SKSubFactory::RealHasComponent(void *pData, const char *pszComponent,
									 PRBool *pbResult)
{
	*pbResult = (pData != NULL);
	return noErr;
}

//----------------------------------------------------------------------------
// RealCreateInstance
//----------------------------------------------------------------------------

SKERR SKSubFactory::RealCreateInstance(void *pData, const char *pszParam,
									   SKRefCount **ppInstance)
{
	return ((SKComponentData_t*)pData)->m_pfInstanceCreator(pszParam, ppInstance);
}

//----------------------------------------------------------------------------
// DeleteValue
//----------------------------------------------------------------------------

void SKSubFactory::DeleteValue(void *pValue)
{
	delete (SKComponentData_t *)pValue;
}

//----------------------------------------------------------------------------
// RegisterComponents
//----------------------------------------------------------------------------

SKERR SKSubFactory::RegisterComponents( SKComponentData_t *pData )
{
	SKComponentData_t *pComponent =
		(SKComponentData_t *)PL_HashTableLookup(m_pHash, pData->m_pszName);
	if(!pComponent)
	{
		pComponent = new SKComponentData_t;
		pComponent->m_pszType = pData->m_pszType;
		pComponent->m_pszName = pData->m_pszName;
		pComponent->m_pfInstanceCreator = pData->m_pfInstanceCreator;
		char *pszKey = PL_strdup(pData->m_pszName);
		PL_HashTableAdd(m_pHash, pszKey, pComponent);
	}
	return noErr;
}

//============================================================================
// SKInlineFactory
//============================================================================

SKInlineFactory::~SKInlineFactory()
{
	if(m_pHash)
		PL_HashTableDestroy(m_pHash);
}

//----------------------------------------------------------------------------
// OnInit
//----------------------------------------------------------------------------

SKERR SKInlineFactory::OnInit()
{
	static SKComponentData_t componentData[] = {
		{"simplifier", "unicode", skStringUnicodeSimplifier::CreateUnicodeSimplifier},
		{"recordset", "native", SKTable::CreateNativeRecordSet},
		{"recordset", "cursor", _Create_SKCursorRecordSet},
		{NULL, NULL, NULL}
	};

	for(SKComponentData_t * pData = componentData; NULL != pData->m_pszType; pData++)
	{
		RegisterComponents(pData);
	}

	return noErr;
}

//----------------------------------------------------------------------------
// RealHasComponent
//----------------------------------------------------------------------------

SKERR SKInlineFactory::RealHasComponent(void *pData, const char *pszComponent,
									 PRBool *pbResult)
{
	return ((SKSubFactory*)pData)->HasComponent(pszComponent, pbResult);
}

//----------------------------------------------------------------------------
// RealCreateInstance
//----------------------------------------------------------------------------

SKERR SKInlineFactory::RealCreateInstance(void *pData, const char *pszParam,
									   SKRefCount **ppInstance)
{
	return ((SKSubFactory*)pData)->CreateInstance(pszParam, ppInstance);
}

//----------------------------------------------------------------------------
// DeleteValue
//----------------------------------------------------------------------------

void SKInlineFactory::DeleteValue(void *pValue)
{
	delete (SKSubFactory *)pValue;
}

//----------------------------------------------------------------------------
// RegisterComponents
//----------------------------------------------------------------------------

SKERR SKInlineFactory::RegisterComponents(SKComponentData_t *pData)
{
	SKSubFactory *pSubFactory =
		(SKSubFactory*)PL_HashTableLookup(m_pHash, pData->m_pszType);
	if(!pSubFactory)
	{
		pSubFactory = new SKSubFactory();
		SKERR err = pSubFactory->Init();
		if(err != noErr)
		{
			delete pSubFactory;
			return err;
		}
		char *pszKey = PL_strdup(pData->m_pszType);
		PL_HashTableAdd(m_pHash, pszKey, pSubFactory);
	}
	return pSubFactory->RegisterComponents(pData);
}


class skFactoryHolder
{
public:
    ~skFactoryHolder()
    {
        if(s_pInstance)
        {
            delete s_pInstance;
            s_pInstance = NULL;
        }
    }
    SKERR GetFactory(SKFactory **ppFactory);
    SKERR SetFactory(SKFactory *pFactory);
    static SKFactory *s_pInstance;
};

SKFactory *skFactoryHolder::s_pInstance = NULL;

SKERR skFactoryHolder::GetFactory(SKFactory **ppFactory)
{
    *ppFactory = NULL;

    if(!s_pInstance)
    {
        s_pInstance = new SKInlineFactory;
        if(!s_pInstance)
            return err_memory;

        SKERR err = ((SKInlineFactory *)s_pInstance)->Init();
        if(err != noErr)
        {
            delete s_pInstance;
            s_pInstance = NULL;
            return err;
        }
    }

    *ppFactory = s_pInstance;

    return noErr;
}

SKERR skFactoryHolder::SetFactory(SKFactory *pFactory)
{
    SK_ASSERT(!s_pInstance || !pFactory || (s_pInstance == pFactory));

    if(s_pInstance && (s_pInstance != pFactory))
        delete s_pInstance;

    s_pInstance = pFactory;

    return noErr;
}

static skFactoryHolder g_skFactoryHolder;

//----------------------------------------------------------------------------
// GetFactory
//----------------------------------------------------------------------------

SKERR SKFactory::GetFactory(SKFactory **ppFactory)
{
    return g_skFactoryHolder.GetFactory(ppFactory);
}

//----------------------------------------------------------------------------
// SetFactory
//----------------------------------------------------------------------------

SKERR SKFactory::SetFactory(SKFactory *pFactory)
{
    return g_skFactoryHolder.SetFactory(pFactory);
}
