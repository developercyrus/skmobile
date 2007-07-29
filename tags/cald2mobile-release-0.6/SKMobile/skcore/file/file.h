/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: file.h,v 1.3.4.2 2005/02/17 15:29:21 krys Exp $
 *
 * Authors: W.P.Dauchy
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

#ifndef __SKC_FILE_H_
#define __SKC_FILE_H_

class SKAPI SKFile : public SKRefCount
{
public:
    SK_REFCOUNT_INTF(SKFile)

                        SKFile ();
                        ~SKFile ();
    virtual SKERR       Check (void);

    // filename
    virtual SKERR       SetFileName (const char *pszFileName,
                                     const char *pszDefaultFileName = NULL);
            SKERR       GetFileName (const char*& psz);
            const char* GetSharedFileName()
                            { return m_pszFileName; }

            void        PushEnvir();
            void        PopEnvir();

    virtual SKERR       ConfigureItem(  char* pszSection,
                                        char* pszName,
                                        char* pcsValue);

protected:
            char        *m_pszFileName;
};

#define err_cnf_invalid 100

//------------------------------------------------------------------------------
#define MAKE_BOOL(s) (!PL_strcmp((s),"YES") || !PL_strcmp((s), "1") || !PL_strcmp((s), "TRUE"))

#else /* __SKC_FILE_H_ */
#error "Multiple inclusions of file.h"
#endif /* __SKC_FILE_H_ */

