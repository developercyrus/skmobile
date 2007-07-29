/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: skcore.h,v 1.16.2.2 2005/02/17 15:29:20 krys Exp $
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

#ifndef __SKC_SKCORE_H_
#define __SKC_SKCORE_H_

#include <nspr/nspr.h>
#include <nspr/plstr.h>
#include <nspr/plhash.h>

#include "skcoreconfig.h"

#include "machine.h"
#include "swap.h"
#include "error.h"
#include "log.h"

// C++ includes
#ifdef __cplusplus

#include "refcount.h"
#include "skptr.h"

#include "sort.h"

#include "envir/envir.h"

#include "factory/factory.h"

#include "file/skfopen.h"
#include "file/file.h"
#include "file/textfile.h"
#include "file/output.h"
#include "file/inifile.h"

#include "integerlist.h"
#include "stringlist.h"

#include "simplelist.h"

#include "unichar.h"
#include "tokenize.h"

#include "simplifier.h"

#include "binary.h"

#endif // __cplusplus

#else // __SKC_SKCORE_H_
#error "Multiple inclusions of skcore.h"
#endif // __SKC_SKCORE_H_

