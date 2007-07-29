/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: parse.h,v 1.6.2.2 2005/02/21 14:22:46 krys Exp $
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

#ifndef __PARSE_H_
#define __PARSE_H_

typedef struct SEARCHTOKEN
{
    skfOperator m_oper;
    char *      m_pszToken; // NULL = ignore this token (stopword for instance)
    PRBool      m_bPhrase;
    void *      m_pPrivateData;
}
SearchToken, *SearchTokenPtr;

SKAPI SKERR     SetOperators(char* pszOps);
//SKAPI char*     GetOperator(Operator op);

#define err_prs_invalid		600
#define err_prs_malloc		601

#define err_prs_syntax		605
#define err_prs_wildcard	606
#define err_prs_overflow	607

#else // __PARSE_H_
#error "Multiple inclusions of parse.h"
#endif // __PARSE_H_

