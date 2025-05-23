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

#define TCP_PORT 7779
#define KGRN "\033[0;32;32m"
#define KCYN "\033[0;36m"
#define KRED "\033[0;32;31m"
#define KYEL "\033[1;33m"
#define KMAG "\033[0;35m"
#define KBLU "\033[0;32;34m"
#define KCYN_L "\033[1;36m"
#define RESET "\033[0m"

static int tcp_sock;
static int callback_http(struct lws *wsi, enum lws_callback_reasons reason,
	void *user, void *in, size_t len)
{
	if (reason==LWS_CALLBACK_RECEIVE) {
		send(tcp_sock, (char*)in, (int)len, 0);
		printf(KCYN_L"[resend.c]\t%.*s\n"RESET, (int)len, (char*)in);
	}
	return 0;
}

int main(void)
{
	struct lws_context_creation_info info;
	struct lws_protocols protocols[]={
		{"http",callback_http,1},
		{NULL,NULL,0}
	};
	struct sockaddr_in server_addr;
	struct lws_context *context;

	assert((tcp_sock = socket(AF_INET, SOCK_STREAM, 0))!=-1);
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family=AF_INET;
	server_addr.sin_port=htons(TCP_PORT);
	server_addr.sin_addr.s_addr=inet_addr("127.0.0.1");;

	assert((connect(tcp_sock, (struct sockaddr *)&server_addr,
		sizeof(server_addr)))!=-1);

	memset(&info, 0, sizeof(info));
	info.port=8080;
	info.protocols=protocols;
	context=lws_create_context(&info);
	assert(context);

	printf(KMAG"[resend.c]\tWebSocket running on 8080\n"RESET);

	printf(KGRN"\n[resend.c]\tPlease open panel.html\n"RESET);
	printf(KGRN"[resend.c]\tPlease open panel.html\n"RESET);
	printf(KGRN"[resend.c]\tPlease open panel.html\n"RESET);

	for (;;)
		lws_service(context, 100);

	lws_context_destroy(context);
	close(tcp_sock);
	return 0;
}

