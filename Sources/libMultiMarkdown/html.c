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

#include <ctype.h>
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
#define print_const(x) d_string_append_c_array(out, x, sizeof(x) - 1)
#define print_char(x) d_string_append_c(out, x)
#define printf(...) d_string_append_printf(out, __VA_ARGS__)
#define print_token(t) d_string_append_c_array(out, &(source[t->start]), t->len)
#define print_localized(x) mmd_print_localized_char_html(out, x, scratch)


/// strdup() not available on all platforms
static char * my_strdup(const char * source) {
	if (source == NULL) {
		return NULL;
	}

	char * result = malloc(strlen(source) + 1);

	if (result) {
		strcpy(result, source);
	}

	return result;
}


// Use Knuth's pseudo random generator to obfuscate email addresses predictably
long ran_num_next(void);

void mmd_print_char_html(DString * out, char c, bool obfuscate) {
	switch (c) {
		case '"':
			print_const("&quot;");
			break;

		case '&':
			print_const("&amp;");
			break;

		case '<':
			print_const("&lt;");
			break;

		case '>':
			print_const("&gt;");
			break;

		case '\n':
		case '\r':
			print_const("<br/>\n");
			break;

		default:
			if (obfuscate && ((int) c == (((int) c) & 127))) {
				if (ran_num_next() % 2 == 0) {
					printf("&#%d;", (int) c);
				} else {
					printf("&#x%x;", (unsigned int) c);
				}
			} else {
				print_char(c);
			}

			break;
	}
}


void mmd_print_string_html(DString * out, const char * str, bool obfuscate) {
	if (str) {
		while (*str != '\0') {
			mmd_print_char_html(out, *str, obfuscate);
			str++;
		}
	}
}


void mmd_print_localized_char_html(DString * out, unsigned short type, scratch_pad * scratch) {
	switch (type) {
		case DASH_N:
			print_const("&#8211;");
			break;

		case DASH_M:
			print_const("&#8212;");
			break;

		case ELLIPSIS:
			print_const("&#8230;");
			break;

		case APOSTROPHE:
			print_const("&#8217;");
			break;

		case QUOTE_LEFT_SINGLE:
			switch (scratch->quotes_lang) {
				case SWEDISH:
					print( "&#8217;");
					break;

				case FRENCH:
					print_const("&#39;");
					break;

				case GERMAN:
					print_const("&#8218;");
					break;

				case GERMANGUILL:
					print_const("&#8250;");
					break;

				default:
					print_const("&#8216;");
			}

			break;

		case QUOTE_RIGHT_SINGLE:
			switch (scratch->quotes_lang) {
				case GERMAN:
					print_const("&#8216;");
					break;

				case GERMANGUILL:
					print_const("&#8249;");
					break;

				default:
					print_const("&#8217;");
			}

			break;

		case QUOTE_LEFT_DOUBLE:
			switch (scratch->quotes_lang) {
				case DUTCH:
				case GERMAN:
					print_const("&#8222;");
					break;

				case GERMANGUILL:
					print_const("&#187;");
					break;

				case FRENCH:
				case SPANISH:
					print_const("&#171;");
					break;

				case SWEDISH:
					print( "&#8221;");
					break;

				default:
					print_const("&#8220;");
			}

			break;

		case QUOTE_RIGHT_DOUBLE:
			switch (scratch->quotes_lang) {
				case GERMAN:
					print_const("&#8220;");
					break;

				case GERMANGUILL:
					print_const("&#171;");
					break;

				case FRENCH:
				case SPANISH:
					print_const("&#187;");
					break;

				case SWEDISH:
				case DUTCH:
				default:
					print_const("&#8221;");
			}

			break;
	}
}


static char * strip_dimension_units(char *original) {
	char *result;
	int i;

	result = my_strdup(original);

	for (i = 0; result[i]; i++) {
		result[i] = tolower(result[i]);
	}

	// Trim anything other than digits
	for (i = 0; result[i]; i++) {
		if (result[i] < '0' || result[i] > '9') {
			result[i] = '\0';
			return result;
		}
	}

	return result;
}


void mmd_export_link_html(DString * out, const char * source, token * text, link * link, scratch_pad * scratch) {
	attr * a = link->attributes;

	if (link->url) {
		print_const("<a href=\"");
		mmd_print_string_html(out, link->url, false);
		print_const("\"");
	} else {
		print_const("<a href=\"\"");
	}

	if (link->title && link->title[0] != '\0') {
		print_const(" title=\"");
		mmd_print_string_html(out, link->title, false);
		print_const("\"");
	}

	while (a) {
		print_const(" ");
		print(a->key);
		print_const("=\"");
		print(a->value);
		print_const("\"");
		a = a->next;
	}

	print_const(">");

	// If we're printing contents of bracket as text, then ensure we include it all
	if (text && text->child && text->child->len > 1) {
		text->child->next->start--;
		text->child->next->len++;
	}

	if (text && text->child) {
		mmd_export_token_tree_html(out, source, text->child, scratch);
	}

	print_const("</a>");
}


void mmd_export_image_html(DString * out, const char * source, token * text, link * link, scratch_pad * scratch, bool is_figure) {
	attr * a = link->attributes;
	char * width = NULL;
	char * height = NULL;

	// Compatibility mode doesn't allow figures
	if (scratch->extensions & EXT_COMPATIBILITY) {
		is_figure = false;
	}

	if (is_figure) {
		// Remove wrapping <p> markers
		d_string_erase(out, out->currentStringLength - 3, 3);
		print_const("<figure>\n");
		scratch->close_para = false;
	}

	if (link->url) {
		if (scratch->store_assets) {
			store_asset(scratch, link->url);
			asset * a = extract_asset(scratch, link->url);

			printf("<img src=\"assets/%s\"", a->asset_path);
		} else {
			if (scratch->remember_assets) {
				store_asset(scratch, link->url);
			}

			printf("<img src=\"%s\"", link->url);
		}
	} else {
		print_const("<img src=\"\"");
	}

	if (text) {
		print_const(" alt=\"");
		print_token_tree_raw(out, source, text->child);
		print_const("\"");
	}

	if (link->label && !(scratch->extensions & EXT_COMPATIBILITY)) {
		// \todo: Need to decide on approach to id's
		char * label = label_from_token(source, link->label);
		printf(" id=\"%s\"", label);
		free(label);
	}

	if (link->title && link->title[0] != '\0') {
		printf(" title=\"%s\"", link->title);
	}

	while (a) {
		if (strcmp(a->key, "width") == 0) {
			width = strip_dimension_units(a->value);

			if (strlen(width) + 2 == strlen(a->value)) {
				if (strcmp(&(a->value[strlen(width)]), "px") == 0) {
					a->value[strlen(width)] = '\0';
				}
			}

			if (strcmp(a->value, width) == 0) {
				print_const(" ");
				print(a->key);
				print_const("=\"");
				print(a->value);
				print_const("\"");
				free(width);
				width = NULL;
			} else {
				free(width);
				width = a->value;
			}
		} else if (strcmp(a->key, "height") == 0) {
			height = strip_dimension_units(a->value);

			if (strlen(height) + 2 == strlen(a->value)) {
				if (strcmp(&(a->value[strlen(height)]), "px") == 0) {
					a->value[strlen(height)] = '\0';
				}
			}

			if (strcmp(a->value, height) == 0) {
				print_const(" ");
				print(a->key);
				print_const("=\"");
				print(a->value);
				print_const("\"");
				free(height);
				height = NULL;
			} else {
				free(height);
				height = a->value;
			}
		} else {
			print_const(" ");
			print(a->key);
			print_const("=\"");
			print(a->value);
			print_const("\"");
		}

		a = a->next;
	}

	if (height || width) {
		print_const(" style=\"");

		if (height) {
			printf("height:%s;", height);
		}

		if (width) {
			printf("width:%s;", width);
		}

		print_const("\"");
	}

	print_const(" />");

	if (is_figure) {
		if (text) {
			print_const("\n<figcaption>");
			mmd_export_token_tree_html(out, source, text->child, scratch);
			print_const("</figcaption>");
		}

		print_const("\n</figure>");
	}
}


void mmd_export_toc_entry_html(DString * out, const char * source, scratch_pad * scratch, size_t * counter, short level) {
	token * entry, * next;
	short entry_level, next_level;
	char * temp_char;

	print_const("\n<ul>\n");

	// Iterate over tokens
	while (*counter < scratch->header_stack->size) {
		// Get token for header
		entry = stack_peek_index(scratch->header_stack, *counter);
		entry_level = raw_level_for_header(entry);

		if (entry_level >= level) {
			// This entry is a direct descendant of the parent
			temp_char = label_from_header(source, entry);
			printf("<li><a href=\"#%s\">", temp_char);
			mmd_export_token_tree_html(out, source, entry->child, scratch);
			print_const("</a>");

			if (*counter < scratch->header_stack->size - 1) {
				next = stack_peek_index(scratch->header_stack, *counter + 1);
				next_level = next->type - BLOCK_H1 + 1;

				if (next_level > entry_level) {
					// This entry has children
					(*counter)++;
					mmd_export_toc_entry_html(out, source, scratch, counter, entry_level + 1);
				}
			}

			print_const("</li>\n");
			free(temp_char);
		} else if (entry_level < level ) {
			// If entry < level, exit this level
			// Decrement counter first, so that we can test it again later
			(*counter)--;
			break;
		}

		// Increment counter
		(*counter)++;
	}

	print_const("</ul>\n");
}


void mmd_export_toc_html(DString * out, const char * source, scratch_pad * scratch) {
	size_t counter = 0;

	mmd_export_toc_entry_html(out, source, scratch, &counter, 0);
}


void mmd_export_token_html(DString * out, const char * source, token * t, scratch_pad * scratch) {
	if (t == NULL) {
		return;
	}

	short	temp_short;
	short	temp_short2;
	short	temp_short3;
	link *	temp_link	= NULL;
	char *	temp_char	= NULL;
	char *	temp_char2	= NULL;
	bool	temp_bool	= 0;
	token *	temp_token	= NULL;
	footnote * temp_note = NULL;

	switch (t->type) {
		case AMPERSAND:
		case AMPERSAND_LONG:
			print_const("&amp;");
			break;

		case ANGLE_LEFT:
			print_const("&lt;");
			break;

		case ANGLE_RIGHT:
			print_const("&gt;");
			break;

		case APOSTROPHE:
			if (!(scratch->extensions & EXT_SMART)) {
				print_token(t);
			} else {
				print_localized(APOSTROPHE);
			}

			break;

		case BACKTICK:
			if (t->mate == NULL) {
				print_token(t);
			} else if (t->mate->type == QUOTE_RIGHT_ALT)
				if (!(scratch->extensions & EXT_SMART)) {
					print_token(t);
				} else {
					print_localized(QUOTE_LEFT_DOUBLE);
				} else if (t->start < t->mate->start) {
				print_const("<code>");
			} else {
				print_const("</code>");
			}

			break;

		case BLOCK_BLOCKQUOTE:
			pad(out, 2, scratch);
			print_const("<blockquote>\n");
			scratch->padded = 2;
			mmd_export_token_tree_html(out, source, t->child, scratch);
			pad(out, 1, scratch);
			print_const("</blockquote>");
			scratch->padded = 0;
			break;

		case BLOCK_DEFINITION:
			pad(out, 2, scratch);
			print_const("<dd>");

			temp_short = scratch->list_is_tight;

			if (t->child) {
				if (!(t->child->next && (t->child->next->type == BLOCK_EMPTY) && t->child->next->next)) {
					scratch->list_is_tight = true;
				}

				mmd_export_token_tree_html(out, source, t->child, scratch);
			}

			print_const("</dd>");
			scratch->padded = 0;

			scratch->list_is_tight = temp_short;
			break;

		case BLOCK_DEFLIST:
			pad(out, 2, scratch);

			// Group consecutive definition lists into a single list.
			// lemon's LALR(1) parser can't properly handle this (to my understanding).

			if (!(t->prev && (t->prev->type == BLOCK_DEFLIST))) {
				print_const("<dl>\n");
			}

			scratch->padded = 2;

			mmd_export_token_tree_html(out, source, t->child, scratch);
			pad(out, 1, scratch);

			if (!(t->next && (t->next->type == BLOCK_DEFLIST))) {
				print_const("</dl>\n");
			}

			scratch->padded = 1;
			break;

		case BLOCK_CODE_FENCED:
			pad(out, 2, scratch);

			temp_char = get_fence_language_specifier(t->child->child, source);

			if (temp_char) {
				if (strncmp("{=", temp_char, 2) == 0) {
					// Raw source
					if (raw_filter_text_matches(temp_char, FORMAT_HTML)) {
						switch (t->child->tail->type) {
							case LINE_FENCE_BACKTICK_3:
							case LINE_FENCE_BACKTICK_4:
							case LINE_FENCE_BACKTICK_5:
								temp_token = t->child->tail;
								break;

							default:
								temp_token = NULL;
						}

						if (temp_token) {
							d_string_append_c_array(out, &source[t->child->next->start], temp_token->start - t->child->next->start);
							scratch->padded = 1;
						} else {
							d_string_append_c_array(out, &source[t->child->start + t->child->len], t->start + t->len - t->child->next->start);
							scratch->padded = 0;
						}
					}

					free(temp_char);
					break;
				}

				print_const("<pre><code");
				printf(" class=\"%s\"", temp_char);
				free(temp_char);
			} else {
				print_const("<pre><code");
			}

			print_const(">");
			mmd_export_token_tree_html_raw(out, source, t->child->next, scratch);
			print_const("</code></pre>");
			scratch->padded = 0;
			break;

		case BLOCK_CODE_INDENTED:
			pad(out, 2, scratch);
			print_const("<pre><code>");
			mmd_export_token_tree_html_raw(out, source, t->child, scratch);
			print_const("</code></pre>");
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
				temp_char = label_from_header(source, t);
				printf("<h%1d id=\"%s\">", temp_short + scratch->base_header_level - 1, temp_char);
				free(temp_char);
			}

			mmd_export_token_tree_html(out, source, t->child, scratch);
			trim_trailing_whitespace_d_string(out);

			printf("</h%1d>", temp_short + scratch->base_header_level - 1);
			scratch->padded = 0;
			break;

		case BLOCK_HR:
			pad(out, 2, scratch);
			print_const("<hr />");
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
			print_const("<ul>");
			scratch->padded = 0;
			mmd_export_token_tree_html(out, source, t->child, scratch);
			pad(out, 1, scratch);
			print_const("</ul>");
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
			print_const("<ol>");
			scratch->padded = 0;
			mmd_export_token_tree_html(out, source, t->child, scratch);
			pad(out, 1, scratch);
			print_const("</ol>");
			scratch->padded = 0;
			scratch->list_is_tight = temp_short;
			break;

		case BLOCK_LIST_ITEM:
			pad(out, 1, scratch);
			print_const("<li>");
			scratch->padded = 2;
			mmd_export_token_tree_html(out, source, t->child, scratch);
			print_const("</li>");
			scratch->padded = 0;
			break;

		case BLOCK_LIST_ITEM_TIGHT:
			pad(out, 1, scratch);
			print_const("<li>");

			if (!scratch->list_is_tight) {
				print_const("<p>");
			}

			scratch->padded = 2;
			mmd_export_token_tree_html(out, source, t->child, scratch);

			if (scratch->close_para) {
				if (!scratch->list_is_tight) {
					print_const("</p>");
				}
			} else {
				scratch->close_para = true;
			}

			print_const("</li>");
			scratch->padded = 0;
			break;

		case BLOCK_META:
			break;

		case BLOCK_PARA:
		case BLOCK_DEF_CITATION:
		case BLOCK_DEF_FOOTNOTE:
		case BLOCK_DEF_GLOSSARY:
			pad(out, 2, scratch);

			if (!scratch->list_is_tight) {
				print_const("<p>");
			}

			mmd_export_token_tree_html(out, source, t->child, scratch);

			if (scratch->citation_being_printed) {
				scratch->footnote_para_counter--;

				if (scratch->footnote_para_counter == 0) {
					printf(" <a href=\"#cnref:%d\" title=\"%s\" class=\"reversecitation\">&#160;&#8617;</a>", scratch->citation_being_printed, LC("return to body"));
				}
			}

			if (scratch->footnote_being_printed) {
				scratch->footnote_para_counter--;

				if (scratch->footnote_para_counter == 0) {
					temp_short = scratch->footnote_being_printed;

					if (scratch->extensions & EXT_RANDOM_FOOT) {
						srand(scratch->random_seed_base + temp_short);
						temp_short = rand() % 32000 + 1;
					}

					printf(" <a href=\"#fnref:%d\" title=\"%s\" class=\"reversefootnote\">&#160;&#8617;</a>", temp_short, LC("return to body"));
				}
			}

			if (scratch->glossary_being_printed) {
				scratch->footnote_para_counter--;

				if (scratch->footnote_para_counter == 0) {
					printf(" <a href=\"#gnref:%d\" title=\"%s\" class=\"reverseglossary\">&#160;&#8617;</a>", scratch->glossary_being_printed, LC("return to body"));
				}
			}

			if (scratch->close_para) {
				if (!scratch->list_is_tight) {
					print_const("</p>");
				}
			} else {
				scratch->close_para = true;
			}

			scratch->padded = 0;
			break;

		case BLOCK_SETEXT_1:
			pad(out, 2, scratch);
			temp_short = 1;

			if (scratch->extensions & EXT_NO_LABELS) {
				printf("<h%1d>", temp_short + scratch->base_header_level - 1);
			} else {
				temp_token = manual_label_from_header(t, source);

				if (temp_token) {
					temp_char = label_from_token(source, temp_token);
				} else {
					temp_char = label_from_token(source, t);
				}

				printf("<h%1d id=\"%s\">", temp_short + scratch->base_header_level - 1, temp_char);
				free(temp_char);
			}

			mmd_export_token_tree_html(out, source, t->child, scratch);
			printf("</h%1d>", temp_short + scratch->base_header_level - 1);
			scratch->padded = 0;
			break;

		case BLOCK_SETEXT_2:
			pad(out, 2, scratch);
			temp_short = 2;

			if (scratch->extensions & EXT_NO_LABELS) {
				printf("<h%1d>", temp_short + scratch->base_header_level - 1);
			} else {
				temp_token = manual_label_from_header(t, source);

				if (temp_token) {
					temp_char = label_from_token(source, temp_token);
				} else {
					temp_char = label_from_token(source, t);
				}

				printf("<h%1d id=\"%s\">", temp_short + scratch->base_header_level - 1, temp_char);
				free(temp_char);
			}

			mmd_export_token_tree_html(out, source, t->child, scratch);
			printf("</h%1d>", temp_short + scratch->base_header_level - 1);
			scratch->padded = 0;
			break;

		case BLOCK_TABLE:
			pad(out, 2, scratch);
			print_const("<table>\n");

			// Are we followed by a caption?
			if (table_has_caption(t)) {
				temp_token = t->next->child;

				if (temp_token->next &&
						temp_token->next->type == PAIR_BRACKET) {
					temp_token = temp_token->next;
				}

				temp_char = label_from_token(source, temp_token);
				printf("<caption style=\"caption-side: bottom;\" id=\"%s\">", temp_char);
				free(temp_char);

				t->next->child->child->type = TEXT_EMPTY;
				t->next->child->child->mate->type = TEXT_EMPTY;
				mmd_export_token_tree_html(out, source, t->next->child->child, scratch);
				print_const("</caption>\n");
				temp_short = 1;
			} else {
				temp_short = 0;
			}

			scratch->padded = 2;
			read_table_column_alignments(source, t, scratch);

			print_const("<colgroup>\n");

			for (int i = 0; i < scratch->table_column_count; ++i) {
				switch (scratch->table_alignment[i]) {
					case 'l':
						print_const("<col style=\"text-align:left;\"/>\n");
						break;

					case 'N':
					case 'L':
						print_const("<col style=\"text-align:left;\" class=\"extended\"/>\n");
						break;

					case 'r':
						print_const("<col style=\"text-align:right;\"/>\n");
						break;

					case 'R':
						print_const("<col style=\"text-align:right;\" class=\"extended\"/>\n");
						break;

					case 'c':
						print_const("<col style=\"text-align:center;\"/>\n");
						break;

					case 'C':
						print_const("<col style=\"text-align:center;\" class=\"extended\"/>\n");
						break;

					default:
						print_const("<col />\n");
						break;
				}
			}

			print_const("</colgroup>\n");
			scratch->padded = 1;

			mmd_export_token_tree_html(out, source, t->child, scratch);
			pad(out, 1, scratch);
			print_const("</table>");
			scratch->padded = 0;

			scratch->skip_token = temp_short;

			break;

		case BLOCK_TABLE_HEADER:
			pad(out, 2, scratch);
			print_const("<thead>\n");
			scratch->in_table_header = 1;
			mmd_export_token_tree_html(out, source, t->child, scratch);
			scratch->in_table_header = 0;
			print_const("</thead>\n");
			scratch->padded = 1;
			break;

		case BLOCK_TABLE_SECTION:
			pad(out, 2, scratch);
			print_const("<tbody>\n");
			scratch->padded = 2;
			mmd_export_token_tree_html(out, source, t->child, scratch);
			print_const("</tbody>");
			scratch->padded = 0;
			break;

		case BLOCK_TERM:
			pad(out, 2, scratch);
			print_const("<dt>");
			mmd_export_token_tree_html(out, source, t->child, scratch);
			print_const("</dt>\n");
			scratch->padded = 2;
			break;

		case BLOCK_TOC:

			// EPUB uses a separate TOC
			if (scratch->output_format == FORMAT_EPUB) {
				break;
			}

			pad(out, 2, scratch);
			print_const("<div class=\"TOC\">\n");

			mmd_export_toc_html(out, source, scratch);
			print_const("</div>");
			scratch->padded = 0;
			break;

		case BRACE_DOUBLE_LEFT:
			print_const("{{");
			break;

		case BRACE_DOUBLE_RIGHT:
			print_const("}}");
			break;

		case BRACKET_LEFT:
			print_const("[");
			break;

		case BRACKET_ABBREVIATION_LEFT:
			print_const("[>");
			break;

		case BRACKET_CITATION_LEFT:
			print_const("[#");
			break;

		case BRACKET_FOOTNOTE_LEFT:
			print_const("[^");
			break;

		case BRACKET_GLOSSARY_LEFT:
			print_const("[?");
			break;

		case BRACKET_IMAGE_LEFT:
			print_const("![");
			break;

		case BRACKET_VARIABLE_LEFT:
			print_const("[\%");
			break;

		case BRACKET_RIGHT:
			print_const("]");
			break;

		case COLON:
			print_const(":");
			break;

		case CRITIC_ADD_OPEN:
			print_const("{++");
			break;

		case CRITIC_ADD_CLOSE:
			print_const("++}");
			break;

		case CRITIC_COM_OPEN:
			print_const("{&gt;&gt;");
			break;

		case CRITIC_COM_CLOSE:
			print_const("&lt;&lt;}");
			break;

		case CRITIC_DEL_OPEN:
			print_const("{--");
			break;

		case CRITIC_DEL_CLOSE:
			print_const("--}");
			break;

		case CRITIC_HI_OPEN:
			print_const("{==");
			break;

		case CRITIC_HI_CLOSE:
			print_const("==}");
			break;

		case CRITIC_SUB_OPEN:
			print_const("{~~");
			break;

		case CRITIC_SUB_DIV:
			print_const("~&gt;");
			break;

		case CRITIC_SUB_CLOSE:
			print_const("~~}");
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
			mmd_export_token_tree_html(out, source, t->child, scratch);
			break;

		case ELLIPSIS:
			if (!(scratch->extensions & EXT_SMART)) {
				print_token(t);
			} else {
				print_localized(ELLIPSIS);
			}

			break;

		case EMPH_START:
			print_const("<em>");
			break;

		case EMPH_STOP:
			print_const("</em>");
			break;

		case EQUAL:
			print_const("=");
			break;

		case ESCAPED_CHARACTER:
			if (!(scratch->extensions & EXT_COMPATIBILITY) &&
					(source[t->start + 1] == ' ')) {
				print_const("&nbsp;");
			} else {
				mmd_print_char_html(out, source[t->start + 1], false);
			}

			break;

		case HASH1:
		case HASH2:
		case HASH3:
		case HASH4:
		case HASH5:
		case HASH6:
			print_token(t);
			break;

		case HTML_ENTITY:
			print_token(t);
			break;

		case HTML_COMMENT_START:
			if (!(scratch->extensions & EXT_SMART)) {
				print_const("&lt;!--");
			} else {
				print_const("&lt;!");
				print_localized(DASH_N);
			}

			break;

		case HTML_COMMENT_STOP:
			if (!(scratch->extensions & EXT_SMART)) {
				print_const("--&gt;");
			} else {
				print_localized(DASH_N);
				print_const("&gt;");
			}

			break;

		case INDENT_SPACE:
			print_char(' ');
			break;

		case INDENT_TAB:
			print_char('\t');
			break;

		case LINE_LIST_BULLETED:
		case LINE_LIST_ENUMERATED:
			mmd_export_token_tree_html(out, source, t->child, scratch);
			break;

		case LINE_SETEXT_2:
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
				print_const("<span class=\"math\">\\[");
			} else {
				print_const("\\[");
			}

			break;

		case MATH_BRACKET_CLOSE:
			if (t->mate) {
				print_const("\\]</span>");
			} else {
				print_const("\\]");
			}

			break;

		case MATH_DOLLAR_SINGLE:
			if (t->mate) {
				(t->start < t->mate->start) ? ( print_const("<span class=\"math\">\\(") ) : ( print_const("\\)</span>") );
			} else {
				print_const("$");
			}

			break;

		case MATH_DOLLAR_DOUBLE:
			if (t->mate) {
				(t->start < t->mate->start) ? ( print_const("<span class=\"math\">\\[") ) : ( print_const("\\]</span>") );
			} else {
				print_const("$$");
			}

			break;

		case MATH_PAREN_OPEN:
			if (t->mate) {
				print_const("<span class=\"math\">\\(");
			} else {
				print_const("\\(");
			}

			break;

		case MATH_PAREN_CLOSE:
			if (t->mate) {
				print_const("\\)</span>");
			} else {
				print_const("\\)");
			}

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

			if (t->next && t->next->type == PAIR_RAW_FILTER) {
				// Raw text?
				if (raw_filter_matches(t->next, source, FORMAT_HTML)) {
					d_string_append_c_array(out, &(source[t->child->start + t->child->len]), t->child->mate->start - t->child->start - t->child->len);
				}

				// Skip over PAIR_RAW_FILTER
				scratch->skip_token = 1;
				break;
			}

			print_const("<code>");
			mmd_export_token_tree_html_raw(out, source, t->child, scratch);
			print_const("</code>");
			break;

		case PAIR_ANGLE:
			temp_char = url_accept(source, t->start + 1, t->len - 2, NULL, true);

			if (temp_char) {
				print_const("<a href=\"");

				if (scan_email(temp_char)) {
					temp_bool = true;

					if (strncmp("mailto:", temp_char, 7) != 0) {
						mmd_print_string_html(out, "mailto:", true);
					}
				} else {
					temp_bool = false;
				}

				mmd_print_string_html(out, temp_char, temp_bool);
				print_const("\">");
				mmd_print_string_html(out, temp_char, temp_bool);
				print_const("</a>");
			} else if (scan_html(&source[t->start])) {
				print_token(t);
			} else {
				mmd_export_token_tree_html(out, source, t->child, scratch);
			}

			free(temp_char);
			break;

		case PAIR_BRACE:
		case PAIR_BRACES:
		case PAIR_RAW_FILTER:
			mmd_export_token_tree_html(out, source, t->child, scratch);
			break;

		case PAIR_BRACKET:
			if ((scratch->extensions & EXT_NOTES) &&
					(t->next && t->next->type == PAIR_BRACKET_CITATION)) {
				goto parse_citation;
			}

		case PAIR_BRACKET_IMAGE:
			parse_brackets(source, scratch, t, &temp_link, &temp_short, &temp_bool);

			if (temp_link) {
				if (t->type == PAIR_BRACKET) {
					// Link
					mmd_export_link_html(out, source, t, temp_link, scratch);
				} else {
					// Image -- should it be a figure (e.g. image is only thing in paragraph)?
					temp_token = t->next;

					if (temp_token &&
							((temp_token->type == PAIR_BRACKET) ||
							 (temp_token->type == PAIR_PAREN))) {
						temp_token = temp_token->next;
					}

					if (temp_token && temp_token->type == TEXT_NL) {
						temp_token = temp_token->next;
					}

					if (temp_token && temp_token->type == TEXT_LINEBREAK) {
						temp_token = temp_token->next;
					}

					if (t->prev || temp_token) {
						mmd_export_image_html(out, source, t, temp_link, scratch, false);
					} else {
						mmd_export_image_html(out, source, t, temp_link, scratch, true);
					}
				}

				if (temp_bool) {
					link_free(temp_link);
				}

				scratch->skip_token = temp_short;

				return;
			}

			// No links exist, so treat as normal
			mmd_export_token_tree_html(out, source, t->child, scratch);
			break;

		case PAIR_BRACKET_ABBREVIATION:

			// Which might also be an "auto-tagged" abbreviation
			if (scratch->extensions & EXT_NOTES) {
				// Note-based syntax enabled

				// Classify this use
				temp_short2 = scratch->used_abbreviations->size;
				temp_short3 = scratch->inline_abbreviations_to_free->size;
				abbreviation_from_bracket(source, scratch, t, &temp_short);

				if (temp_short == -1) {
					// This instance is not properly formed
					print_const("[>");
					mmd_export_token_tree_html(out, source, t->child->next, scratch);
					print_const("]");
					break;
				}

				// Get instance of the note used
				temp_note = stack_peek_index(scratch->used_abbreviations, temp_short - 1);

				if (t->child) {
					t->child->type = TEXT_EMPTY;
					t->child->mate->type = TEXT_EMPTY;
				}

				if (temp_short3 == scratch->inline_abbreviations_to_free->size) {
					// This is a reference definition

					if (temp_short2 == scratch->used_abbreviations->size) {
						// This is a re-use of a previously used note
						print_const("<abbr title=\"");
						mmd_print_string_html(out, temp_note->clean_text, false);
						print_const("\">");

						if (t->child) {
							mmd_export_token_tree_html(out, source, t->child, scratch);
						} else {
							print_token(t);
						}

						print_const("</abbr>");
					} else {
						// This is the first time this note was used
						mmd_print_string_html(out, temp_note->clean_text, false);
						print_const(" (<abbr title=\"");
						mmd_print_string_html(out, temp_note->clean_text, false);
						print_const("\">");

						if (t->child) {
							mmd_export_token_tree_html(out, source, t->child, scratch);
						} else {
							print_token(t);
						}

						print_const("</abbr>)");
					}
				} else {
					// This is an inline definition (and therefore the first use)
					mmd_print_string_html(out, temp_note->clean_text, false);
					print_const(" (<abbr title=\"");
					mmd_print_string_html(out, temp_note->clean_text, false);
					print_const("\">");
					mmd_print_string_html(out, temp_note->label_text, false);
					print_const("</abbr>)");
				}
			} else {
				// Note-based syntax disabled
				mmd_export_token_tree_html(out, source, t->child, scratch);
			}

			break;

		case PAIR_BRACKET_CITATION:
parse_citation:
			temp_bool = true;		// Track whether this is regular vs 'not cited'
			temp_token = t;			// Remember whether we need to skip ahead

			if (scratch->extensions & EXT_NOTES) {
				// Note-based syntax enabled

				if (t->type == PAIR_BRACKET) {
					// This is a locator for a subsequent citation (e.g. `[foo][#bar]`)
					temp_char = text_inside_pair(source, t);
					temp_char2 = label_from_string(temp_char);

					if (strcmp(temp_char2, "notcited") == 0) {
						free(temp_char);
						temp_char = my_strdup("");
						temp_bool = false;
					}

					free(temp_char2);

					// Move ahead to actual citation
					t = t->next;
				} else {
					// This is the actual citation (e.g. `[#foo]`)
					// No locator
					temp_char = my_strdup("");
				}

				// Classify this use
				temp_short2 = scratch->used_citations->size;
				citation_from_bracket(source, scratch, t, &temp_short);

				if (temp_short == -1) {
					// This instance is not properly formed
					print_const("[#");
					mmd_export_token_tree_html(out, source, t->child->next, scratch);
					print_const("]");

					free(temp_char);
					break;
				}

				if (temp_bool) {
					// This is a regular citation

					if (temp_char[0] == '\0') {
						// No locator

						if (temp_short2 == scratch->used_citations->size) {
							// This is a re-use of a previously used note
							printf("<a href=\"#cn:%d\" title=\"%s\" class=\"citation\">(%d)</a>",
								   temp_short, LC("see citation"), temp_short);
						} else {
							// This is the first time this note was used
							printf("<a href=\"#cn:%d\" id=\"cnref:%d\" title=\"%s\" class=\"citation\">(%d)</a>",
								   temp_short, temp_short, LC("see citation"), temp_short);
						}
					} else {
						// Locator present

						if (temp_short2 == scratch->used_citations->size) {
							// This is a re-use of a previously used note
							printf("<a href=\"#cn:%d\" title=\"%s\" class=\"citation\">(%s, %d)</a>",
								   temp_short, LC("see citation"), temp_char, temp_short);
						} else {
							// This is the first time this note was used
							printf("<a href=\"#cn:%d\" id=\"cnref:%d\" title=\"%s\" class=\"citation\">(%s, %d)</a>",
								   temp_short, temp_short, LC("see citation"), temp_char, temp_short);
						}
					}
				} else {
					// This is a "nocite"
				}

				if (temp_token != t) {
					// Skip citation on next pass
					scratch->skip_token = 1;
				}

				free(temp_char);
			} else {
				// Note-based syntax disabled
				mmd_export_token_tree_html(out, source, t->child, scratch);
			}

			break;

		case PAIR_BRACKET_FOOTNOTE:
			if (scratch->extensions & EXT_NOTES) {
				// Note-based syntax enabled

				// Classify this use
				temp_short2 = scratch->used_footnotes->size;
				footnote_from_bracket(source, scratch, t, &temp_short);

				if (temp_short == -1) {
					// This instance is not properly formed
					print_const("[^");
					mmd_export_token_tree_html(out, source, t->child->next, scratch);
					print_const("]");
					break;
				}

				if (temp_short2 == scratch->used_footnotes->size) {
					// This is a re-use of a previously used note

					if (scratch->extensions & EXT_RANDOM_FOOT) {
						srand(scratch->random_seed_base + temp_short);
						temp_short3 = rand() % 32000 + 1;
					} else {
						temp_short3 = temp_short;
					}

					printf("<a href=\"#fn:%d\" title=\"%s\" class=\"footnote\"><sup>%d</sup></a>",
						   temp_short3, LC("see footnote"), temp_short);
				} else {
					// This is the first time this note was used

					if (scratch->extensions & EXT_RANDOM_FOOT) {
						srand(scratch->random_seed_base + temp_short);
						temp_short3 = rand() % 32000 + 1;
					} else {
						temp_short3 = temp_short;
					}

					printf("<a href=\"#fn:%d\" id=\"fnref:%d\" title=\"%s\" class=\"footnote\"><sup>%d</sup></a>",
						   temp_short3, temp_short3, LC("see footnote"), temp_short);
				}
			} else {
				// Note-based syntax disabled
				mmd_export_token_tree_html(out, source, t->child, scratch);
			}

			break;

		case PAIR_BRACKET_GLOSSARY:

			// Which might also be an "auto-tagged" glossary
			if (scratch->extensions & EXT_NOTES) {
				// Note-based syntax enabled

				// Classify this use
				temp_short2 = scratch->used_glossaries->size;
				glossary_from_bracket(source, scratch, t, &temp_short);

				if (temp_short == -1) {
					// This instance is not properly formed
					print_const("[?");

					if (t->child) {
						mmd_export_token_tree_html(out, source, t->child->next, scratch);
					} else {
						print_token(t);
					}

					print_const("]");
					break;
				}

				// Get instance of the note used
				temp_note = stack_peek_index(scratch->used_glossaries, temp_short - 1);

				if (temp_short2 == scratch->used_glossaries->size) {
					// This is a re-use of a previously used note

					printf("<a href=\"#gn:%d\" title=\"%s\" class=\"glossary\">",
						   temp_short, LC("see glossary"));
					mmd_print_string_html(out, temp_note->clean_text, false);
					print_const("</a>");
				} else {
					// This is the first time this note was used


					printf("<a href=\"#gn:%d\" id=\"gnref:%d\" title=\"%s\" class=\"glossary\">",
						   temp_short, temp_short, LC("see glossary"));
					mmd_print_string_html(out, temp_note->clean_text, false);
					print_const("</a>");
				}
			} else {
				// Note-based syntax disabled
				mmd_export_token_tree_html(out, source, t->child, scratch);
			}

			break;

		case PAIR_BRACKET_VARIABLE:
			temp_char = text_inside_pair(source, t);
			temp_char2 = extract_metadata(scratch, temp_char);

			if (temp_char2) {
				mmd_print_string_html(out, temp_char2, false);
			} else {
				mmd_export_token_tree_html(out, source, t->child, scratch);
			}

			// Don't free temp_char2 (it belongs to meta *)
			free(temp_char);
			break;

		case PAIR_CRITIC_ADD:

			// Ignore if we're rejecting
			if (scratch->extensions & EXT_CRITIC_REJECT) {
				break;
			}

			if (scratch->extensions & EXT_CRITIC) {
				t->child->type = TEXT_EMPTY;
				t->child->mate->type = TEXT_EMPTY;

				if (scratch->extensions & EXT_CRITIC_ACCEPT) {
					mmd_export_token_tree_html(out, source, t->child, scratch);
				} else {
					print_const("<ins>");
					mmd_export_token_tree_html(out, source, t->child, scratch);
					print_const("</ins>");
				}
			} else {
				mmd_export_token_tree_html(out, source, t->child, scratch);
			}

			break;

		case PAIR_CRITIC_DEL:

			// Ignore if we're accepting
			if (scratch->extensions & EXT_CRITIC_ACCEPT) {
				break;
			}

			if (scratch->extensions & EXT_CRITIC) {
				t->child->type = TEXT_EMPTY;
				t->child->mate->type = TEXT_EMPTY;

				if (scratch->extensions & EXT_CRITIC_REJECT) {
					mmd_export_token_tree_html(out, source, t->child, scratch);
				} else {
					print_const("<del>");
					mmd_export_token_tree_html(out, source, t->child, scratch);
					print_const("</del>");
				}
			} else {
				mmd_export_token_tree_html(out, source, t->child, scratch);
			}

			break;

		case PAIR_CRITIC_COM:

			// Ignore if we're rejecting or accepting
			if ((scratch->extensions & EXT_CRITIC_REJECT) ||
					(scratch->extensions & EXT_CRITIC_ACCEPT)) {
				break;
			}

			if (scratch->extensions & EXT_CRITIC) {
				t->child->type = TEXT_EMPTY;
				t->child->mate->type = TEXT_EMPTY;
				print_const("<span class=\"critic comment\">");
				mmd_export_token_tree_html(out, source, t->child, scratch);
				print_const("</span>");
			} else {
				mmd_export_token_tree_html(out, source, t->child, scratch);
			}

			break;

		case PAIR_CRITIC_HI:

			// Ignore if we're rejecting or accepting
			if ((scratch->extensions & EXT_CRITIC_REJECT) ||
					(scratch->extensions & EXT_CRITIC_ACCEPT)) {
				t->child->type = TEXT_EMPTY;
				t->child->mate->type = TEXT_EMPTY;
				mmd_export_token_tree_html(out, source, t->child, scratch);
				break;
			}

			if (scratch->extensions & EXT_CRITIC) {
				t->child->type = TEXT_EMPTY;
				t->child->mate->type = TEXT_EMPTY;
				print_const("<mark>");
				mmd_export_token_tree_html(out, source, t->child, scratch);
				print_const("</mark>");
			} else {
				mmd_export_token_tree_html(out, source, t->child, scratch);
			}

			break;

		case CRITIC_SUB_DIV_A:
			print_const("~");
			break;

		case CRITIC_SUB_DIV_B:
			print_const("&gt;");
			break;

		case PAIR_CRITIC_SUB_DEL:
			if ((scratch->extensions & EXT_CRITIC) &&
					(t->next) &&
					(t->next->type == PAIR_CRITIC_SUB_ADD)) {
				t->child->type = TEXT_EMPTY;
				t->child->mate->type = TEXT_EMPTY;

				if (scratch->extensions & EXT_CRITIC_ACCEPT) {

				} else if (scratch->extensions & EXT_CRITIC_REJECT) {
					mmd_export_token_tree_html(out, source, t->child, scratch);
				} else {
					print_const("<del>");
					mmd_export_token_tree_html(out, source, t->child, scratch);
					print_const("</del>");
				}
			} else {
				mmd_export_token_tree_html(out, source, t->child, scratch);
			}

			break;

		case PAIR_CRITIC_SUB_ADD:
			if ((scratch->extensions & EXT_CRITIC) &&
					(t->prev) &&
					(t->prev->type == PAIR_CRITIC_SUB_DEL)) {
				t->child->type = TEXT_EMPTY;
				t->child->mate->type = TEXT_EMPTY;

				if (scratch->extensions & EXT_CRITIC_REJECT) {

				} else if (scratch->extensions & EXT_CRITIC_ACCEPT) {
					mmd_export_token_tree_html(out, source, t->child, scratch);
				} else {
					print_const("<ins>");
					mmd_export_token_tree_html(out, source, t->child, scratch);
					print_const("</ins>");
				}
			} else {
				mmd_export_token_tree_html(out, source, t->child, scratch);
			}

			break;

		case PAIR_HTML_COMMENT:
			print_token(t);
			break;

		case PAIR_MATH:
			print_const("<span class=\"math\">");
			mmd_export_token_tree_html_math(out, source, t->child, scratch);
			print_const("</span>");
			break;

		case PAIR_EMPH:
		case PAIR_PAREN:
		case PAIR_QUOTE_DOUBLE:
		case PAIR_QUOTE_SINGLE:
		case PAIR_STAR:
		case PAIR_STRONG:
		case PAIR_SUBSCRIPT:
		case PAIR_SUPERSCRIPT:
		case PAIR_UL:
			mmd_export_token_tree_html(out, source, t->child, scratch);
			break;

		case PAREN_LEFT:
			print_char('(');
			break;

		case PAREN_RIGHT:
			print_char(')');
			break;

		case PIPE:
			print_token(t);
			break;

		case PLUS:
			print_token(t);
			break;

		case QUOTE_SINGLE:
			if ((t->mate == NULL) || (!(scratch->extensions & EXT_SMART))) {
				print_const("'");
			} else {
				(t->start < t->mate->start) ? ( print_localized(QUOTE_LEFT_SINGLE) ) : ( print_localized(QUOTE_RIGHT_SINGLE) );
			}

			break;

		case QUOTE_DOUBLE:
			if ((t->mate == NULL) || (!(scratch->extensions & EXT_SMART))) {
				print_const("&quot;");
			} else {
				(t->start < t->mate->start) ? ( print_localized(QUOTE_LEFT_DOUBLE) ) : ( print_localized(QUOTE_RIGHT_DOUBLE) );
			}

			break;

		case QUOTE_RIGHT_ALT:
			if ((t->mate == NULL) || (!(scratch->extensions & EXT_SMART))) {
				print_const("''");
			} else {
				print_localized(QUOTE_RIGHT_DOUBLE);
			}

			break;

		case SLASH:
		case STAR:
			print_token(t);
			break;

		case STRONG_START:
			print_const("<strong>");
			break;

		case STRONG_STOP:
			print_const("</strong>");
			break;

		case SUBSCRIPT:
			if (t->mate) {
				(t->start < t->mate->start) ? (print_const("<sub>")) : (print_const("</sub>"));
			} else if (t->len != 1) {
				print_const("<sub>");
				mmd_export_token_html(out, source, t->child, scratch);
				print_const("</sub>");
			} else {
				print_const("~");
			}

			break;

		case SUPERSCRIPT:
			if (t->mate) {
				(t->start < t->mate->start) ? (print_const("<sup>")) : (print_const("</sup>"));
			} else if (t->len != 1) {
				print_const("<sup>");
				mmd_export_token_html(out, source, t->child, scratch);
				print_const("</sup>");
			} else {
				print_const("^");
			}

			break;

		case TABLE_CELL:
			if (scratch->in_table_header) {
				print_const("\t<th");
			} else {
				print_const("\t<td");
			}

			switch (scratch->table_alignment[scratch->table_cell_count]) {
				case 'l':
				case 'L':
					print_const(" style=\"text-align:left;\"");
					break;

				case 'r':
				case 'R':
					print_const(" style=\"text-align:right;\"");
					break;

				case 'c':
				case 'C':
					print_const(" style=\"text-align:center;\"");
					break;
			}

			if (t->next && t->next->type == TABLE_DIVIDER) {
				if (t->next->len > 1) {
					printf(" colspan=\"%d\"", t->next->len);
				}
			}

			print_const(">");
			mmd_export_token_tree_html(out, source, t->child, scratch);

			if (scratch->in_table_header) {
				print_const("</th>\n");
			} else {
				print_const("</td>\n");
			}

			if (t->next) {
				scratch->table_cell_count += t->next->len;
			} else {
				scratch->table_cell_count++;
			}

			break;

		case TABLE_DIVIDER:
			break;

		case TABLE_ROW:
			print_const("<tr>\n");
			scratch->table_cell_count = 0;
			mmd_export_token_tree_html(out, source, t->child, scratch);
			print_const("</tr>\n");
			break;

		case TEXT_LINEBREAK:
			if (t->next) {
				print_const("<br />\n");
				scratch->padded = 1;
			}

			break;

		case CODE_FENCE:
		case TEXT_EMPTY:
		case MANUAL_LABEL:
			break;

		case TEXT_NL:
			if (t->next) {
				print_char('\n');
			}

			break;

		case RAW_FILTER_LEFT:
		case TEXT_BACKSLASH:
		case TEXT_BRACE_LEFT:
		case TEXT_BRACE_RIGHT:
		case TEXT_HASH:
		case TEXT_NUMBER_POSS_LIST:
		case TEXT_PERCENT:
		case TEXT_PERIOD:
		case TEXT_PLAIN:
		case TOC:
			print_token(t);
			break;

		case UL:
			print_token(t);
			break;

		default:
			fprintf(stderr, "Unknown token type: %d (%lu:%lu)\n", t->type, t->start, t->len);
			token_describe(t, source);
			break;
	}
}


void mmd_export_token_tree_html(DString * out, const char * source, token * t, scratch_pad * scratch) {

	// Prevent stack overflow with "dangerous" input causing extreme recursion
	if (scratch->recurse_depth == kMaxExportRecursiveDepth) {
		return;
	}

	scratch->recurse_depth++;

	while (t != NULL) {
		if (scratch->skip_token) {
			scratch->skip_token--;
		} else {
			mmd_export_token_html(out, source, t, scratch);
		}

		t = t->next;
	}

	scratch->recurse_depth--;
}


void mmd_export_token_html_raw(DString * out, const char * source, token * t, scratch_pad * scratch) {
	if (t == NULL) {
		return;
	}

	char * temp;

	switch (t->type) {
		case BACKTICK:
			print_token(t);
			break;

		case AMPERSAND:
			print_const("&amp;");
			break;

		case AMPERSAND_LONG:
			print_const("&amp;amp;");
			break;

		case ANGLE_RIGHT:
			print_const("&gt;");
			break;

		case ANGLE_LEFT:
			print_const("&lt;");
			break;

		case CRITIC_COM_OPEN:
			print_const("{&gt;&gt;");
			break;

		case CRITIC_COM_CLOSE:
			print_const("&lt;&lt;}");
			break;

		case CRITIC_SUB_DIV:
			print_const("~&gt;");
			break;

		case CRITIC_SUB_DIV_A:
			print_const("~");
			break;

		case CRITIC_SUB_DIV_B:
			print_const("&gt;");
			break;

		case ESCAPED_CHARACTER:
			print_const("\\");

			if (t->next && t->next->type == TEXT_EMPTY && source[t->start + 1] == ' ') {
			} else {
				mmd_print_char_html(out, source[t->start + 1], false);
			}

			break;

		case HTML_COMMENT_START:
			print_const("&lt;!--");
			break;

		case HTML_COMMENT_STOP:
			print_const("--&gt;");
			break;

		case HTML_ENTITY:
			print_const("&amp;");
			d_string_append_c_array(out, &(source[t->start + 1]), t->len - 1);
			break;

		case MARKER_LIST_BULLET:
		case MARKER_LIST_ENUMERATOR:
			print_token(t);

			temp = NULL;

			if (t->next) {
				temp = (char *) &source[t->next->start];
			}

			source = (char *) &source[t->start + t->len];

			while (char_is_whitespace(*source) &&
					((temp == NULL) ||
					 (source < temp))) {
				print_char(*source);
				source++;
			}

			break;

		case MATH_BRACKET_OPEN:
			print_const("\\\\[");
			break;

		case MATH_BRACKET_CLOSE:
			print_const("\\\\]");
			break;

		case MATH_DOLLAR_SINGLE:
			print_const("$");
			break;

		case MATH_DOLLAR_DOUBLE:
			print_const("$$");
			break;

		case MATH_PAREN_OPEN:
			print_const("\\\\(");
			break;

		case MATH_PAREN_CLOSE:
			print_const("\\\\)");
			break;

		case QUOTE_DOUBLE:
			print_const("&quot;");
			break;

		case SUBSCRIPT:
			if (t->child) {
				print_const("~");
				mmd_export_token_tree_html_raw(out, source, t->child, scratch);
			} else {
				print_token(t);
			}

			break;

		case SUPERSCRIPT:
			if (t->child) {
				print_const("^");
				mmd_export_token_tree_html_raw(out, source, t->child, scratch);
			} else {
				print_token(t);
			}

			break;

		case CODE_FENCE:
			if (t->next) {
				t->next->type = TEXT_EMPTY;
			}

		case TEXT_EMPTY:
			break;

		default:
			if (t->child) {
				mmd_export_token_tree_html_raw(out, source, t->child, scratch);
			} else {
				print_token(t);
			}

			break;
	}
}


void mmd_export_token_html_math(DString * out, const char * source, token * t, scratch_pad * scratch) {
	if (t == NULL) {
		return;
	}

	switch (t->type) {
		case MATH_BRACKET_OPEN:
			print_const("\\[");
			break;

		case MATH_BRACKET_CLOSE:
			print_const("\\]");
			break;

		case MATH_DOLLAR_SINGLE:
			if (t->mate) {
				(t->start < t->mate->start) ? ( print_const("\\(") ) : ( print_const("\\)") );
			} else {
				print_const("$");
			}

			break;

		case MATH_DOLLAR_DOUBLE:
			if (t->mate) {
				(t->start < t->mate->start) ? ( print_const("\\[") ) : ( print_const("\\]") );
			} else {
				print_const("$$");
			}

			break;

		case MATH_PAREN_OPEN:
			print_const("\\(");
			break;

		case MATH_PAREN_CLOSE:
			print_const("\\)");
			break;

		default:
			mmd_export_token_html_raw(out, source, t, scratch);
			break;
	}
}


void mmd_start_complete_html(DString * out, const char * source, scratch_pad * scratch) {
	meta * m;

	print_const("<!DOCTYPE html>\n<html xmlns=\"http://www.w3.org/1999/xhtml\"");
	HASH_FIND_STR(scratch->meta_hash, "language", m);

	if (m) {
		printf(" lang=\"%s\"", m->value);
	} else {
		switch (scratch->language) {
			case LC_ES:
				print_const(" lang=\"es\"");
				break;

			case LC_DE:
				print_const(" lang=\"de\"");
				break;

			case LC_FR:
				print_const(" lang=\"fr\"");
				break;

			case LC_NL:
				print_const(" lang=\"nl\"");
				break;

			case LC_SV:
				print_const(" lang=\"sv\"");
				break;

			default:
				print_const(" lang=\"en\"");
		}
	}

	print_const(">\n<head>\n\t<meta charset=\"utf-8\"/>\n");

	// Iterate over metadata keys

	for (m = scratch->meta_hash; m != NULL; m = m->hh.next) {
		if (strcmp(m->key, "baseheaderlevel") == 0) {
		} else if (strcmp(m->key, "bibliostyle") == 0) {
		} else if (strcmp(m->key, "bibtex") == 0) {
		} else if (strcmp(m->key, "css") == 0) {
			print_const("\t<link type=\"text/css\" rel=\"stylesheet\" href=\"");

			if (scratch->store_assets) {
				store_asset(scratch, m->value);
				asset * a = extract_asset(scratch, m->value);

				mmd_print_string_html(out, "assets/", false);
				mmd_print_string_html(out, a->asset_path, false);
			} else {
				mmd_print_string_html(out, m->value, false);
			}

			print_const("\"/>\n");
		} else if (strcmp(m->key, "htmlfooter") == 0) {
		} else if (strcmp(m->key, "htmlheader") == 0) {
			print(m->value);
			print_char('\n');
		} else if (strcmp(m->key, "htmlheaderlevel") == 0) {
		} else if (strcmp(m->key, "language") == 0) {
		} else if (strcmp(m->key, "latexbegin") == 0) {
		} else if (strcmp(m->key, "latexconfig") == 0) {
		} else if (strcmp(m->key, "latexfooter") == 0) {
		} else if (strcmp(m->key, "latexheader") == 0) {
		} else if (strcmp(m->key, "latexheaderlevel") == 0) {
		} else if (strcmp(m->key, "latexinput") == 0) {
		} else if (strcmp(m->key, "latexleader") == 0) {
		} else if (strcmp(m->key, "latexmode") == 0) {
		} else if (strcmp(m->key, "mmdfooter") == 0) {
		} else if (strcmp(m->key, "mmdheader") == 0) {
		} else if (strcmp(m->key, "odfheader") == 0) {
		} else if (strcmp(m->key, "quoteslanguage") == 0) {
		} else if (strcmp(m->key, "title") == 0) {
			print_const("\t<title>");
			mmd_print_string_html(out, m->value, false);
			print_const("</title>\n");
		} else if (strcmp(m->key, "transcludebase") == 0) {
		} else if (strcmp(m->key, "xhtmlheader") == 0) {
			print(m->value);
			print_char('\n');
		} else if (strcmp(m->key, "xhtmlheaderlevel") == 0) {
		} else {
			print_const("\t<meta name=\"");
			mmd_print_string_html(out, m->key, false);
			print_const("\" content=\"");
			mmd_print_string_html(out, m->value, false);
			print_const("\"/>\n");
		}
	}

	print_const("</head>\n<body>\n\n");
}


void mmd_end_complete_html(DString * out, const char * source, scratch_pad * scratch) {
	meta * m;

	// Iterate over metadata keys

	for (m = scratch->meta_hash; m != NULL; m = m->hh.next) {
		if (strcmp(m->key, "htmlfooter") == 0) {
			print(m->value);
			print_char('\n');
		}
	}

	print_const("\n\n</body>\n</html>\n");
}


void mmd_export_token_tree_html_raw(DString * out, const char * source, token * t, scratch_pad * scratch) {
	while (t != NULL) {
		if (scratch->skip_token) {
			scratch->skip_token--;
		} else {
			mmd_export_token_html_raw(out, source, t, scratch);
		}

		t = t->next;
	}
}


void mmd_export_token_tree_html_math(DString * out, const char * source, token * t, scratch_pad * scratch) {
	while (t != NULL) {
		if (scratch->skip_token) {
			scratch->skip_token--;
		} else {
			mmd_export_token_html_math(out, source, t, scratch);
		}

		t = t->next;
	}
}


void mmd_export_footnote_list_html(DString * out, const char * source, scratch_pad * scratch) {
	if (scratch->used_footnotes->size > 0) {
		footnote * note;
		token * content;

		pad(out, 2, scratch);
		print_const("<div class=\"footnotes\">\n<hr />\n<ol>");
		scratch->padded = 0;

		for (int i = 0; i < scratch->used_footnotes->size; ++i) {
			// Export footnote
			pad(out, 2, scratch);

			printf("<li id=\"fn:%d\">\n", i + 1);
			scratch->padded = 6;

			note = stack_peek_index(scratch->used_footnotes, i);
			content = note->content;

			scratch->footnote_para_counter = 0;

			// We need to know which block is the last one in the footnote
			while (content) {
				if (content->type == BLOCK_PARA) {
					scratch->footnote_para_counter++;
				}

				content = content->next;
			}

			content = note->content;
			scratch->footnote_being_printed = i + 1;

			mmd_export_token_tree_html(out, source, content, scratch);

			pad(out, 1, scratch);
			printf("</li>");
			scratch->padded = 0;
		}

		pad(out, 2, scratch);
		print_const("</ol>\n</div>");
		scratch->padded = 0;
		scratch->footnote_being_printed = 0;
	}
}


void mmd_export_glossary_list_html(DString * out, const char * source, scratch_pad * scratch) {
	if (scratch->used_glossaries->size > 0) {
		footnote * note;
		token * content;

		pad(out, 2, scratch);
		print_const("<div class=\"glossary\">\n<hr />\n<ol>");
		scratch->padded = 0;

		for (int i = 0; i < scratch->used_glossaries->size; ++i) {
			// Export glossary
			pad(out, 2, scratch);

			printf("<li id=\"gn:%d\">\n", i + 1);
			scratch->padded = 6;

			note = stack_peek_index(scratch->used_glossaries, i);
			content = note->content;

			// Print term
			mmd_print_string_html(out, note->clean_text, false);
			print_const(": ");

			// Print contents
			scratch->footnote_para_counter = 0;

			// We need to know which block is the last one in the footnote
			while (content) {
				if (content->type == BLOCK_PARA) {
					scratch->footnote_para_counter++;
				}

				content = content->next;
			}

			content = note->content;
			scratch->glossary_being_printed = i + 1;

			mmd_export_token_tree_html(out, source, content, scratch);

			pad(out, 1, scratch);
			printf("</li>");
			scratch->padded = 0;
		}

		pad(out, 2, scratch);
		print_const("</ol>\n</div>");
		scratch->padded = 0;
		scratch->glossary_being_printed = 0;
	}
}


void mmd_export_citation_list_html(DString * out, const char * source, scratch_pad * scratch) {
	if (scratch->used_citations->size > 0) {
		footnote * note;
		token * content;

		pad(out, 2, scratch);
		print_const("<div class=\"citations\">\n<hr />\n<ol>");
		scratch->padded = 0;

		for (int i = 0; i < scratch->used_citations->size; ++i) {
			// Export footnote
			pad(out, 2, scratch);

			printf("<li id=\"cn:%d\">\n", i + 1);
			scratch->padded = 6;

			note = stack_peek_index(scratch->used_citations, i);
			content = note->content;

			scratch->footnote_para_counter = 0;

			// We need to know which block is the last one in the footnote
			while (content) {
				if (content->type == BLOCK_PARA) {
					scratch->footnote_para_counter++;
				}

				content = content->next;
			}

			content = note->content;
			scratch->citation_being_printed = i + 1;

			mmd_export_token_tree_html(out, source, content, scratch);

			pad(out, 1, scratch);
			printf("</li>");
			scratch->padded = 0;
		}

		pad(out, 2, scratch);
		print_const("</ol>\n</div>");
		scratch->padded = 0;
		scratch->citation_being_printed = 0;
	}
}



