/**

	libCoreUtilities -- Reusable component libraries

	@file char.c

	@brief Character lookup utility functions


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

#include "char.h"

#ifdef TEST
	#include "CuTest.h"
#endif


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

#pragma mark base64 de/encoding

// Source - https://stackoverflow.com/a/48818578
// Posted by OGCJN, modified by community. See post 'Timeline' for change history
// Retrieved 2026-02-21, License - CC BY-SA 3.0

static const char base64_encoding_table[] = { 
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/' };

static const unsigned char base64_decoding_table[256] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x3f,
	0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
	0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
	0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/// caller must call free() on returned pointer!
char* base64_encode(const unsigned char *data, size_t input_length, size_t *output_length) {

	const int mod_table[] = { 0, 2, 1 };

	*output_length = 4 * ((input_length + 2) / 3);

	char *encoded_data = (char*)malloc(*output_length + 1);	// since we're returning a string, let's NUL-terminate it
	encoded_data[*output_length] = 0;

	if (encoded_data == NULL)
		return NULL;

	for (int i = 0, j = 0; i < input_length;) {
		uint32_t octet_a = i < input_length ? data[i++] : 0;
		uint32_t octet_b = i < input_length ? data[i++] : 0;
		uint32_t octet_c = i < input_length ? data[i++] : 0;
		uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;
		encoded_data[j++] = base64_encoding_table[(triple >> 3 * 6) & 0x3F];
		encoded_data[j++] = base64_encoding_table[(triple >> 2 * 6) & 0x3F];
		encoded_data[j++] = base64_encoding_table[(triple >> 1 * 6) & 0x3F];
		encoded_data[j++] = base64_encoding_table[(triple >> 0 * 6) & 0x3F];
	}

	for (int i = 0; i < mod_table[input_length % 3]; i++)
		encoded_data[*output_length - 1 - i] = '=';

	return encoded_data;
};

/// caller must call free() on returned pointer!
unsigned char* base64_decode(const unsigned char *data, size_t input_length, size_t *output_length) {

	if (input_length % 4 != 0)
		return NULL;

	*output_length = input_length / 4 * 3;

	if (data[input_length - 1] == '=') (*output_length)--;
	if (data[input_length - 2] == '=') (*output_length)--;

	unsigned char* decoded_data = (unsigned char*)malloc(*output_length);

	if (decoded_data == NULL)
		return NULL;

	for (int i = 0, j = 0; i < input_length;) {
		uint32_t sextet_a = data[i] == '=' ? 0 & i++ : base64_decoding_table[data[i++]];
		uint32_t sextet_b = data[i] == '=' ? 0 & i++ : base64_decoding_table[data[i++]];
		uint32_t sextet_c = data[i] == '=' ? 0 & i++ : base64_decoding_table[data[i++]];
		uint32_t sextet_d = data[i] == '=' ? 0 & i++ : base64_decoding_table[data[i++]];
		uint32_t triple = (sextet_a << 3 * 6) + (sextet_b << 2 * 6) + (sextet_c << 1 * 6) + (sextet_d << 0 * 6);
		if (j < *output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
		if (j < *output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
		if (j < *output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
	}

	return decoded_data;
};


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
