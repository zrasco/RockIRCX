/*
** match.cpp
*/

#include "stdafx.h"
#include <stdlib.h>
#include <ctype.h>

int r_match(char *, char *);

/* Compare if a given string (name) matches the given mask (which can contain
 * wild cards: '*' - match any number of chars, '?' - match any single
 * character.  return  0, if match 1, if no match
 */

/* Concept by JoelKatz (David Schwartz) <djls@gate.net>
   Thanks to Barubary
 */

unsigned char touppertab[], tolowertab[];
#define tolowertab2 tolowertab

int match(char *mask, char *string)
{
	int l, p;
	char *rmask = mask, *rstring = string;

	while ((l = *(rmask++))) {
		if ((l == '*') || (l =='?'))
			continue;
		if (l == '\\') {
			if(!(l = *(rmask++)))
				return r_match(mask, string);
		}
		p = *(rstring++);
		while ((p!=0) && (tolowertab2[p] != tolowertab2[l]))
			p = *(rstring++);
		/* If we're out of string, no match */
		if (!p)
			return 1;
	}

	return r_match(mask, string);
}

/* Iterative matching function, rather than recursive.
 * Written by Douglas A Lewis (dalewis@acsu.buffalo.edu)
 */

int r_match(char *mask, char *name)
{
	unsigned char *m, *n;
	char *ma, *na;
	int wild, q, t = 1;

	m = (unsigned char*)mask;
	n = (unsigned char*)name;
	ma = mask;
	na = name;
	wild = 0;
	q = 0;

	while (t) {
		if (*m == '*') {
			while (*m == '*')
				m++;
			wild = 1;
			ma = (char *)m;
			na = (char *)n;
		}

		if (!*m) {
			if (!*n)
				return 0;
			for (m--; (m > (unsigned char*)mask) && (*m == '?'); --m);

			if ((*m == '*') && (m > (unsigned char*)mask)
			&& (m[-1] != '\\'))
				return 0;

			if (!wild)
				return 1;
			m = (unsigned char*)ma;
			n = (unsigned char*)++na;
		} else if (!*n) {
			while(*m == '*')
				m++;
			return (*m != 0);
		}
		if ((*m == '\\') && ((m[1] == '*') || (m[1] == '?'))) {
			m++;
			q = 1;
		} else
			q = 0;

		if ((tolower(*m) != tolower(*n)) && ((*m != '?') || q)) {
			if (!wild)
				return 1;
			m = (unsigned char*)ma;
			n = (unsigned char*)++na;
		} else {
			if (*m)
			m++;
			if (*n)
			n++;
		}
	}
}

/* collapse a pattern string into minimal components.  This particular version
 * is "in place", so that it changes the pattern which is to be reduced to
 * a "minimal" size. */
char * collapse(char *pattern)
{
	char *s, *s1, *t;

	s = pattern;

	if (pattern == NULL)
		return pattern;

	/* Collapse all \** into \*, \*[?]+\** into \*[?]+ */
	for (; *s; s++)
		if (*s == '\\')
			if (!*(s + 1))
				break;
			else
				s++;
		else if (*s == '*') {
			if (*(t = s1 = s + 1) == '*')
				while (*t == '*')
					t++;
			else if (*t == '?')
				for (t++, s1++; *t == '*' || *t == '?'; t++)
					if (*t == '?')
						*s1++ = *t;
			while ((*s1++ = *t++));
		}
	return pattern;
}


/* Case insensitive comparison of two NULL terminated strings.
 * returns  0, if s1 equal to s2
 *         <0, if s1 lexicographically less than s2
 *         >0, if s1 lexicographically greater than s2
 */
int smycmp(char *s1, char *s2)
{
	int res;
	unsigned char *str1, *str2;

	str1 = (unsigned char*)s1;
	str2 = (unsigned char*)s2;

	while (!(res = toupper(*str1) - toupper(*str2))) {
		if (*str1 == '\0')
			return 0;
		str1++;
		str2++;
	}
	return (res);
}


int myncmp(char *str1, char *str2, int n)
{
	int res;
	unsigned char *s1 = (unsigned char*)str1, *s2 = (unsigned char*)str2;

	while (!(res = toupper(*s1) - toupper(*s2))) {
		s1++; s2++; n--;
		if (n == 0 || (*s1 == '\0' && *s2 == '\0'))
			return 0;
	}
	return (res);
}

int isstrdigit(char *str)
{
	char *s = str;
	while (*s) {
		if (!isdigit(*s++))
		return 0;
	}

	return 1;
}


unsigned char tolowertab[] = {
	0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa,
	0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11, 0x12, 0x13, 0x14,
	0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
	0x1e, 0x1f,
	' ', '!', '"', '#', '$', '%', '&', 0x27, '(', ')',
	'*', '+', ',', '-', '.', '/',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	':', ';', '<', '=', '>', '?',
	'@', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
	'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
	't', 'u', 'v', 'w', 'x', 'y', 'z', '[', '\\', ']', '^',
	'_',
	'`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
	'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
	't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~',
	0x7f,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
	0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
	0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
	0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9,
	0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
	0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9,
	0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
	0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9,
	0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
	0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9,
	0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
	0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
	0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
	0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

unsigned char touppertab[] = {
	0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa,
	0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11, 0x12, 0x13, 0x14,
	0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
	0x1e, 0x1f,
	' ', '!', '"', '#', '$', '%', '&', 0x27, '(', ')',
	'*', '+', ',', '-', '.', '/',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
	':', ';', '<', '=', '>', '?',
	'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
	'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S',
	'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^',
	0x5f,
	'`', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
	'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S',
	'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '{', '|', '}', '~',
	0x7f,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
	0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
	0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
	0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9,
	0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
	0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9,
	0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
	0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9,
	0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
	0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9,
	0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
	0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
	0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
	0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};