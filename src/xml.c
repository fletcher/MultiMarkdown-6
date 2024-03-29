/* Generated by re2c 1.3 on Tue Sep 28 18:26:57 2021 */
/**

	MultiMarkdown -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file xml.c

	@brief Utilities to help parse XML files


	@author	Fletcher T. Penney
	@bug

**/

/*

	Copyright © 2016 - 2019 Fletcher T. Penney.


	The `MultiMarkdown 6` project is released under the MIT License..

	GLibFacade.c and GLibFacade.h are from the MultiMarkdown v4 project:

		https://github.com/fletcher/MultiMarkdown-4/

	MMD 4 is released under both the MIT License and GPL.


	CuTest is released under the zlib/libpng license. See CuTest.c for the
	text of the license.

	uthash library:
		Copyright (c) 2005-2016, Troy D. Hanson

		Licensed under Revised BSD license

	miniz library:
		Copyright 2013-2014 RAD Game Tools and Valve Software
		Copyright 2010-2014 Rich Geldreich and Tenacious Software LLC

		Licensed under the MIT license

	argtable3 library:
		Copyright (C) 1998-2001,2003-2011,2013 Stewart Heitmann
		<sheitmann@users.sourceforge.net>
		All rights reserved.

		Licensed under the Revised BSD License


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


	## Revised BSD License ##

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are
	met:
	    * Redistributions of source code must retain the above copyright
	      notice, this list of conditions and the following disclaimer.
	    * Redistributions in binary form must reproduce the above
	      copyright notice, this list of conditions and the following
	      disclaimer in the documentation and/or other materials provided
	      with the distribution.
	    * Neither the name of the <organization> nor the
	      names of its contributors may be used to endorse or promote
	      products derived from this software without specific prior
	      written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT
	HOLDER> BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES LOSS OF USE, DATA, OR
	PROFITS OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
	LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


*/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "d_string.h"
#include "xml.h"


#define print(x) d_string_append(out, x)
#define print_const(x) d_string_append_c_array(out, x, sizeof(x) - 1)
#define print_char(x) d_string_append_c(out, x)
#define printf(...) d_string_append_printf(out, __VA_ARGS__)


/// strndup not available on all platforms
static char * my_strndup(const char * source, size_t n) {
	if (source == NULL) {
		return NULL;
	}

	size_t len = 0;
	char * result;
	const char * test = source;

	// strlen is too slow if strlen(source) >> n
	for (len = 0; len < n; ++len) {
		if (*test == '\0') {
			break;
		}

		test++;
	}

	result = malloc(len + 1);

	if (result) {
		memcpy(result, source, len);
		result[len] = '\0';
	}

	return result;
}





/// skip through whitespace
size_t xml_scan_wsnl(const char * c) {
	const char * start = c;


	{
		unsigned char yych;
		yych = *c;

		switch (yych) {
			case '\t':
			case '\n':
			case '\r':
			case ' ':
				goto yy4;

			default:
				goto yy2;
		}

yy2:
		++c;
		{
			return 0;
		}
yy4:
		yych = *++c;

		switch (yych) {
			case '\t':
			case '\n':
			case '\r':
			case ' ':
				goto yy4;

			default:
				goto yy6;
		}

yy6: {
			return (size_t)( c - start );
		}
	}

}


/// scan generic attribute_name
size_t xml_scan_attribute_name(const char * c) {
	;
	const char * start = c;


	{
		unsigned char yych;
		yych = *c;

		switch (yych) {
			case ':':
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
			case 'G':
			case 'H':
			case 'I':
			case 'J':
			case 'K':
			case 'L':
			case 'M':
			case 'N':
			case 'O':
			case 'P':
			case 'Q':
			case 'R':
			case 'S':
			case 'T':
			case 'U':
			case 'V':
			case 'W':
			case 'X':
			case 'Y':
			case 'Z':
			case '_':
			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
			case 'g':
			case 'h':
			case 'i':
			case 'j':
			case 'k':
			case 'l':
			case 'm':
			case 'n':
			case 'o':
			case 'p':
			case 'q':
			case 'r':
			case 's':
			case 't':
			case 'u':
			case 'v':
			case 'w':
			case 'x':
			case 'y':
			case 'z':
				goto yy11;

			default:
				goto yy9;
		}

yy9:
		++c;
		{
			return 0;
		}
yy11:
		yych = *++c;

		switch (yych) {
			case '-':
			case '.':
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case ':':
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
			case 'G':
			case 'H':
			case 'I':
			case 'J':
			case 'K':
			case 'L':
			case 'M':
			case 'N':
			case 'O':
			case 'P':
			case 'Q':
			case 'R':
			case 'S':
			case 'T':
			case 'U':
			case 'V':
			case 'W':
			case 'X':
			case 'Y':
			case 'Z':
			case '_':
			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
			case 'g':
			case 'h':
			case 'i':
			case 'j':
			case 'k':
			case 'l':
			case 'm':
			case 'n':
			case 'o':
			case 'p':
			case 'q':
			case 'r':
			case 's':
			case 't':
			case 'u':
			case 'v':
			case 'w':
			case 'x':
			case 'y':
			case 'z':
				goto yy11;

			default:
				goto yy13;
		}

yy13: {
			return (size_t)( c - start );
		}
	}

}


/// scan until start of value, if present
size_t xml_scan_until_value(const char * c) {
	const char * marker = NULL;
	const char * start = c;


	{
		unsigned char yych;
		yych = *c;

		switch (yych) {
			case '\t':
			case '\n':
			case '\r':
			case ' ':
				goto yy18;

			case '=':
				goto yy19;

			default:
				goto yy16;
		}

yy16:
		++c;
yy17: {
			return 0;
		}
yy18:
		yych = *(marker = ++c);

		switch (yych) {
			case '\t':
			case '\n':
			case '\r':
			case ' ':
				goto yy20;

			case '=':
				goto yy23;

			default:
				goto yy17;
		}

yy19:
		yych = *(marker = ++c);

		switch (yych) {
			case '\t':
			case '\n':
			case '\r':
			case ' ':
				goto yy23;

			case '"':
				marker = c;
				goto yy25;

			case '\'':
				marker = c;
				goto yy27;

			default:
				goto yy17;
		}

yy20:
		yych = *++c;

		switch (yych) {
			case '\t':
			case '\n':
			case '\r':
			case ' ':
				goto yy20;

			case '=':
				goto yy23;

			default:
				goto yy22;
		}

yy22:
		c = marker;
		goto yy17;
yy23:
		yych = *++c;

		switch (yych) {
			case '\t':
			case '\n':
			case '\r':
			case ' ':
				goto yy23;

			case '"':
				marker = c;
				goto yy25;

			case '\'':
				marker = c;
				goto yy27;

			default:
				goto yy22;
		}

yy25:
		yych = *++c;

		switch (yych) {
			case 0x00:
				goto yy22;

			case '"':
				goto yy29;

			default:
				goto yy25;
		}

yy27:
		yych = *++c;

		switch (yych) {
			case 0x00:
				goto yy22;

			case '\'':
				goto yy29;

			default:
				goto yy27;
		}

yy29:
		++c;
		c = marker;
		{
			return (size_t)( c - start );
		}
	}

}


/// scan value
size_t xml_scan_value(const char * c) {
	const char * marker = NULL;
	const char * start = c;


	{
		unsigned char yych;
		yych = *c;

		switch (yych) {
			case '"':
				goto yy35;

			case '\'':
				goto yy36;

			default:
				goto yy33;
		}

yy33:
		++c;
yy34: {
			return 0;
		}
yy35:
		yych = *(marker = ++c);

		if (yych <= 0x00) {
			goto yy34;
		}

		goto yy38;
yy36:
		yych = *(marker = ++c);

		if (yych <= 0x00) {
			goto yy34;
		}

		goto yy43;
yy37:
		yych = *++c;
yy38:

		switch (yych) {
			case 0x00:
				goto yy39;

			case '"':
				goto yy40;

			default:
				goto yy37;
		}

yy39:
		c = marker;
		goto yy34;
yy40:
		++c;
		{
			return (size_t)( c - start );
		}
yy42:
		yych = *++c;
yy43:

		switch (yych) {
			case 0x00:
				goto yy39;

			case '\'':
				goto yy40;

			default:
				goto yy42;
		}
	}

}


/// Does the string include encoded newline?
size_t xml_scan_encoded_newline(const char * c, size_t len) {
	const char * marker = NULL;
	const char * start = c;

scan:

	if ((*c == '\0') || ((c - start) > len)) {
		// Not found
		return -1;
	}


	{
		unsigned char yych;
		yych = *c;

		switch (yych) {
			case '&':
				goto yy48;

			default:
				goto yy46;
		}

yy46:
		++c;
yy47: {
			goto scan;
		}
yy48:
		yych = *(marker = ++c);

		switch (yych) {
			case '#':
				goto yy49;

			default:
				goto yy47;
		}

yy49:
		yych = *++c;

		switch (yych) {
			case '1':
				goto yy51;

			default:
				goto yy50;
		}

yy50:
		c = marker;
		goto yy47;
yy51:
		yych = *++c;

		switch (yych) {
			case '0':
			case '3':
				goto yy52;

			default:
				goto yy50;
		}

yy52:
		yych = *++c;

		switch (yych) {
			case ';':
				goto yy53;

			default:
				goto yy50;
		}

yy53:
		++c;
		{
			return (size_t)(c - start);
		}
	}

}


/// Decode XML encoded text and print to DString
void print_xml_as_text(DString * out, const char * source, size_t start, size_t len) {
	const char * s_start = &source[start];
	const char * s_stop = &source[start + len];

	char * c = (char *) s_start;

	while (c < s_stop) {
		switch (*c) {
			case '&':
				switch (*++c) {
					case '#':
						if (strncmp(c, "#10;", 4) == 0) {
							print_char('\n');
							c += 4;
							continue;
						}

						if (strncmp(c, "#9;", 3) == 0) {
							print_char('\t');
							c += 3;
							continue;
						}

						if (strncmp(c, "#13;", 4) == 0) {
							print_char('\r');
							c += 4;
							continue;
						}

						break;

					case 'a':
						if (strncmp(c, "amp;", 4) == 0) {
							print_char('&');
							c += 4;
							continue;
						}

						if (strncmp(c, "apos;", 5) == 0) {
							print_char('\'');
							c += 5;
							continue;
						}

						break;

					case 'l':
						if (strncmp(c, "lt;", 3) == 0) {
							print_char('<');
							c += 3;
							continue;
						}

						break;

					case 'g':
						if (strncmp(c, "gt;", 3) == 0) {
							print_char('>');
							c += 3;
							continue;
						}

						break;

					case 'q':
						if (strncmp(c, "quot;", 5) == 0) {
							print_char('"');
							c += 5;
							continue;
						}

						break;

					default:
						break;
				}

				print_char('&');
				continue;
				break;

			default:
				print_char(*c);
				break;
		}

		c++;
	}
}


/// Parse XML text for attribute and value
size_t xml_extract_attribute(const char * source, size_t start, char ** attr, char ** value) {
	size_t cursor = start;
	size_t len = 0;

	if (*attr) {
		free(*attr);
		*attr = NULL;
	}

	if (*value) {
		free(*value);
		*value = NULL;
	}

	// Skip leading whitespace
	cursor += xml_scan_wsnl(&source[start]);

	len = xml_scan_attribute_name(&source[cursor]);

	if (len) {
		// Copy attribute name
		*attr = my_strndup(&source[cursor], len);

		cursor += len;

		// Value?
		cursor += xml_scan_until_value(&source[cursor]);
		len = xml_scan_value(&source[cursor]);

		if (len) {
			*value = my_strndup(&source[cursor + 1], len - 2);
		}

		cursor += len;
	}


	return cursor - start;
}


/// Extract attribute with specified name
char * xml_extract_named_attribute(const char * source, size_t start, const char * name) {
	char * lower_name = my_strndup(name, strlen(name));
	char * result = NULL;

	// Use lower case for easy comparison
	for (int i = 0; lower_name[i]; i++) {
		lower_name[i] = tolower(lower_name[i]);
	}

	char * attr = NULL, * value = NULL, * lower_attr = NULL;

	do {
		start += xml_extract_attribute(source, start, &attr, &value);

		if (attr) {
			lower_attr = my_strndup(attr, strlen(attr));

			// Use lower case for easy comparison
			for (int i = 0; lower_name[i]; i++) {
				lower_attr[i] = tolower(lower_attr[i]);
			}

			if (strcmp(lower_name, lower_attr) == 0) {
				// Match
				result = value;
				value = NULL;
				free(lower_attr);
				goto finish;
			}

			free(lower_attr);
		}
	} while (attr);

finish:
	free(attr);
	free(value);
	free(lower_name);

	return result;
}
