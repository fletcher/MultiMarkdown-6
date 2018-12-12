/**

MultiMarkdown 6 -- Lightweight markup processor to produce HTML, LaTeX, and more.

@file rtf.c

@brief Convert token tree to RTF output.


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
#include "rtf.h"
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
#define print_localized(x) mmd_print_localized_char_rtf(out, x, scratch)


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


void mmd_print_localized_char_rtf(DString * out, unsigned short type, scratch_pad * scratch) {
	switch (type) {
		case DASH_N:
			print_const("\\'96");
			break;

		case DASH_M:
			print_const("\\'97");
			break;

		case ELLIPSIS:
			print_const("\\'85");
			break;

		case APOSTROPHE:
			print_const("\\'92");
			break;

		case QUOTE_LEFT_SINGLE:
			switch (scratch->quotes_lang) {
				case SWEDISH:
					print( "\\'92");
					break;

				case FRENCH:
					print_const("'");
					break;

				case GERMAN:
					print_const("\\'91");
					break;

				case GERMANGUILL:
					print_const("\\'9b");
					break;

				default:
					print_const("\\'91");
			}

			break;

		case QUOTE_RIGHT_SINGLE:
			switch (scratch->quotes_lang) {
				case GERMAN:
					print_const("\\'92");
					break;

				case GERMANGUILL:
					print_const("\\'8b");
					break;

				default:
					print_const("\\'92");
			}

			break;

		case QUOTE_LEFT_DOUBLE:
			switch (scratch->quotes_lang) {
				case DUTCH:
				case GERMAN:
					print_const("\\'93");
					break;

				case GERMANGUILL:
					print_const("\\'bb");
					break;

				case FRENCH:
				case SPANISH:
					print_const("\\'ab");
					break;

				case SWEDISH:
					print( "\\'94");
					break;

				default:
					print_const("\\'93");
			}

			break;

		case QUOTE_RIGHT_DOUBLE:
			switch (scratch->quotes_lang) {
				case GERMAN:
					print_const("\\'94");
					break;

				case GERMANGUILL:
					print_const("\\'ab");
					break;

				case FRENCH:
				case SPANISH:
					print_const("\\'bb");
					break;

				case SWEDISH:
				case DUTCH:
				default:
					print_const("\\'94");
			}

			break;
	}
}


static void mmd_export_token_tree_rtf(DString * out, const char * source, token * t, scratch_pad * scratch);


static void mmd_export_token_rtf(DString * out, const char * source, token * t, scratch_pad * scratch) {
	switch (t->type) {
		case AMPERSAND:
		case AMPERSAND_LONG:
			print_const("&");
			break;

		case ANGLE_LEFT:
			print_const("<");
			break;

		case ANGLE_RIGHT:
			print_const(">");
			break;

		case APOSTROPHE:
			if (!(scratch->extensions & EXT_SMART)) {
				print_token(t);
			} else {
				print_localized(APOSTROPHE);
			}

			break;

		case DOC_START_TOKEN:
			print_const("{\\rtf1\\ansi\\ansicpg1252\\cocoartf1504\\cocoasubrtf830\n{\\fonttbl\\f0\\fswiss\\fcharset0 Helvetica;}\n{\\colortbl;\\red255\\green255\\blue255;\\red191\\green191\\blue191;}\n{\\*\\expandedcolortbl;;\\csgray\\c79525;}\n{\\*\\listtable{\\list\\listtemplateid1\\listhybrid{\\listlevel\\levelnfc23\\levelnfcn23\\leveljc0\\leveljcn0\\levelfollow0\\levelstartat1\\levelspace360\\levelindent0{\\*\\levelmarker \\{disc\\}}{\\leveltext\\leveltemplateid1\\'01\\uc0\\u8226 ;}{\\levelnumbers;}\\fi-360\\li720\\lin720 }{\\listname ;}\\listid1}}\n{\\*\\listoverridetable{\\listoverride\\listid1\\listoverridecount0\\ls1}}\n\\margl1440\\margr1440\\vieww10800\\viewh8400\\viewkind0\n\\pard\\tx720\\tx1440\\tx2160\\tx2880\\tx3600\\tx4320\\tx5040\\tx5760\\tx6480\\tx7200\\tx7920\\tx8640\\pardirnatural\\partightenfactor0\\sa360\n\n\\f0\\fs24 \\cf0 ");

			if (t->child) {
				mmd_export_token_tree_rtf(out, source, t->child, scratch);
			}

			print_const("}");
			break;

		case BLOCK_EMPTY:
			print_const("\\\n");

			if (t->child) {
				mmd_export_token_tree_rtf(out, source, t->child, scratch);
			}

			break;

		case BLOCK_H1:
			print_const("\\pard\\tx720\\tx1440\\tx2160\\tx2880\\tx3600\\tx4320\\tx5040\\tx5760\\tx6480\\tx7200\\tx7920\\tx8640\\pardirnatural\\partightenfactor0\n\n\\b\\fs36 \\cf0 ");

			if (t->child) {
				mmd_export_token_tree_rtf(out, source, t->child, scratch);
			}

			print_const("\n\\b0\\fs24 \\\n");
			break;

		case BLOCK_H2:
			print_const("\n\\b\\fs32 ");

			if (t->child) {
				mmd_export_token_tree_rtf(out, source, t->child, scratch);
			}

			print_const("\n\\b0\\fs24 \\\n");
			break;

		case BLOCK_H3:
			print_const("\n\\b\\fs28 ");

			if (t->child) {
				mmd_export_token_tree_rtf(out, source, t->child, scratch);
			}

			print_const("\n\\b0\\fs24 \\\n");
			break;

		case BLOCK_H4:
			print_const("\n\\i\\b\\fs26 ");

			if (t->child) {
				mmd_export_token_tree_rtf(out, source, t->child, scratch);
			}

			print_const("\n\\i0\\b0\\fs24 \\\n");
			break;

		case BLOCK_H5:
			print_const("\\pard\\tx720\\tx1440\\tx2160\\tx2880\\tx3600\\tx4320\\tx5040\\tx5760\\tx6480\\tx7200\\tx7920\\tx8640\\pardirnatural\\partightenfactor0\n\n\\b \\cf0 ");

			if (t->child) {
				mmd_export_token_tree_rtf(out, source, t->child, scratch);
			}

			print_const("\n\\b0 \\\n");
			break;

		case BLOCK_H6:
			print_const("\\pard\\tx720\\tx1440\\tx2160\\tx2880\\tx3600\\tx4320\\tx5040\\tx5760\\tx6480\\tx7200\\tx7920\\tx8640\\pardirnatural\\partightenfactor0\n\n\\i\\b \\cf0 ");

			if (t->child) {
				mmd_export_token_tree_rtf(out, source, t->child, scratch);
			}

			print_const("\n\\i0\\b0 \\\n");
			break;

		case BLOCK_LIST_BULLETED_LOOSE:
			print_const("\\pard\\tx220\\tx720\\tx1440\\tx2160\\tx2880\\tx3600\\tx4320\\tx5040\\tx5760\\tx6480\\tx7200\\tx7920\\tx8640\\li720\\fi-720\\pardirnatural\\partightenfactor0\n\\ls1\\ilvl0\\cf0 ");

			if (t->child) {
				mmd_export_token_tree_rtf(out, source, t->child, scratch);
			}

			print_const("\\pard\\tx720\\tx1440\\tx2160\\tx2880\\tx3600\\tx4320\\tx5040\\tx5760\\tx6480\\tx7200\\tx7920\\tx8640\\pardirnatural\\partightenfactor0\n\\cf0 ");
			break;

		case BLOCK_LIST_ITEM:
			print_const("{\\listtext\t\\'95\t}");

			if (t->child) {
				mmd_export_token_tree_rtf(out, source, t->child, scratch);
			}

			break;

		case BLOCK_META:
			break;

		case BLOCK_TABLE:
			print_const("\n\\itap1\\trowd \\taflags1 \\trgaph108\\trleft-108 \\trbrdrt\\brdrnil \\trbrdrl\\brdrnil \\trbrdrr\\brdrnil \n\\clvertalc \\clshdrawnil \\clbrdrt\\brdrs\\brdrw20\\brdrcf2 \\clbrdrl\\brdrs\\brdrw20\\brdrcf2 \\clbrdrb\\brdrs\\brdrw20\\brdrcf2 \\clbrdrr\\brdrs\\brdrw20\\brdrcf2 \\clpadl100 \\clpadr100 \\gaph\\cellx4320\n\\clvertalc \\clshdrawnil \\clbrdrt\\brdrs\\brdrw20\\brdrcf2 \\clbrdrl\\brdrs\\brdrw20\\brdrcf2 \\clbrdrb\\brdrs\\brdrw20\\brdrcf2 \\clbrdrr\\brdrs\\brdrw20\\brdrcf2 \\clpadl100 \\clpadr100 \\gaph\\cel");

			if (t->child) {
				mmd_export_token_tree_rtf(out, source, t->child, scratch);
			}

			break;

		case BLOCK_TABLE_SECTION:
			print_const("bl\\itap1\\tx720\\tx1440\\tx2160\\tx2880\\tx3600\\tx4320\\tx5040\\tx5760\\tx6480\\tx7200\\tx7920\\tx8640\\pardirnatural\\partightenfactor0\n\\cf0 musical\\cell \n\\pard\\intbl\\itap1\\tx720\\tx1440\\tx2160\\tx2880\\tx3600\\tx4320\\tx5040\\tx5760\\tx6480\\tx7200\\tx7920\\tx8640\\pardirnatural\\partightenfactor0\n\\cf0 suspicion\\cell \\row\n\n\\itap1\\trowd \\taflags1 \\trgaph108\\trleft-108 \\trbrdrl\\brdrnil \\trbrdrt\\brdrnil \\trbrdrr\\brdrnil \n\\clvertalc \\clshdrawnil \\clbrdrt\\brdrs\\brdrw20\\brdrcf2 \\clbrdrl\\brdrs\\brdrw");

			if (t->child) {
				mmd_export_token_tree_rtf(out, source, t->child, scratch);
			}

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

		case ELLIPSIS:
			if (!(scratch->extensions & EXT_SMART)) {
				print_token(t);
			} else {
				print_localized(ELLIPSIS);
			}

			break;

		case EMPH_START:
			print_const("\\i ");
			break;

		case EMPH_STOP:
			print_const("\\i0 ");
			break;

		case EQUAL:
			print_const("=");
			break;

		case NON_INDENT_SPACE:
			print_char(' ');
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
			mmd_export_token_tree_rtf(out, source, t->child, scratch);
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
				print_const("\"");
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
			print_const("\\b ");
			break;

		case STRONG_STOP:
			print_const("\\b0 ");
			break;

		case TEXT_NL:
			if (t->next) {
				print_char('\n');
			}

			break;

		case CODE_FENCE:
		case TEXT_EMPTY:
		case MANUAL_LABEL:
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
		case TOC_SINGLE:
		case TOC_RANGE:
			print_token(t);
			break;

		default:
			if (t->type != 77) {
				fprintf(stderr, "Unknown token type: %d (%lu:%lu)\n", t->type, t->start, t->len);
			}

			if (t->child) {
				mmd_export_token_tree_rtf(out, source, t->child, scratch);
			} else {
				//	print_token(t);
			}

			break;
	}
}



static void mmd_export_token_tree_rtf(DString * out, const char * source, token * t, scratch_pad * scratch) {

	// Prevent stack overflow with "dangerous" input causing extreme recursion
	if (scratch->recurse_depth == kMaxExportRecursiveDepth) {
		return;
	}

	scratch->recurse_depth++;

	while (t != NULL) {
		if (scratch->skip_token) {
			scratch->skip_token--;
		} else {
			mmd_export_token_rtf(out, source, t, scratch);
		}

		t = t->next;
	}

	scratch->recurse_depth--;
}


void mmd_export_rtf(DString * out, const char * source, token * t, scratch_pad * scratch) {
	mmd_export_token_tree_rtf(out, source, t, scratch);
}



