/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: field.h,v 1.45.4.3 2005/02/21 14:22:39 krys Exp $
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

#ifndef __FIELD_H_
#define __FIELD_H_

typedef PRUint32 (*valueReader_t)(const void *pBuffer);

class SKPageFile;

//
//  errors
// ---------------------------------------------------------------------
#define err_fld_invalid 1100
#define err_fld_malloc  1101
#define err_fld_unknown  1110
#define err_fld_size  1111
#define err_fld_conf 1112

//
//  Fields
// --------------------------------------------------------------------
typedef enum _FieldType
{
    SKFT_UNKNOWN = 0,
    SKFT_LINK    = 0x08,
    SKFT_DATA    = 0x09,
    SKFT_SBYTE   = 0x10,
    SKFT_SSHORT  = 0x11,
    SKFT_SLONG   = 0x12,
    SKFT_S24     = 0x13,
    SKFT_UBYTE   = 0x18,
    SKFT_USHORT  = 0x19,
    SKFT_ULONG   = 0x20,
    SKFT_U24     = 0x21
} FieldType;

inline FieldType TypeFromName(const char * szTypeName)
{
    if(!szTypeName) return SKFT_UNKNOWN;
    if(!PL_strcmp("LINK", szTypeName)) return SKFT_LINK;
    if(!PL_strcmp("DATA", szTypeName)) return SKFT_DATA;
    if(!PL_strcmp("SBYTE", szTypeName)) return SKFT_SBYTE;
    if(!PL_strcmp("SSHORT", szTypeName)) return SKFT_SSHORT;
    if(!PL_strcmp("SLONG", szTypeName)) return SKFT_SLONG;
    if(!PL_strcmp("S24", szTypeName)) return SKFT_S24;
    if(!PL_strcmp("UBYTE", szTypeName)) return SKFT_UBYTE;
    if(!PL_strcmp("USHORT", szTypeName)) return SKFT_USHORT;
    if(!PL_strcmp("ULONG", szTypeName)) return SKFT_ULONG;
    if(!PL_strcmp("U24", szTypeName)) return SKFT_U24;
    return SKFT_UNKNOWN;
}

inline PRBool IsASignedFieldType(FieldType t)
{
    return ((t >= 0x10) && (t < 0x18));
}

inline PRBool IsAnUnsignedFieldType(FieldType t)
{
    return ((t >= 0x18) && (t <= 0x21));
}

inline PRBool IsANumericFieldType(FieldType t)
{
    return ((t >= 0x10) && (t <= 0x21));
}

inline PRBool IsACompositeFieldType(FieldType t)
{
    return ((t == SKFT_LINK) || (t == SKFT_DATA));
}

#define FIELD_ESCAPE_CHAR '$'
#define INVALID_VALUE   (0xFFFFFFFF)

class SKFile;
class SKSubTable;

//
//  Field definition
// ---------------------------------------------------------------------
class SKField : public SKIField
{
public:
    SK_REFCOUNT_INTF_DEFAULT(SKField)

                        SKField();
    virtual             ~SKField();

    /* configuration interface */
    virtual SKERR       GetName(char **ppszName) const;
    virtual SKERR       GetLinkSubRecordSet(SKIRecordSet **ppRecordSet);

            PRUint32    GetSize() { return m_lSize; }
            PRUint32    GetOffset() const { return m_lOffset; }

            SKERR       SetOffset(PRUint32 offset);
            SKERR       SetName(const char * name);
            SKERR       SetType(FieldType t);
            SKERR       SetType(const char * TypeName);


            SKERR       Configure(char* pszToken, char* pszValue);
            SKERR       ConfigureComposite(PRUint32 lId,
                                           char* pszToken, char* pszValue);
            SKERR       ConfigureFile(PRUint32 lId,
                                      char* pszToken, char* pszValue);
            SKERR       ConfigureLink(char* pszToken, char* pszValue);

            SKERR       Check();

    /* reading interface */
            SKERR       GetUNumFieldValue(const void* pBuffer,
                                          PRUint32 *plValue) const;

            SKERR       GetSNumFieldValue(const void* pBuffer,
                                          PRInt32 *plValue) const;

            SKERR       GetDataFieldValue(const void* pBuffer,
                                          SKBinary** ppBinary,
                                          const void* pNextBuffer = NULL) const;

            SKERR       GetStreamFieldValue(const void* pBuffer,
                                            SKIStream** ppStream,
                                            const void* pNextBuffer=NULL) const;

    /* native internal interface */
            SKERR       GetOffsetField(SKField** ppField);
            SKERR       GetCountField(SKField** ppField);
            SKERR       GetPackField(SKField** ppField);

            SKERR       GetFile(SKPageFile** ppFile, PRUint32 lPackId = 0)const;
            SKERR       GetFile(const void * pBuffer, SKPageFile** ppFile)const;

            const char* GetSharedName() { return m_pszName; }

            FieldType GetType() const { return m_eFieldType; }

            void        SetPosition(PRUint32 lPosition)
                            { m_lPosition = lPosition; };
            PRUint32    GetPosition() const
                            { return m_lPosition; };

    virtual SKERR   GetFieldType(skfFieldType* pType);
    virtual SKERR   IsData(PRBool * bIsData);
    virtual SKERR   IsUNum(PRBool * bIsUNum);
    virtual SKERR   IsSNum(PRBool * bIsSNum);
    virtual SKERR   IsLink(PRBool * bIsLink);
private:
    void ComputeOffsets();

    char*               m_pszName;
    PRUint32            m_lPosition;
    PRUint32            m_lOffset;
    PRUint32            m_lSize;
    FieldType           m_eFieldType;

    /* Sub fields for composite field */
    skPtr<SKField>      m_pFieldOffset;
    skPtr<SKField>      m_pFieldCount;
    skPtr<SKField>      m_pFieldPack; // only DATA fields

    /* Sub files for DATA fields */
    skPtr<SKPageFile>*  m_ppFiles;
    PRUint32            m_lFilesArraySize;

    /* Sub recordset for LINK fields */
    skPtr<SKIRecordSet> m_pSubRecordSet;
};

//
//  class SKFldCollection
// ---------------------------------------------------------------------
class SKFldCollection : public SKIFldCollection
{
public:
    SK_REFCOUNT_INTF_DEFAULT(SKFldCollection)

                    SKFldCollection(void);
    virtual         ~SKFldCollection(void);

    // used by initializer
    virtual SKERR   Configure (char * szToken, char* szValue);
    virtual SKERR   Check();

    // used to read fields
    virtual SKERR   GetField (const char *pszField, SKIField** ppField) const;
    virtual SKERR   GetFieldCount (PRUint32 *plCount) const
                        { *plCount = m_lFieldCount;  return noErr; }
    virtual SKERR   GetField(PRUint32 lIndex, SKIField **ppField) const
                        {
                            SK_ASSERT(lIndex < m_lFieldCount);
                            for(PRUint32 i = 0; i < m_lFieldCount; ++i)
                            {
                                skPtr<SKField>* papField = &m_ppFields[i];
                                if((*papField)->GetPosition() == lIndex)
                                    return papField->CopyTo((SKField**)ppField);
                            }
                            *ppField = NULL;
                            return err_fld_unknown;
                        }
    virtual SKERR   GetFieldName(PRUint32 lIndex, char **ppszName) const;

    // size of record in bytes
    inline  PRUint32   GetRecordSize () const
                                { return m_lRecordSize;}

    // used to read data from a record
            SKERR   GetUNumFieldValue ( const void* buffer,
                                        const char* szField,
                                        PRUint32 *plValue) const;

            SKERR   GetSNumFieldValue ( const void* buffer,
                                        const char* szField,
                                        PRInt32 *plValue) const;
protected:
            PRUint32            m_lRecordSize;
            PRUint32            m_lTotalFieldSize;
            PRUint32            m_lFieldCount;
            skPtr<SKField>*     m_ppFields;
            PRPackedBool        m_bSorted;

    inline  void            Sort(void);
};

#endif /* __FIELD_H_ */

