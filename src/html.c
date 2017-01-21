/**

	MultiMarkdown 6 -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file html.c

	@brief Convert token tree to HTML output.


	@author	Fletcher T. Penney
	@bug	

**/

/*

	Copyright © 2016 - 2017 Fletcher T. Penney.


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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "char.h"
#include "d_string.h"
#include "html.h"
#include "libMultiMarkdown.h"
#include "parser.h"
#include "token.h"
#include "scanners.h"
#include "writer.h"


#define LC(x) x


#define print(x) d_string_append(out, x)
#define print_char(x) d_string_append_c(out, x)
#define printf(...) d_string_append_printf(out, __VA_ARGS__)
//#define print_token(t) d_string_append_c_array(out, &(source[t->start + offset]), t->len)
#define print_token(t) d_string_append_c_array(out, &(source[t->start]), t->len)
#define print_localized(x) mmd_print_localized_char_html(out, x, scratch)

// Use Knuth's pseudo random generator to obfuscate email addresses predictably
long ran_num_next();

void mmd_print_char_html(DString * out, char c, bool obfuscate) {
	switch (c) {
		case '"':
			print("&quot;");
			break;
		case '&':
			print("&amp;");
			break;
		case '<':
			print("&lt;");
			break;
		case '>':
			print("&gt;");
			break;
		default:
			if (obfuscate && ((int) c == (((int) c) & 127))) {
				if (ran_num_next() % 2 == 0)
					printf("&#%d;", (int) c);
				else
					printf("&#x%x;", (unsigned int) c);
			} else {
				print_char(c);
			}
			break;
	}
}


void mmd_print_string_html(DString * out, const char * str, bool obfuscate) {
	while (*str != '\0') {
		mmd_print_char_html(out, *str, obfuscate);
		str++;
	}
}


void mmd_print_localized_char_html(DString * out, unsigned short type, scratch_pad * scratch) {
	int language = 0;

	switch (type) {
		case DASH_N:
			print("&#8211;");
			break;
		case DASH_M:
			print("&#8212;");
			break;
		case ELLIPSIS:
			print("&#8230;");
			break;
		case APOSTROPHE:
			print("&#8217;");
			break;
		case QUOTE_LEFT_SINGLE:
			switch (language) {
				case SWEDISH:
					print( "&#8217;");
					break;
				case FRENCH:
					print("&#39;");
					break;
				case GERMAN:
					print("&#8218;");
					break;
				case GERMANGUILL:
					print("&#8250;");
					break;
				default:
					print("&#8216;");
				}
			break;
		case QUOTE_RIGHT_SINGLE:
			switch (language) {
				case GERMAN:
					print("&#8216;");
					break;
				case GERMANGUILL:
					print("&#8249;");
					break;
				default:
					print("&#8217;");
				}
			break;
		case QUOTE_LEFT_DOUBLE:
			switch (language) {
				case DUTCH:
				case GERMAN:
					print("&#8222;");
					break;
				case GERMANGUILL:
					print("&#187;");
					break;
				case FRENCH:
					print("&#171;");
					break;
				case SWEDISH:
					print( "&#8221;");
					break;
				default:
					print("&#8220;");
				}
			break;
		case QUOTE_RIGHT_DOUBLE:
			switch (language) {
				case GERMAN:
					print("&#8220;");
					break;
				case GERMANGUILL:
					print("&#171;");
					break;
				case FRENCH:
					print("&#187;");
					break;
				case SWEDISH:
				case DUTCH:
				default:
					print("&#8221;");
				}
			break;
	}
}


void mmd_export_link_html(DString * out, const char * source, token * text, link * link, size_t offset, scratch_pad * scratch) {
	attr * a = link->attributes;

	if (link->url) {
		print("<a href=\"");
		mmd_print_string_html(out, link->url, false);
		print("\"");
	} else
		print("<a href=\"\"");

	if (link->title && link->title[0] != '\0') {
		print(" title=\"");
		mmd_print_string_html(out, link->title, false);
		print("\"");
	}

	while (a) {
		print(" ");
		print(a->key);
		print("=\"");
		print(a->value);
		print("\"");
		a = a->next;
	}

	print(">");

	mmd_export_token_tree_html(out, source, text->child, offset, scratch);

	print("</a>");
}


void mmd_export_image_html(DString * out, const char * source, token * text, link * link, size_t offset, scratch_pad * scratch) {
	attr * a = link->attributes;

	if (link->url)
		printf("<img src=\"%s\"", link->url);
	else
		print("<img src=\"\"");

	if (text) {
		print(" alt=\"");
		print_token_tree_raw(out, source, text->child);
		print("\"");
	}

	if (0 && link->label) {
		// \todo: Need to decide on approach to id's
		char * label = label_from_token(source, link->label);
		printf(" id=\"%s\"", label);
		free(label);
	}

	if (link->title && link->title[0] != '\0')
		printf(" title=\"%s\"", link->title);

	while (a) {
		print(" ");
		print(a->key);
		print("=\"");
		print(a->value);
		print("\"");
		a = a->next;
	}

	print(" />");
}


void mmd_export_token_html(DString * out, const char * source, token * t, size_t offset, scratch_pad * scratch) {
	if (t == NULL)
		return;

	short temp_short;
	link * temp_link = NULL;
	char * temp_char = NULL;
	bool temp_bool = 0;
	token * temp_token = NULL;

	switch (t->type) {
		case AMPERSAND:
		case AMPERSAND_LONG:
			print("&amp;");
			break;
		case ANGLE_LEFT:
			print("&lt;");
			break;
		case ANGLE_RIGHT:
			print("&gt;");
			break;
		case APOSTROPHE:
			if (!(scratch->extensions & EXT_SMART)) {
				print_token(t);
			} else {
				print_localized(APOSTROPHE);
			}
			break;
		case BACKTICK:
			if (t->mate == NULL)
				print_token(t);
			else if (t->mate->type == QUOTE_RIGHT_ALT)
				if (!(scratch->extensions & EXT_SMART)) {
					print_token(t);
				} else {
					print_localized(QUOTE_LEFT_DOUBLE);
				}
			else if (t->start < t->mate->start) {
				print("<code>");
			} else {
				print("</code>");
			}
			break;
		case BLOCK_BLOCKQUOTE:
			pad(out, 2, scratch);
			print("<blockquote>\n");
			scratch->padded = 2;
			mmd_export_token_tree_html(out, source, t->child, t->start + offset, scratch);
			pad(out, 1, scratch);
			print("</blockquote>");
			scratch->padded = 0;
			break;
		case BLOCK_CODE_FENCED:
		case BLOCK_CODE_INDENTED:
			pad(out, 2, scratch);
			print("<pre><code>");
			mmd_export_token_tree_html_raw(out, source, t->child, t->start + offset, scratch);
			print("</code></pre>");
			scratch->padded = 0;
			break;
		case BLOCK_EMPTY:
			break;
		case BLOCK_H1:
		case BLOCK_H2:
		case BLOCK_H3:
		case BLOCK_H4:
		case BLOCK_H5:
		case BLOCK_H6:
			pad(out, 2, scratch);
			temp_short = t->type - BLOCK_H1 + 1;
			if (scratch->extensions & EXT_NO_LABELS) {
				printf("<h%1d>", temp_short);
			} else {
				temp_char = label_from_token(source, t);
				printf("<h%1d id=\"%s\">", temp_short, temp_char);
				free(temp_char);
			}
			mmd_export_token_tree_html(out, source, t->child, t->start + offset, scratch);
			printf("</h%1d>", temp_short);
			scratch->padded = 0;
			break;
		case BLOCK_HR:
			pad(out, 2, scratch);
			print("<hr />");
			scratch->padded = 0;
			break;
		case BLOCK_HTML:
			pad(out, 2, scratch);
			print_token_raw(out, source, t);
			scratch->padded = 1;
			break;
		case BLOCK_LIST_BULLETED_LOOSE:
		case BLOCK_LIST_BULLETED:
			temp_short = scratch->list_is_tight;
			switch (t->type) {
				case BLOCK_LIST_BULLETED_LOOSE:
					scratch->list_is_tight = false;
					break;
				case BLOCK_LIST_BULLETED:
					scratch->list_is_tight = true;
					break;
			}
			pad(out, 2, scratch);
			print("<ul>");
			scratch->padded = 0;
			mmd_export_token_tree_html(out, source, t->child, offset, scratch);
			pad(out, 1, scratch);
			print("</ul>");
			scratch->padded = 0;
			scratch->list_is_tight = temp_short;
			break;
		case BLOCK_LIST_ENUMERATED_LOOSE:
		case BLOCK_LIST_ENUMERATED:
			temp_short = scratch->list_is_tight;
			switch (t->type) {
				case BLOCK_LIST_ENUMERATED_LOOSE:
					scratch->list_is_tight = false;
					break;
				case BLOCK_LIST_ENUMERATED:
					scratch->list_is_tight = true;
					break;
			}
			pad(out, 2, scratch);
			print("<ol>");
			scratch->padded = 0;
			mmd_export_token_tree_html(out, source, t->child, offset, scratch);
			pad(out, 1, scratch);
			print("</ol>");
			scratch->padded = 0;
			scratch->list_is_tight = temp_short;
			break;
		case BLOCK_LIST_ITEM:
			pad(out, 1, scratch);
			print("<li>");
			scratch->padded = 2;
			mmd_export_token_tree_html(out, source, t->child, offset, scratch);
			print("</li>");
			scratch->padded = 0;
			break;
		case BLOCK_LIST_ITEM_TIGHT:
			pad(out, 1, scratch);
			print("<li>");

			if (!scratch->list_is_tight)
				print("<p>");

			scratch->padded = 2;
			mmd_export_token_tree_html(out, source, t->child, offset, scratch);

			if (!scratch->list_is_tight)
				print("</p>");

			print("</li>");
			scratch->padded = 0;
			break;
		case BLOCK_PARA:
		case BLOCK_DEF_CITATION:
		case BLOCK_DEF_FOOTNOTE:
			pad(out, 2, scratch);
	
			if (!scratch->list_is_tight)
				print("<p>");

			mmd_export_token_tree_html(out, source, t->child, offset, scratch);

			if (scratch->footnote_being_printed) {
				scratch->footnote_para_counter--;

				if (scratch->footnote_para_counter == 0) {
					printf(" <a href=\"#fnref:%d\" title=\"%s\" class=\"reversefootnote\">&#160;&#8617;</a>", scratch->footnote_being_printed, LC("return to body"));
				}
			}

			if (scratch->citation_being_printed) {
				scratch->footnote_para_counter--;

				if (scratch->footnote_para_counter == 0) {
					printf(" <a href=\"#cnref:%d\" title=\"%s\" class=\"reversecitation\">&#160;&#8617;</a>", scratch->citation_being_printed, LC("return to body"));
				}
			}

			if (!scratch->list_is_tight)
				print("</p>");
			scratch->padded = 0;
			break;
		case BRACE_DOUBLE_LEFT:
			print("{{");
			break;
		case BRACE_DOUBLE_RIGHT:
			print("}}");
			break;
		case BRACKET_LEFT:
			print("[");			
			break;
		case BRACKET_CITATION_LEFT:
			print("[#");
			break;
		case BRACKET_FOOTNOTE_LEFT:
			print("[^");
			break;
		case BRACKET_IMAGE_LEFT:
			print("![");
			break;
		case BRACKET_VARIABLE_LEFT:
			print("[\%");
			break;
		case BRACKET_RIGHT:
			print("]");
			break;
		case COLON:
			print(":");
			break;
		case CRITIC_ADD_OPEN:
			print("{++");
			break;
		case CRITIC_ADD_CLOSE:
			print("++}");
			break;
		case CRITIC_COM_OPEN:
			print("{&gt;&gt;");
			break;
		case CRITIC_COM_CLOSE:
			print("&lt;&lt;}");
			break;
		case CRITIC_DEL_OPEN:
			print("{--");
			break;
		case CRITIC_DEL_CLOSE:
			print("--}");
			break;
		case CRITIC_HI_OPEN:
			print("{==");
			break;
		case CRITIC_HI_CLOSE:
			print("==}");
			break;
		case CRITIC_SUB_OPEN:
			print("{~~");
			break;
		case CRITIC_SUB_DIV:
			print("~&gt;");
			break;
		case CRITIC_SUB_CLOSE:
			print("~~}");
			break;
		case DASH_M:
			if (!(scratch->extensions & EXT_SMART)) {
				print_token(t);
			} else {
				print_localized(DASH_M);
			}
			break;
		case DASH_N:
			if (!(scratch->extensions & EXT_SMART)) {
				print_token(t);
			} else {
				print_localized(DASH_N);
			}
			break;
		case DOC_START_TOKEN:
			mmd_export_token_tree_html(out, source, t->child, offset, scratch);
			break;
		case ELLIPSIS:
			if (!(scratch->extensions & EXT_SMART)) {
				print_token(t);
			} else {
				print_localized(ELLIPSIS);
			}
			break;
		case EMPH_START:
			print("<em>");
			break;
		case EMPH_STOP:
			print("</em>");
			break;
		case ESCAPED_CHARACTER:
			mmd_print_char_html(out, source[t->start + 1], false);
			break;
		case HASH1:
		case HASH2:
		case HASH3:
		case HASH4:
		case HASH5:
		case HASH6:
			print_token(t);
			break;
		case INDENT_SPACE:
			print_char(' ');
			break;
		case INDENT_TAB:
			print_char('\t');
			break;
		case LINE_LIST_BULLETED:
		case LINE_LIST_ENUMERATED:
			mmd_export_token_tree_html(out, source, t->child, offset, scratch);
			break;
		case MARKER_BLOCKQUOTE:
		case MARKER_H1:
		case MARKER_H2:
		case MARKER_H3:
		case MARKER_H4:
		case MARKER_H5:
		case MARKER_H6:
			break;
		case MARKER_LIST_BULLET:
		case MARKER_LIST_ENUMERATOR:
			break;
		case MATH_BRACKET_OPEN:
			if (t->mate) {
				print("<span class=\"math\">\\[");
			} else
				print("\\[");
			break;
		case MATH_BRACKET_CLOSE:
			if (t->mate) {
				print("\\]</span>");
			} else
				print("\\]");
			break;
		case MATH_DOLLAR_SINGLE:
			if (t->mate) {
				(t->start < t->mate->start) ? ( print("<span class=\"math\">\\(") ) : ( print("\\)</span>") );
			} else {
				print("$");
			}
			break;
		case MATH_DOLLAR_DOUBLE:
			if (t->mate) {
				(t->start < t->mate->start) ? ( print("<span class=\"math\">\\[") ) : ( print("\\]</span>") );
			} else {
				print("$$");
			}
			break;
		case MATH_PAREN_OPEN:
			if (t->mate) {
				print("<span class=\"math\">\\(");
			} else
				print("\\(");
			break;
		case MATH_PAREN_CLOSE:
			if (t->mate) {
				print("\\)</span>");
			} else
				print("\\)");
			break;
		case NON_INDENT_SPACE:
			print_char(' ');
			break;
		case PAIR_BACKTICK:
			// Strip leading whitespace
			switch (t->child->next->type) {
				case TEXT_NL:
				case INDENT_TAB:
				case INDENT_SPACE:
				case NON_INDENT_SPACE:
					t->child->next->type = TEXT_EMPTY;
					break;
				case TEXT_PLAIN:
					while (t->child->next->len && char_is_whitespace(source[t->child->next->start])) {
						t->child->next->start++;
						t->child->next->len--;
					}
					break;
			}

			// Strip trailing whitespace
			switch (t->child->mate->prev->type) {
				case TEXT_NL:
				case INDENT_TAB:
				case INDENT_SPACE:
				case NON_INDENT_SPACE:
					t->child->mate->prev->type = TEXT_EMPTY;
					break;
				case TEXT_PLAIN:
					while (t->child->mate->prev->len && char_is_whitespace(source[t->child->mate->prev->start + t->child->mate->prev->len - 1])) {
						t->child->mate->prev->len--;
					}
					break;
			}
			t->child->type = TEXT_EMPTY;
			t->child->mate->type = TEXT_EMPTY;
			print("<code>");
			mmd_export_token_tree_html_raw(out, source, t->child, offset, scratch);
			print("</code>");
			break;
		case PAIR_ANGLE:
			temp_token = t;

			temp_char = url_accept(source, &temp_token, true);

			if (temp_char) {
				if (scan_email(temp_char))
					temp_bool = true;
				else
					temp_bool = false;
				print("<a href=\"");
				mmd_print_string_html(out, temp_char, temp_bool);
				print("\">");
				mmd_print_string_html(out, temp_char, temp_bool);
				print("</a>");
			} else if (scan_html(&source[t->start])) {
				print_token(t);
			} else {
				mmd_export_token_tree_html(out, source, t->child, offset, scratch);
			}

			free(temp_char);
			break;
		case PAIR_BRACES:
			mmd_export_token_tree_html(out, source, t->child, offset, scratch);
			break;
		case PAIR_BRACKET:
		case PAIR_BRACKET_IMAGE:
			parse_brackets(source, scratch, t, &temp_link, &temp_short, &temp_bool);

			if (temp_link) {
				if (t->type == PAIR_BRACKET) {
					// Link
					mmd_export_link_html(out, source, t, temp_link, offset, scratch);
				} else {
					// Image
					mmd_export_image_html(out, source, t, temp_link, offset, scratch);
				}
				
				if (temp_bool) {
					link_free(temp_link);
				}

				scratch->skip_token = temp_short;

				return;
			}

			// No links exist, so treat as normal
			mmd_export_token_tree_html(out, source, t->child, offset, scratch);
			break;
		case PAIR_BRACKET_CITATION:
			if (scratch->extensions & EXT_NOTES) {
				citation_from_bracket(source, scratch, t, &temp_short);

				if (temp_short < scratch->used_citations->size) {
					// Re-using previous citation
					printf("<a href=\"#cn:%d\" title=\"%s\" class=\"citation\">[%d]</a>",
						   temp_short, LC("see citation"), temp_short);
				} else {
					// This is a new citation
					printf("<a href=\"#cn:%d\" id=\"cnref:%d\" title=\"%s\" class=\"citation\">[%d]</a>",
						   temp_short, temp_short, LC("see citation"), temp_short);
				}
			} else {
				// Footnotes disabled
				mmd_export_token_tree_html(out, source, t->child, offset, scratch);
			}
			break;
		case PAIR_BRACKET_FOOTNOTE:
			if (scratch->extensions & EXT_NOTES) {
				footnote_from_bracket(source, scratch, t, &temp_short);

				if (temp_short < scratch->used_footnotes->size) {
					// Re-using previous footnote
					printf("<a href=\"#fn:%d\" title=\"%s\" class=\"footnote\">[%d]</a>",
						   temp_short, LC("see footnote"), temp_short);
				} else {
					// This is a new footnote
					printf("<a href=\"#fn:%d\" id=\"fnref:%d\" title=\"%s\" class=\"footnote\">[%d]</a>",
						   temp_short, temp_short, LC("see footnote"), temp_short);
				}
			} else {
				// Footnotes disabled
				mmd_export_token_tree_html(out, source, t->child, offset, scratch);
			}
			break;
		case PAIR_CRITIC_ADD:
			// Ignore if we're rejecting
			if (scratch->extensions & EXT_CRITIC_REJECT)
				break;
			if (scratch->extensions & EXT_CRITIC) {
				t->child->type = TEXT_EMPTY;
				t->child->mate->type = TEXT_EMPTY;
				if (scratch->extensions & EXT_CRITIC_ACCEPT) {
					mmd_export_token_tree_html(out, source, t->child, offset, scratch);
				} else {
					print("<ins>");
					mmd_export_token_tree_html(out, source, t->child, offset, scratch);
					print("</ins>");
				}
			} else {
				mmd_export_token_tree_html(out, source, t->child, offset, scratch);				
			}
			break;
		case PAIR_CRITIC_DEL:
			// Ignore if we're accepting
			if (scratch->extensions & EXT_CRITIC_ACCEPT)
				break;
			if (scratch->extensions & EXT_CRITIC) {
				t->child->type = TEXT_EMPTY;
				t->child->mate->type = TEXT_EMPTY;
				if (scratch->extensions & EXT_CRITIC_REJECT) {
					mmd_export_token_tree_html(out, source, t->child, offset, scratch);
				} else {
					print("<del>");
					mmd_export_token_tree_html(out, source, t->child, offset, scratch);
					print("</del>");
				}
			} else {
				mmd_export_token_tree_html(out, source, t->child, offset, scratch);				
			}
			break;
		case PAIR_CRITIC_COM:
			// Ignore if we're rejecting or accepting
			if ((scratch->extensions & EXT_CRITIC_REJECT) ||
				(scratch->extensions & EXT_CRITIC_ACCEPT))
				break;
			if (scratch->extensions & EXT_CRITIC) {
				t->child->type = TEXT_EMPTY;
				t->child->mate->type = TEXT_EMPTY;
				print("<span class=\"critic comment\">");
				mmd_export_token_tree_html(out, source, t->child, offset, scratch);
				print("</span>");
			} else {
				mmd_export_token_tree_html(out, source, t->child, offset, scratch);
			}
			break;
		case PAIR_CRITIC_HI:
			// Ignore if we're rejecting or accepting
			if ((scratch->extensions & EXT_CRITIC_REJECT) ||
				(scratch->extensions & EXT_CRITIC_ACCEPT))
				break;
			if (scratch->extensions & EXT_CRITIC) {
				t->child->type = TEXT_EMPTY;
				t->child->mate->type = TEXT_EMPTY;
				print("<mark>");
				mmd_export_token_tree_html(out, source, t->child, offset, scratch);
				print("</mark>");
			} else {
				mmd_export_token_tree_html(out, source, t->child, offset, scratch);
			}
			break;
		case CRITIC_SUB_DIV_A:
			print("~");
			break;
		case CRITIC_SUB_DIV_B:
			print("&gt;");
			break;
		case PAIR_CRITIC_SUB_DEL:
			if ((scratch->extensions & EXT_CRITIC) &&
				(t->next->type == PAIR_CRITIC_SUB_ADD)) {
				t->child->type = TEXT_EMPTY;
				t->child->mate->type = TEXT_EMPTY;
				if (scratch->extensions & EXT_CRITIC_ACCEPT) {

				} else if (scratch->extensions & EXT_CRITIC_REJECT) {
					mmd_export_token_tree_html(out, source, t->child, offset, scratch);
				} else {
					print("<del>");
					mmd_export_token_tree_html(out, source, t->child, offset, scratch);
					print("</del>");
				}
			} else {
				mmd_export_token_tree_html(out, source, t->child, offset, scratch);
			}
			break;
		case PAIR_CRITIC_SUB_ADD:
			if ((scratch->extensions & EXT_CRITIC) &&
				(t->prev->type == PAIR_CRITIC_SUB_DEL)) {
				t->child->type = TEXT_EMPTY;
				t->child->mate->type = TEXT_EMPTY;
				if (scratch->extensions & EXT_CRITIC_REJECT) {

				} else if (scratch->extensions & EXT_CRITIC_ACCEPT) {
					mmd_export_token_tree_html(out, source, t->child, offset, scratch);
				} else {
					print("<ins>");
					mmd_export_token_tree_html(out, source, t->child, offset, scratch);
					print("</ins>");
				}
			} else {
				mmd_export_token_tree_html(out, source, t->child, offset, scratch);
			}
			break;
		case PAIR_MATH:
		case PAIR_PAREN:
		case PAIR_QUOTE_DOUBLE:
		case PAIR_QUOTE_SINGLE:
		case PAIR_STAR:
		case PAIR_UL:
			mmd_export_token_tree_html(out, source, t->child, offset, scratch);
			break;
		case PAREN_LEFT:
			print("(");
			break;
		case PAREN_RIGHT:
			print(")");
			break;
		case PIPE:
			print_token(t);
			break;
		case PLUS:
			print_token(t);
			break;
		case QUOTE_SINGLE:
			if ((t->mate == NULL) || (!(scratch->extensions & EXT_SMART)))
				print("'");
			else
				(t->start < t->mate->start) ? ( print_localized(QUOTE_LEFT_SINGLE) ) : ( print_localized(QUOTE_RIGHT_SINGLE) );
			break;
		case QUOTE_DOUBLE:
			if ((t->mate == NULL) || (!(scratch->extensions & EXT_SMART)))
				print("&quot;");
			else
				(t->start < t->mate->start) ? ( print_localized(QUOTE_LEFT_DOUBLE) ) : ( print_localized(QUOTE_RIGHT_DOUBLE) );
			break;
		case QUOTE_RIGHT_ALT:
			if ((t->mate == NULL) || (!(scratch->extensions & EXT_SMART)))
				print("''");
			else
				print_localized(QUOTE_RIGHT_DOUBLE);
			break;
		case STAR:
			print_token(t);
			break;
		case STRONG_START:
			print("<strong>");
			break;
		case STRONG_STOP:
			print("</strong>");
			break;
		case SUBSCRIPT:
			if (t->mate) {
				(t->start < t->mate->start) ? (print("<sub>")) : (print("</sub>"));
			} else if (t->len != 1) {
				print("<sub>");
				mmd_export_token_html(out, source, t->child, offset, scratch);
				print("</sub>");
			} else {
				print("~");
			}
			break;
		case SUPERSCRIPT:
			if (t->mate) {
				(t->start < t->mate->start) ? (print("<sup>")) : (print("</sup>"));
			} else if (t->len != 1) {
				print("<sup>");
				mmd_export_token_html(out, source, t->child, offset, scratch);
				print("</sup>");
			} else {
				print("^");
			}	
			break;
		case TEXT_LINEBREAK:
			if (t->next) {
				print("<br />\n");
				scratch->padded = 1;
			}
			break;
		case CODE_FENCE:
		case TEXT_EMPTY:
			break;
		case TEXT_NL:
			if (t->next)
				print_char('\n');
			break;
		case TEXT_NUMBER_POSS_LIST:
		case TEXT_PERIOD:
		case TEXT_PLAIN:
			print_token(t);
			break;
		case UL:
			print_token(t);
			break;
		default:
			fprintf(stderr, "Unknown token type: %d\n", t->type);
			break;
	}
}


void mmd_export_token_tree_html(DString * out, const char * source, token * t, size_t offset, scratch_pad * scratch) {
	while (t != NULL) {
		if (scratch->skip_token) {
			scratch->skip_token--;
		} else {
			mmd_export_token_html(out, source, t, offset, scratch);
		}

		t = t->next;
	}
}


void mmd_export_token_html_raw(DString * out, const char * source, token * t, size_t offset, scratch_pad * scratch) {
	if (t == NULL)
		return;

	switch (t->type) {
		case BACKTICK:
			print_token(t);
			break;
		case AMPERSAND:
			print("&amp;");
			break;
		case AMPERSAND_LONG:
			print("&amp;amp;");
			break;
		case ANGLE_RIGHT:
			print("&gt;");
			break;
		case ANGLE_LEFT:
			print("&lt;");
			break;
		case ESCAPED_CHARACTER:
			print("\\");
			mmd_print_char_html(out, source[t->start + 1], false);
			break;
		case QUOTE_DOUBLE:
			print("&quot;");
			break;
		case CODE_FENCE:
			t->next->type = TEXT_EMPTY;
		case TEXT_EMPTY:
			break;
		default:
			if (t->child)
				mmd_export_token_tree_html_raw(out, source, t->child, offset, scratch);
			else
				print_token(t);
			break;
	}
}


void mmd_export_token_tree_html_raw(DString * out, const char * source, token * t, size_t offset, scratch_pad * scratch) {
	while (t != NULL) {
		if (scratch->skip_token) {
			scratch->skip_token--;
		} else {
			mmd_export_token_html_raw(out, source, t, offset, scratch);
		}

		t = t->next;
	}
}


void mmd_export_footnote_list_html(DString * out, const char * source, scratch_pad * scratch) {
	if (scratch->used_footnotes->size > 0) {
		footnote * note;
		token * content;

		pad(out, 2, scratch);
		print("<div class=\"footnotes\">\n<hr />\n<ol>");
		scratch->padded = 0;

		for (int i = 0; i < scratch->used_footnotes->size; ++i)
		{
			// Export footnote
			pad(out, 2, scratch);

			printf("<li id=\"fn:%d\">\n", i + 1);
			scratch->padded = 6;

			note = stack_peek_index(scratch->used_footnotes, i);
			content = note->content;

			scratch->footnote_para_counter = 0;

			// We need to know which block is the last one in the footnote
			while(content) {
				if (content->type == BLOCK_PARA)
					scratch->footnote_para_counter++;
				
				content = content->next;
			}

			content = note->content;
			scratch->footnote_being_printed = i + 1;

			mmd_export_token_tree_html(out, source, content, 0, scratch);

			pad(out, 1, scratch);
			printf("</li>");
			scratch->padded = 0;
		}

		pad(out, 2, scratch);
		print("</ol>\n</div>");
		scratch->padded = 0;
	}
}


void mmd_export_citation_list_html(DString * out, const char * source, scratch_pad * scratch) {
	if (scratch->used_citations->size > 0) {
		footnote * note;
		token * content;

		pad(out, 2, scratch);
		print("<div class=\"citations\">\n<hr />\n<ol>");
		scratch->padded = 0;

		for (int i = 0; i < scratch->used_citations->size; ++i)
		{
			// Export footnote
			pad(out, 2, scratch);

			printf("<li id=\"cn:%d\">\n", i + 1);
			scratch->padded = 6;

			note = stack_peek_index(scratch->used_citations, i);
			content = note->content;

			scratch->footnote_para_counter = 0;

			// We need to know which block is the last one in the footnote
			while(content) {
				if (content->type == BLOCK_PARA)
					scratch->footnote_para_counter++;
				
				content = content->next;
			}

			content = note->content;
			scratch->citation_being_printed = i + 1;

			mmd_export_token_tree_html(out, source, content, 0, scratch);

			pad(out, 1, scratch);
			printf("</li>");
			scratch->padded = 0;
		}

		pad(out, 2, scratch);
		print("</ol>\n</div>");
		scratch->padded = 0;
	}
}



