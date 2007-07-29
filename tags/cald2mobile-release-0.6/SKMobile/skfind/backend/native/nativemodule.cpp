/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: nativemodule.cpp,v 1.12.4.2 2005/02/21 14:22:39 krys Exp $
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

#include <skfind/skfind.h>

#include "skfind/backend/native/native.h"

SKERR SKTable::CreateNativeRecordSet(const char* pszParam, SKRefCount **ppInstance)
{

    SK_ASSERT(NULL != pszParam);
    SK_ASSERT(NULL != ppInstance);
    if(!pszParam || !ppInstance)
        return err_invalid;

    *ppInstance = NULL;

    skPtr<SKTable> pTable;
    *pTable.already_AddRefed() = sk_CreateInstance(SKTable)();
    if(!pTable)
        return err_memory;

    if(pszParam && *pszParam)
    {
        SKERR err = pTable->SetFileName(pszParam);
        if(err != noErr)
            return err;
    }

    *ppInstance = pTable;
    (*ppInstance)->AddRef();

    return noErr;
}
/*
// Module
static SKComponentData_t g_ComponentData[] = {
    {"recordset", "native", CreateNativeRecordSet},
    {NULL, NULL, NULL}
};

extern "C" {

void SKAPI SKModuleInitialize(SKEnvir *pEnv, SKFactory *pFactory)
{
    SKEnvir::SetEnvir(pEnv);
    SKFactory::SetFactory(pFactory);
    pEnv->ImportValue(  SKF_BE_NATIVE_DEFAULT_OPENMODE_ENVIR_VAR,
                        SKF_BE_NATIVE_DEFAULT_OPENMODE_ENVIR_VAR, PR_FALSE);
    pEnv->ImportValue(  SKF_BE_NATIVE_AUTOMAPLIMIT_ENVIR_VAR,
                        SKF_BE_NATIVE_AUTOMAPLIMIT_ENVIR_VAR, PR_FALSE);
}

SKAPI SKComponentData_t *SKModuleGetComponentData()
{
    return g_ComponentData;
}

}
*/