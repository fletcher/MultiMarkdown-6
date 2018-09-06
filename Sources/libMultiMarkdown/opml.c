/**

	MultiMarkdown -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file opml.c

	@brief Export to OPML (Outline Processor Markup Language)


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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "opml.h"
#include "parser.h"

#define print(x) d_string_append(out, x)
#define print_const(x) d_string_append_c_array(out, x, sizeof(x) - 1)
#define print_char(x) d_string_append_c(out, x)
#define printf(...) d_string_append_printf(out, __VA_ARGS__)
#define print_token(t) d_string_append_c_array(out, &(source[t->start]), t->len)
#define print_localized(x) mmd_print_localized_char_latex(out, x, scratch)



void mmd_print_source_opml(DString * out, const char * source, size_t start, size_t len) {
	const char * s_start = &source[start];
	const char * s_stop = &source[start + len];

	char * c = (char *) s_start;

	while (c < s_stop) {
		switch (*c) {
			case '&':
				print_const("&amp;");
				break;

			case '<':
				print_const("&lt;");
				break;

			case '>':
				print_const("&gt;");
				break;

			case '"':
				print_const("&quot;");
				break;

			case '\'':
				print_const("&apos;");
				break;

			case '\n':
				print_const("&#10;");
				break;

			case '\r':
				print_const("&#13;");
				break;

			case '\t':
				print_const("&#9;");
				break;

			default:
				print_char(*c);
				break;
		}

		c++;
	}
}


void mmd_check_preamble_opml(DString * out, token * t, scratch_pad * scratch) {
	if (t) {
		token * walker = t->child;

		while (walker) {
			switch (walker->type) {
				case BLOCK_META:
					walker = walker->next;
					break;

				case BLOCK_H1:
				case BLOCK_H2:
				case BLOCK_H3:
				case BLOCK_H4:
				case BLOCK_H5:
				case BLOCK_H6:
				case BLOCK_SETEXT_1:
				case BLOCK_SETEXT_2:
					walker = NULL;
					break;

				default:
					print_const("<outline text=\"&gt;&gt;Preamble&lt;&lt;\" _note=\"");
					scratch->opml_item_closed = 0;
					stack_push(scratch->outline_stack, walker);
					walker = NULL;
					break;
			}
		}
	}
}


/// Export title from metadata
void mmd_export_title_opml(DString * out, const char * source, scratch_pad * scratch) {
	meta * m;

	HASH_FIND_STR(scratch->meta_hash, "title", m);

	if (m) {
		print_const("<head><title>");

		size_t len = strlen(m->value);
		mmd_print_source_opml(out, m->value, 0, len);

		print_const("</title></head>\n");
	}
}


/// Export all metadata
void mmd_export_metadata_opml(DString * out, const char * source, scratch_pad * scratch) {
	meta * m;
	size_t len;

	if (scratch->meta_hash) {
		print_const("<outline text=\"&gt;&gt;Metadata&lt;&lt;\">\n");

		for (m = scratch->meta_hash; m != NULL; m = m->hh.next) {
			print_const("<outline text=\"");
			len = strlen(m->key);
			mmd_print_source_opml(out, m->key, 0, len);

			print_const("\" _note=\"");
			len = strlen(m->value);
			mmd_print_source_opml(out, m->value, 0, len);

			print_const("\"/>\n");
		}

		print_const("</outline>\n");
	}
}


/// Track outline levels to create proper outline structure
void mmd_outline_add_opml(DString * out, const char * source, token * current, scratch_pad * scratch) {
	token * t;
	short level;		// Header level we are adding
	short t_level;
	stack * s = scratch->outline_stack;

	if (current->type != DOC_START_TOKEN) {
		switch (current->type) {
			case BLOCK_SETEXT_1:
				level = 1;
				break;

			case BLOCK_SETEXT_2:
				level = 2;
				break;

			case BLOCK_H1:
			case BLOCK_H2:
			case BLOCK_H3:
			case BLOCK_H4:
			case BLOCK_H5:
			case BLOCK_H6:
				level = 1 + current->type - BLOCK_H1;
				break;

			default:
				level = 100;
				break;
		}

		level += scratch->base_header_level - 1;
	} else {
		level = 0;
	}

	if (s->size) {
		t = stack_peek(s);

		// Close last outline item?
		if (scratch->opml_item_closed == 0) {
			// Insert direct contents of that item?
			size_t start;
			size_t len;

			switch (t->type) {
				case BLOCK_SETEXT_1:
				case BLOCK_SETEXT_2:
					if (t->next) {
						start = t->next->start;
					} else {
						start = t->start + t->len;
					}

					break;

				case BLOCK_H1:
				case BLOCK_H2:
				case BLOCK_H3:
				case BLOCK_H4:
				case BLOCK_H5:
				case BLOCK_H6:
					start = t->start + t->len;
					break;

				default:
					start = t->start;
					break;
			}

			if (current->type != DOC_START_TOKEN) {
				len = current->start - start;
			} else {
				// Finish out document
				len = current->start + current->len - start;
			}

			// Output as XML string
			mmd_print_source_opml(out, source, start, len);

			print_const("\">");
			scratch->opml_item_closed = 1;
		}

		while (t) {
			switch (t->type) {
				case BLOCK_SETEXT_1:
					t_level = 1;
					break;

				case BLOCK_SETEXT_2:
					t_level = 2;
					break;

				case BLOCK_H1:
				case BLOCK_H2:
				case BLOCK_H3:
				case BLOCK_H4:
				case BLOCK_H5:
				case BLOCK_H6:
					t_level = 1 + t->type - BLOCK_H1;
					break;

				default:
					t_level = 100;
					break;
			}

			t_level += scratch->base_header_level - 1;

			if (t_level >= level) {
				// Close out level
				print_const("</outline>\n");

				stack_pop(s);
				t = stack_peek(s);
			} else {
				// Nothing to close
				t = NULL;
			}
		}
	}


	// Add current level to stack and open
	if (current->type != DOC_START_TOKEN) {
		stack_push(s, current);
		print_const("<outline");
		scratch->opml_item_closed = 0;
	}
}


/// Extract header title
void mmd_export_header_opml(DString * out, const char * source, token * t, scratch_pad * scratch) {
	if (t && t->child) {
		size_t start = t->start;
		size_t stop = t->start + t->len;

		token * walker = t->child;

		while (walker) {
			switch (walker->type) {
				case MARKER_H1:
				case MARKER_H2:
				case MARKER_H3:
				case MARKER_H4:
				case MARKER_H5:
				case MARKER_H6:
					walker = walker->next;
					break;

				default:
					start = walker->start;
					walker = NULL;
					break;
			}
		}

		walker = t->child->tail;

		while (walker) {
			switch (walker->type) {
				case TEXT_NL:
				case TEXT_NL_SP:
				case INDENT_TAB:
				case INDENT_SPACE:
				case NON_INDENT_SPACE:
				case MARKER_H1:
				case MARKER_H2:
				case MARKER_H3:
				case MARKER_H4:
				case MARKER_H5:
				case MARKER_H6:
					walker = walker->prev;
					break;

				default:
					stop = walker->start + walker->len;
					walker = NULL;
					break;
			}
		}

		mmd_print_source_opml(out, source, start, stop - start);
	}
}


void mmd_export_token_opml(DString * out, const char * source, token * t, scratch_pad * scratch) {
	if (t == NULL) {
		return;
	}

	switch (t->type) {
		case DOC_START_TOKEN:
			print_const("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<opml version=\"1.0\">\n");

			// Export metadata
			mmd_export_title_opml(out, source, scratch);

			print_const("<body>\n");

			// Check for content before first header
			mmd_check_preamble_opml(out, t, scratch);

			// Export body
			mmd_export_token_tree_opml(out, source, t->child, scratch);

			// Close out any existing outline levels
			mmd_outline_add_opml(out, source, t, scratch);

			mmd_export_metadata_opml(out, source, scratch);

			print_const("</body>\n</opml>\n");

			break;

		case BLOCK_H1:
		case BLOCK_H2:
		case BLOCK_H3:
		case BLOCK_H4:
		case BLOCK_H5:
		case BLOCK_H6:
		case BLOCK_SETEXT_1:
		case BLOCK_SETEXT_2:
			mmd_outline_add_opml(out, source, t, scratch);

			print_const(" text=\"");
			mmd_export_header_opml(out, source, t, scratch);
			trim_trailing_whitespace_d_string(out);
			print_const("\" _note=\"");
			break;

		default:
			// Skip everything else
			break;
	}
}


void mmd_export_token_tree_opml(DString * out, const char * source, token * t, scratch_pad * scratch) {

	// Prevent stack overflow with "dangerous" input causing extreme recursion
	if (scratch->recurse_depth == kMaxExportRecursiveDepth) {
		return;
	}

	scratch->recurse_depth++;

	while (t != NULL) {
		if (scratch->skip_token) {
			scratch->skip_token--;
		} else {
			mmd_export_token_opml(out, source, t, scratch);
		}

		t = t->next;
	}

	scratch->recurse_depth--;
}
