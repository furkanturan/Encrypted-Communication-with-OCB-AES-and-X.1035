/* sha3.h
 *
 * The sha3 hash function (aka Keccak).
 */

/* nettle, low-level cryptographics library
 *
 * Copyright (C) 2012 Niels Möller
 *  
 * The nettle library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 * 
 * The nettle library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with the nettle library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02111-1301, USA.
 */
 
#ifndef NETTLE_SHA3_H_INCLUDED
#define NETTLE_SHA3_H_INCLUDED

#include "Globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The sha3 state is a 5x5 matrix of 64-bit words. In the notation of
   Keccak description, S[x,y] is element x + 5*y, so if x is
   interpreted as the row index and y the column index, it is stored
   in column-major order. */
#define SHA3_STATE_LENGTH 25

/* The "width" is 1600 bits or 200 octets */
struct sha3_state
{
  uint64_t a[SHA3_STATE_LENGTH];
};

#define SHA3_256_DIGEST_SIZE 32
#define SHA3_256_DATA_SIZE 136

struct sha3_256_ctx
{
  struct sha3_state state;
  unsigned index;
  uint8_t block[SHA3_256_DATA_SIZE];
};

void
sha3_256_init (struct sha3_256_ctx *ctx);

void
sha3_256_update (struct sha3_256_ctx *ctx,
		 unsigned length,
		 const uint8_t *data);

void
sha3_256_digest(struct sha3_256_ctx *ctx,
		unsigned length,
		uint8_t *digest);

void sha3(uint8_t *hashResult, uint8_t *data, int nofDataBytes);

#ifdef __cplusplus
}
#endif

#endif /* NETTLE_SHA3_H_INCLUDED */
