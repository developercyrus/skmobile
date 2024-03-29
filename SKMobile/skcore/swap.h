/* BEGIN LICENSE */
/*****************************************************************************
 * SKCore : the SK core library
 * Copyright (C) 1995-2005 IDM <skcontact @at@ idm .dot. fr>
 * $Id: swap.h,v 1.1.4.2 2005/02/17 15:29:20 krys Exp $
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

#ifndef __SKC_SWAP_H_
#define __SKC_SWAP_H_

// a b > b a
#define SWAP_16(x)    ((x >> 8 ) & 0xff)                \
                   | (( x        & 0xff) << 8)

// a b c > c b a
#define SWAP_24(x)    ((x >> 16) & 0xff)                \
                   | (((x >> 8 ) & 0xff) << 8 )         \
                   | (( x        & 0xff) << 16)

// a b c d > d c b a
#define SWAP_32(x)    ((x >> 24) & 0xff)                \
                   | (((x >> 16) & 0xff) << 8 )         \
                   | (((x >> 8 ) & 0xff) << 16)         \
                   | (( x        & 0xff) << 24)

#define REVERSE_16(x)                   x = SWAP_16(x)
#define REVERSE_24(x)                   x = SWAP_24(x)
#define REVERSE_32(x)                   x = SWAP_32(x)

#else
#error "Multiple inclusions of swap.h"
#endif

