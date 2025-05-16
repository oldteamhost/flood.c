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

#include "../include/random.h"

void	(*SRANDOM)(u_long);
u_long	(*RANDOM)(void);

void Srandom(u_long seed)
{
	SRANDOM(seed);
}

u_long Random(void)
{
	return RANDOM();
}

void random_set(int id)
{
	switch (id) {
		case 1:
			SRANDOM=cmwc_seed;
			RANDOM=cmwc;
			printf("[%s]\tset random \"%s\" id=%d\n",
				__FILE__, "cmwc", id);
			break;
		case 2:
			SRANDOM=splitmix64_seed;
			RANDOM=splitmix64;
			printf("[%s]\tset random \"%s\" id=%d\n",
				__FILE__, "splitmix64", id);
			break;
		case 3:
			SRANDOM=romuduojr_seed;
			RANDOM=romuduojr;
			printf("[%s]\tset random \"%s\" id=%d\n",
				__FILE__, "romuduojr", id);
			break;
		case 0:
		default:
			SRANDOM=xoroshiro128plus_seed;
			RANDOM=xoroshiro128plus;
			printf("[%s]\tset random \"%s\" id=%d\n",
				__FILE__, "xoroshiro128plus", id);
			break;
	}
}

u_long randnum(u_long min, u_long max)
{
	if (min>max)
		 return 0;
	if (min==max)
		 return min;
	return min+(RANDOM()%(max-min+1UL));
}

u_int random_u32(void)
{
	return (u_int)randnum(0,UINT_MAX);
}

u_short random_u16(void)
{
	return (u_short)randnum(0,USHRT_MAX);
}

u_char random_u8(void)
{
	return (u_char)randnum(0,UCHAR_MAX);
}

u_int random_ipv4(void)
{
	u_int res;

	res=htonl(((u_int)(random_u8())<< 24)|
		((u_int)(random_u8())<<16)|((u_int)
		(random_u8())<<8)|(u_int)random_u8());

	return res;
}

static u_long Q[4096], c=362436;

u_long cmwc(void)
{
	u_long x, r=0xfffffffe;
	u_long t, a=18782LL;
	static u_long i=4095;

	i=(i+1)&4095;
	t=a*Q[i]+c;
	c=(t>>32);
	x=t+c;

	if (x<c) {
		x++;
		c++;
	}

	return (Q[i]=r-x);
}

void cmwc_seed(u_long seed)
{
	u_int i;
	Q[0]=seed;
	Q[1]=seed+PHI;
	Q[2]=seed+PHI+PHI;
	for (i=3;i<4096;i++)
		Q[i]=Q[i-3]^Q[i-2]^PHI^i;
}

static u_long s[2] = {1, 2};

u_long xoroshiro128plus(void)
{
	u_long s0=s[0];
	u_long s1=s[1];
	u_long res=s0+s1;

#define rotl(x,k)	((x<<k)|(x>>(64-k)))
	s1^=s0;
	s[0]=rotl(s0,55)^s1^(s1<<14);
	s[1]=rotl(s1,36);

	return res;
}

void xoroshiro128plus_seed(u_long seed)
{
	s[0]=seed;
	/* golden ratio */
	s[1]=seed^0x9E3779B97F4A7C15ull;
	s[1]=(s[0]==0&&s[1]==0)?0x1:s[1];
}

static u_long splitmix64_state;

void splitmix64_seed(u_long seed)
{
	splitmix64_state=(u_long)seed;
}

u_long splitmix64(void)
{
	u_long z=(splitmix64_state+=0x9e3779b97f4a7c15ULL);
	z=(z^(z>>30))*0xbf58476d1ce4e5b9ULL;
	z=(z^(z>>27))*0x94d049bb133111ebULL;
	z=z^(z>>31);
	return (u_long)z;
}

static u_long romu_x, romu_y;

void romuduojr_seed(u_long seed)
{
	romu_x=seed^0xA5A5A5A5A5A5A5A5UL;
	romu_y=seed*0x5851F42D4C957F2DUL+1;
}

u_long romuduojr(void)
{
	u_long xp=romu_x;
	romu_x=15241094284759029579u*romu_y;
	romu_y=romu_y-xp;
	romu_y=ROTL64(romu_y,27);
	return xp;
}

u_long random_seed_u64(void)
{
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts)!=0)
		return -1;
	return ((u_long)(ts.tv_sec*1000000000ULL+ts.tv_nsec));
}

void printmethods_random(void)
{
	printf("Random\n");
	printf("\t(%d) %s\n",0,"xoroshiro128plus");
	printf("\t(%d) %s\n",1,"cmwc");
	printf("\t(%d) %s\n",2,"splitmix64");
	printf("\t(%d) %s\n",3,"romuduojr");
}

