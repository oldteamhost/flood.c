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

#include "../include/utils.h"

#define ip_check_carry(x) \
	(x=(x>>16)+(x&0xffff),(~(x+(x>>16))&0xffff))

char *ip4_util_strsrc(void)
{
	struct ifaddrs *ifap, *ifa;
	struct sockaddr_in *sa;
	char *localip = NULL;

	if (getifaddrs(&ifap) == -1)
		return NULL;
	for (ifa = ifap; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET)
			continue;
		sa = (struct sockaddr_in *) ifa->ifa_addr;
		if (!(ifa->ifa_flags & IFF_LOOPBACK)) {
			localip = (char*)malloc(INET_ADDRSTRLEN);
			inet_ntop(AF_INET, &(sa->sin_addr), localip, INET_ADDRSTRLEN);
			break;
		}
	}

	freeifaddrs(ifap);
	return localip;
}

static int ip_check_add(const void *buf, size_t len, int check)
{
	u_short *sp=(u_short*)buf;
	int n, sn;

	sn=len/2;
	n=(sn+15)/16;
	switch (sn%16) {
		case 0: do {
			check+=*sp++;
		case 15:
			check+=*sp++;
		case 14:
			check+=*sp++;
		case 13:
			check+=*sp++;
		case 12:
			check+=*sp++;
		case 11:
			check+=*sp++;
		case 10:
			check+=*sp++;
		case 9:
			check+=*sp++;
		case 8:
			check+=*sp++;
		case 7:
			check+=*sp++;
		case 6:
			check+=*sp++;
		case 5:
			check+=*sp++;
		case 4:
			check+=*sp++;
		case 3:
			check+=*sp++;
		case 2:
			check+=*sp++;
		case 1:
			check+=*sp++;
		} while (--n>0);
	}

	if (len&1)
		check+=htons(*(u_char*)sp<<8);
	return check;
}

u_short in_check(u_short *ptr, int nbytes)
{
	int sum;
	sum=ip_check_add(ptr, nbytes, 0);
	return ip_check_carry(sum);
}

u_short ip4_pseudocheck(const u_int src, const u_int dst,
	u_char proto, u_short len, const void *hstart)
{
	struct pseudo {
		u_int	src;
		u_int	dst;
		u_char	zero;
		u_char	proto;
		u_short	length;
	} hdr;
	int sum;

	hdr.src=src;
	hdr.dst=dst;
	hdr.zero=0;
	hdr.proto=proto;
	hdr.length=htons(len);

	sum=ip_check_add(&hdr, sizeof(hdr), 0);
	sum=ip_check_add(hstart, len, sum);
	sum=ip_check_carry(sum);

	/* RFC 768: "If the computed  checksum  is zero,  it is transmitted  as all
	* ones (the equivalent  in one's complement  arithmetic).   An all zero
	* transmitted checksum  value means that the transmitter  generated  no
	* checksum" */
	if (proto==IPPROTO_UDP&&sum==0)
		sum=0xFFFF;

	return sum;
}

char *resolve_ipv4(const char *hostname)
{
	static char ip[INET_ADDRSTRLEN];
	struct addrinfo hints={0},*res;
	struct sockaddr_in *addr;

	memset(ip,0,sizeof(ip));
	hints.ai_family=AF_INET;

	if (getaddrinfo(hostname, NULL, &hints, &res)==0) {
		addr=(struct sockaddr_in*)res->ai_addr;
		inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
		printf("[%s]\tdest ipv4 (%s)\n", __FILE__,ip);
		freeaddrinfo(res);
		return ip;
	}

	return NULL;
}

const char *resolve_dns(u_int ipv4)
{
	static char res[2048+2];
	struct sockaddr_in sa;
	char dnsbuf[2048];

	memset(dnsbuf, 0, sizeof(dnsbuf));
	memset(&sa, 0, sizeof(sa));

	sa.sin_family=AF_INET;
	sa.sin_addr.s_addr=ipv4;

	if (getnameinfo((struct sockaddr*)&sa,
			 sizeof(sa), dnsbuf, sizeof(dnsbuf),
			NULL, 0, 0)==0) {
		snprintf(res, sizeof(res), "%s", dnsbuf);
		printf("[%s]\tdest dns (%s)\n", __FILE__,res);
		return res;
	}

	return "\?\?\?";
}
