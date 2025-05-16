/*
 * Copyright (c) 2025, oldteam. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef FLOOD_RANDOM_H
#define FLOOD_RANDOM_H

#include "include.h"

#define ROTL64(d,lrot) (((d)<<(lrot))|((d)>>(64-(lrot))))
#define PHI 0x9e3779b9

/*
 * main interface for random
 */

void	Srandom(u_long seed);	/* set random seed */
u_long	Random(void);		/* random num */

void	random_set(int id);
u_long	randnum(u_long min, u_long max);
u_int	random_u32(void);
u_short	random_u16(void);
u_char	random_u8(void);
u_int	random_ipv4(void);
u_long	random_seed_u64(void);

/*	cmwc method	*/
u_long	cmwc(void);
void	cmwc_seed(u_long seed);

/*	xoroshiro128plus method		*/
u_long	xoroshiro128plus(void);
void	xoroshiro128plus_seed(u_long seed);

/*	splitmix64 method		*/
u_long	splitmix64(void);
void	splitmix64_seed(u_long seed);

/*	romuduojr method	*/
void	romuduojr_seed(u_long seed);
u_long	romuduojr(void);

void	printmethods_random(void);

#endif
