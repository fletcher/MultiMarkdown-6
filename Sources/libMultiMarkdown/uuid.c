/**

	MultiMarkdown -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file uuid.c

	@brief


	@author	Fletcher T. Penney
	@bug

**/

/*

	Copyright © 2016 - 2017 Fletcher T. Penney.


	The `MultiMarkdown 6` project is released under the MIT License..

	GLibFacade.c and GLibFacade.h are from the MultiMarkdown v4 project:

		https://github.com/fletcher/MultiMarkdown-4/

	MMD 4 is released under both the MIT License and GPL.


	CuTest is released under the zlib/libpng license. See CuTest.c for the
	text of the license.

	uthash library:
		Copyright (c) 2005-2016, Troy D. Hanson

		Licensed under BSD Revised license

	miniz library:
		Copyright 2013-2014 RAD Game Tools and Valve Software
		Copyright 2010-2014 Rich Geldreich and Tenacious Software LLC

		Licensed under the MIT license


	## The MIT License ##

	Permission is hereby granted, free of charge, to any person obtaining
	a copy of this software and associated documentation files (the
	"Software"), to deal in the Software without restriction, including
	without limitation the rights to use, copy, modify, merge, publish,
	distribute, sublicense, and/or sell copies of the Software, and to
	permit persons to whom the Software is furnished to do so, subject to
	the following conditions:

	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
	CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
	TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


*/


#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "uuid.h"


#define SETBIT(a, n) (a[n/CHAR_BIT] |= (1<<(n % CHAR_BIT)))
#define CLEARBIT(a, n) (a[n/CHAR_BIT] &= ~(1<<(n % CHAR_BIT)))

char * uuid_string_from_bits(unsigned char * raw);

char * uuid_new(void) {
	unsigned char raw[16];

	// Get 128 bits of random goodness
	for (int i = 0; i < 16; ++i) {
		raw[i] = rand() % 256;
	}

//	Need to set certain bits for v4 compliance
	CLEARBIT(raw, 52);
	CLEARBIT(raw, 53);
	SETBIT(raw, 54);
	CLEARBIT(raw, 55);
	CLEARBIT(raw, 70);
	SETBIT(raw, 71);

	return uuid_string_from_bits(raw);
}

char * uuid_string_from_bits(unsigned char * raw) {
	char * result = malloc(37);

	sprintf(result, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
	        raw[0], raw[1], raw[2], raw[3], raw[4], raw[5], raw[6], raw[7],
	        raw[8], raw[9], raw[10], raw[11], raw[12], raw[13], raw[14], raw[15] );

	return result;
}



// http://stackoverflow.com/questions/322938/recommended-way-to-initialize-srand
// http://www.concentric.net/~Ttwang/tech/inthash.htm
unsigned long mix(unsigned long a, unsigned long b, unsigned long c) {
	a = a - b;
	a = a - c;
	a = a ^ (c >> 13);
	b = b - c;
	b = b - a;
	b = b ^ (a << 8);
	c = c - a;
	c = c - b;
	c = c ^ (b >> 13);
	a = a - b;
	a = a - c;
	a = a ^ (c >> 12);
	b = b - c;
	b = b - a;
	b = b ^ (a << 16);
	c = c - a;
	c = c - b;
	c = c ^ (b >> 5);
	a = a - b;
	a = a - c;
	a = a ^ (c >> 3);
	b = b - c;
	b = b - a;
	b = b ^ (a << 10);
	c = c - a;
	c = c - b;
	c = c ^ (b >> 15);
	return c;
}


void custom_seed_rand(void) {
	// Seed random number generator
	// This is not a "cryptographically secure" random seed,
	// but good enough for an EPUB id....
	unsigned long seed = mix(clock(), time(NULL), clock());
	srand(seed);
}

