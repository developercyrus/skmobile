/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: factory.h,v 1.11.4.5 2005/02/17 15:29:21 krys Exp $
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

#ifndef __SKC_FACTORY_H_
#define __SKC_FACTORY_H_

#define SK_COM_PATH_ENVIR_VAR           "SK_COM_PATH"
#define SK_COM_PATH_APPEND_ENVIR_VAR    "SK_COM_PATH_APPEND"
#define SK_DEBUG_MODULE_ENVIR_VAR       "SK_DEBUG_MODULE"

#ifdef SK_MACOSX
// moved from mac/sysresource.h
SKAPI void RaiseMacResourceLimit();
#endif

//============================================================================
// SKFactory
//============================================================================

class SKAPI SKFactory
{
public:
    SKFactory();
    virtual ~SKFactory();

    static SKERR GetFactory(SKFactory **ppFactory);
    static SKERR SetFactory(SKFactory *pFactory);

    SKERR Init();

    virtual SKERR Terminate();
    virtual SKERR Reactivate();
    SKERR AddDisk(const char * pszVolumeName, PRBool bIncludeRemote);

    SKERR HasComponent(const char *pszComponent, PRBool *pbResult);
    SKERR CreateInstance(const char *pszParam, SKRefCount **ppInstance);

    static SKERR UrlDecode(char*s);

protected:
    virtual SKERR OnInit() = 0;

    virtual SKERR RealHasComponent(void *pData, const char *pszComponent,
                                   PRBool *pbResult) = 0;
    virtual SKERR RealCreateInstance(void *pData, const char *pszParam,
                                     SKRefCount **ppInstance) = 0;

    static void * PR_CALLBACK AllocTable(void *pool, PRSize size);
    static void PR_CALLBACK FreeTable(void *pool, void *item);
    static PLHashEntry * PR_CALLBACK AllocEntry(void *pool, const void *key);
    static void PR_CALLBACK FreeEntry(void *pool, PLHashEntry *he,PRUintn flag);
    static PLHashAllocOps m_sAllocOps;

    virtual void DeleteValue(void *pValue) = 0;

    PLHashTable *m_pHash;
    PRBool      m_bIsProtected;
    PRBool      m_bIsTerminated;
};

typedef SKERR (*SKComponentInstanceCreator_t)(const char *pszParam,
                                              SKRefCount **ppInstance);

typedef struct SKComponentData_s
{
    char *m_pszType;
    char *m_pszName;
    SKComponentInstanceCreator_t m_pfInstanceCreator;
} SKComponentData_t;

typedef void (*SKModuleInitializer_t)(SKEnvir *pEnv, SKFactory *pFactory);
typedef SKComponentData_t *(*SKModuleComponentDataGetter_t)();

class SKSubFactory : public SKFactory
{
public:
	virtual ~SKSubFactory();

	SKERR RegisterComponents(SKComponentData_t *pData);

protected:

	virtual SKERR OnInit();

	virtual SKERR RealHasComponent(void *pData, const char *pszComponent,
		PRBool *pbResult);

	virtual SKERR RealCreateInstance(void *pData, const char *pszParam,
		SKRefCount **ppInstance);

	virtual void DeleteValue(void *pValue);
};


class SKInlineFactory : public SKFactory
{
public:

	virtual ~SKInlineFactory() ;

protected:

	virtual SKERR OnInit() ;

	virtual SKERR RealHasComponent(void *pData, const char *pszComponent,
		PRBool *pbResult) ;

	virtual SKERR RealCreateInstance(void *pData, const char *pszParam,
		SKRefCount **ppInstance) ;

	virtual void DeleteValue(void *pValue) ;

	SKERR RegisterComponents(SKComponentData_t *pData) ;

};

#define SK_COMPONENT_DEFAULT_CREATOR(_class)                            \
static SKERR _Create_##_class(const char *pszParam,                     \
                             SKRefCount **ppInstance)                   \
{                                                                       \
    if(ppInstance)                                                      \
        *ppInstance = NULL;                                             \
                                                                        \
    SK_ASSERT(NULL != pszParam);                                                \
    SK_ASSERT(NULL != ppInstance);                                              \
    if(!pszParam || !ppInstance)                                        \
        return err_invalid;                                             \
                                                                        \
    skPtr<_class> pInstance;                                            \
    *pInstance.already_AddRefed() = sk_CreateInstance(_class)();        \
    if(!pInstance)                                                      \
        return err_memory;                                              \
                                                                        \
    *ppInstance = pInstance;                                            \
    (*ppInstance)->AddRef();                                            \
                                                                        \
    return noErr;                                                       \
}

#define err_fac_terminated       710
#define err_fac_not_initialised  711

#endif // __SKC_FACTORY_H_
