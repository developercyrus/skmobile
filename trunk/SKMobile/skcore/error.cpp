/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: error.cpp,v 1.1.2.3 2005/02/17 15:29:20 krys Exp $
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

#include "machine.h"
#include "error.h"
#include "log.h"

SKERR SKError(SKERR lErrCode, const char* pszFormat, ...)
{
    va_list argp;
    va_start(argp, pszFormat);

    skConsoleLog::vLog(pszFormat, argp);

    va_end(argp);

    return lErrCode;
}

SKERR SKFirstError(SKERR err1, SKERR err2)
{
    if(err1 != noErr)
        return err1;

    return err2;
}
