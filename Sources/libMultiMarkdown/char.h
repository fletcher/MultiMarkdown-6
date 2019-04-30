/**

 MultiMarkdown 6 -- Lightweight markup processor to produce HTML, LaTeX, and more.

 @file char.h

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


#ifndef CHAR_SMART_STRING_H
#define CHAR_SMART_STRING_H

#ifdef TEST
#include "CuTest.h"
#endif

/// Define character types
enum char_types {
	CHAR_WHITESPACE			= 1 << 0,	//!< ' ','\t'
	CHAR_PUNCTUATION		= 1 << 1,	//!< .!?,;:"'`~(){}[]#$%+-=<>&@\/^*_|
	CHAR_ALPHA				= 1 << 2,	//!< a-zA-Z
	CHAR_DIGIT				= 1 << 3,	//!< 0-9
	CHAR_LINE_ENDING		= 1 << 4,	//!< \n,\r,\0
	CHAR_INTRAWORD			= 1 << 5,	//!< Punctuation that might be inside a word -'
	CHAR_UPPER				= 1 << 6,	// ASCII upper case
	CHAR_LOWER				= 1 << 7,	// ASCII lower case
};


// Is character whitespace?
int char_is_whitespace(char c);

// Is character a newline, return, or EOF?
int char_is_line_ending(char c);

// Is character part of Windows line ending ('\r\n')?
int char_is_windows_line_ending(char * c);

// Is character punctuation?
int char_is_punctuation(char c);

// Is character alpha?
int char_is_alpha(char c);

// Is character digit?
int char_is_digit(char c);

// Is character alphanumeric?
int char_is_alphanumeric(char c);

// Is character lower case?
int char_is_lower_case(char c);

// Is character upper case?
int char_is_upper_case(char c);

// Is character a valid intraword character?
int char_is_intraword(char c);

// Is character either whitespace or line ending?
int char_is_whitespace_or_line_ending(char c);

// Is character either whitespace or punctuation?
int char_is_whitespace_or_punctuation(char c);

// Is character either whitespace or line ending or punctuation?
int char_is_whitespace_or_line_ending_or_punctuation(char c);

// Is byte a UTF-8 continuation byte
#define char_is_continuation_byte(x) ((x & 0xC0) == 0x80)

// Is byte the first byte of a multibyte UTF-8 sequence?
#define char_is_lead_multibyte(x) ((x & 0xC0) == 0xC0)


#endif

