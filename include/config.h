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

#ifndef FLOOD_CONFIG_H
#define FLOOD_CONFIG_H

#include "include.h"

#define MAXNAMELEN	1000
#define MAXERRLEN	70000
#define MAXVALLEN	2048

#define VALTYPE_NUM	0
#define VALTYPE_KEYWORD 2
	#define KEYWORD_NULL		"NULL"
	#define KEYWORD_RANDOM		"RAND"
		#define KEYWORD_RANDOM_IPV4	"IPV4"
		#define KEYWORD_RANDOM_DNS	"DNS"
	#define KEYWORD_RANDOM_ALWAYS	"ARAND"
#define VALTYPE_STR	1

#define ASCII_VALID_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZ"	\
	"abcdefghijklmnopqrstuvwxyz0123456789!\"#$%&'"	\
	"()*+,-./:;<=>?@[\\]^_`{|}~ "

#define ARANDVAL SIZE_MAX
#define ARANDVAL_STR "\a"

#define ISVALIDCH(ch)					\
	(strchr(ASCII_VALID_CHARS, (ch))!=NULL)

typedef struct __node_cfg {
	char	name[MAXNAMELEN];
	char	val[MAXVALLEN];
	int	valtype;
} cfgn_t;

u_long libcrand(u_long min, u_long max);

typedef struct __opts {
#define	OPT_PPS				__pps
#define OPT_PPS_NULL			1000
#define	OPT_PPS_NAME			"pps"
#define	OPT_PPS_ARAND_SUPPORT		0
	size_t OPT_PPS;
#define	OPT_TIME			__time
#define	OPT_TIME_NULL			10
#define	OPT_TIME_NAME			"time"
#define	OPT_TIME_ARAND_SUPPORT		0
	size_t OPT_TIME;
#define	OPT_DSTPORT			__dstport
#define	OPT_DSTPORT_NULL		80
#define	OPT_DSTPORT_NAME		"dstport"
#define	OPT_DSTPORT_ARAND_SUPPORT	1
	size_t OPT_DSTPORT;
#define	OPT_TARGET			__target
#define	OPT_TARGET_NULL			"127.0.0.1"
#define	OPT_TARGET_NAME			"target"
#define	OPT_TARGET_ARAND_SUPPORT	0
	char OPT_TARGET[65535];
#define	OPT_SOURCE			__source
#define	OPT_SOURCE_NULL			"NULL"
#define	OPT_SOURCE_NAME			"source"
#define	OPT_SOURCE_ARAND_SUPPORT	1
	char OPT_SOURCE[65535];
#define	OPT_METHOD			__method
#define	OPT_METHOD_NULL			0
#define	OPT_METHOD_NAME			"method"
#define	OPT_METHOD_ARAND_SUPPORT	0
	size_t OPT_METHOD;
#define	OPT_SRCPORT			__srcport
#define	OPT_SRCPORT_NULL		ARANDVAL
#define	OPT_SRCPORT_NAME		"srcport"
#define	OPT_SRCPORT_ARAND_SUPPORT	1
	size_t OPT_SRCPORT;
#define	OPT_INTERFACE			__interface
#define	OPT_INTERFACE_NULL		"NULL"
#define	OPT_INTERFACE_NAME		"interface"
#define	OPT_INTERFACE_ARAND_SUPPORT	0
	char OPT_INTERFACE[65535];
#define	OPT_UNLIMIT			__unlimit
#define	OPT_UNLIMIT_NULL		0
#define	OPT_UNLIMIT_NAME		"unlimit"
#define	OPT_UNLIMIT_ARAND_SUPPORT	0
	size_t OPT_UNLIMIT;
#define	OPT_RANDOM			__random
#define	OPT_RANDOM_NULL			3
#define	OPT_RANDOM_NAME			"random"
#define	OPT_RANDOM_ARAND_SUPPORT	0
	size_t OPT_RANDOM;
#define	OPT_BADSUM			__badsum
#define	OPT_BADSUM_NULL			0
#define	OPT_BADSUM_NAME			"badsum"
#define	OPT_BADSUM_ARAND_SUPPORT	1
	size_t OPT_BADSUM;
} opts_t;

int config_cfgncreate(cfgn_t *c, char *err,
	const char *name, const char *val);
int config_commentsdel(int fd, char *err, char *template);	/* return <newfd> tmpfile, close <fd> */
int config_cfgnparse(char *err, int fd, opts_t *o);
int config_apply(cfgn_t *c, opts_t *o, char *err);
int config(const char *filename, opts_t *o, char *err);

#endif
