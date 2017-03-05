/**

	MultiMarkdown -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file odf.c

	@brief Convert token tree to Flat OpenDocument (ODF/FODT) output


	@author	Fletcher T. Penney
	@bug	

**/

/*

	Copyright © 2016 - 2017 Fletcher T. Penney.


	The `MultiMarkdown 6` project is released under the MIT License..
	
	GLibFacade.c and GLibFacade.h are from the MultiMarkdown v4 project:
	
		https://github.com/fletcher/MultiMarkdown-4/
	
	MMD 4 is released under both the MIT License and GPL.
	
	
	CuTest is released under the zlib/libpng license. See CuTest.c for the
	text of the license.
	
	
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
	

*/


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "char.h"
#include "i18n.h"
#include "odf.h"
#include "parser.h"
#include "scanners.h"

#define print(x) d_string_append(out, x)
#define print_const(x) d_string_append_c_array(out, x, sizeof(x) - 1)
#define print_char(x) d_string_append_c(out, x)
#define printf(...) d_string_append_printf(out, __VA_ARGS__)
#define print_token(t) d_string_append_c_array(out, &(source[t->start]), t->len)
#define print_localized(x) mmd_print_localized_char_odf(out, x, scratch)


void mmd_print_char_odf(DString * out, char c) {
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
		case '\t':
			print_const("<text:tab/>");
		default:
			print_char(c);
			break;
	}
}


void mmd_print_string_odf(DString * out, const char * str) {
	if (str == NULL)
		return;
	
	while (*str != '\0') {
		mmd_print_char_odf(out, *str);
		str++;
	}
}


void mmd_print_localized_char_odf(DString * out, unsigned short type, scratch_pad * scratch) {
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


void mmd_export_link_odf(DString * out, const char * source, token * text, link * link, scratch_pad * scratch) {
	attr * a = link->attributes;

	if (link->url) {
		print_const("<text:a xlink:type=\"simple\" xlink:href=\"");
		mmd_print_string_odf(out, link->url);
		print_const("\"");
	} else
		print_const("<a xlink:type=\"simple\" xlink:href=\"\"");

	if (link->title && link->title[0] != '\0') {
		print_const(" office:name=\"");
		mmd_print_string_odf(out, link->title);
		print_const("\"");
	}

	print_const(">");

	// If we're printing contents of bracket as text, then ensure we include it all
	if (text && text->child && text->child->len > 1) {
		text->child->next->start--;
		text->child->next->len++;
	}
	
	mmd_export_token_tree_odf(out, source, text->child, scratch);

	print_const("</text:a>");
}


static char * correct_dimension_units(char *original) {
	char *result;
	int i;
	
	result = strdup(original);
	
	for (i = 0; result[i]; i++)
		result[i] = tolower(result[i]);
	
	if (strstr(&result[strlen(result)-2],"px")) {
		result[strlen(result)-2] = '\0';
		strcat(result, "pt");
	}
	
	return result;
}


void mmd_export_image_odf(DString * out, const char * source, token * text, link * link, scratch_pad * scratch, bool is_figure) {
	attr * a = link->attributes;
	char * height = NULL;
	char * width = NULL;

	print_const("<draw:frame text:anchor-type=\"as-char\"\ndraw:z-index=\"0\" draw:style-name=\"fr1\"");

	// Check attributes for dimensions
	while (a) {
		if (strcmp("height", a->key) == 0) {
			height = correct_dimension_units(a->value);
		} else if (strcmp("width", a->key) == 0) {
			width = correct_dimension_units(a->value);
		}

		a = a->next;
	}

	if (width) {
		printf(" svg:width=\"%s\">\n", width);
	} else {
		print_const(" svg:width=\"95%\">\n");
	}

	print_const("<draw:text-box><text:p><draw:frame text:anchor-type=\"as-char\" draw:z-index=\"1\" ");

	if (height && width) {
		printf("svg:height=\"%s\" ", height);
		printf("svg:width=\"%s\" ", width);
	}

	if (height)
		free(height);

	if (width)
		free(width);

	if (link->url)
		printf(">\n<draw:image xlink:href=\"%s\"", link->url);

	print_const(" xlink:type=\"simple\" xlink:show=\"embed\" xlink:actuate=\"onLoad\" draw:filter-name=\"&lt;All formats&gt;\"/>\n</draw:frame></text:p>");

	if (is_figure) {
		if (text) {
			print_const("\n<text:p>Figure <text:sequence text:name=\"Figure\" text:formula=\"ooow:Figure+1\" style:num-format=\"1\"> Update Fields to calculate numbers</text:sequence>: ");
			mmd_export_token_tree_odf(out, source, text->child, scratch);
			print_const("</text:p>");
		}
	}

	print_const("\n</draw:text-box></draw:frame>\n");

	scratch->padded = 1;
}



void mmd_export_toc_entry_odf(DString * out, const char * source, scratch_pad * scratch, size_t * counter, short level) {
	token * entry, * next;
	short entry_level, next_level;
	char * temp_char;

	print_const("\n<text:list text:style-name=\"L1\">\n");

	// Iterate over tokens
	while (*counter < scratch->header_stack->size) {
		// Get token for header
		entry = stack_peek_index(scratch->header_stack, *counter);
		entry_level = raw_level_for_header(entry);

		if (entry_level >= level) {
			// This entry is a direct descendant of the parent
			temp_char = label_from_header(source, entry);
			printf("<text:list-item><text:p text:style-name=\"P1\"><text:a xlink:type=\"simple\" xlink:href=\"#%s\">", temp_char);
			mmd_export_token_tree_odf(out, source, entry->child, scratch);
			print_const("</text:a></text:p>");

			if (*counter < scratch->header_stack->size - 1) {
				next = stack_peek_index(scratch->header_stack, *counter + 1);
				next_level = next->type - BLOCK_H1 + 1;
				if (next_level > entry_level) {
					// This entry has children
					(*counter)++;
					mmd_export_toc_entry_odf(out, source, scratch, counter, entry_level + 1);
				}
			}

			print_const("</text:list-item>\n");
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

	print_const("</text:list>\n");
}

void mmd_export_toc_odf(DString * out, const char * source, scratch_pad * scratch) {
	token * entry;
	size_t counter = 0;

	mmd_export_toc_entry_odf(out, source, scratch, &counter, 0);
}


void mmd_export_token_odf(DString * out, const char * source, token * t, scratch_pad * scratch) {
	if (t == NULL)
		return;

	short	temp_short;
	short	temp_short2;
	short	temp_short3;
	link *	temp_link	= NULL;
	char *	temp_char	= NULL;
	char *	temp_char2	= NULL;
	char *	temp_char3	= NULL;
	bool	temp_bool	= 0;
	token *	temp_token	= NULL;
	footnote * temp_note = NULL;

	switch (t->type) {
		case DOC_START_TOKEN:
			mmd_export_token_tree_odf(out, source, t->child, scratch);
			break;
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
			if (t->mate == NULL)
				print_token(t);
			else if (t->mate->type == QUOTE_RIGHT_ALT)
				if (!(scratch->extensions & EXT_SMART)) {
					print_token(t);
				} else {
					print_localized(QUOTE_LEFT_DOUBLE);
				}
			else if (t->start < t->mate->start) {
				print_const("<text:span text:style-name=\"Source_20_Text\">");
			} else {
				print_const("</text:span>");
			}
			break;
		case BLOCK_BLOCKQUOTE:
			pad(out, 2, scratch);
			scratch->padded = 2;
			temp_short2 = scratch->odf_para_type;

			scratch->odf_para_type = BLOCK_BLOCKQUOTE;

			mmd_export_token_tree_odf(out, source, t->child, scratch);
			scratch->padded = 0;
			scratch->odf_para_type = temp_short2;
			break;
		case BLOCK_CODE_FENCED:
			pad(out, 2, scratch);
			print_const("<text:p text:style-name=\"Preformatted Text\">");
			mmd_export_token_tree_odf_raw(out, source, t->child->next, scratch);
			print_const("</text:p>");
			scratch->padded = 0;
			break;
		case BLOCK_CODE_INDENTED:
			pad(out, 2, scratch);
			print_const("<text:p text:style-name=\"Preformatted Text\">");
			mmd_export_token_tree_odf_raw(out, source, t->child, scratch);
			print_const("</text:p>");
			scratch->padded = 0;
			break;
		case BLOCK_DEFINITION:
			pad(out, 2, scratch);
			temp_short2 = scratch->odf_para_type;
			scratch->odf_para_type = BLOCK_DEFINITION;

			temp_short = scratch->list_is_tight;
			if (!(t->child->next && (t->child->next->type == BLOCK_EMPTY) && t->child->next->next))
				scratch->list_is_tight = true;

			if (t->child && t->child->type != BLOCK_PARA) {
				print_const("<text:p text:style-name=\"Quotations\">");
				mmd_export_token_tree_odf(out, source, t->child, scratch);
				print_const("</text:p>");
			} else {
				mmd_export_token_tree_odf(out, source, t->child, scratch);				
			}
			scratch->padded = 0;

			scratch->list_is_tight = temp_short;
			scratch->odf_para_type = temp_short2;
			break;
		case BLOCK_DEFLIST:
			pad(out, 2, scratch);

			// Group consecutive definition lists into a single list.
			// lemon's LALR(1) parser can't properly handle this (to my understanding).

//			if (!(t->prev && (t->prev->type == BLOCK_DEFLIST)))
//				print_const("<dl>\n");
	
			scratch->padded = 2;

			mmd_export_token_tree_odf(out, source, t->child, scratch);
			pad(out, 1, scratch);

//			if (!(t->next && (t->next->type == BLOCK_DEFLIST)))
//				print_const("</dl>\n");

			scratch->padded = 1;
			break;
		case BLOCK_EMPTY:
			break;
		case BLOCK_H1:
		case BLOCK_H2:
		case BLOCK_H3:
		case BLOCK_H4:
		case BLOCK_H5:
		case BLOCK_H6:
		case BLOCK_SETEXT_1:
		case BLOCK_SETEXT_2:
			pad(out, 2, scratch);
			switch (t->type) {
				case BLOCK_SETEXT_1:
					temp_short = 1;
					break;
				case BLOCK_SETEXT_2:
					temp_short = 2;
					break;
				default:
					temp_short = t->type - BLOCK_H1 + 1;
			}

			printf("<text:h text:outline-level=\"%d\">", temp_short + scratch->base_header_level - 1);

			if (scratch->extensions & EXT_NO_LABELS) {
				mmd_export_token_tree_odf(out, source, t->child, scratch);
			} else {
				temp_char = label_from_header(source, t);
				printf("<text:bookmark text:name=\"%s\"/>", temp_char);
				mmd_export_token_tree_odf(out, source, t->child, scratch);
				printf("<text:bookmark-end text:name=\"%s\"/>", temp_char);
				free(temp_char);
			}

			print_const("</text:h>");
			scratch->padded = 0;
			break;
		case BLOCK_HR:
			pad(out, 2, scratch);
			print_const("<text:p text:style-name=\"Horizontal_20_Line\"/>");
			scratch->padded = 0;
			break;
		case BLOCK_HTML:
			// Don't print HTML
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
			print_const("<text:list text:style-name=\"L1\">");
			scratch->padded = 1;
			mmd_export_token_tree_odf(out, source, t->child, scratch);
			pad(out, 2, scratch);
			print_const("</text:list>");
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
			print_const("<text:list text:style-name=\"L2\">");
			scratch->padded = 1;
			mmd_export_token_tree_odf(out, source, t->child, scratch);
			pad(out, 2, scratch);
			print_const("</text:list>");
			scratch->padded = 0;
			scratch->list_is_tight = temp_short;
			break;
		case BLOCK_LIST_ITEM:
			pad(out, 2, scratch);
			print_const("<text:list-item>\n");
			scratch->padded = 2;
			mmd_export_token_tree_odf(out, source, t->child, scratch);
			print_const("</text:list-item>");
			scratch->padded = 0;
			break;
		case BLOCK_LIST_ITEM_TIGHT:
			pad(out, 2, scratch);
			print_const("<text:list-item>\n");

			if (t->child && t->child->type != BLOCK_PARA)
				print_const("<text:p text:style-name=\"P1\">\n");

			scratch->padded = 2;
			mmd_export_token_tree_odf(out, source, t->child, scratch);

			if (t->child && t->child->type != BLOCK_PARA)
			print_const("</text:p>");

			print_const("</text:list-item>");
			scratch->padded = 0;
			break;
		case BLOCK_META:
			break;
		case BLOCK_PARA:
			pad(out, 2, scratch);
			print_const("<text:p");

			switch (scratch->odf_para_type) {
				case BLOCK_BLOCKQUOTE:
				case BLOCK_DEFINITION:
					print_const(" text:style-name=\"Quotations\">");
					break;
				case PAIR_BRACKET_ABBREVIATION:
				case PAIR_BRACKET_CITATION:
				case PAIR_BRACKET_FOOTNOTE:
				case PAIR_BRACKET_GLOSSARY:
					print_const(" text:style-name=\"Footnote\">");
					break;
				default:				
					print_const(" text:style-name=\"Standard\">");
					break;
			}

			mmd_export_token_tree_odf(out, source, t->child, scratch);

			print_const("</text:p>");
			scratch->padded = 0;
			break;
		case BLOCK_TABLE:
			pad(out, 2, scratch);
			print_const("<table:table>\n");

			scratch->padded = 2;
			read_table_column_alignments(source, t, scratch);

			for (int i = 0; i < scratch->table_column_count; ++i)
			{
				print_const("<table:table-column/>\n");
//				switch (scratch->table_alignment[i]) {
//					case 'l':
//						print_const("<col style=\"text-align:left;\"/>\n");
//						break;
//					case 'L':
//						print_const("<col style=\"text-align:left;\" class=\"extended\"/>\n");
//						break;
//					case 'r':
//						print_const("<col style=\"text-align:right;\"/>\n");
//						break;
//					case 'R':
//						print_const("<col style=\"text-align:right;\" class=\"extended\"/>\n");
//						break;
//					case 'c':
//						print_const("<col style=\"text-align:center;\"/>\n");
//						break;
//					case 'C':
//						print_const("<col style=\"text-align:center;\" class=\"extended\"/>\n");
//						break;
//					default:
//						print_const("<col />\n");
//						break;
//				}
			}
			scratch->padded = 1;

			mmd_export_token_tree_odf(out, source, t->child, scratch);
			pad(out, 1, scratch);
			print_const("</table:table>\n");

			// Are we followed by a caption?
			if (table_has_caption(t)) {
				temp_token = t->next->child;

				if (temp_token->next &&
					temp_token->next->type == PAIR_BRACKET) {
					temp_token = temp_token->next;
				}

				temp_char = label_from_token(source, temp_token);
				printf("<text:p><text:bookmark text:name=\"%s\"/>Table <text:sequence text:name=\"Table\" text:formula=\"ooow:Table+1\" style:num-format=\"1\"> Update Fields to calculate numbers</text:sequence>:", temp_char);

				t->next->child->child->type = TEXT_EMPTY;
				t->next->child->child->mate->type = TEXT_EMPTY;
				mmd_export_token_tree_odf(out, source, t->next->child->child, scratch);

				printf("<text:bookmark-end text:name=\"%s\"/></text:p>\n", temp_char);

				free(temp_char);
				temp_short = 1;
			} else {
				temp_short = 0;
			}

			scratch->padded = 0;
			scratch->skip_token = temp_short;

			break;
		case BLOCK_TABLE_HEADER:
			pad(out, 2, scratch);
			scratch->in_table_header = 1;
			mmd_export_token_tree_odf(out, source, t->child, scratch);
			scratch->in_table_header = 0;
			scratch->padded = 1;
			break;
		case BLOCK_TABLE_SECTION:
			pad(out, 2, scratch);
			scratch->padded = 2;
			mmd_export_token_tree_odf(out, source, t->child, scratch);
			scratch->padded = 0;
			break;
		case BLOCK_TOC:
			temp_short = 0;
			temp_short2 = 0;
			pad(out, 2, scratch);

			mmd_export_toc_odf(out, source, scratch);

			scratch->padded = 1;
			break;
		case BLOCK_TERM:
			pad(out, 2, scratch);
			print_const("<text:p><text:span text:style-name=\"MMD-Bold\">");
			mmd_export_token_tree_odf(out, source, t->child, scratch);
			print_const("</text:span></text:p>\n");
			scratch->padded = 2;
			break;
		case BRACE_DOUBLE_LEFT:
			print_const("{{");
			break;
		case BRACE_DOUBLE_RIGHT:
			print_const("}}");
			break;
		case BRACKET_CITATION_LEFT:
			print_const("[#");
		case BRACKET_LEFT:
			print_const("[");			
			break;
		case BRACKET_RIGHT:
			print_const("]");
			break;
		case BRACKET_VARIABLE_LEFT:
			print_const("[\%");
			break;
		case COLON:
			print_char(':');
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
		case ELLIPSIS:
			if (!(scratch->extensions & EXT_SMART)) {
				print_token(t);
			} else {
				print_localized(ELLIPSIS);
			}
			break;
		case EMPH_START:
			print_const("<text:span text:style-name=\"MMD-Italic\">");
			break;
		case EMPH_STOP:
			print_const("</text:span>");
			break;
		case EQUAL:
			print_char('=');
			break;
		case ESCAPED_CHARACTER:
			if (!(scratch->extensions & EXT_COMPATIBILITY) &&
				(source[t->start + 1] == ' ')) {
				print_const(" ");		// This is a non-breaking space character
			} else {
				mmd_print_char_odf(out, source[t->start + 1]);
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
		case INDENT_SPACE:
			print_char(' ');
			break;
		case INDENT_TAB:
			print_const("<text:tab/>");
			break;
		case LINE_LIST_BULLETED:
		case LINE_LIST_ENUMERATED:
			mmd_export_token_tree_odf(out, source, t->child, scratch);
			break;
		case MANUAL_LABEL:
		case MARKER_BLOCKQUOTE:
		case MARKER_H1:
		case MARKER_H2:
		case MARKER_H3:
		case MARKER_H4:
		case MARKER_H5:
		case MARKER_H6:
		case MARKER_LIST_BULLET:
		case MARKER_LIST_ENUMERATOR:
			break;
		case MATH_BRACKET_OPEN:
			if (t->mate) {
				print_const("<text:span text:style-name=\"math\">\\[");
			} else
				print_const("\\[");
			break;
		case MATH_BRACKET_CLOSE:
			if (t->mate) {
				print_const("\\]</text:span>");
			} else
				print_const("\\]");
			break;
		case MATH_DOLLAR_SINGLE:
			if (t->mate) {
				(t->start < t->mate->start) ? ( print_const("<text:span text:style-name=\"math\">\\(") ) : ( print_const("\\)</text:span>") );
			} else {
				print_const("$");
			}
			break;
		case MATH_DOLLAR_DOUBLE:
			if (t->mate) {
				(t->start < t->mate->start) ? ( print_const("<text:span text:style-name=\"math\">\\[") ) : ( print_const("\\]</text:span>") );
			} else {
				print_const("$$");
			}
			break;
		case MATH_PAREN_OPEN:
			if (t->mate) {
				print_const("<text:span text:style-name=\"math\">\\(");
			} else
				print_const("\\(");
			break;
		case MATH_PAREN_CLOSE:
			if (t->mate) {
				print_const("\\)</text:span>");
			} else
				print_const("\\)");
			break;
		case NON_INDENT_SPACE:
			print_char(' ');
			break;
		case PAIR_ANGLE:
			temp_char = url_accept(source, t->start + 1, t->len - 2, NULL, true);

			if (temp_char) {
				print_const("<text:a xlink:type=\"simple\" xlink:href=\"");

				if (scan_email(temp_char)) {
					temp_bool = true;

					if (strncmp("mailto:", temp_char, 7) != 0) {
						print_const("mailto:");
					}
				} else {
					temp_bool = false;
				}

				mmd_print_string_odf(out, temp_char);
				print_const("\">");
				mmd_print_string_odf(out, temp_char);
				print_const("</text:a>");
			} else if (scan_html(&source[t->start])) {
				// We ignore HTML blocks
				if (scan_html_comment(&source[t->start])) {
					// But allow HTML comments as raw LaTeX
					d_string_append_c_array(out, &source[t->start + 4], t->len - 4 - 3);
				}
			} else {
				mmd_export_token_tree_odf(out, source, t->child, scratch);
			}

			free(temp_char);
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
			print_const("<text:span text:style-name=\"Source_20_Text\">");
			mmd_export_token_tree_odf_raw(out, source, t->child, scratch);
			print_const("</text:span>");
			break;
		case PAIR_BRACES:
			mmd_export_token_tree_odf(out, source, t->child, scratch);
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
					mmd_export_link_odf(out, source, t, temp_link, scratch);
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
						mmd_export_image_odf(out, source, t, temp_link, scratch, false);
					} else {
						mmd_export_image_odf(out, source, t, temp_link, scratch, true);
					}
				}
				
				if (temp_bool) {
					link_free(temp_link);
				}

				scratch->skip_token = temp_short;

				return;
			}

			// No links exist, so treat as normal
			mmd_export_token_tree_odf(out, source, t->child, scratch);
			break;
		case PAIR_BRACKET_CITATION:
			parse_citation:
			temp_bool = true;   // Track whether this is a 'not cited'
			temp_token = t;     // Remember whether we need to skip ahead
			
			if (scratch->extensions & EXT_NOTES) {
				if (t->type == PAIR_BRACKET) {
					// This is a locator for subsequent citation (e.g. `[foo][#bar]`
					temp_char = text_inside_pair(source, t);
					temp_char2 = label_from_string(temp_char);

					if (strcmp(temp_char2, "notcited") == 0) {
						free(temp_char);
						temp_char = strdup("");
						temp_bool = false;
					}

					free(temp_char2);

					// Process the actual citation
					t = t->next;
				} else {
					// This is just a citation (e.g. `[#foo]`)
					temp_char = strdup("");
				}

				temp_short3 = scratch->used_citations->size;

				citation_from_bracket(source, scratch, t, &temp_short);

				temp_short2 = scratch->odf_para_type;
				scratch->odf_para_type = PAIR_BRACKET_CITATION;

				if (temp_short3 == scratch->used_citations->size) {
					// Re-using previous citation
					print_const("<text:span text:style-name=\"Footnote_20_anchor\"><text:note-ref text:note-class=\"endnote\" text:reference-format=\"text\" ");
					printf("text:ref-name=\"cite%d\">%d</text:note-ref></text:span>", temp_short, temp_short);
				} else {
					// New citation
					printf("<text:note text:id=\"cite%d\" text:note-class=\"endnote\"><text:note-body>", temp_short);
					temp_note = stack_peek_index(scratch->used_citations, temp_short - 1);

					mmd_export_token_tree_odf(out, source, temp_note->content, scratch);
					print_const("</text:note-body></text:note>");
				}

				scratch->odf_para_type = temp_short2;

				if (temp_token != t) {
					// Skip citation on next pass
					scratch->skip_token = 1;
				}
			} else {
				// Footnotes disabled
				mmd_export_token_tree_odf(out, source, t->child, scratch);
			}

			free(temp_char);
			break;
		case PAIR_BRACKET_FOOTNOTE:
			if (scratch->extensions & EXT_NOTES) {
				footnote_from_bracket(source, scratch, t, &temp_short);

				temp_short2 = scratch->odf_para_type;
				scratch->odf_para_type = PAIR_BRACKET_FOOTNOTE;

				if (temp_short < scratch->used_footnotes->size) {
					// Re-using previous footnote
					print("\\footnote{reuse");

					print("}");
				} else {
					// This is a new footnote
					printf("<text:note text:id=\"fn%d\" text:note-class=\"footnote\"><text:note-body>", temp_short);
					temp_note = stack_peek_index(scratch->used_footnotes, temp_short - 1);

					mmd_export_token_tree_odf(out, source, temp_note->content, scratch);
					print_const("</text:note-body></text:note>");
				}

				scratch->odf_para_type = temp_short2;
			} else {
				// Footnotes disabled
				mmd_export_token_tree_odf(out, source, t->child, scratch);
			}
			break;
		case PAIR_BRACKET_ABBREVIATION:
			if (scratch->extensions & EXT_NOTES) {
				abbreviation_from_bracket(source, scratch, t, &temp_short);

				if (temp_short == -1) {
					print_const("[>");
					mmd_export_token_tree_odf(out, source, t->child, scratch);
					print_const("]");
					break;
				}

				temp_short2 = scratch->odf_para_type;
				scratch->odf_para_type = PAIR_BRACKET_ABBREVIATION;

				if (temp_short < scratch->used_abbreviations->size) {
					// Re-using previous footnote
					print("\\footnote{reuse");

					print("}");
				} else {
					// This is a new abbreviation item
					temp_note = stack_peek_index(scratch->used_abbreviations, temp_short - 1);

					mmd_print_string_odf(out, temp_note->label_text); 

					printf("<text:note text:id=\"gn%d\" text:note-class=\"glossary\"><text:note-body>", temp_short);

					mmd_export_token_tree_odf(out, source, temp_note->content, scratch);
					print_const("</text:note-body></text:note>");
				}

				scratch->odf_para_type = temp_short2;
			} else {
				// Footnotes disabled
				mmd_export_token_tree_odf(out, source, t->child, scratch);
			}
			break;
		case PAIR_BRACKET_GLOSSARY:
			if (scratch->extensions & EXT_NOTES) {
				glossary_from_bracket(source, scratch, t, &temp_short);

				if (temp_short == -1) {
					print_const("[?");
					mmd_export_token_tree_odf(out, source, t->child, scratch);
					print_const("]");
					break;
				}

				temp_short2 = scratch->odf_para_type;
				scratch->odf_para_type = PAIR_BRACKET_GLOSSARY;

				if (temp_short < scratch->used_glossaries->size) {
					// Re-using previous footnote
					print("\\footnote{reuse");

					print("}");
				} else {
					// This is a new glossary item
					temp_note = stack_peek_index(scratch->used_glossaries, temp_short - 1);

					mmd_print_string_odf(out, temp_note->clean_text); 

					printf("<text:note text:id=\"gn%d\" text:note-class=\"glossary\"><text:note-body>", temp_short);

					mmd_export_token_tree_odf(out, source, temp_note->content, scratch);
					print_const("</text:note-body></text:note>");
				}

				scratch->odf_para_type = temp_short2;
			} else {
				// Footnotes disabled
				mmd_export_token_tree_odf(out, source, t->child, scratch);
			}
			break;
		case PAIR_BRACKET_VARIABLE:
			temp_char = text_inside_pair(source, t);
			temp_char2 = extract_metadata(scratch, temp_char);

			if (temp_char2)
				mmd_print_string_odf(out, temp_char2);
			else
				mmd_export_token_tree_odf(out, source, t->child, scratch);

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
					mmd_export_token_tree_odf(out, source, t->child, scratch);
				} else {
					print_const("<text:span text:style-name=\"Underline\">");
					mmd_export_token_tree_odf(out, source, t->child, scratch);
					print_const("</text:span>");
				}
			} else {
				mmd_export_token_tree_odf(out, source, t->child, scratch);				
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
					mmd_export_token_tree_odf(out, source, t->child, scratch);
				} else {
					print_const("<text:span text:style-name=\"Strike\">");
					mmd_export_token_tree_odf(out, source, t->child, scratch);
					print_const("</text:span>");
				}
			} else {
				mmd_export_token_tree_odf(out, source, t->child, scratch);				
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
				print_const("<text:span text:style-name=\"Comment\">");
				mmd_export_token_tree_odf(out, source, t->child, scratch);
				print_const("</text:span>");
			} else {
				mmd_export_token_tree_odf(out, source, t->child, scratch);
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
				print_const("<text:span text:style-name=\"Highlight\">");
				mmd_export_token_tree_odf(out, source, t->child, scratch);
				print_const("</text:span>");
			} else {
				mmd_export_token_tree_odf(out, source, t->child, scratch);
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
				(t->next->type == PAIR_CRITIC_SUB_ADD)) {
				t->child->type = TEXT_EMPTY;
				t->child->mate->type = TEXT_EMPTY;
				if (scratch->extensions & EXT_CRITIC_ACCEPT) {

				} else if (scratch->extensions & EXT_CRITIC_REJECT) {
					mmd_export_token_tree_odf(out, source, t->child, scratch);
				} else {
					print_const("<text:span text:style-name=\"Strike\">");
					mmd_export_token_tree_odf(out, source, t->child, scratch);
					print_const("</text:span>");
				}
			} else {
				mmd_export_token_tree_odf(out, source, t->child, scratch);
			}
			break;
		case PAIR_CRITIC_SUB_ADD:
			if ((scratch->extensions & EXT_CRITIC) &&
				(t->prev->type == PAIR_CRITIC_SUB_DEL)) {
				t->child->type = TEXT_EMPTY;
				t->child->mate->type = TEXT_EMPTY;
				if (scratch->extensions & EXT_CRITIC_REJECT) {

				} else if (scratch->extensions & EXT_CRITIC_ACCEPT) {
					mmd_export_token_tree_odf(out, source, t->child, scratch);
				} else {
					print_const("<text:span text:style-name=\"Underline\">");
					mmd_export_token_tree_odf(out, source, t->child, scratch);
					print_const("</text:span>");
				}
			} else {
				mmd_export_token_tree_odf(out, source, t->child, scratch);
			}
			break;
		case PAIR_MATH:
		case PAIR_PAREN:
		case PAIR_QUOTE_DOUBLE:
		case PAIR_QUOTE_SINGLE:
		case PAIR_STAR:
		case PAIR_UL:
			mmd_export_token_tree_odf(out, source, t->child, scratch);
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
			print_char('+');
			break;
		case QUOTE_SINGLE:
			if ((t->mate == NULL) || (!(scratch->extensions & EXT_SMART)))
				print_const("'");
			else
				(t->start < t->mate->start) ? ( print_localized(QUOTE_LEFT_SINGLE) ) : ( print_localized(QUOTE_RIGHT_SINGLE) );
			break;
		case QUOTE_DOUBLE:
			if ((t->mate == NULL) || (!(scratch->extensions & EXT_SMART)))
				print_const("&quot;");
			else
				(t->start < t->mate->start) ? ( print_localized(QUOTE_LEFT_DOUBLE) ) : ( print_localized(QUOTE_RIGHT_DOUBLE) );
			break;
		case QUOTE_RIGHT_ALT:
			if ((t->mate == NULL) || (!(scratch->extensions & EXT_SMART)))
				print_const("''");
			else
				print_localized(QUOTE_RIGHT_DOUBLE);
			break;
		case SLASH:
			print_char('/');
			break;
		case STAR:
			print_char('*');
			break;
		case STRONG_START:
			print_const("<text:span text:style-name=\"MMD-Bold\">");
			break;
		case STRONG_STOP:
			print_const("</text:span>");
			break;
		case SUBSCRIPT:
			if (t->mate) {
				(t->start < t->mate->start) ? (print_const("<text:span text:style-name=\"MMD-Subscript\">")) : (print_const("</text:span>"));
			} else if (t->len != 1) {
				print_const("<text:span text:style-name=\"MMD-Subscript\">");
				mmd_export_token_odf(out, source, t->child, scratch);
				print_const("</text:span>");
			} else {
				print_const("~");
			}
			break;
		case SUPERSCRIPT:
			if (t->mate) {
				(t->start < t->mate->start) ? (print_const("<text:span text:style-name=\"MMD-Superscript\">")) : (print_const("</text:span>"));
			} else if (t->len != 1) {
				print_const("<text:span text:style-name=\"MMD-Superscript\">");
				mmd_export_token_odf(out, source, t->child, scratch);
				print_const("</text:span>");
			} else {
				print_const("^");
			}	
			break;
		case TABLE_CELL:
			print_const("<table:table-cell");

			if (t->next && t->next->type == TABLE_DIVIDER) {
				if (t->next->len > 1) {
					printf(" table:number-columns-spanned=\"%d\"", t->next->len);
				}
			}

			if (scratch->in_table_header) {
				print_const(">\n<text:p text:style-name=\"Table_20_Heading\"");
			} else {
				print_const(">\n<text:p");
				switch (scratch->table_alignment[scratch->table_cell_count]) {
					case 'l':
					case 'L':
					default:
						print_const(" text:style-name=\"MMD-Table\"");
						break;
					case 'r':
					case 'R':
						print_const(" text:style-name=\"MMD-Table-Right\"");
						break;
					case 'c':
					case 'C':
						print_const(" text:style-name=\"MMD-Table-Center\"");
						break;
				}
			}

			print_const(">");
			mmd_export_token_tree_odf(out, source, t->child, scratch);

			print_const("</text:p>\n</table:table-cell>\n");

			if (t->next)
				scratch->table_cell_count += t->next->len;
			else
				scratch->table_cell_count++;
			
			break;
		case TABLE_DIVIDER:
			break;
		case TABLE_ROW:
			print_const("<table:table-row>\n");
			scratch->table_cell_count = 0;
			mmd_export_token_tree_odf(out, source, t->child, scratch);
			print_const("</table:table-row>\n");
			break;
		case TEXT_EMPTY:
			break;
		case TEXT_LINEBREAK:
			if (t->next) {
				print_const("<text:line-break/>\n");
				scratch->padded = 0;
			}
			break;
		case TEXT_NL:
			if (t->next)
				print_char('\n');
			break;
		case TEXT_BACKSLASH:
		case TEXT_BRACE_LEFT:
		case TEXT_BRACE_RIGHT:
		case TEXT_HASH:
		case TEXT_NUMBER_POSS_LIST:
		case TEXT_PERCENT:
		case TEXT_PERIOD:
		case TEXT_PLAIN:
		case TOC:
		case UL:
			print_token(t);
			break;
		default:
			fprintf(stderr, "Unknown token type: %d\n", t->type);
			token_describe(t, source);
			break;
	}
}


void mmd_export_token_tree_odf(DString * out, const char * source, token * t, scratch_pad * scratch) {

	// Prevent stack overflow with "dangerous" input causing extreme recursion
	if (scratch->recurse_depth == kMaxExportRecursiveDepth) {
		return;
	}

	scratch->recurse_depth++;

	while (t != NULL) {
		if (scratch->skip_token) {
			scratch->skip_token--;
		} else {
			mmd_export_token_odf(out, source, t, scratch);
		}

		t = t->next;
	}

	scratch->recurse_depth--;
}


void mmd_export_token_odf_raw(DString * out, const char * source, token * t, scratch_pad * scratch) {
	if (t == NULL)
		return;

	switch (t->type) {
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
		case ESCAPED_CHARACTER:
			print_const("\\");
			mmd_print_char_odf(out, source[t->start + 1]);
			break;
		case INDENT_TAB:
			print_const("<text:tab/>");
			break;
		case QUOTE_DOUBLE:
			print_const("&quot;");
			break;
		case CODE_FENCE:
			if (t->next)
				t->next->type = TEXT_EMPTY;
		case TEXT_EMPTY:
			break;
		case TEXT_NL:
			print_const("<text:line-break/>");
			break;
		default:
			if (t->child)
				mmd_export_token_tree_odf_raw(out, source, t->child, scratch);
			else
				print_token(t);
			break;
	}
}


void mmd_export_token_tree_odf_raw(DString * out, const char * source, token * t, scratch_pad * scratch) {
	while (t != NULL) {
		if (scratch->skip_token) {
			scratch->skip_token--;
		} else {
			mmd_export_token_odf_raw(out, source, t, scratch);
		}

		t = t->next;
	}
}


void mmd_start_complete_odf(DString * out, const char * source, scratch_pad * scratch) {
	print_const("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" \
"<office:document xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\"\n" \
"     xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\"\n" \
"     xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\"\n" \
"     xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\"\n" \
"     xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\"\n" \
"     xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\"\n" \
"     xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n" \
"     xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n" \
"     xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\"\n" \
"     xmlns:number=\"urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0\"\n" \
"     xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\"\n" \
"     xmlns:chart=\"urn:oasis:names:tc:opendocument:xmlns:chart:1.0\"\n" \
"     xmlns:dr3d=\"urn:oasis:names:tc:opendocument:xmlns:dr3d:1.0\"\n" \
"     xmlns:math=\"http://www.w3.org/1998/Math/MathML\"\n" \
"     xmlns:form=\"urn:oasis:names:tc:opendocument:xmlns:form:1.0\"\n" \
"     xmlns:script=\"urn:oasis:names:tc:opendocument:xmlns:script:1.0\"\n" \
"     xmlns:config=\"urn:oasis:names:tc:opendocument:xmlns:config:1.0\"\n" \
"     xmlns:ooo=\"http://openoffice.org/2004/office\"\n" \
"     xmlns:ooow=\"http://openoffice.org/2004/writer\"\n" \
"     xmlns:oooc=\"http://openoffice.org/2004/calc\"\n" \
"     xmlns:dom=\"http://www.w3.org/2001/xml-events\"\n" \
"     xmlns:xforms=\"http://www.w3.org/2002/xforms\"\n" \
"     xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\"\n" \
"     xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n" \
"     xmlns:rpt=\"http://openoffice.org/2005/report\"\n" \
"     xmlns:of=\"urn:oasis:names:tc:opendocument:xmlns:of:1.2\"\n" \
"     xmlns:xhtml=\"http://www.w3.org/1999/xhtml\"\n" \
"     xmlns:grddl=\"http://www.w3.org/2003/g/data-view#\"\n" \
"     xmlns:tableooo=\"http://openoffice.org/2009/table\"\n" \
"     xmlns:field=\"urn:openoffice:names:experimental:ooo-ms-interop:xmlns:field:1.0\"\n" \
"     xmlns:formx=\"urn:openoffice:names:experimental:ooxml-odf-interop:xmlns:form:1.0\"\n" \
"     xmlns:css3t=\"http://www.w3.org/TR/css3-text/\"\n" \
"     office:version=\"1.2\"\n" \
"     grddl:transformation=\"http://docs.oasis-open.org/office/1.2/xslt/odf2rdf.xsl\"\n" \
"     office:mimetype=\"application/vnd.oasis.opendocument.text\">\n");
    
	/* Font Declarations */
	print_const("<office:font-face-decls>\n" \
	"   <style:font-face style:name=\"Courier New\" svg:font-family=\"'Courier New'\"\n" \
    "                    style:font-adornments=\"Regular\"\n" \
    "                    style:font-family-generic=\"modern\"\n" \
    "                    style:font-pitch=\"fixed\"/>\n" \
    "</office:font-face-decls>\n");
    
    /* Append basic style information */
    print_const("<office:styles>\n" \
    "<style:style style:name=\"Standard\" style:family=\"paragraph\" style:class=\"text\">\n" \
    "      <style:paragraph-properties fo:margin-top=\"0in\" fo:margin-bottom=\"0.15in\"" \
    "     fo:text-align=\"justify\" style:justify-single-word=\"false\"/>\n" \
    "   </style:style>\n" \
    "<style:style style:name=\"Preformatted_20_Text\" style:display-name=\"Preformatted Text\"\n" \
    "             style:family=\"paragraph\"\n" \
    "             style:parent-style-name=\"Standard\"\n" \
    "             style:class=\"html\">\n" \
    "   <style:paragraph-properties fo:margin-top=\"0in\" fo:margin-bottom=\"0in\" fo:text-align=\"start\"\n" \
    "                               style:justify-single-word=\"false\"/>\n" \
    "   <style:text-properties style:font-name=\"Courier New\" fo:font-size=\"11pt\"\n" \
    "                          style:font-name-asian=\"Courier New\"\n" \
    "                          style:font-size-asian=\"11pt\"\n" \
    "                          style:font-name-complex=\"Courier New\"\n" \
    "                          style:font-size-complex=\"11pt\"/>\n" \
    "</style:style>\n" \
    "<style:style style:name=\"Source_20_Text\" style:display-name=\"Source Text\"\n" \
    "             style:family=\"text\">\n" \
    "   <style:text-properties style:font-name=\"Courier New\" style:font-name-asian=\"Courier New\"\n" \
    "                          style:font-name-complex=\"Courier New\"\n" \
    "                          fo:font-size=\"11pt\"/>\n" \
    "</style:style>\n" \
    "<style:style style:name=\"List\" style:family=\"paragraph\"\n" \
    "             style:parent-style-name=\"Standard\"\n" \
    "             style:class=\"list\">\n" \
    "   <style:paragraph-properties fo:text-align=\"start\" style:justify-single-word=\"false\"/>\n" \
    "   <style:text-properties style:font-size-asian=\"12pt\"/>\n" \
    "</style:style>\n" \
    "<style:style style:name=\"Quotations\" style:family=\"paragraph\"\n" \
    "             style:parent-style-name=\"Standard\"\n" \
    "             style:class=\"html\">\n" \
    "   <style:paragraph-properties fo:margin-left=\"0.3937in\" fo:margin-right=\"0.3937in\" fo:margin-top=\"0in\"\n" \
    "                               fo:margin-bottom=\"0.1965in\"\n" \
    "                               fo:text-align=\"justify\"" \
    "                               style:justify-single-word=\"false\"" \
    "                               fo:text-indent=\"0in\"\n" \
    "                               style:auto-text-indent=\"false\"/>\n" \
    "</style:style>\n" \
    "<style:style style:name=\"Table_20_Heading\" style:display-name=\"Table Heading\"\n" \
    "             style:family=\"paragraph\"\n" \
    "             style:parent-style-name=\"Table_20_Contents\"\n" \
    "             style:class=\"extra\">\n" \
    "   <style:paragraph-properties fo:text-align=\"center\" style:justify-single-word=\"false\"\n" \
    "                               text:number-lines=\"false\"\n" \
    "                               text:line-number=\"0\"/>\n" \
    "   <style:text-properties fo:font-weight=\"bold\" style:font-weight-asian=\"bold\"\n" \
    "                          style:font-weight-complex=\"bold\"/>\n" \
    "</style:style>\n" \
    "<style:style style:name=\"Horizontal_20_Line\" style:display-name=\"Horizontal Line\"\n" \
    "             style:family=\"paragraph\"\n" \
    "             style:parent-style-name=\"Standard\"\n" \
    "             style:class=\"html\">\n" \
    "   <style:paragraph-properties fo:margin-top=\"0in\" fo:margin-bottom=\"0.1965in\"\n" \
    "                               style:border-line-width-bottom=\"0.0008in 0.0138in 0.0008in\"\n" \
    "                               fo:padding=\"0in\"\n" \
    "                               fo:border-left=\"none\"\n" \
    "                               fo:border-right=\"none\"\n" \
    "                               fo:border-top=\"none\"\n" \
    "                               fo:border-bottom=\"0.0154in double #808080\"\n" \
    "                               text:number-lines=\"false\"\n" \
    "                               text:line-number=\"0\"\n" \
    "                               style:join-border=\"false\"/>\n" \
    "   <style:text-properties fo:font-size=\"6pt\" style:font-size-asian=\"6pt\" style:font-size-complex=\"6pt\"/>\n" \
    "</style:style>\n" \
	"<style:style style:name=\"Footnote_20_anchor\" style:display-name=\"Footnote anchor\"" \
	"              style:family=\"text\">" \
	"    <style:text-properties style:text-position=\"super 58%\"/>" \
	" </style:style>\n" \
	"  <text:notes-configuration text:note-class=\"footnote\" text:default-style-name=\"Footnote\" text:citation-style-name=\"Footnote_20_Symbol\" text:citation-body-style-name=\"Footnote_20_anchor\" text:master-page-name=\"Footnote\" style:num-format=\"a\" text:start-value=\"0\" text:footnotes-position=\"page\" text:start-numbering-at=\"page\"/>\n" \
	"  <text:notes-configuration text:note-class=\"endnote\" text:default-style-name=\"Endnote\" text:citation-style-name=\"Endnote_20_Symbol\" text:citation-body-style-name=\"Endnote_20_anchor\" text:master-page-name=\"Endnote\" style:num-format=\"1\" text:start-value=\"0\"/>\n" \
    "</office:styles>\n");

    /* Automatic style information */
    print_const("<office:automatic-styles>" \
    "   <style:style style:name=\"MMD-Italic\" style:family=\"text\">\n" \
    "      <style:text-properties fo:font-style=\"italic\" style:font-style-asian=\"italic\"\n" \
    "                             style:font-style-complex=\"italic\"/>\n" \
    "   </style:style>\n" \
    "   <style:style style:name=\"MMD-Bold\" style:family=\"text\">\n" \
    "      <style:text-properties fo:font-weight=\"bold\" style:font-weight-asian=\"bold\"\n" \
    "                             style:font-weight-complex=\"bold\"/>\n" \
    "   </style:style>\n" \
    "   <style:style style:name=\"MMD-Superscript\" style:family=\"text\">\n" \
    "      <style:text-properties style:text-position=\"super 58%\"/>\n" \
    "   </style:style>\n" \
    "   <style:style style:name=\"MMD-Subscript\" style:family=\"text\">\n" \
    "      <style:text-properties style:text-position=\"sub 58%\"/>\n" \
    "   </style:style>\n" \
    "   <style:style style:name=\"Strike\" style:family=\"text\">\n" \
    "      <style:text-properties style:text-line-through-style=\"solid\" />\n" \
    "   </style:style>\n" \
    "   <style:style style:name=\"Underline\" style:family=\"text\">\n" \
    "      <style:text-properties style:text-underline-style=\"solid\" style:text-underline-color=\"font-color\"/>\n" \
    "   </style:style>\n" \
    "   <style:style style:name=\"Highlight\" style:family=\"text\">\n" \
    "      <style:text-properties fo:background-color=\"#FFFF00\" />\n" \
    "   </style:style>\n" \
    "   <style:style style:name=\"Comment\" style:family=\"text\">\n" \
    "      <style:text-properties fo:color=\"#0000BB\" />\n" \
    "   </style:style>\n" \
    "<style:style style:name=\"MMD-Table\" style:family=\"paragraph\" style:parent-style-name=\"Standard\">\n" \
    "   <style:paragraph-properties fo:margin-top=\"0in\" fo:margin-bottom=\"0.05in\"/>\n" \
    "</style:style>\n" \
    "<style:style style:name=\"MMD-Table-Center\" style:family=\"paragraph\" style:parent-style-name=\"MMD-Table\">\n" \
    "   <style:paragraph-properties fo:text-align=\"center\" style:justify-single-word=\"false\"/>\n" \
    "</style:style>\n" \
    "<style:style style:name=\"MMD-Table-Right\" style:family=\"paragraph\" style:parent-style-name=\"MMD-Table\">\n" \
    "   <style:paragraph-properties fo:text-align=\"right\" style:justify-single-word=\"false\"/>\n" \
    "</style:style>\n" \
    "<style:style style:name=\"P2\" style:family=\"paragraph\" style:parent-style-name=\"Standard\"\n" \
    "             style:list-style-name=\"L2\">\n" \
    "<style:paragraph-properties fo:text-align=\"start\" style:justify-single-word=\"false\"/>\n" \
    "</style:style>\n" \
	"<style:style style:name=\"fr1\" style:family=\"graphic\" style:parent-style-name=\"Frame\">\n" \
	"   <style:graphic-properties style:print-content=\"true\" style:vertical-pos=\"top\"\n" \
	"                             style:vertical-rel=\"baseline\"\n" \
	"                             fo:padding=\"0in\"\n" \
	"                             fo:border=\"none\"\n" \
	"                             style:shadow=\"none\"/>\n" \
	"</style:style>\n" \
    "<style:style style:name=\"P1\" style:family=\"paragraph\" style:parent-style-name=\"Standard\"\n" \
    "             style:list-style-name=\"L1\"/>\n" \
	"<text:list-style style:name=\"L1\">\n" \
	"	<text:list-level-style-bullet text:level=\"1\" text:style-name=\"Numbering_20_Symbols\" style:num-suffix=\".\" text:bullet-char=\"•\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"0.5in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"0.5in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-bullet>\n" \
	"	<text:list-level-style-bullet text:level=\"2\" text:style-name=\"Numbering_20_Symbols\" style:num-suffix=\".\" text:bullet-char=\"◦\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"0.75in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"0.75in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-bullet>\n" \
	"	<text:list-level-style-bullet text:level=\"3\" text:style-name=\"Numbering_20_Symbols\" style:num-suffix=\".\" text:bullet-char=\"▪\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"1in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"1in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-bullet>\n" \
	"	<text:list-level-style-number text:level=\"4\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"1.25in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"1.25in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"5\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"1.5in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"1.5in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"6\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"1.75in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"1.75in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"7\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"2in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"2in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"8\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"2.25in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"2.25in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"9\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"2.5in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"2.5in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"10\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"2.75in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"2.75in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"</text:list-style>\n" \
	"<text:list-style style:name=\"L2\">\n" \
	"	<text:list-level-style-number text:level=\"1\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"0.5in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"0.5in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"2\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"0.75in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"0.75in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"3\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"1in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"1in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"4\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"1.25in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"1.25in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"5\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"1.5in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"1.5in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"6\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"1.75in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"1.75in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"7\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"2in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"2in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"8\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"2.25in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"2.25in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"9\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"2.5in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"2.5in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"	<text:list-level-style-number text:level=\"10\" text:style-name=\"Standard\" style:num-suffix=\".\" style:num-format=\"1\">\n" \
	"		<style:list-level-properties text:list-level-position-and-space-mode=\"label-alignment\">\n" \
	"			<style:list-level-label-alignment text:label-followed-by=\"listtab\" text:list-tab-stop-position=\"2.75in\" fo:text-indent=\"-0.25in\" fo:margin-left=\"2.75in\"/>\n" \
	"		</style:list-level-properties>\n" \
	"	</text:list-level-style-number>\n" \
	"</text:list-style>\n" \
    "</office:automatic-styles>\n" \
	" <office:master-styles>\n" \
	"  <style:master-page style:name=\"Endnote\" >\n" \
	"    <style:header><text:h text:outline-level=\"2\">Bibliography</text:h></style:header></style:master-page>\n" \
	"  <style:master-page style:name=\"Footnote\" style:page-layout-name=\"pm2\"/>\n" \
	" </office:master-styles>\n");

		// Iterate over metadata keys
	meta * m;

	if (scratch->meta_hash)
		print_const("<office:meta>\n");

	for (m = scratch->meta_hash; m != NULL; m = m->hh.next) {
		if (strcmp(m->key, "baseheaderlevel") == 0) {
		} else if (strcmp(m->key, "bibtex") == 0) {
		} else if (strcmp(m->key, "css") == 0) {
		} else if (strcmp(m->key, "htmlfooter") == 0) {
		} else if (strcmp(m->key, "htmlheader") == 0) {
		} else if (strcmp(m->key, "htmlheaderlevel") == 0) {
		} else if (strcmp(m->key, "lang") == 0) {
		} else if (strcmp(m->key, "language") == 0) {
		} else if (strcmp(m->key, "latexbegin") == 0) {
		} else if (strcmp(m->key, "latexconfig") == 0) {
		} else if (strcmp(m->key, "latexfooter") == 0) {
		} else if (strcmp(m->key, "latexheaderlevel") == 0) {
		} else if (strcmp(m->key, "latexinput") == 0) {
		} else if (strcmp(m->key, "latexleader") == 0) {
		} else if (strcmp(m->key, "latexmode") == 0) {
		} else if (strcmp(m->key, "mmdfooter") == 0) {
		} else if (strcmp(m->key, "mmdheader") == 0) {
		} else if (strcmp(m->key, "quoteslanguage") == 0) {
		} else if (strcmp(m->key, "title") == 0) {
			print_const("\t<dc:title>");
			mmd_print_string_odf(out, m->value);
			print_const("</dc:title>\n");
		} else if (strcmp(m->key, "transcludebase") == 0) {
		} else if (strcmp(m->key, "xhtmlheader") == 0) {
			print(m->value);
			print_char('\n');
		} else if (strcmp(m->key, "xhtmlheaderlevel") == 0) {
		} else {
			print_const("\t<meta:user-defined meta:name=\"");
			mmd_print_string_odf(out, m->key);
			print_const("\">");
			mmd_print_string_odf(out, m->value);
			print_const("</meta:user-defined>\n");
		}
	}

	if (scratch->meta_hash)
		print_const("</office:meta>\n");

	print_const("<office:body>\n<office:text>\n");
}


void mmd_end_complete_odf(DString * out, const char * source, scratch_pad * scratch) {
	pad(out, 1, scratch);
	print_const("</office:text>\n</office:body>\n</office:document>");
}

