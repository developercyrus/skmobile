/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: error.h,v 1.6.4.4 2005/02/17 15:29:20 krys Exp $
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

#ifndef __SKC_ERROR_H_
#define __SKC_ERROR_H_

typedef PRInt32 SKERR;

#ifndef noErr
 #define noErr 0
#endif

#define err_invalid                     1
#define err_null                        2
#define err_config                      3
#define err_failure                     10
#define err_notfound                    11
#define err_memory                      12
#define err_not_handled                 14
#define err_not_implemented             15
#define err_warning                     16
#define err_error                       17
#define err_fatal                       18
#define err_interrupted                 19

SKAPI SKERR SKError(SKERR lError, const char* pszFormat, ...);
SKAPI SKERR SKFirstError(SKERR err1, SKERR err2);

#endif // __SKC_ERROR_H_

