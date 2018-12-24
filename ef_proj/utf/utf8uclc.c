/*-
 * Copyright (c) 2008 CAS Dev Team
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the CAS Dev. Team nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *      utf8uclc.c
 *
 * $CAS$
 */

#include <stdio.h>
#include <string.h>
#include <chacu.h>

typedef struct TestMap
{
	const char  * src;
	const int     src_len;
} TestMap;

static TestMap szTestMap[] =
{

	{ "1234567890abcdefghijklmnopqrstuvwxyz", sizeof("1234567890abcdefghijklmnopqrstuvwxyz") },
	{ "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ", sizeof("1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ") },
	{ "1234567890АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ", sizeof("1234567890АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ") },
	{ "1234567890абвгдеёжзийклмнопрстуфхцчшщъыьэюя", sizeof("1234567890абвгдеёжзийклмнопрстуфхцчшщъыьэюя") },
	{ "العربية",          sizeof("العربية")          },
	{ "Žemaitėška",       sizeof("Žemaitėška")       },
	{ "Català",           sizeof("Català")           },
	{ "Español",          sizeof("Español")          },
	{ "Français",         sizeof("Français")         },
	{ "日本語",              sizeof("日本語")              },
	{ "Lietuvių",         sizeof("Lietuvių")         },
	{ "Norsk (bokmål)",   sizeof("Norsk (bokmål)")   },
	{ "Português",        sizeof("Português")        },
	{ "Slovenčina",       sizeof("Slovenčina")       },
	{ "Slovenščina",      sizeof("Slovenščina")      },
	{ "Тоҷикӣ",           sizeof("Тоҷикӣ")           },
	{ "Türkçe",           sizeof("Türkçe")           },
	{ "Українська",       sizeof("Українська")       },
	{ "Vèneto",           sizeof("Vèneto")           },
	{ "Volapük",          sizeof("Volapük")          },
	{ "Herzliche Grüße",  sizeof("Herzliche Grüße")  },
	{ "Сердечный привет", sizeof("Сердечный привет") },
	{ "F\300\200oo",      sizeof("F\300\200oo")      },
	{ "F\300\200",        sizeof("F\300\200")        },
//	{ ""
	{ NULL, 0 }
};

int main(void)
{
	int    iPos = 0;
	char * szDst = NULL;
	int    iDstLen = 0;

	while(szTestMap[iPos].src!= NULL)
	{
		if (utf8lcstr(szTestMap[iPos].src, szTestMap[iPos].src_len, &szDst, &iDstLen) != 0) { fprintf(stderr, "ERROR!\n"); return -1; }
		fprintf(stdout, "src: `%s` real %d chars %d\n", szTestMap[iPos].src, strlen(szTestMap[iPos].src), utf8strnlen(szTestMap[iPos].src, szTestMap[iPos].src_len));
		fprintf(stdout, "dst: `%s` chars %d %d\n", szDst, iDstLen, utf8strlen(szDst));
		free(szDst);

		if (utf8ucstr(szTestMap[iPos].src, szTestMap[iPos].src_len, &szDst, &iDstLen) != 0) { fprintf(stderr, "ERROR!\n"); return -1; }
		fprintf(stdout, "dst: `%s` chars %d %d\n\n", szDst, iDstLen, utf8strlen(szDst));
		free(szDst);
		++iPos;
	}

	char  * szSrc1   = "Сердечный привет";
	int     iSrc1Len = sizeof("Сердечный привет");

	char  * szSrc2   = "СеРдЕчНыЙ пРиВеТ";
	int     iSrc2Len = sizeof("СеРдЕчНыЙ пРиВеТ");

	char  * szSrc3   = "F";
	int     iSrc3Len = sizeof("F");

	fprintf(stdout, "String: `%s`; length: %d %d\n", szSrc1, utf8strnlen(szSrc1, iSrc1Len), utf8strlen(szSrc3));
	fprintf(stdout, "String: `%s`; length: %d %d\n", szSrc2, utf8strnlen(szSrc2, iSrc2Len), utf8strlen(szSrc3));
	fprintf(stdout, "String: `%s`; %d length: %d %d\n", szSrc3, iSrc3Len, utf8strnlen(szSrc3, iSrc3Len), utf8strlen(szSrc3));

	fprintf(stdout, "Compare strings: `%s` <> `%s`: %d\n", szSrc1, szSrc2, utf8strncmp(szSrc1, iSrc1Len, szSrc2, iSrc2Len));
	fprintf(stdout, "Compare strings: `%s` <> `%s`: %s\n\n", szSrc1, szSrc2, utf8strncasecmp(szSrc1, iSrc1Len, szSrc2, iSrc2Len) == 0 ? "equal" : "not equal");

	fprintf(stdout, "Compare strings: `%s` <> `%s`: %d\n", szSrc2, szSrc1, utf8strncmp(szSrc2, iSrc2Len, szSrc1, iSrc1Len));
	fprintf(stdout, "Compare strings: `%s` <> `%s`: %s\n\n", szSrc2, szSrc1, utf8strncasecmp(szSrc2, iSrc2Len, szSrc1, iSrc1Len) == 0 ? "equal" : "not equal");

	fprintf(stdout, "Compare strings: `%s` <> `%s`: %d\n", szSrc1, szSrc3, utf8strncmp(szSrc1, iSrc1Len, szSrc3, iSrc3Len));
	fprintf(stdout, "Compare strings: `%s` <> `%s`: %s\n", szSrc1, szSrc3, utf8strncasecmp(szSrc1, iSrc1Len, szSrc3, iSrc3Len) == 0 ? "equal" : "not equal");

	unsigned int iUCSResult;
	const char sU[] = {0xC4, 0xB0, 0x00 };
	int iUTF8Length = 2;
	utf8tow(sU, iUTF8Length, &iUCSResult);
	fprintf(stdout, "WIDEL 0x%04X\n", iUCSResult);
return 0;
}
/* End. */


