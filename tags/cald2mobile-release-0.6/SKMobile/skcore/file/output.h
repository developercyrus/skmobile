/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: output.h,v 1.6.2.2 2005/02/17 15:29:21 krys Exp $
 *
 * Authors:  Mathieu Poumeyrol <poumeyrol @at@ idm .dot. fr>
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

#ifndef __SKC_OUTPUT_H_
#define __SKC_OUTPUT_H_

class SKAPI SKFileOutput : public SKRefCount
{
public:
    SK_REFCOUNT_INTF_DEFAULT(SKFileOutput)

                        SKFileOutput ();
                        ~SKFileOutput ();

    // filename
            const char* GetSharedFileName()
                           { return m_pszFileName; }
            
    virtual SKERR Create(const char* pcFileName);
    virtual SKERR Append(const char* pcFileName);
    virtual SKERR Write(const char* pcBuffer, PRUint32 iSize);
    virtual SKERR Printf(const char* pcFormat, ...);
    virtual SKERR vPrintf(const char* pcFormat, va_list va);
    virtual SKERR Flush();
    virtual SKERR Close();

            PRUint32 GetWritten() 
                { return m_iWritten; }

protected:
    virtual SKERR Open(const char* pcFileName, PRUint32 lFlags);
            char        *m_pszFileName;
            PRFileDesc* m_pFD;

            char*       m_pcBuffer;
            PRUint32    m_iBufferSize;
            PRUint32    m_iDeadLength;

            PRUint32    m_iWritten;
};

#define err_cnf_invalid 100

#else /* __SKC_OUTPUT_H_ */
#error "Multiple inclusions of output.h"
#endif /* __SKC_OUTPUT_H_ */

