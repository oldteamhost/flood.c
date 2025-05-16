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

#include "../include/methods.h"

static method_t _methods[100];
static size_t _num_method=0;

void importmethod(flood_t run, const char *name)
{
	_methods[_num_method].run=run;
	_methods[_num_method].name=name;
	printf("[%s]\tinit method \"%s\" id=%ld\n",
		__FILE__,name,_num_method);
	++_num_method;
}

method_t *methods(void)
{
	return _methods;
}

void printmethods(void)
{
	size_t n;
	printf("Methods\n\t");
	for (n=0;n<_num_method;n++) {
		printf("(%ld) %s", n, _methods[n].name);
		if ((n+1)!=_num_method)
			putchar(';');
		if ((n%3)==2&&(n+1)!=_num_method) {
			putchar(0x0a);
			putchar('\t');
		}
		else if ((n + 1) != _num_method)
			putchar(' ');

	}
	putchar(0x0a);
}

void printmethod(int id)
{
	printf("[%s]\tyour flood method \"%s\" id=%d\n",
		__FILE__,_methods[id].name,id);
}

u_char validmethod(size_t id)
{
	if (id>_num_method)
		return 0;
	return 1;
}

void execmethod(size_t id, method_args_t *a)
{
	_methods[id].run(a);
}

