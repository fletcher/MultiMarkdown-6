/**

	libCoreUtilities -- Reusable component libraries

	@file char_lookup.c

	@brief Create lookup table for char.c


	@author	Fletcher T. Penney
	@bug

 **/

/*

	Copyright © 2016-2020 Fletcher T. Penney.


	MIT License

	Copyright (c) 2016-2020 Fletcher T. Penney

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.

*/


#include <stdlib.h>
#include <stdio.h>

#include "char.h"

// Shortcuts to assign values to characters in lookup table
#define punctuation(x)	table[x] |= CHAR_PUNCTUATION
#define whitespace(x)	table[x] |= CHAR_WHITESPACE
#define alpha(x)		table[x] |= CHAR_ALPHA
#define digit(x)		table[x] |= CHAR_DIGIT
#define line_ending(x)	table[x] |= CHAR_LINE_ENDING
#define intraword(x)	table[x] |= CHAR_INTRAWORD
#define upper(x)		table[x] |= CHAR_UPPER
#define lower(x)		table[x] |= CHAR_LOWER

#define USE_EXTENDED_ASCII_disabled 1

int main( int argc, char ** argv ) {
	unsigned char table[256] = {0};

	// Define punctuation
	punctuation('.');
	punctuation('!');
	punctuation('?');

	punctuation(',');
	punctuation(';');
	punctuation(':');

	punctuation('"');
	punctuation('\'');
	punctuation('`');
	punctuation('~');

	punctuation('(');
	punctuation(')');
	punctuation('{');
	punctuation('}');
	punctuation('[');
	punctuation(']');

	punctuation('#');
	punctuation('$');
	punctuation('%');
	punctuation('+');
	punctuation('-');
	punctuation('=');
	punctuation('<');
	punctuation('>');

	punctuation('&');
	punctuation('@');
	punctuation('\\');
	punctuation('/');
	punctuation('^');

	punctuation('*');
	punctuation('_');

	punctuation('|');


	// Define intra-word punctuation
	intraword('\'');
	intraword('-');

	// Define whitespace
	whitespace(' ');
	whitespace('\t');


	// Define line endings
	line_ending('\n');
	line_ending('\r');
	line_ending('\0');		// Count EOF as line ending


	// Define digits
	for (char i = '0'; i <= '9'; ++i) {
		digit(i);
	}

	// Define alpha
	for (char i = 'a'; i <= 'z'; ++i) {
		alpha(i);
		lower(i);
	}

	for (char i = 'A'; i <= 'Z'; ++i) {
		alpha(i);
		upper(i);
	}


	// Extended ASCII
#ifdef USE_EXTENDED_ASCII

	// Punctuation ranges
	for (int i = 132; i < 138; ++i) {
		punctuation(i);
	}

	for (int i = 145; i < 156; ++i) {
		punctuation(i);
	}

	for (int i = 161; i < 192; ++i) {
		punctuation(i);
	}


	// Alphanumeric ranges
	for (int i = 192; i < 215; ++i) {
		alpha(i);
	}

	for (int i = 216; i < 247; ++i) {
		alpha(i);
	}

	for (int i = 248; i < 256; ++i) {
		alpha(i);
	}

	// Isolated extended ASCII characters
	for (int i = 128; i < 256; ++i) {
		switch (i) {
			case 128:
			case 130:
			case 139:
			case 215:
			case 247:
				punctuation(i);
				break;

			case 160:
				whitespace(i);
				break;

			case 131:
			case 138:
			case 140:
			case 142:
			case 156:
			case 158:
			case 159:
				alpha(i);
				break;

			default:
				break;
		}
	}

#endif


	// Print output as 16 x 16 table
	for (int i = 0; i < 16; ++i) {
		for (int j = 0; j < 16; ++j) {
			fprintf(stdout, "%3d,", table[i * 16 + j]);
		}

		fprintf(stdout, "\n");
	}
}
