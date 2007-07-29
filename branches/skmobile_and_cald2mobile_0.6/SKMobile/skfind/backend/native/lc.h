/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: lc.h,v 1.12.2.4 2005/02/22 09:03:04 bozo Exp $
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

#ifndef __LC_H_
#define __LC_H_

//    Structure of a page in LC
typedef struct LCPAGEREC
{
    char*               first_key;
    char*               last_key;
    PRUint32            first_num;
    PRUint32            last_num;
    PRUint32            offset;
    PRUint32            size;
}
LcPageRec, *LcPagePtr;

// class LC
class SKLCFile : public SKFile
{
public:
    SK_REFCOUNT_INTF_DEFAULT(SKLCFile)

                        SKLCFile ();
    virtual             ~SKLCFile (void);
    virtual SKERR       Check (void);

    typedef enum { UINT, SINT, STRING } LookupType;
    
            SKERR       Lookup (const char* pszKey, LookupType eType,
                                PRUint32 *plPageNum);

            PRUint16    GetHeaderLines (void) { return m_iHeaderLines; }
            void        SetHeaderLines (PRUint16 iHeaderLines)
                            { m_iHeaderLines = iHeaderLines; }

            PRUint32    GetPageCount (void)
                            { return m_lPageCount; }
            SKERR       ConfigureItem (char* szSection, char* szToken, 
                                        char* szValue);
            LcPageRec*  GetPage (PRUint32 lPageNum)
                            { return (m_Pages ? &m_Pages[lPageNum] : NULL); }

private:
    SKERR       Load (void);

    PRUint32    m_lPageCount;
    PRUint16    m_iHeaderLines;
    LcPageRec*  m_Pages;
    char*       m_szBuffer;
    PRPackedBool m_bLoaded;
};


//    Error codes
#define err_lc_invalid                  300
#define err_lc_malloc                   301
#define err_lc_fopen                    302
#define err_lc_fread                    303

#define err_lc_format                   305
#define err_lc_notfound                 306

#else // __LC_H_
#error "Multiple inclusions of lc.h"
#endif // __LC_H_

