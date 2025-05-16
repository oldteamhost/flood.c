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

#include "../include/interface.h"

eth_t *ethopen_index(int ifindex)
{
	struct sockaddr_ll sll;
	struct tpacket_req3 tp;
	int ver=TPACKET_V3;	/* aee */
	eth_t *eth=NULL;

	if (!(eth=calloc(1,sizeof(eth_t))))
		goto fail;
	if ((eth->fd=socket(AF_PACKET,SOCK_RAW,0))==-1)
		goto fail;
	if (setsockopt(eth->fd,SOL_PACKET,PACKET_VERSION,
		&ver,sizeof(ver))<0)
	    goto fail;

	memset(&sll,0,sizeof(struct sockaddr_ll));
	sll.sll_family=AF_PACKET;
	sll.sll_protocol=ETH_P_ALL;
	sll.sll_ifindex=ifindex;
	if ((bind(eth->fd,(struct sockaddr*)&sll,
			sizeof(sll)))==-1)
		goto fail;

	/* PACKET_TX_RING (since Linux 2.6.31) */
	memset(&tp,0,sizeof(tp));
	tp.tp_block_size=1<<20;
	tp.tp_block_nr=64;
	tp.tp_frame_size=2048;
	tp.tp_frame_nr=(tp.tp_block_size/
		tp.tp_frame_size)*tp.tp_block_nr;
	if (setsockopt(eth->fd,SOL_PACKET,PACKET_TX_RING,
			&tp,sizeof(tp))==-1)
		goto fail;

	eth->frame_size=tp.tp_frame_size;
	eth->frame_nr=tp.tp_frame_nr;

	eth->ring=mmap(NULL,(tp.tp_block_size*tp.tp_block_nr),
		PROT_READ|PROT_WRITE,MAP_SHARED|MAP_LOCKED|
		MAP_POPULATE,eth->fd,0);
	if (eth->ring==MAP_FAILED)
		goto fail;

	eth->current=0;
	return eth;
fail:
	if (eth) {
		if (eth->fd>=0)
			close(eth->fd);
		free(eth);
	}

	return NULL;
}
eth_t *ethopen(const char *dev)
{
	int ifindex;
	ifindex=if_nametoindex(dev);
	return ethopen_index(ifindex);
}

/* https://github.com/torvalds/linux/blob/master/tools/testing/selftests/net/psock_tpacket.c */
static inline int __v3_tx_kernel_ready(struct tpacket3_hdr *hdr)
{
	return !(hdr->tp_status&(TP_STATUS_SEND_REQUEST|TP_STATUS_SENDING));
}

#define BATCH_SIZE 128

ssize_t	ethsend(eth_t *e, u_char *frame, size_t frmlen)
{
	struct tpacket3_hdr *hdr=NULL;
	size_t tried=0,start=e->current;
	static size_t bc=0;

	if (!e||!frame||frmlen>e->frame_size)
		return -1;

	while (tried<e->frame_nr) {
		hdr=(struct tpacket3_hdr*)(e->ring+((start+
			tried)%e->frame_nr)*e->frame_size);
		if (__v3_tx_kernel_ready(hdr)) {
			memcpy((char*)hdr+TPACKET_ALIGN(sizeof(*hdr)),
				frame,frmlen);
			hdr->tp_len=frmlen;
			hdr->tp_snaplen=frmlen;
			hdr->tp_next_offset=0;
			hdr->tp_status=TP_STATUS_SEND_REQUEST;

			e->current=((((start+tried)%e->frame_nr))+1)
				%e->frame_nr;

			if (++bc>=BATCH_SIZE) {

				/* Come on, we've done this before. */
				if (sendto(e->fd,NULL,0,MSG_DONTWAIT,NULL,0)<0)
					return -1;
				bc=0;
			}
			return 0;
		}
		tried++;
	}
	return 0;
}

void ethclose(eth_t *e)
{
	if (e) {
		if (e->ring)
			munmap(e->ring,e->frame_size*e->frame_nr);
		if (e->fd>=0)
			close(e->fd);
		free(e);
	}
}

void getupif(int *index, struct ether_addr *src,
	struct in_addr *srcip4, struct in_addr *gateway,
	struct ether_addr *dst, const char *your)
{
	struct if_nameindex	*ifni, *start;
	struct ifreq		ifr={0};
	const char		*device;
	int			fd;
	char			iface_buffer[IFNAMSIZ];
	unsigned long		dest,gate;
	char			line[1024];
	FILE			*f;
	char			ip[32],hw_type[32];
	char			flags[32],mac[32];
	char			mask[32],dev[32];
	struct ether_addr	*_tmp;

	/*
	 * get source ipv4, source mac, ifindex
	 */

	*index=-1;
	if ((fd=socket(AF_INET,SOCK_DGRAM,0))<0)
		return;

	ifni=if_nameindex();
	start=ifni;

	for (;ifni->if_name;ifni++) {
		if (your&&strcmp(your,ifni->if_name)!=0)
			continue;

		snprintf(ifr.ifr_name, IFNAMSIZ, "%s", ifni->if_name);
		device=ifr.ifr_name;
		if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0)
			break;

		if ((ifr.ifr_flags&IFF_UP)&&
				!(ifr.ifr_flags&IFF_LOOPBACK)&&
				!(ifr.ifr_flags&IFF_POINTOPOINT)) {
			if (ioctl(fd, SIOCGIFHWADDR,&ifr)<0)
				break;
			memcpy(src->ether_addr_octet,
			ifr.ifr_ifru.ifru_hwaddr.sa_data,6);
			if (ioctl(fd,SIOCGIFADDR,&ifr)<0)
				break;
			memcpy(&srcip4->s_addr,(ifr.ifr_addr.sa_data+2),4);
			*index=ifni->if_index;
			break;
		}
		if (your)
			break;
	}

	if_freenameindex(start);
	close(fd);

	/*
	 * get gateway ipv4 in internet, dest mac
	 */

	if (!(f=fopen("/proc/net/route", "r")))
		return;
	if (!(fgets(line, sizeof(line), f)))	/* skip hdr */
		return;
	while (fgets(line, sizeof(line), f)) {
		if (sscanf(line,"%15s %lx %lx",iface_buffer,&dest,&gate)!=3)
			continue;
		if (dest==0) {	/* is way in internet */
			if (strcmp(device, iface_buffer)==0) {
				gateway->s_addr=((u_int)gate);
				break;
			}
		}
	}
	fclose(f);
	if (!(f=fopen("/proc/net/arp", "r")))
		return;
	if (!(fgets(line, sizeof(line), f)))	/* skip hdr */
		return;
	while (fgets(line,sizeof(line),f)) {
		sscanf(line, "%31s %31s %31s %31s %31s %31s",
			 ip, hw_type, flags, mac, mask, dev);
		if (strcmp(dev, device))
			continue;
		if (strcmp(ip, inet_ntoa(*gateway)))	/* for internet */
			continue;
		_tmp=ether_aton(mac);
		*dst=*_tmp;
		break;
	}

	fclose(f);
}
