/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: envir.h,v 1.5.4.2 2005/02/17 15:29:20 krys Exp $
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

#ifndef __SKC_ENVIR_H_
#define __SKC_ENVIR_H_

#define SK_HOME_ENVIR_VAR               "SK_HOME"

class SKAPI SKEnvir
{
public:
    virtual ~SKEnvir() { }

    virtual void Lock() = 0;
    virtual void Unlock() = 0;

    static SKERR GetEnvir(SKEnvir** ppEnvir);
    static SKERR SetEnvir(SKEnvir* pEnvir);
    virtual SKERR Push() = 0;
    virtual SKERR Pop() = 0;
    virtual SKERR GetValue(const char* pszKey, char** ppszValue) = 0;
    virtual SKERR AppendToValue(const char* pszKey, const char* pszValue) = 0;
    virtual SKERR PrependToValue(const char* pszKey, const char* pszValue) = 0;
    virtual SKERR SetValue(const char* pszKey, const char* pszValue) = 0;

    virtual SKERR ImportValue(const char *pszKey, const char *pszSystemKey,
                              PRBool bForce) = 0;

	SKERR Init(const char * skHome);

protected:
    SKEnvir() {};
};

#endif // __SKC_ENVIR_H_

