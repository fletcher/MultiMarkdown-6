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
#include "i18n.h"
#include "libMultiMarkdown.h"
#include "parser.h"
#include "token.h"
#include "scanners.h"
#include "writer.h"


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
			switch (scratch->quotes_lang) {
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
			switch (scratch->quotes_lang) {
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
			switch (scratch->quotes_lang) {
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
			switch (scratch->quotes_lang) {
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

	// If we're printing contents of bracket as text, then ensure we include it all
	if (text && text->child && text->child->len > 1) {
		text->child->next->start--;
		text->child->next->len++;
	}
	
	mmd_export_token_tree_html(out, source, text->child, offset, scratch);

	print("</a>");
}


void mmd_export_image_html(DString * out, const char * source, token * text, link * link, size_t offset, scratch_pad * scratch, bool is_figure) {
	attr * a = link->attributes;

	// Compatibility mode doesn't allow figures
	if (scratch->extensions & EXT_COMPATIBILITY)
		is_figure = false;

	if (is_figure) {
		// Remove wrapping <p> markers
		d_string_erase(out, out->currentStringLength - 3, 3);
		print("<figure>\n");
		scratch->close_para = false;
	}

	if (link->url)
		printf("<img src=\"%s\"", link->url);
	else
		print("<img src=\"\"");

	if (text) {
		print(" alt=\"");
		print_token_tree_raw(out, source, text->child);
		print("\"");
	}

	if (link->label && !(scratch->extensions & EXT_COMPATIBILITY)) {
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

	if (is_figure) {
		if (text) {
			print("\n<figcaption>");
			mmd_export_token_tree_html(out, source, text->child, offset, scratch);
			print("</figcaption>");
		}
		print("\n</figure>");
	}
}


void mmd_export_token_html(DString * out, const char * source, token * t, size_t offset, scratch_pad * scratch) {
	if (t == NULL)
		return;

	short	temp_short;
	short	temp_short2;
	link *	temp_link	= NULL;
	char *	temp_char	= NULL;
	char *	temp_char2	= NULL;
	bool	temp_bool	= 0;
	token *	temp_token	= NULL;

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
		case BLOCK_DEFINITION:
			pad(out, 2, scratch);
			print("<dd>");

			temp_short = scratch->list_is_tight;
			if (!(t->child->next && (t->child->next->type == BLOCK_EMPTY) && t->child->next->next))
				scratch->list_is_tight = true;

			mmd_export_token_tree_html(out, source, t->child, offset, scratch);
			print("</dd>");
			scratch->padded = 0;

			scratch->list_is_tight = temp_short;
			break;
		case BLOCK_DEFLIST:
			pad(out, 2, scratch);

			// Group consecutive definition lists into a single list.
			// lemon's LALR(1) parser can't properly handle this (to my understanding).

			if (!(t->prev && (t->prev->type == BLOCK_DEFLIST)))
				print("<dl>\n");
	
			scratch->padded = 2;

			mmd_export_token_tree_html(out, source, t->child, t->start + offset, scratch);
			pad(out, 1, scratch);

			if (!(t->next && (t->next->type == BLOCK_DEFLIST)))
				print("</dl>\n");

			scratch->padded = 1;
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
				printf("<h%1d>", temp_short + scratch->base_header_level - 1);
			} else {
				temp_char = label_from_token(source, t);
				printf("<h%1d id=\"%s\">", temp_short + scratch->base_header_level - 1, temp_char);
				free(temp_char);
			}
			mmd_export_token_tree_html(out, source, t->child, t->start + offset, scratch);
			printf("</h%1d>", temp_short + scratch->base_header_level - 1);
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

			if (scratch->close_para) {
				if (!scratch->list_is_tight)
					print("</p>");
			} else {
				scratch->close_para = true;
			}

			print("</li>");
			scratch->padded = 0;
			break;
		case BLOCK_META:
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

			if (scratch->close_para) {
				if (!scratch->list_is_tight)
					print("</p>");
			} else {
				scratch->close_para = true;
			}
			scratch->padded = 0;
			break;
		case BLOCK_TABLE:
			pad(out, 2, scratch);
			print("<table>\n");
			scratch->padded = 2;
			read_table_column_alignments(source, t, scratch);

			print("<colgroup>\n");
			for (int i = 0; i < scratch->table_column_count; ++i)
			{
				switch (scratch->table_alignment[i]) {
					case 'l':
						print("<col style=\"text-align:left;\"/>\n");
						break;
					case 'L':
						print("<col style=\"text-align:left;\" class=\"extended\"/>\n");
						break;
					case 'r':
						print("<col style=\"text-align:right;\"/>\n");
						break;
					case 'R':
						print("<col style=\"text-align:right;\" class=\"extended\"/>\n");
						break;
					case 'c':
						print("<col style=\"text-align:center;\"/>\n");
						break;
					case 'C':
						print("<col style=\"text-align:center;\" class=\"extended\"/>\n");
						break;
					default:
						print("<col />\n");
						break;
				}
			}
			print("</colgroup>\n");
			scratch->padded = 1;

			mmd_export_token_tree_html(out, source, t->child, offset, scratch);
			pad(out, 1, scratch);
			print("</table>");
			scratch->padded = 0;
			break;
		case BLOCK_TABLE_HEADER:
			pad(out, 2, scratch);
			print("<thead>\n");
			scratch->in_table_header = 1;
			mmd_export_token_tree_html(out, source, t->child, offset, scratch);
			scratch->in_table_header = 0;
			print("</thead>\n");
			scratch->padded = 1;
			break;
		case BLOCK_TABLE_SECTION:
			pad(out, 2, scratch);
			print("<tbody>\n");
			scratch->padded = 2;
			mmd_export_token_tree_html(out, source, t->child, offset, scratch);
			print("</tbody>");
			scratch->padded = 0;
			break;
		case BLOCK_TERM:
			pad(out, 2, scratch);
			print("<dt>");
			mmd_export_token_tree_html(out, source, t->child, offset, scratch);
			print("</dt>\n");
			scratch->padded = 2;
			break;
		case BLOCK_TOC:
			temp_short = 0;
			temp_short2 = 0;
			pad(out, 2, scratch);
			print("<div class=\"TOC\">");

			for (int i = 0; i < scratch->header_stack->size; ++i)
			{
				temp_token = stack_peek_index(scratch->header_stack, i);

				if (temp_token->type == temp_short2) {
					// Same level -- close list item
					print("</li>\n");
				}

				if (temp_short == 0) {
					// First item
					print("\n<ul>\n");
					temp_short = temp_token->type;
					temp_short2 = temp_short;
				}

				// Indent?
				if (temp_token->type == temp_short2) {
					// Same level -- NTD
				} else if (temp_token->type == temp_short2 + 1) {
					// Indent
					print("\n\n<ul>\n");
					temp_short2++;
				} else if (temp_token->type < temp_short2) {
					// Outdent
					print("</li>\n");
					while (temp_short2 > temp_token->type) {
						if (temp_short2 > temp_short)
							print("</ul></li>\n");
						else
							temp_short = temp_short2 - 1;

						temp_short2--;
					}
				} else {
					// Skipped more than one level -- ignore
					continue;
				}

				temp_char = label_from_token(source, temp_token);

				printf("<li><a href=\"#%s\">", temp_char);
				mmd_export_token_tree_html(out, source, temp_token->child, offset, scratch);
				print("</a>");
				free(temp_char);
			}

			while (temp_short2 > (temp_short)) {
				print("</ul>\n");
				temp_short2--;
			}
			
			if (temp_short)
				print("</li>\n</ul>\n");

			print("</div>");
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
					// Image -- should it be a figure (e.g. image is only thing in paragraph)?
					temp_token = t->next;

					if (temp_token &&
						((temp_token->type == PAIR_BRACKET) ||
						(temp_token->type == PAIR_PAREN))) {
						temp_token = temp_token->next;
					}

					if (temp_token && temp_token->type == TEXT_NL)
						temp_token = temp_token->next;

					if (temp_token && temp_token->type == TEXT_LINEBREAK)
						temp_token = temp_token->next;

					if (t->prev || temp_token) {
						mmd_export_image_html(out, source, t, temp_link, offset, scratch, false);
					} else {
						mmd_export_image_html(out, source, t, temp_link, offset, scratch, true);
					}
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
		case PAIR_BRACKET_VARIABLE:
			temp_char = text_inside_pair(source, t);
			temp_char2 = extract_metadata(scratch, temp_char);

			if (temp_char2)
				mmd_print_string_html(out, temp_char2, false);
			else
				mmd_export_token_tree_html(out, source, t->child, offset, scratch);

			// Don't free temp_char2 (it belongs to meta *)
			free(temp_char);
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
		case TABLE_CELL:
			if (scratch->in_table_header) {
				print("\t<th");
			} else {
				print("\t<td");
			}
			switch (scratch->table_alignment[scratch->table_cell_count]) {
				case 'l':
				case 'L':
					print(" style=\"text-align:left;\"");
					break;
				case 'r':
				case 'R':
					print(" style=\"text-align:right;\"");
					break;
				case 'c':
				case 'C':
					print(" style=\"text-align:center;\"");
					break;
			}
			if (t->next->type == TABLE_DIVIDER) {
				if (t->next->len > 1) {
					printf(" colspan=\"%d\"", t->next->len);
				}
			}
			print(">");
			mmd_export_token_tree_html(out, source, t->child, offset, scratch);
			if (scratch->in_table_header) {
				print("</th>\n");
			} else {
				print("</td>\n");
			}
			scratch->table_cell_count += t->next->len;
			break;
		case TABLE_DIVIDER:
			break;
		case TABLE_ROW:
			print("<tr>\n");
			scratch->table_cell_count = 0;
			mmd_export_token_tree_html(out, source, t->child, offset, scratch);
			print("</tr>\n");
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
		case TOC:
			print_token(t);
			break;
		case UL:
			print_token(t);
			break;
		default:
			fprintf(stderr, "Unknown token type: %d\n", t->type);
			token_describe(t, source);
			break;
	}
}


void mmd_export_token_tree_html(DString * out, const char * source, token * t, size_t offset, scratch_pad * scratch) {

	// Prevent stack overflow with "dangerous" input causing extreme recursion
	if (scratch->recurse_depth == kMaxExportRecursiveDepth) {
		return;
	}

	scratch->recurse_depth++;

	while (t != NULL) {
		if (scratch->skip_token) {
			scratch->skip_token--;
		} else {
			mmd_export_token_html(out, source, t, offset, scratch);
		}

		t = t->next;
	}

	scratch->recurse_depth--;
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


void mmd_start_complete_html(DString * out, const char * source, scratch_pad * scratch) {
	print("<!DOCTYPE html>\n<html>\n<head>\n\t<meta charset=\"utf-8\"/>\n");

	// Iterate over metadata keys
	meta * m;

	for (m = scratch->meta_hash; m != NULL; m = m->hh.next) {
		if (strcmp(m->key, "baseheaderlevel") == 0) {
		} else if (strcmp(m->key, "bibtex") == 0) {
		} else if (strcmp(m->key, "css") == 0) {
			print("\t<link type=\"text/css\" rel=\"stylesheet\" href=\"");
			mmd_print_string_html(out, m->value, false);
			print("\"/>\n");
		} else if (strcmp(m->key, "htmlfooter") == 0) {
		} else if (strcmp(m->key, "htmlheader") == 0) {
			print(m->value);
			print_char('\n');
		} else if (strcmp(m->key, "htmlheaderlevel") == 0) {
		} else if (strcmp(m->key, "lang") == 0) {
		} else if (strcmp(m->key, "latexfooter") == 0) {
		} else if (strcmp(m->key, "latexinput") == 0) {
		} else if (strcmp(m->key, "latexmode") == 0) {
		} else if (strcmp(m->key, "mmdfooter") == 0) {
		} else if (strcmp(m->key, "mmdheader") == 0) {
		} else if (strcmp(m->key, "quoteslanguage") == 0) {
		} else if (strcmp(m->key, "title") == 0) {
			print("\t<title>");
			mmd_print_string_html(out, m->value, false);
			print("</title>\n");
		} else if (strcmp(m->key, "transcludebase") == 0) {
		} else if (strcmp(m->key, "xhtmlheader") == 0) {
			print(m->value);
			print_char('\n');
		} else if (strcmp(m->key, "xhtmlheaderlevel") == 0) {
		} else {
			print("\t<meta name=\"");
			mmd_print_string_html(out, m->key, false);
			print("\" content=\"");
			mmd_print_string_html(out, m->value, false);
			print("\"/>\n");
		}
	}

	print("</head>\n<body>\n\n");
}


void mmd_end_complete_html(DString * out, const char * source, scratch_pad * scratch) {
	print("\n\n</body>\n</html>\n");
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



