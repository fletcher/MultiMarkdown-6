/**

 MultiMarkdown 6 -- Lightweight markup processor to produce HTML, LaTeX, and more.

 @file char.c

 @brief Character lookup utility functions


 @author	Fletcher T. Penney
 @bug

 **/

/*

 Copyright © 2016 - 2019 Fletcher T. Penney.


 The `MultiMarkdown 6` project is released under the MIT License..

 GLibFacade.c and GLibFacade.h are from the MultiMarkdown v4 project:

 https://github.com/fletcher/MultiMarkdown-4/

 MMD 4 is released under both the MIT License and GPL.


 CuTest is released under the zlib/libpng license. See CuTest.c for the text
 of the license.


 ## The MIT License ##

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.

 */

#include <stdlib.h>

#include "char.h"


/// Create this lookup table using char_lookup.c
static unsigned char smart_char_type[256] = {
	16,  0,  0,  0,  0,  0,  0,  0,  0,  1, 16,  0,  0, 16,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	1,  2,  2,  2,  2,  2,  2, 34,  2,  2,  2,  2,  2, 34,  2,  2,
	8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  2,  2,  2,  2,  2,  2,
	2, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68,
	68, 68, 68, 68, 68, 68, 68, 68, 68, 68, 68,  2,  2,  2,  2,  2,
	2, 132, 132, 132, 132, 132, 132, 132, 132, 132, 132, 132, 132, 132, 132, 132,
	132, 132, 132, 132, 132, 132, 132, 132, 132, 132, 132,  2,  2,  2,  2,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
};


static int CHAR_ALPHANUMERIC = CHAR_ALPHA | CHAR_DIGIT;

static int CHAR_WHITESPACE_OR_PUNCTUATION = CHAR_WHITESPACE | CHAR_PUNCTUATION;

static int CHAR_WHITESPACE_OR_LINE_ENDING = CHAR_WHITESPACE | CHAR_LINE_ENDING;

static int CHAR_WHITESPACE_OR_LINE_ENDING_OR_PUNCTUATION = CHAR_WHITESPACE | CHAR_LINE_ENDING | CHAR_PUNCTUATION;

static int CHAR_ALPHA_OR_INTRAWORD = CHAR_ALPHA | CHAR_INTRAWORD;


// Is character whitespace?
int char_is_whitespace(char c) {
	return smart_char_type[(unsigned char) c] & CHAR_WHITESPACE;
}

// Is character a newline, return, or EOF?
int char_is_line_ending(char c) {
	return smart_char_type[(unsigned char) c] & CHAR_LINE_ENDING;
}

// Is character part of Windows line ending ('\r\n')?
int char_is_windows_line_ending(char * c) {
	if (*c == '\n') {
		return (*(c - 1) == '\r') ? 1 : 0;
	}

	if (*c == '\r') {
		return (*(c + 1) == '\n') ? 1 : 0;
	}

	return 0;
}

#ifdef TEST
void Test_char_is_windows_line_ending(CuTest * tc) {
	char * test = "\r\n\n";

	CuAssertIntEquals(tc, 1, char_is_windows_line_ending(&test[0]));
	CuAssertIntEquals(tc, 1, char_is_windows_line_ending(&test[1]));
	CuAssertIntEquals(tc, 0, char_is_windows_line_ending(&test[2]));
}
#endif

// Is character punctuation?
int char_is_punctuation(char c) {
	return smart_char_type[(unsigned char) c] & CHAR_PUNCTUATION;
}

// Is character alpha?
int char_is_alpha(char c) {
	return smart_char_type[(unsigned char) c] & CHAR_ALPHA;
}

// Is character digit?
int char_is_digit(char c) {
	return smart_char_type[(unsigned char) c] & CHAR_DIGIT;
}

// Is character alphanumeric?
int char_is_alphanumeric(char c) {
	return smart_char_type[(unsigned char) c] & CHAR_ALPHANUMERIC;
}

// Is character lower case?
int char_is_lower_case(char c) {
	return smart_char_type[(unsigned char) c] & CHAR_LOWER;
}

// Is character upper case?
int char_is_upper_case(char c) {
	return smart_char_type[(unsigned char) c] & CHAR_UPPER;
}

// Is character a valid intraword character?
int char_is_intraword(char c) {
	return smart_char_type[(unsigned char) c] & CHAR_ALPHA_OR_INTRAWORD;
}

// Is character either whitespace or line ending?
int char_is_whitespace_or_line_ending(char c) {
	return smart_char_type[(unsigned char) c] & CHAR_WHITESPACE_OR_LINE_ENDING;
}

// Is character either whitespace or punctuation?
int char_is_whitespace_or_punctuation(char c) {
	return smart_char_type[(unsigned char) c] & CHAR_WHITESPACE_OR_PUNCTUATION;
}

// Is character either whitespace or line ending or punctuation?
int char_is_whitespace_or_line_ending_or_punctuation(char c) {
	return smart_char_type[(unsigned char) c] & CHAR_WHITESPACE_OR_LINE_ENDING_OR_PUNCTUATION;
}

// From https://www.cl.cam.ac.uk/~mgk25/ucs/utf8_check.c

/*
 * The utf8_check() function scans the '\0'-terminated string starting
 * at s. It returns a pointer to the first byte of the first malformed
 * or overlong UTF-8 sequence found, or NULL if the string contains
 * only correct UTF-8. It also spots UTF-8 sequences that could cause
 * trouble if converted to UTF-16, namely surrogate characters
 * (U+D800..U+DFFF) and non-Unicode positions (U+FFFE..U+FFFF). This
 * routine is very likely to find a malformed sequence if the input
 * uses any other encoding than UTF-8. It therefore can be used as a
 * very effective heuristic for distinguishing between UTF-8 and other
 * encodings.
 *
 * I wrote this code mainly as a specification of functionality; there
 * are no doubt performance optimizations possible for certain CPUs.
 *
 * Markus Kuhn <http://www.cl.cam.ac.uk/~mgk25/> -- 2005-03-30
 * License: http://www.cl.cam.ac.uk/~mgk25/short-license.html
 */

// Is the string valid UTF-8? (Returns pointer to first sequence)
unsigned char * utf8_check(unsigned char * s) {
	while (*s) {
		if (*s < 0x80)
			/* 0xxxxxxx */
		{
			s++;
		} else if ((s[0] & 0xe0) == 0xc0) {
			/* 110XXXXx 10xxxxxx */
			if ((s[1] & 0xc0) != 0x80 ||
					(s[0] & 0xfe) == 0xc0) {                      /* overlong? */
				return s;
			} else {
				s += 2;
			}
		} else if ((s[0] & 0xf0) == 0xe0) {
			/* 1110XXXX 10Xxxxxx 10xxxxxx */
			if ((s[1] & 0xc0) != 0x80 ||
					(s[2] & 0xc0) != 0x80 ||
					(s[0] == 0xe0 && (s[1] & 0xe0) == 0x80) ||    /* overlong? */
					(s[0] == 0xed && (s[1] & 0xe0) == 0xa0) ||    /* surrogate? */
					(s[0] == 0xef && s[1] == 0xbf &&
					 (s[2] & 0xfe) == 0xbe)) {                    /* U+FFFE or U+FFFF? */
				return s;
			} else {
				s += 3;
			}
		} else if ((s[0] & 0xf8) == 0xf0) {
			/* 11110XXX 10XXxxxx 10xxxxxx 10xxxxxx */
			if ((s[1] & 0xc0) != 0x80 ||
					(s[2] & 0xc0) != 0x80 ||
					(s[3] & 0xc0) != 0x80 ||
					(s[0] == 0xf0 && (s[1] & 0xf0) == 0x80) ||    /* overlong? */
					(s[0] == 0xf4 && s[1] > 0x8f) || s[0] > 0xf4) { /* > U+10FFFF? */
				return s;
			} else {
				s += 4;
			}
		} else {
			return s;
		}
	}

	return NULL;
}


#ifdef TEST
void Test_utf8_check(CuTest * tc) {
	unsigned char * check;

	CuAssertPtrEquals(tc, NULL, utf8_check((unsigned char *) "This is plain ASCII"));
	CuAssertPtrEquals(tc, NULL, utf8_check((unsigned char *) "This ü UTF-8"));
	CuAssertPtrEquals(tc, NULL, utf8_check((unsigned char *) "This 👪"));

	CuAssertPtrEquals(tc, NULL, utf8_check((unsigned char *) "Ḽơᶉëᶆ ȋṕšᶙṁ ḍỡḽǭᵳ ʂǐť ӓṁệẗ, ĉṓɲṩḙċťᶒțûɾ ấɖḯƥĭṩčįɳġ ḝłįʈ, șếᶑ ᶁⱺ ẽḭŭŝḿꝋď ṫĕᶆᶈṓɍ ỉñḉīḑȋᵭṵńť ṷŧ ḹẩḇőꝛế éȶ đꝍꞎôꝛȇ ᵯáꞡᶇā ąⱡîɋṹẵ"));

	CuAssertPtrNotNull(tc, utf8_check((unsigned char *) "\xe2\x28\xa1"));
}
#endif
