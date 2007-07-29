/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: envir.cpp,v 1.7.4.2 2005/02/17 15:29:20 krys Exp $
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

#include <nspr/nspr.h>
#include <nspr/plstr.h>
#include <nspr/plhash.h>

#include "../machine.h"
#include "../error.h"

#include "envir.h"

#include "../refcount.h"
#include "../skptr.h"
#include "../file/skfopen.h"
#include "../file/file.h"
#include "../file/textfile.h"
#include "../file/inifile.h"

class State
{
public:
    State()
    {
        m_pHash = PL_NewHashTable(32, PL_HashString,
                    PL_CompareStrings, PL_CompareValues,
                    &m_sAllocOps, NULL);
    };

    State(const State* pState)
    {
        m_pHash = PL_NewHashTable(32, PL_HashString,
                    PL_CompareStrings, PL_CompareValues,
                    &m_sAllocOps, NULL);
        PL_HashTableEnumerateEntries(pState->m_pHash, CloneElement, this);
    };
    ~State()
    {
        PL_HashTableDestroy(m_pHash);
    };

    SKERR SetValue(const char* pKey, const char* pValue)
    {
        char * pOldValue;
        if((pOldValue = (char*) PL_HashTableLookup(m_pHash, pKey)))
        {
            PL_HashTableRemove(m_pHash, pKey);
        }
        char* pHashKey = PL_strdup(pKey);
        char* pHashValue = PL_strdup(pValue);
        PL_HashTableAdd(m_pHash, pHashKey, pHashValue);
        return noErr;
    }

    SKERR GetValue(const char* pKey, char **pValue)
    {
        *pValue = (char *)PL_HashTableLookup(m_pHash, pKey);

        if(*pValue)
        {
            *pValue = PL_strdup(*pValue);
        }
        else
            *pValue = PL_strdup("");

        return noErr;
    }

    SKERR ImportValue(const char *pszKey, const char *pszSystemKey,
                      PRBool bForce)
    {
        char *pszValue;
        if(!bForce)
        {
            pszValue = (char *)PL_HashTableLookup(m_pHash, pszKey);
            if(pszValue)
                return noErr;
        }

        return LoadEnvirFromIni(pszSystemKey);
    }

	SKERR LoadEnvirFromIni(const char * key = NULL)
	{
		static char ENV_INI[] = "sk.ini";
		static char ENV_SECTION[] = "envir";

		SKERR err        = noErr;
		char * homeDir = NULL;
		err = GetValue(SK_HOME_ENVIR_VAR, &homeDir);
		if(err != noErr || !homeDir || 0 == PL_strlen(homeDir))
		{
			return err_notfound;
		}

		char envirFilename[1024];
		PR_snprintf(envirFilename, sizeof envirFilename - 1, "%s/sk.ini", homeDir);
		SKTextFile iniFile;
		err = iniFile.SetFileName(envirFilename);
		if(err != noErr)
		{
			SKError(err_failure, "[SKTextFile::LoadEnvirFromIni] "
				"Failed to set filename, error = %d, envirFilename = %s", err, envirFilename);
		}

		IniParser parser(&iniFile);

		char* pszSection = NULL;
		char* pszName    = NULL;
		char* pszValue   = NULL;
		PRUint32 line = 0;
		while (    (err == noErr)
			&& parser.readIniLine(&pszSection, &pszName, &pszValue, &line))
		{
			if(NULL != pszValue && 0 == strcmp(pszSection, ENV_SECTION) && 
				(NULL == key || 0 == strcmp(key, pszName)) )
			{
				err = SetValue(pszName, pszValue);
				if(err != noErr)
					SKError(err_failure, "[SKTextFile::LoadEnvirFromIni] "
					"Stops parsing. Error %d on line %d of %s : %s/%s=%s",
					err, line, ENV_INI, pszSection, pszName, pszValue);
			}
		}
		return noErr;
	}

private:

    static PRIntn PR_CALLBACK CloneElement(PLHashEntry *he, PRIntn i,
                                           void* pThis)
    {
        ((State*)pThis)->SetValue((char*)he->key, (char*)he->value);
        return HT_ENUMERATE_NEXT;
    }

    static void * PR_CALLBACK AllocTable(void *pool, PRSize size)
    {
        return PR_MALLOC(size);
    }

    static void PR_CALLBACK FreeTable(void *pool, void *item)
    {
        PR_Free(item);
    }

    static PLHashEntry * PR_CALLBACK AllocEntry(void *pool, const void *key)
    {
        return PR_NEW(PLHashEntry);
    }

    static void PR_CALLBACK FreeEntry(void *pool, PLHashEntry *he, PRUintn flag)
    {
        if(flag == HT_FREE_ENTRY)
        {
            PL_strfree((char*)he->key);
            PL_strfree((char*)he->value);
            PR_Free(he);
        }
        else if(flag == HT_FREE_VALUE)
        {
            PL_strfree((char*)he->key);
            PL_strfree((char*)he->value);
        }
    }

    static PLHashAllocOps m_sAllocOps;

    PLHashTable *m_pHash;
};

PLHashAllocOps State::m_sAllocOps =
    { State::AllocTable, State::FreeTable,
      State::AllocEntry, State::FreeEntry };

class Token : public PRCListStr, public State
{
public:
    Token() {};
    Token(const State * ps) : State(ps) { };
private:
};

class SKRealEnvir : public SKEnvir
{
public:
    SKRealEnvir()
    {
        m_List = NULL;
        m_pLock = NULL;
    }
    virtual ~SKRealEnvir()
    {
        if(m_pLock)
            PR_DestroyLock(m_pLock);

        if(m_List)
        {
            while(!PR_CLIST_IS_EMPTY(m_List))
            {
                // Don't use Pop() here because it creates an infinite loop
                Token* pt = (Token*) PR_LIST_TAIL(m_List);
                PR_REMOVE_LINK(pt);
                delete pt;
            }
            delete m_List;
        }
    }
    SKERR Init()
    {
        m_pLock = PR_NewLock();
        if(!m_pLock)
            return err_memory;

        m_List = new Token;
        PR_INIT_CLIST(m_List);
        Token *pt = new Token();
        PR_APPEND_LINK(pt, m_List);

		SKERR err = ImportValue(SK_HOME_ENVIR_VAR, SK_HOME_ENVIR_VAR, PR_FALSE);
		if(err == noErr || err == err_notfound)
			return noErr;
		else
			return err;
    }

    virtual void Lock()
    {
        PR_Lock(m_pLock);
    }
    virtual void Unlock()
    {
        PR_Unlock(m_pLock);
    }

    SKERR Push()
    {
        SK_ASSERT(!PR_CLIST_IS_EMPTY(m_List));
        Token* pt = (Token*) PR_LIST_TAIL(m_List);
        pt = new Token(pt);
        if(!pt)
            return err_memory;
        PR_APPEND_LINK(pt, m_List);
        return noErr;
    }
    SKERR Pop()
    {
        SK_ASSERT(!PR_CLIST_IS_EMPTY(m_List));
        Token* pt = (Token*) PR_LIST_TAIL(m_List);
        PR_REMOVE_LINK(pt);
        if(PR_CLIST_IS_EMPTY(m_List))
        {
            PR_APPEND_LINK(pt, m_List);
            return SKError(err_invalid,"[SKRealEnvir::Pop()]"
                    "Attempt to flush entirely the context stack.");
        }
        delete pt;
        return noErr;
    }
    SKERR SetValue(const char* pszKey, const char* pszValue)
    {
        return ((Token*) PR_LIST_TAIL(m_List))->SetValue(pszKey, pszValue);
    }
    SKERR AppendToValue(const char* pszKey, const char* pszValue)
    {
        char *pszVal, *pszNewVal;
        SKERR err = ((Token*) PR_LIST_TAIL(m_List))->GetValue(pszKey, &pszVal);
        if(err != noErr)
            return err;
        if(pszVal)
        {
            PRUint32 iLen1 = PL_strlen(pszVal), iLen2 = PL_strlen(pszValue);
            pszNewVal = (char*)PR_Malloc(iLen1 + iLen2 + 1);
            SK_ASSERT(NULL != pszNewVal);
            PL_strcpy(pszNewVal, pszVal);
            PL_strcpy(pszNewVal + iLen1, pszValue);
            PL_strfree(pszVal);
            err = ((Token*) PR_LIST_TAIL(m_List))->SetValue(pszKey, pszNewVal);
            PR_Free(pszNewVal);
            return err;
        }
        else
        {
            PL_strfree(pszVal);
            return ((Token*) PR_LIST_TAIL(m_List))->SetValue(pszKey, pszValue);
        }
    }
    SKERR PrependToValue(const char* pszKey, const char* pszValue)
    {
        char *pszVal, *pszNewVal;
        SKERR err = ((Token*) PR_LIST_TAIL(m_List))->GetValue(pszKey, &pszVal);
        if(err != noErr)
            return err;
        if(pszVal)
        {
            PRUint32 iLen1 = PL_strlen(pszVal), iLen2 = PL_strlen(pszValue);
            pszNewVal = (char*)PR_Malloc(iLen1 + iLen2 + 1);
            SK_ASSERT(NULL != pszNewVal);
            PL_strcpy(pszNewVal, pszValue);
            PL_strcpy(pszNewVal + iLen2, pszVal);
            PL_strfree(pszVal);
            err = ((Token*) PR_LIST_TAIL(m_List))->SetValue(pszKey, pszNewVal);
            PR_Free(pszNewVal);
            return err;
        }
        else
        {
            PL_strfree(pszVal);
            return ((Token*) PR_LIST_TAIL(m_List))->SetValue(pszKey, pszValue);
        }
    }
    SKERR GetValue(const char* pszKey, char **ppszValue)
    {
        return ((Token*) PR_LIST_TAIL(m_List))->GetValue(pszKey, ppszValue);
    }

    virtual SKERR ImportValue(const char *pszKey, const char *pszSystemKey,
                              PRBool bForce)
    {
        return ((Token*) PR_LIST_TAIL(m_List))
            ->ImportValue(pszKey, pszSystemKey, bForce);
    }

private:
    PRLock *m_pLock;
    Token* m_List;
};

class skEnvirHolder
{
public:
    ~skEnvirHolder()
    {
        if(s_pInstance)
        {
            delete s_pInstance;
            s_pInstance = NULL;
        }
    }
    SKERR GetEnvir(SKEnvir **ppEnvir);
    SKERR SetEnvir(SKEnvir *pEnvir);
    static SKEnvir *s_pInstance;
};

SKEnvir *skEnvirHolder::s_pInstance = NULL;

SKERR skEnvirHolder::GetEnvir(SKEnvir **ppEnvir)
{
    *ppEnvir = NULL;

    if(!s_pInstance)
    {
        s_pInstance = new SKRealEnvir;
        if(!s_pInstance)
            return err_memory;

        SKERR err = ((SKRealEnvir *)s_pInstance)->Init();
        if(err != noErr)
        {
            delete s_pInstance;
            s_pInstance = NULL;
            return err;
        }
    }

    *ppEnvir = s_pInstance;

    return noErr;
}

SKERR skEnvirHolder::SetEnvir(SKEnvir *pEnvir)
{
    SK_ASSERT(!s_pInstance || (s_pInstance == pEnvir));

    if(s_pInstance && (s_pInstance != pEnvir))
        delete s_pInstance;

    s_pInstance = pEnvir;

    return noErr;
}

static skEnvirHolder g_skEnvirHolder;

SKERR SKEnvir::GetEnvir(SKEnvir **ppEnvir)
{
    return g_skEnvirHolder.GetEnvir(ppEnvir);
}

SKERR SKEnvir::SetEnvir(SKEnvir *pEnvir)
{
    return g_skEnvirHolder.SetEnvir(pEnvir);
}

SKERR SKEnvir::Init(const char * skHome)
{
	SKERR err = SetValue(SK_HOME_ENVIR_VAR, skHome);
	if(err != noErr)
	{
		return SKError(err_invalid,"[SKEnvir::Init()]"
			"Failed to set SH_HOME to %s.", skHome);
	}

	err = this->ImportValue(NULL, NULL, PR_TRUE);
	if(err != noErr)
	{
		return SKError(err_invalid,"[SKEnvir::Init()]"
			"Failed to load all properties from INI.");
	}

	return noErr;
}

