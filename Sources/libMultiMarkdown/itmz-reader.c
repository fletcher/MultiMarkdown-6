/**

	MultiMarkdown -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file itmz-reader.c

	@brief


	@author	Fletcher T. Penney
	@bug

**/

/*

	Copyright © 2016 - 2018 Fletcher T. Penney.


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

#include <stdio.h>
#include <stdlib.h>

#include "mmd.h"
#include "itmz-reader.h"
#include "itmz-lexer.h"
#include "itmz-parser.h"
#include "token.h"
#include "xml.h"
#include "zip.h"


// Basic parser function declarations
void * ITMZAlloc();
void ITMZ();
void ITMZFree();
void ITMZTrace();


#define print(x) d_string_append(out, x)
#define print_const(x) d_string_append_c_array(out, x, sizeof(x) - 1)
#define print_char(x) d_string_append_c(out, x)
#define printf(...) d_string_append_printf(out, __VA_ARGS__)


/// Create a token chain from source ITMZ string
token * tokenize_itmz_string(mmd_engine * e, size_t start, size_t len) {

	// Create a scanner (for re2c)
	Scanner s;
	s.start = &e->dstr->str[start];
	s.cur = s.start;

	// Where do we stop parsing?
	const char * stop = &e->dstr->str[start] + len;

	int type;								// TOKEN type
	token * t;								// Create tokens for incorporation

	token * root = token_new(0, start, len);		// Store the final parse tree here

	const char * last_stop = &e->dstr->str[start];	// Remember where last token ended

	do {
		// Scan for next token (type of 0 means there is nothing left);
		type = itmz_scan(&s, stop);

		//if (type && s.start != last_stop) {
		if (s.start != last_stop) {
			// We skipped characters between tokens

			if (type) {
				// Create a default token type for the skipped characters
				//			t = token_new(TEXT_PLAIN, (size_t)(last_stop - e->dstr->str), (size_t)(s.start - last_stop));
			} else {
				if (stop > last_stop) {
					// Source text ends without newline
					//				t = token_new(TEXT_PLAIN, (size_t)(last_stop - e->dstr->str), (size_t)(stop - last_stop));
				}
			}
		} else if (type == 0 && stop > last_stop) {
			// Source text ends without newline
			//		t = token_new(TEXT_PLAIN, (size_t)(last_stop - e->dstr->str), (size_t)(stop - last_stop));
		}


		switch (type) {
			case 0:
				// 0 means we finished with input
				break;

			case ITMZ_WSNL:
				// Ignore for now
				break;

			default:
				t = token_new(type, (size_t)(s.start - e->dstr->str), (size_t)(s.cur - s.start));
				token_chain_append(root, t);
				break;
		}

		// Remember where token ends to detect skipped characters
		last_stop = s.cur;
	} while (type != 0);

	return root;
}


void parse_itmz_token_chain(mmd_engine * e, token * chain) {

	void* pParser = ITMZAlloc (malloc);		// Create a parser (for lemon)
	token * walker = chain->next;				// Walk the existing tree
	token * remainder;							// Hold unparsed tail of chain

	#ifndef NDEBUG
	ITMZTrace(stderr, "parser >>");
	#endif

	// Remove existing token tree
	e->root = NULL;

	while (walker != NULL) {
		remainder = walker->next;

		ITMZ(pParser, walker->type, walker, e);

		walker = remainder;
	}

	// Signal finish to parser
	#ifndef NDEBUG
	fprintf(stderr, "\nFinish parse\n");
	#endif
	ITMZ(pParser, 0, NULL, e);

	if (e->root) {
		// Successful parse -- process to new source document
		DString * final = d_string_new("");
		DString * metadata = d_string_new("");
		DString * out = final;

		size_t header_level = -1;	// ITMZ has a dummy root note
		size_t start, len;

		walker = chain->next;

		while (walker) {
			switch (walker->type) {
				case ITMZ_TOPIC_PREAMBLE:
				case ITMZ_TOPIC_OPEN:
				case ITMZ_TOPIC_SELF_CLOSE:
					header_level++;

					if (header_level == 0) {
						// ITMZ has a dummy parent node
						break;
					}

					// Advance over `<topic`
					start = walker->start + 6;

					char * text = xml_extract_named_attribute(e->dstr->str, start, "text");
					len = strlen(text);

					if (strcmp("&gt;&gt;Preamble&lt;&lt;", text) != 0) {
						if (out == metadata) {
							print_xml_as_text(out, text, 0, len);
							print_const(":\t");
						} else {
							// Print header

							if (xml_scan_encoded_newline(text, len) == -1) {
								// ATX header
								for (int i = 0; i < header_level; ++i) {
									print_char('#');
								}

								print_char(' ');
							}

							print_xml_as_text(out, text, 0, len);

							if (xml_scan_encoded_newline(text, len) == -1) {
								// ATX header
								print_char(' ');

								for (int i = 0; i < header_level; ++i) {
									print_char('#');
								}
							} else {
								// Setext Header
								switch (header_level) {
									case 1:
										print_const("\n======");
										break;

									default:
										print_const("\n------");
										break;
								}
							}

							print_const("\n");
						}
					}

					free(text);

					// Print contents of topic
					text = xml_extract_named_attribute(e->dstr->str, start, "note");

					if (text) {
						print_xml_as_text(out, text, 0, strlen(text));

						free(text);
					}

					if (out == metadata) {
						print_const("  \n");
					}

					if (walker->type == ITMZ_TOPIC_SELF_CLOSE) {
						header_level--;
					}

					break;

				case ITMZ_TOPIC_METADATA:
					// Now handle metadata
					out = metadata;
					header_level++;
					break;

				case ITMZ_TOPIC_CLOSE:
					header_level--;
					break;

				default:
					break;
			}

			walker = walker->next;
		}

		// Append body to metadata
		d_string_append_c_array(metadata, final->str, final->currentStringLength);

		// TODO: How to safely swap the new text, given that we might not own e->dstr->str?

		free(e->dstr->str);
		e->dstr->str = metadata->str;
		e->dstr->currentStringLength = metadata->currentStringLength;

		d_string_free(metadata, false);
		d_string_free(final, true);
	} else {
		// Unsuccessful parse -- free token chain
	}

	// Clean up token chain
	token_tree_free(chain);

	ITMZFree(pParser, free);
}


/// Create a token chain from source OPML string
void mmd_convert_itmz_string(mmd_engine * e, size_t start, size_t len) {
	// Need to extract mapdata.xml file from the zip archive
	DString * text = d_string_new("");

	mz_bool status = unzip_file_from_data(e->dstr->str, e->dstr->currentStringLength, "mapdata.xml", text);

	if (status) {
		free(e->dstr->str);
		e->dstr->str = text->str;
		e->dstr->currentStringLength = text->currentStringLength;

		d_string_free(text, false);

		// Now convert mapdata.xml -> MMD text
		token * chain = tokenize_itmz_string(e, 0, e->dstr->currentStringLength);
		parse_itmz_token_chain(e, chain);
	}
}
