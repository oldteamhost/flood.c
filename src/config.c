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

static inline void unescape(char *str)
{
	char *write=str;
	char *read=str;

	while (*read) {
		if (*read=='\\') {
			read++;
			switch (*read) {
			case '\\': *write++='\\'; break;
			case '\'': *write++='\''; break;
			case '\"': *write++='\"'; break;
			case '=':  *write++='=';  break;
			case ';':  *write++=';';  break;
			case '\0':
				*write++='\\';
				read--;
				break;
			default:
				*write++='\\';
				*write++=*read;
				break;
			}
			if (*read)
				read++;
		}
		else
			*write++=*read++;
	}
	*write='\0';
}

static inline void removequotes(char *str)
{
	char first,last;
	size_t len;

	len=strlen(str);
	if (len<2)
		 return;

	first=str[0];
	last=str[len-1];

	if ((first=='"'||first=='\'')&&(last==first)) {
		memmove(str,str+1,len-1);
		str[len-2]='\0';
	}
}

int config_cfgncreate(cfgn_t *c, char *err, const char *name, const char *val)
{
	size_t n,n1;

	if (!name||!val||!c) {
		snprintf(err,MAXERRLEN,
			"[%s]\terr: not init vars %s",
			__FILE__,__func__);
		return -1;
	}

	n=strlen(name);
	if (n>=MAXNAMELEN) {
		snprintf(err,MAXERRLEN,
			"[%s]\terr: name \"%s\" is long, %ld; max=%d",
			__FILE__,name,n,MAXNAMELEN-1);
		return -1;
	}
	for (n1=0;n1<n;++n1) {
		if (!ISVALIDCH(name[n1])) {
			snprintf(err,MAXERRLEN,
				"[%s]\terr: is not valid char in name \"%s\" (%c)",
				__FILE__,name,name[n1]);
			return -1;
		}
	}
	n=strlen(val);
	if (n>=MAXVALLEN) {
		snprintf(err,MAXERRLEN,
			"[%s]\terr: value \"%s\" is long, %ld; max=%d",
			__FILE__,val,n,MAXVALLEN-1);
		return -1;
	}
	for (n1=0;n1<n;++n1) {
		if (!ISVALIDCH(val[n1])) {
			snprintf(err,MAXERRLEN,
				"[%s]\terr: is not valid char in value \"%s\"(%c)",
				__FILE__,val,name[n1]);
			return -1;
		}
	}
	
	strncpy(c->name,name,MAXNAMELEN);
	c->name[MAXNAMELEN-1]='\0';

	strncpy(c->val,val,MAXVALLEN);
	c->val[MAXVALLEN-1]='\0';
	
	n=strlen(val);

	/* is string */
	if (((c->val[n-1]=='"'&&c->val[n-2]!='\\'&&c->val[0]=='"')||
      		(c->val[n-1]=='\''&&c->val[n-2]!='\\'&& c->val[0]=='\''))) {

		c->valtype=VALTYPE_STR;
		if (n==2) /* only "" '' */
			c->val[0]='\0';
		else
			removequotes(c->val);
		unescape(c->val);
		return 0;
	}

	/* is num */
	c->valtype=VALTYPE_NUM;

	/* is keyword */
	for (n=0;n<strlen(c->val);n++)
		if (isalpha(c->val[n]))
			goto L0;
	return 0;
L0:
	c->valtype=VALTYPE_KEYWORD;
	return 0;
}

static inline char *remove_comments(const char* buf, size_t len, int *inlevel)
{
	size_t		i=0,j=0;
	char		*result;
	int		level;
	int		in_string=0;
	int		escape=0;

	result=malloc(len+1);
	if (!result)
		return NULL;

	level=*inlevel;
	while (i<len) {
		char c=buf[i];
		char next=(i+1<len)?buf[i+1]:0;

		if (escape) {
			escape=0;
			if (level==0)
				result[j++]=buf[i];
			i++;
			continue;
		}

		if (in_string) {
			if (c=='\\')
				escape=1;
			else if ((in_string==1&&c=='"')||(in_string==2&&c=='\''))
				in_string=0;
			if (level==0)
				result[j++]=buf[i++];
			else
				i++;
			continue;
		}

		if (c=='"') {
			in_string=1;
			if (level==0)
				result[j++]=buf[i++];
			else
				i++;
			continue;
		}

		if (c=='\'') {
			in_string=2;
			if (level==0)
				result[j++]=buf[i++];
			else
				i++;
			continue;
		}

		if (i+1<len&&c=='/'&&next=='*') {
			level++;
			i+=2;
		}
		else if (level>0&&i+1<len&&c=='*'&&next=='/') {
			level--;
			i+=2;
		}
		else {
			if (level==0)
				result[j++]=buf[i++];
			else
				i++;
		}
	}

	result[j]='\0';
	*inlevel=level;
	return result;
}


int config_commentsdel(int fd, char *err, char *template)
{
	char		buffer[65535], *new=NULL;
	int		newfd, level=0;
	ssize_t		n;

	if ((newfd=mkstemp(template))==-1) {
		snprintf(err,MAXERRLEN,
			"[%s]\terr: failed create tmpfile %s",
			__FILE__,__func__);
		return -1;
	}
	lseek(newfd,0,SEEK_SET);
	lseek(fd,0,SEEK_SET);

	while ((n=read(fd,buffer,sizeof(buffer)))>0) {
		if (!(new=remove_comments(buffer,n,&level))) {	/* save level comment for next read() */
			snprintf(err,MAXERRLEN,
				"[%s]\terr: failed remove comments %s",
				__FILE__,__func__);
			close(newfd);
			return -1;
		}
		if (write(newfd,new,strlen(new))!=(ssize_t)strlen(new)) {
			snprintf(err,MAXERRLEN,
				"[%s]\terr: failed write in tmpfile %s",
				__FILE__,__func__);
			free(new);
			close(newfd);
			return -1;
		}
		free(new);
	}
	lseek(newfd,0,SEEK_SET);

	return newfd;
}

static inline void remwhite(char *str)
{
	char *src=str;
	char *dst=str;
	bool a=0,b=0;

	while (*src) {
		if (*src=='\''&&!b) {
			a=!a;
			*dst++=*src++;
		}
		else if (*src=='"'&&!a) {
			b=!b;
			*dst++=*src++;
		}
		else if (!a&&!b&&
			       (*src==' '||*src=='\t'||*src=='\n')) {
			src++;
		}
		else
			*dst++=*src++;
	}

	*dst='\0';
}

static inline int parse(cfgn_t *c, char *input, char *err)
{
	char left[MAXNAMELEN];
	char right[MAXVALLEN];
	int i=0,j=0;

	while (input[i]!='\0') {
		if (input[i]=='='&&(i==0||input[i-1]!='\\'))
			break;
		left[i]=input[i];
		i++;
	}
	left[i]='\0';
	if (input[i]!='=') {
		snprintf(err,MAXERRLEN,
			"[%s]\terr: not found \"=\" %s",
				__FILE__,input);
		return -1;
	}
	i++;	/* skip = */
	while (input[i]!='\0') {
		if (input[i]==';'&&(i==0||input[i-1]!='\\'))
			break;
		right[j++]=input[i++];
	}
	right[j]='\0';
	if (strlen(right)==0) {
		snprintf(err,MAXERRLEN,
			"[%s]\terr: value not found %s",
				__FILE__,input);
		return -1;
	}

	return config_cfgncreate(c,err,left,right);
}

u_long libcrand(u_long min, u_long max)
{
	u_long range,limit,rand_val;

	if (min>max) {
		u_long tmp;
		tmp=min;
		min=max;
		max=tmp;
	}

	range=max-min+1;
	limit=ULONG_MAX-(ULONG_MAX%range);

	do {
		rand_val=((u_long)rand()<<32)|rand();
	} while (rand_val>=limit);

	return min+(rand_val%range);
}

static inline u_int random_u32(void)
{
	return ((u_int)rand()<<16)|(rand()&0xFFFF);
}

static inline void random_str(char *buf, size_t len, const char *dictionary)
{
	size_t dict_len=strlen(dictionary);
	size_t i;
	for (i=0;i<len;i++)
		buf[i]=dictionary[random_u32()%dict_len];
	buf[len]='\0';
}

static inline void random_ipv4(char *buf)
{
	snprintf(buf, 16, "%u.%u.%u.%u",
		random_u32() % 256,
		random_u32() % 256,
		random_u32() % 256,
		random_u32() % 256);
}

static inline void random_dns(char *buf, size_t len)
{
	const char *dictionary="abcdefghijklmnopqrstuvwxyz0123456789";
	size_t label_len=(len>64?10+random_u32()%10:len-5);
	if (label_len+5>=len)
		label_len=len-6;
	random_str(buf, label_len, dictionary);
	memcpy(buf+label_len, ".com", 5);
}

static inline int str_size_t(const char *str, size_t *out, size_t max)
{
	const char *p=str;
	size_t result,nxt;
	int digit;

	result=0;
	while (isspace(*p))
		 p++;
	if (*p=='\0')
		return 0;
	while (*p) {
		if (!isdigit(*p))
			return 0;
		digit=*p-'0';
		if (result>(SIZE_MAX-digit)/10)
			 return 0;
		nxt=result*10+digit;
		if (nxt>max)
			return 0;
		result=nxt;
		p++;
	}
	*out=result;
	return 1;
}

static inline int is_word_boundary(char c)
{
	return c=='\0'||!isalnum((u_char)c);
}

static inline int contains_word(const char *str, const char *word)
{
	const char *pos=str;

	while ((pos=strstr(pos, word))) {
		if (pos==str||is_word_boundary(*(pos-1)))
			return 1;
		pos++;
	}

	return 0;
}

int config_apply(cfgn_t *c, opts_t *o, char *err)
{

#define CMP(x,y) (!(strcmp((x),(y))))

#define NUMOPT(name, _opt, null, _limit, arands) \
	switch(c->valtype) { \
		case VALTYPE_NUM: \
			if (!(str_size_t(c->val,&o->_opt,_limit))) { \
				snprintf(err,MAXERRLEN, \
					"[%s]\terr: failed convert %s (limit=%ld)", \
					__FILE__,(name),(size_t)_limit); \
				return -1; \
			} \
			return 0; \
		case VALTYPE_STR: \
			snprintf(err,MAXERRLEN, \
				"[%s]\terr: %s is only number", \
				__FILE__,(name)); \
			return -1; \
		case VALTYPE_KEYWORD: \
			if (CMP(c->val,KEYWORD_NULL)) { \
				o->_opt=(null); \
				return 0; \
			} \
			else if (CMP(c->val,KEYWORD_RANDOM_ALWAYS)) { \
				if (!arands) { \
					snprintf(err,MAXERRLEN, \
						"[%s]\terr: ARAND not supported on \"%s\"", \
						__FILE__,(name)); \
					return -1; \
				} \
				o->_opt=ARANDVAL; \
				return 0; \
			} \
			else if (contains_word(c->val,KEYWORD_RANDOM)) { \
				size_t a,b; \
				if (sscanf(c->val, KEYWORD_RANDOM"/%zu,%zu",&a,&b)!=2) { \
					snprintf(err,MAXERRLEN, \
						"[%s]\terr: %s invalid syntax random value %s", \
						__FILE__,(name),c->val); \
					return -1; \
				} \
				o->_opt=libcrand(a,b); \
				return 0; \
			} \
			snprintf(err,MAXERRLEN, \
				"[%s]\terr: not found keyword (%s) %s", \
				__FILE__,c->val, (name)); \
			return -1; \
		default: \
			snprintf(err,MAXERRLEN, \
				"[%s]\terr: unknown type ??? (%s) %s", \
				__FILE__,c->val, (name)); \
			return -1; \
	}

#define STROPT(name, _opt, null, arands) \
	switch(c->valtype) { \
		case VALTYPE_NUM: \
			snprintf(err,MAXERRLEN, \
				"[%s]\terr: %s is only string", \
				__FILE__,(name)); \
			return -1; \
		case VALTYPE_STR: \
			snprintf(o->_opt,sizeof(o->_opt),"%s",c->val);\
			return 0; \
		case VALTYPE_KEYWORD: \
			if (CMP(c->val,KEYWORD_NULL)) { \
				snprintf(o->_opt,sizeof(o->_opt),"%s",(null));\
				return 0; \
			} \
			else if (CMP(c->val,KEYWORD_RANDOM_ALWAYS)) { \
				if (!arands) { \
					snprintf(err,MAXERRLEN, \
						"[%s]\terr: ARAND not supported on \"%s\"", \
						__FILE__,(name)); \
					return -1; \
				} \
				snprintf(o->_opt,sizeof(o->_opt),"%s",ARANDVAL_STR);\
				return 0; \
			} \
			else if (contains_word(c->val,KEYWORD_RANDOM)) { \
				char b[65535]; \
				size_t a=0; \
				u_char f=1;\
				if (sscanf(c->val, KEYWORD_RANDOM"/%zu,%s",&a,b)!=2) { \
					if (sscanf(c->val, KEYWORD_RANDOM"/%s",b)!=1) { \
						snprintf(err,MAXERRLEN, \
							"[%s]\terr: %s invalid syntax random value %s", \
							__FILE__,(name),c->val); \
						return -1; \
					} \
					f=0; \
				} \
				if (f) { \
					a--; \
					if (a>sizeof(o->_opt)-1) { \
						snprintf(err,MAXERRLEN, \
							"[%s]\terr: %s limits!! %s", \
							__FILE__,(name),c->val); \
						return -1; \
					} \
					random_str(o->_opt,a,b); \
				} \
				else { \
					if (CMP(b,KEYWORD_RANDOM_IPV4)) {  \
						random_ipv4(o->_opt); \
					} \
					else if (CMP(b,KEYWORD_RANDOM_DNS)) { \
						random_dns(o->_opt,15); \
					} \
					else { \
						snprintf(err,MAXERRLEN, \
							"[%s]\terr: not found keyword RAND %s", \
							__FILE__,(name)); \
						return -1; \
					} \
				} \
				return 0; \
			} \
			snprintf(err,MAXERRLEN, \
				"[%s]\terr: not found keyword (%s) %s", \
				__FILE__,c->val, (name)); \
			return -1; \
		default: \
			snprintf(err,MAXERRLEN, \
				"[%s]\terr: unknown type ??? (%s) %s", \
				__FILE__,c->val,(name)); \
			return -1; \
	}

	if (CMP(c->name,OPT_PPS_NAME)) {
		NUMOPT(OPT_PPS_NAME,OPT_PPS,OPT_PPS_NULL,
			SIZE_MAX,OPT_PPS_ARAND_SUPPORT);
	}
	else if (CMP(c->name,OPT_TIME_NAME)) {
		NUMOPT(OPT_TIME_NAME,OPT_TIME,OPT_TIME_NULL,
			SIZE_MAX,OPT_TIME_ARAND_SUPPORT);
	}
	else if (CMP(c->name,OPT_DSTPORT_NAME)) {
		NUMOPT(OPT_DSTPORT_NAME,OPT_DSTPORT,OPT_DSTPORT_NULL,
			USHRT_MAX,OPT_DSTPORT_ARAND_SUPPORT);
	}
	else if (CMP(c->name,OPT_TARGET_NAME)) {
		STROPT(OPT_TARGET_NAME,OPT_TARGET,OPT_TARGET_NULL,
			OPT_TARGET_ARAND_SUPPORT);
	}
	else if (CMP(c->name,OPT_SOURCE_NAME)) {
		STROPT(OPT_SOURCE_NAME,OPT_SOURCE,OPT_SOURCE_NULL,
			OPT_SOURCE_ARAND_SUPPORT);
	}
	else if (CMP(c->name,OPT_METHOD_NAME)) {
		NUMOPT(OPT_METHOD_NAME,OPT_METHOD,OPT_METHOD_NULL,
			USHRT_MAX,OPT_METHOD_ARAND_SUPPORT);
	}
	else if (CMP(c->name,OPT_SRCPORT_NAME)) {
		NUMOPT(OPT_SRCPORT_NAME,OPT_SRCPORT,OPT_SRCPORT_NULL,
			USHRT_MAX,OPT_SRCPORT_ARAND_SUPPORT);
	}
	else if (CMP(c->name,OPT_INTERFACE_NAME)) {
		STROPT(OPT_INTERFACE_NAME,OPT_INTERFACE,OPT_INTERFACE_NULL,
			OPT_INTERFACE_ARAND_SUPPORT);
	}
	else if (CMP(c->name,OPT_UNLIMIT_NAME)) {
		NUMOPT(OPT_UNLIMIT_NAME,OPT_UNLIMIT,OPT_UNLIMIT_NULL,
			1,OPT_UNLIMIT_ARAND_SUPPORT);
	}
	else if (CMP(c->name,OPT_RANDOM_NAME)) {
		NUMOPT(OPT_RANDOM_NAME,OPT_RANDOM,OPT_RANDOM_NULL,
			USHRT_MAX,OPT_RANDOM_ARAND_SUPPORT);
	}
	else if (CMP(c->name,OPT_BADSUM_NAME)) {
		NUMOPT(OPT_BADSUM_NAME,OPT_BADSUM,OPT_BADSUM_NULL,
			1,OPT_BADSUM_ARAND_SUPPORT);
	}
	else {
		snprintf(err,MAXERRLEN,
			"[%s]\terr: unknown name \"%s\"",
			__FILE__,(c->name));
		return -1;
	}

	return 0;

#undef CMP
#undef NUMOPT 
}

int config_cfgnparse(char *err, int fd, opts_t *o)
{

	char		buffer[65535];
	char		command[65535];
	ssize_t		n,i,cmdlen;

	cmdlen=0;
	while ((n=read(fd,buffer,sizeof(buffer)))>0) {
		for (i=0;i<n;i++) {
			if (buffer[i]==';'&&(i==0||buffer[i-1]!='\\')) {
				cfgn_t		node={0};

				command[cmdlen++]=';';
				command[cmdlen]='\0';
				remwhite(command);
				if ((parse(&node,command,err))==-1)
					return -1;
				if ((config_apply(&node,o,err))==-1)
					return -1;
				printf("[%s]\tsuccess apply \"%s\" option val=%s type=%d\n",
					__FILE__,node.name,node.val,node.valtype);
				cmdlen=0;
			}
			else {
				command[cmdlen++]=buffer[i];
				if (cmdlen>=sizeof(command)-1)
					return -1;
			}
		}
	}
	return 0;
}

int config(const char *filename, opts_t *o, char *err)
{
	char		template[]="/tmp/configtmpXXXXXX";
	int		fd, newfd;

	printf("[%s]\tconfig \"%s\" processing\n",
		__FILE__,filename);
	srand(time(NULL));
	if (err)
		*err='\0';
	else
		return -1;
	if ((fd=open(filename,O_APPEND))==-1) {
		snprintf(err,MAXERRLEN,
			"[%s]\terr: failed open fd %s",
			__FILE__,__func__);
		return -1;
	}
	if (((newfd=config_commentsdel(fd,err,
			template)))==-1) {
		close(fd);
		return -1;
	}
	close(fd);
	if (config_cfgnparse(err,newfd,o)==-1) {
		unlink(template);
		close(newfd);
		return -1;
	}
	
	unlink(template);
	close(newfd);
	return 0;
}
