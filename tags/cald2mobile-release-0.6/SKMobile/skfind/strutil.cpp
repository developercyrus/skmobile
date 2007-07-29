/* BEGIN LICENSE */
/*****************************************************************************
 * SKFind : the SK search engine
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: strutil.cpp,v 1.1.2.3 2005/02/21 14:22:44 krys Exp $
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

#include <skcore/skcore.h>

#include "strutil.h"

//	------------------------------------------------------------------------------
//
//	NextString
//	renvoie le mot qui suit dans l'ordre lexicographique
//	next key is "ac" for "ab", "abc" for "abb", "ad" for "acz"
//
//	------------------------------------------------------------------------------
char *sk_nextString (char *pszWord)
{
	if (pszWord && *pszWord) 
	{
		// advances to last char of the word
		char* s;
		for (s=pszWord; *s; s++) {}
		s--;

		if ((unsigned char)*s==255)
		{
			*s = 0;
			sk_nextString (pszWord);
		}
		else 
			(*s)++;
	}
	return pszWord;
}

//	------------------------------------------------------------------------------
char *sk_upper(char *psz)
{
	if(psz)
	{
		char *t;
		for (t=psz;*t;t++)
			if (*t >= 'a' && *t <= 'z')
				*t -= 0x20;
	}
	return psz;
}

char *sk_invertString(char *pszWord)
{
    char* a = pszWord;
    char* b = a;
    char c;
    for(; *b; b++){}
    --b;

    while(a < b)
    {
        c = *a;
        *(a++) = *b;
        *(b--) = c;
    }
    return pszWord;
}

