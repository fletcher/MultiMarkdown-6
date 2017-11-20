/**

	MultiMarkdown 6 -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file latex.c

	@brief Convert token tree to LaTeX output.


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
#include "i18n.h"
#include "latex.h"
#include "parser.h"
#include "scanners.h"

#define print(x) d_string_append(out, x)
#define print_const(x) d_string_append_c_array(out, x, sizeof(x) - 1)
#define print_char(x) d_string_append_c(out, x)
#define printf(...) d_string_append_printf(out, __VA_ARGS__)
#define print_token(t) d_string_append_c_array(out, &(source[t->start]), t->len)
#define print_localized(x) mmd_print_localized_char_latex(out, x, scratch)


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


void mmd_print_char_latex(DString * out, char c) {
	switch (c) {
		case '\\':
			print_const("\\textbackslash{}");
			break;

		case '~':
			print_const("\\ensuremath{\\sim}");
			break;

		case '/':
			print_const("\\slash ");
			break;

		case '^':
			print_const("\\^{}");
			break;

		case '<':
		case '>':
			print_char('$');
			print_char(c);
			print_char('$');
			break;

		case '|':
			print_const("\\textbar{}");
			break;

		case '#':
		case '{':
		case '}':
		case '$':
		case '%':
		case '&':
		case '_':
			print_char('\\');

		default:
			print_char(c);
			break;
	}
}


void mmd_print_string_latex(DString * out, const char * str) {
	if (str == NULL) {
		return;
	}

	while (*str != '\0') {
		mmd_print_char_latex(out, *str);
		str++;
	}
}


void mmd_print_localized_char_latex(DString * out, unsigned short type, scratch_pad * scratch) {
	switch (type) {
		case DASH_N:
			print_const("--");
			break;

		case DASH_M:
			print_const("---");
			break;

		case ELLIPSIS:
			print_const("{\\ldots}");
			break;

		case APOSTROPHE:
			print_const("'");
			break;

		case QUOTE_LEFT_SINGLE:
			switch (scratch->quotes_lang) {
				case SWEDISH:
					print_const("'");
					break;

				case FRENCH:
					print_const("'");
					break;

				case GERMAN:
					print_const("‚");
					break;

				case GERMANGUILL:
					print_const("›");
					break;

				default:
					print_const("`");
			}

			break;

		case QUOTE_RIGHT_SINGLE:
			switch (scratch->quotes_lang) {
				case GERMAN:
					print_const("`");
					break;

				case GERMANGUILL:
					print_const("‹");
					break;

				default:
					print_const("'");
			}

			break;

		case QUOTE_LEFT_DOUBLE:
			switch (scratch->quotes_lang) {
				case DUTCH:
				case GERMAN:
					print_const("„");
					break;

				case GERMANGUILL:
					print_const("»");
					break;

				case FRENCH:
					print_const("«");
					break;

				case SWEDISH:
					print_const("''");
					break;

				default:
					print_const("``");
			}

			break;

		case QUOTE_RIGHT_DOUBLE:
			switch (scratch->quotes_lang) {
				case GERMAN:
					print_const("``");
					break;

				case GERMANGUILL:
					print_const("«");
					break;

				case FRENCH:
					print_const("»");
					break;

				case SWEDISH:
				case DUTCH:
				default:
					print_const("''");
			}

			break;
	}
}


void mmd_export_link_latex(DString * out, const char * source, token * text, link * link, scratch_pad * scratch) {
	char * temp_char;

	if (link->url) {
		if (link->url[0] == '#') {
			// Internal link
			if (text && text->child) {
				temp_char = label_from_token(source, text);

				if (strcmp(temp_char, &(link->url[1])) == 0) {
					// [bar][bar] or [bar](#bar) or [bar]
					printf("\\autoref{%s}", &(link->url)[1]);
				} else {
					mmd_export_token_tree_latex(out, source, text->child, scratch);
					print_const(" (");
					printf("\\autoref{%s}", &(link->url)[1]);
					print_const(")");
				}

				free(temp_char);
			} else {
				printf("\\autoref{%s}", &(link->url)[1]);
			}

			return;
		} else {
			printf("\\href{%s}", link->url);
		}
	} else {
		print_const("\\href{}");
	}

	print_const("{");

	// If we're printing contents of bracket as text, then ensure we include it all
	if (text && text->child && text->child->len > 1) {
		text->child->next->start--;
		text->child->next->len++;
	}

	if (text && text->child) {
		mmd_export_token_tree_latex(out, source, text->child, scratch);
	}

	print_const("}");

	// Reprint as footnote for printed copies
	printf("\\footnote{\\href{%s}{", link->url);
	mmd_print_string_latex(out, link->url);
	print_const("}}");
}


static char * correct_dimension_units(char *original) {
	char *result;
	int i;

	result = my_strdup(original);

	for (i = 0; result[i]; i++) {
		result[i] = tolower(result[i]);
	}

	if (strstr(&result[strlen(result) - 2], "px")) {
		result[strlen(result) - 2] = '\0';
		strcat(result, "pt");
	}

	return result;
}


void mmd_export_image_latex(DString * out, const char * source, token * text, link * link, scratch_pad * scratch, bool is_figure) {
	attr * a = link->attributes;
	char * height = NULL;
	char * width = NULL;
	float temp_float;

	// Compatibility mode doesn't allow figures
	if (scratch->extensions & EXT_COMPATIBILITY) {
		is_figure = false;
	}

	if (is_figure) {
		print_const("\\begin{figure}[htbp]\n\\centering\n");
		scratch->close_para = false;
	}

	// Check attributes for dimensions
	while (a) {
		if (strcmp("height", a->key) == 0) {
			height = correct_dimension_units(a->value);
		} else if (strcmp("width", a->key) == 0) {
			width = correct_dimension_units(a->value);
		}

		a = a->next;
	}

	print_const("\\includegraphics[");

	if (height || width) {
		if (!height || !width) {
			// One not specified, preserve aspect
			print_const("keepaspectratio,");
		}

		if (width) {
			// Width specified
			if (width[strlen(width) - 1] == '%') {
				// specified as percent
				width[strlen(width) - 1] = '\0';
				temp_float = strtod(width, NULL);
				temp_float = temp_float / 100.0f;
				printf("width=%.4f\\textwidth,", temp_float);
			} else {
				printf("width=%s,", width);
			}

			free(width);
		} else {
			// Default width
			print_const("width=\\textwidth,");
		}

		if (height) {
			// Height specified
			if (height[strlen(height) - 1] == '%') {
				// specified as percent
				height[strlen(height) - 1] = '\0';
				temp_float = strtod(height, NULL);
				temp_float = temp_float / 100.0f;
				printf("height=%.4f\\textheight", temp_float);
			} else {
				printf("height=%s", height);
			}

			free(height);
		} else {
			// Default height
			print_const("height=0.75\\textheight");
		}
	} else {
		// no dimensions specified, use sensible defaults
		print_const("keepaspectratio,width=\\textwidth,height=0.75\\textheight");
	}

	if (link->url) {
		printf("]{%s}", link->url);
	} else {
		print_const("]{}");
	}


	if (is_figure) {
		print_const("\n");

		if (text) {
			print_const("\\caption{");
			mmd_export_token_tree_latex(out, source, text->child, scratch);
			print_const("}\n");
		}

		if (link->label) {
			// \todo: Need to decide on approach to id's
			char * label = label_from_token(source, link->label);
			printf("\\label{%s}\n", label);
			free(label);
		}

		print_const("\\end{figure}");
	}
}


void mmd_export_toc_entry_latex(DString * out, const char * source, scratch_pad * scratch, size_t * counter, short level) {
	token * entry, * next;
	short entry_level, next_level;
	char * temp_char;

	print_const("\\begin{itemize}\n\n");

	// Iterate over tokens
	while (*counter < scratch->header_stack->size) {
		// Get token for header
		entry = stack_peek_index(scratch->header_stack, *counter);
		entry_level = raw_level_for_header(entry);

		if (entry_level >= level) {
			// This entry is a direct descendant of the parent
			temp_char = label_from_header(source, entry);
			print_const("\\item{} ");
			mmd_export_token_tree_latex(out, source, entry->child, scratch);
			printf("(\\autoref{%s})\n\n", temp_char);

			if (*counter < scratch->header_stack->size - 1) {
				next = stack_peek_index(scratch->header_stack, *counter + 1);
				next_level = next->type - BLOCK_H1 + 1;

				if (next_level > entry_level) {
					// This entry has children
					(*counter)++;
					mmd_export_toc_entry_latex(out, source, scratch, counter, entry_level + 1);
				}
			}

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

	print_const("\\end{itemize}\n\n");
}


void mmd_export_toc_latex(DString * out, const char * source, scratch_pad * scratch) {
	size_t counter = 0;

	mmd_export_toc_entry_latex(out, source, scratch, &counter, 0);
}


void mmd_export_token_latex(DString * out, const char * source, token * t, scratch_pad * scratch) {
	if (t == NULL) {
		return;
	}

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
		case AMPERSAND:
		case AMPERSAND_LONG:
			print_const("\\&");
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

		case BACKTICK:
			print_token(t);
			break;

		case BLOCK_BLOCKQUOTE:
			pad(out, 2, scratch);
			print_const("\\begin{quote}\n");
			scratch->padded = 2;
			mmd_export_token_tree_latex(out, source, t->child, scratch);
			pad(out, 1, scratch);
			print_const("\\end{quote}");
			scratch->padded = 0;
			break;

		case BLOCK_CODE_FENCED:
			pad(out, 2, scratch);

			temp_char = get_fence_language_specifier(t->child->child, source);

			if (temp_char) {
				if (strncmp("{=", temp_char, 2) == 0) {
					// Raw source
					if (raw_filter_text_matches(temp_char, FORMAT_LATEX)) {
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

				printf("\\begin{lstlisting}[language=%s]\n", temp_char);
			} else {
				print_const("\\begin{verbatim}\n");
			}

			mmd_export_token_tree_latex_raw(out, source, t->child->next, scratch);

			if (temp_char) {
				print_const("\\end{lstlisting}");
				free(temp_char);
			} else {
				print_const("\\end{verbatim}");
			}

			scratch->padded = 0;
			break;

		case BLOCK_CODE_INDENTED:
			pad(out, 2, scratch);
			print_const("\\begin{verbatim}\n");
			mmd_export_token_tree_latex_raw(out, source, t->child, scratch);
			print_const("\\end{verbatim}");
			scratch->padded = 0;
			break;

		case BLOCK_DEFINITION:
			pad(out, 2, scratch);

			temp_short = scratch->list_is_tight;

			if (!(t->child->next && (t->child->next->type == BLOCK_EMPTY) && t->child->next->next)) {
				scratch->list_is_tight = true;
			}

			mmd_export_token_tree_latex(out, source, t->child, scratch);
			scratch->padded = 0;

			scratch->list_is_tight = temp_short;
			break;

		case BLOCK_DEFLIST:
			pad(out, 2, scratch);

			// Group consecutive definition lists into a single list.
			// lemon's LALR(1) parser can't properly handle this (to my understanding).

			if (!(t->prev && (t->prev->type == BLOCK_DEFLIST))) {
				print_const("\\begin{description}\n");
			}

			scratch->padded = 2;

			mmd_export_token_tree_latex(out, source, t->child, scratch);
			pad(out, 1, scratch);

			if (!(t->next && (t->next->type == BLOCK_DEFLIST))) {
				print_const("\\end{description}\n");
			}

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

			switch (temp_short + scratch->base_header_level - 1) {
				case 1:
					print_const("\\part{");
					break;

				case 2:
					print_const("\\chapter{");
					break;

				case 3:
					print_const("\\section{");
					break;

				case 4:
					print_const("\\subsection{");
					break;

				case 5:
					print_const("\\subsubsection{");
					break;

				case 6:
					print_const("\\paragraph{");
					break;

				case 7:
					print_const("\\subparagraph{");
					break;
			}

			mmd_export_token_tree_latex(out, source, t->child, scratch);

			if (scratch->extensions & EXT_NO_LABELS) {
				print_const("}");
			} else {
				temp_token = manual_label_from_header(t, source);

				if (temp_token) {
					temp_char = label_from_token(source, temp_token);
				} else {
					temp_char = label_from_token(source, t);
				}

				printf("}\n\\label{%s}", temp_char);
				free(temp_char);
			}

			scratch->padded = 0;
			break;

		case BLOCK_HR:
			pad(out, 2, scratch);
			print_const("\\begin{center}\\rule{3in}{0.4pt}\\end{center}");
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
			print_const("\\begin{itemize}");
			scratch->padded = 1;
			mmd_export_token_tree_latex(out, source, t->child, scratch);
			pad(out, 2, scratch);
			print_const("\\end{itemize}");
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
			print_const("\\begin{enumerate}");
			scratch->padded = 1;
			mmd_export_token_tree_latex(out, source, t->child, scratch);
			pad(out, 2, scratch);
			print_const("\\end{enumerate}");
			scratch->padded = 0;
			scratch->list_is_tight = temp_short;
			break;

		case BLOCK_LIST_ITEM:
			pad(out, 2, scratch);
			print_const("\\item{} ");
			scratch->padded = 2;
			mmd_export_token_tree_latex(out, source, t->child, scratch);
			scratch->padded = 0;
			break;

		case BLOCK_LIST_ITEM_TIGHT:
			pad(out, 2, scratch);
			print_const("\\item{} ");
			scratch->padded = 2;
			mmd_export_token_tree_latex(out, source, t->child, scratch);
			scratch->padded = 0;
			break;

		case BLOCK_META:
			break;

		case BLOCK_PARA:
			pad(out, 2, scratch);
			mmd_export_token_tree_latex(out, source, t->child, scratch);
			scratch->padded = 0;
			break;

		case BLOCK_TABLE:
			pad(out, 2, scratch);

			print_const("\\begin{table}[htbp]\n");

			print_const("\\begin{minipage}{\\linewidth}\n\\setlength{\\tymax}{0.5\\linewidth}\n\\centering\n\\small\n");

			// Are we followed by a caption?
			if (table_has_caption(t)) {
				temp_token = t->next->child;

				if (temp_token->next &&
						temp_token->next->type == PAIR_BRACKET) {
					temp_token = temp_token->next;
				}

				temp_char = label_from_token(source, temp_token);

				t->next->child->child->type = TEXT_EMPTY;
				t->next->child->child->mate->type = TEXT_EMPTY;

				print_const("\\caption{");
				mmd_export_token_tree_latex(out, source, t->next->child->child, scratch);
				print_const("}\n");

				printf("\\label{%s}\n", temp_char);
				free(temp_char);

				temp_short = 1;
			} else {
				temp_short = 0;
			}

			read_table_column_alignments(source, t, scratch);

			mmd_export_token_tree_latex(out, source, t->child, scratch);
			pad(out, 1, scratch);
			print_const("\n\\end{tabulary}\n\\end{minipage}");

			print_const("\n\\end{table}");

			scratch->skip_token = temp_short;
			scratch->padded = 0;
			break;

		case BLOCK_TABLE_HEADER:
			pad(out, 2, scratch);

			print_const("\\begin{tabulary}{\\textwidth}{@{}");

			for (int i = 0; i < scratch->table_column_count; ++i) {
				switch (scratch->table_alignment[i]) {
					case 'l':
					case 'L':
					case 'r':
					case 'R':
					case 'c':
					case 'C':
						print_char(scratch->table_alignment[i]);
						break;

					case 'N':
						print_char('L');
						break;

					default:
						print_char('l');
						break;
				}
			}

			print_const("@{}} \\toprule\n");

			scratch->in_table_header = 1;
			mmd_export_token_tree_latex(out, source, t->child, scratch);
			scratch->in_table_header = 0;

			print_const("\\midrule\n");

			scratch->padded = 1;
			break;

		case BLOCK_TABLE_SECTION:
			pad(out, 2, scratch);
			scratch->padded = 2;
			mmd_export_token_tree_latex(out, source, t->child, scratch);
			print_const("\\bottomrule");
			scratch->padded = 0;
			break;

		case BLOCK_TERM:
			pad(out, 2, scratch);
			print_const("\\item[");
			mmd_export_token_tree_latex(out, source, t->child, scratch);
			print_const("]");
			scratch->padded = 0;
			break;

		case BLOCK_TOC:
			pad(out, 2, scratch);
			print_const("\\tableofcontents");
			scratch->padded = 0;
			break;

		case BRACE_DOUBLE_LEFT:
			print_const("\\{\\{");
			break;

		case BRACE_DOUBLE_RIGHT:
			print_const("\\}\\}");
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

		case CODE_FENCE:
			break;

		case COLON:
			print_const(":");
			break;

		case CRITIC_ADD_OPEN:
			print_const("\\{++");
			break;

		case CRITIC_ADD_CLOSE:
			print_const("++\\}");
			break;

		case CRITIC_COM_OPEN:
			print_const("\\{>>");
			break;

		case CRITIC_COM_CLOSE:
			print_const("<<\\}");
			break;

		case CRITIC_DEL_OPEN:
			print_const("\\{--");
			break;

		case CRITIC_DEL_CLOSE:
			print_const("--\\}");
			break;

		case CRITIC_HI_OPEN:
			print_const("\\{==");
			break;

		case CRITIC_HI_CLOSE:
			print_const("==\\}");
			break;

		case CRITIC_SUB_OPEN:
			print_const("\\{~~");
			break;

		case CRITIC_SUB_DIV:
			print_const("~>");
			break;

		case CRITIC_SUB_CLOSE:
			print_const("~~\\}");
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
			mmd_export_token_tree_latex(out, source, t->child, scratch);
			break;

		case ELLIPSIS:
			if (!(scratch->extensions & EXT_SMART)) {
				print_token(t);
			} else {
				print_localized(ELLIPSIS);
			}

			break;

		case EMPH_START:
			print_const("\\emph{");
			break;

		case EMPH_STOP:
			print_const("}");
			break;

		case EQUAL:
			print_const("=");
			break;

		case ESCAPED_CHARACTER:
			if (!(scratch->extensions & EXT_COMPATIBILITY) &&
					(source[t->start + 1] == ' ')) {
				print_const("~");
			} else {
				mmd_print_char_latex(out, source[t->start + 1]);
			}

			break;

		case HASH1:
		case HASH2:
		case HASH3:
		case HASH4:
		case HASH5:
		case HASH6:
			for (int i = 0; i < t->len; ++i) {
				if (source[t->start + i] == '#') {
					print_char('\\');
					print_char('#');
				} else {
					mmd_print_char_latex(out, source[t->start + i]);
				}
			}

			break;

		case HTML_ENTITY:
			if (source[t->start + 1] == '#') {
				print_const("\\&\\#");
				d_string_append_c_array(out, &(source[t->start + 2]), t->len - 2);
			} else {
				print_const("\\");
				print_token(t);
			}

			break;

		case HTML_COMMENT_START:
			if (!(scratch->extensions & EXT_SMART)) {
				print_const("<!--");
			} else {
				print_const("<!");
				print_localized(DASH_N);
			}

			break;

		case HTML_COMMENT_STOP:
			if (!(scratch->extensions & EXT_SMART)) {
				print_const("-->");
			} else {
				print_localized(DASH_N);
				print_const(">");
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
			mmd_export_token_tree_latex(out, source, t->child, scratch);
			break;

		case LINE_SETEXT_2:
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
			print_const("\\[");
			break;

		case MATH_BRACKET_CLOSE:
			print_const("\\]");
			break;

		case MATH_DOLLAR_SINGLE:
			if (t->mate) {
				print_const("$");
			} else {
				print_const("\\$");
			}

			break;

		case MATH_DOLLAR_DOUBLE:
			if (t->mate) {
				print_const("$$");
			} else {
				print_const("\\$\\$");
			}

			break;

		case MATH_PAREN_OPEN:
			print_const("\\(");
			break;

		case MATH_PAREN_CLOSE:
			print_const("\\)");
			break;

		case NON_INDENT_SPACE:
			print_char(' ');
			break;

		case PAIR_ANGLE:
			temp_char = url_accept(source, t->start + 1, t->len - 2, NULL, true);

			if (temp_char) {
				print_const("\\href{");

				if (scan_email(temp_char)) {
					if (strncmp("mailto:", temp_char, 7) != 0) {
						print_const("mailto:");
					}
				}

				print(temp_char);
				print_const("}{");
				mmd_print_string_latex(out, temp_char);
				print_const("}");
			} else if (scan_html(&source[t->start])) {
				// We ignore HTML blocks
				if (scan_html_comment(&source[t->start])) {
					// But allow HTML comments as raw LaTeX
					d_string_append_c_array(out, &source[t->start + 4], t->len - 4 - 3);
				}
			} else {
				mmd_export_token_tree_latex(out, source, t->child, scratch);
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

			if (t->next && t->next->type == PAIR_RAW_FILTER) {
				// Raw text?
				if (raw_filter_matches(t->next, source, FORMAT_LATEX)) {
					d_string_append_c_array(out, &(source[t->child->start + t->child->len]), t->child->mate->start - t->child->start - t->child->len);
				}

				// Skip over PAIR_RAW_FILTER
				scratch->skip_token = 1;
				break;
			}

			print_const("\\texttt{");
			mmd_export_token_tree_latex_tt(out, source, t->child, scratch);
			print_const("}");
			break;

		case PAIR_BRACE:
		case PAIR_BRACES:
			mmd_export_token_tree_latex(out, source, t->child, scratch);
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
					mmd_export_link_latex(out, source, t, temp_link, scratch);
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
						mmd_export_image_latex(out, source, t, temp_link, scratch, false);
					} else {
						mmd_export_image_latex(out, source, t, temp_link, scratch, true);
					}
				}

				if (temp_bool) {
					link_free(temp_link);
				}

				scratch->skip_token = temp_short;

				return;
			}

			// No links exist, so treat as normal
			mmd_export_token_tree_latex(out, source, t->child, scratch);
			break;

		case PAIR_BRACKET_ABBREVIATION:

			// Which might also be an "auto-tagged" abbreviation
			if (scratch->extensions & EXT_NOTES) {
				// Note-based syntax enabled

				// Classify this use
				temp_short3 = scratch->inline_abbreviations_to_free->size;
				abbreviation_from_bracket(source, scratch, t, &temp_short);

				if (temp_short == -1) {
					// This instance is not properly formed
					print_const("[>");
					mmd_export_token_tree_latex(out, source, t->child->next, scratch);
					print_const("]");
					break;
				}

				// Get instance of the note used
				temp_note = stack_peek_index(scratch->used_abbreviations, temp_short - 1);

				if (temp_short3 == scratch->inline_abbreviations_to_free->size) {
					// This is a reference definition
					printf("\\gls{%s}", temp_note->label_text);
				} else {
					// This is an inline definition
					print_const("\\newacronym{");
					print(temp_note->label_text);
					print_const("}{");
					print(temp_note->label_text);
					print_const("}{");
					print(temp_note->clean_text);
					print_const("}");

					printf("\\gls{%s}", temp_note->label_text);
				}
			} else {
				// Note-based syntax disabled
				mmd_export_token_tree_latex(out, source, t->child, scratch);
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
				citation_from_bracket(source, scratch, t, &temp_short);

				if (temp_short == -1) {
					// Ensure we aren't using BibTeX
					if (!scratch->bibtex_file) {
						// This instance is not properly formed
						print_const("[#");
						mmd_export_token_tree_latex(out, source, t->child->next, scratch);
						print_const("]");

						free(temp_char);
						break;
					}
				}

				// Get instance of the note used
				if (temp_short == -1) {
					temp_note = NULL;
				} else {
					temp_note = stack_peek_index(scratch->used_citations, temp_short - 1);
				}

				if (temp_bool) {
					// This is a regular citation

					// Are we citep vs citet?
					temp_char2 = clean_inside_pair(source, t, false);

					if (temp_char2[strlen(temp_char2) - 1] == ';') {
						temp_bool = true;		// citet
						temp_char2[strlen(temp_char2) - 1] = '\0';
					} else {
						temp_bool = false;		// citep
					}

					if (temp_char[0] == '\0') {
						// No locator
						if (temp_bool) {
							print_const("\\citet");
						} else {
							print_const("~\\citep");
						}
					} else {
						// Locator present

						// Are there two arguments in the locator?
						// e.g. `[foo\]\[bar]`
						temp_char3 = strstr(temp_char, "\\]\\[");

						if (temp_char3) {
							// Convert `\]\[` to `][`
							temp_char[temp_char3 - temp_char] = ']';
							memmove(temp_char3 + 1, temp_char3 + 3, strlen(temp_char3) - 2);
						}

						if (temp_bool) {
							printf("\\citet[%s]", temp_char);
						} else {
							printf("~\\citep[%s]", temp_char);
						}
					}

					if (temp_note) {
						printf("{%s}", temp_note->label_text);
					} else {
						printf("{%s}", &temp_char2[1]);
					}

					free(temp_char2);
				} else {
					// This is a "nocite"
					if (temp_note) {
						printf("~\\nocite{%s}", temp_note->label_text);
					} else {
						temp_char2 = clean_inside_pair(source, t, false);
						printf("~\\nocite{%s}", &temp_char2[1]);
						free(temp_char2);
					}
				}

				if (temp_token != t) {
					// Skip citation on next pass
					scratch->skip_token = 1;
				}

				free(temp_char);
			} else {
				// Note-based syntax disabled
				mmd_export_token_tree_latex(out, source, t->child, scratch);
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
					print_const("[?");
					mmd_export_token_tree_latex(out, source, t->child->next, scratch);
					print_const("]");
					break;
				}

				if (temp_short2 == scratch->used_footnotes->size) {
					// This is a re-use of a previously used note

					// TODO: This would work, assuming no URL's are converted to
					// footnotes without affecting the numbering.
					// Could add a NULL to the used_footnotes stack??

					// Additionally, re-using an old footnote would require flipping back
					// through the document to find it...

					// printf("\\footnotemark[%d]", temp_short);

					print_const("\\footnote{");
					temp_note = stack_peek_index(scratch->used_footnotes, temp_short - 1);

					mmd_export_token_tree_latex(out, source, temp_note->content, scratch);
					print_const("}");
				} else {
					// This is the first time this note was used
					print_const("\\footnote{");
					temp_note = stack_peek_index(scratch->used_footnotes, temp_short - 1);
					// Reset padding counter in case of multiple footnotes in single paragraph
					scratch->padded = 2;
					mmd_export_token_tree_latex(out, source, temp_note->content, scratch);
					print_const("}");
				}
			} else {
				// Note-based syntax disabled
				mmd_export_token_tree_latex(out, source, t->child, scratch);
			}

			break;

		case PAIR_BRACKET_GLOSSARY:

			// Which might also be an "auto-tagged" glossary
			if (scratch->extensions & EXT_NOTES) {
				// Note-based syntax enabled

				// Classify this use
				temp_short2 = scratch->used_glossaries->size;
				temp_short3 = scratch->inline_glossaries_to_free->size;
				glossary_from_bracket(source, scratch, t, &temp_short);

				if (temp_short == -1) {
					// This instance is not properly formed
					print_const("[?");

					if (t->child) {
						mmd_export_token_tree_latex(out, source, t->child->next, scratch);
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

					print("\\gls{");
					print(temp_note->clean_text);
					print("}");
				} else {
					// This is the first time this note was used

					if (temp_short3 == scratch->inline_glossaries_to_free->size) {
						// This is a reference definition
						print_const("\\gls{");
						print(temp_note->clean_text);
						print_const("}");
					} else {
						// This is an inline definition
						print_const("\\newglossaryentry{");
						print(temp_note->clean_text);

						print_const("}{name=");
						print(temp_note->clean_text);

						print_const(", description={");

						// We skip over temp_note->content, since that is the term in use
						mmd_export_token_tree_latex(out, source, temp_note->content, scratch);
						print_const("}}\\gls{");
						print(temp_note->clean_text);
						print_const("}");
					}
				}
			} else {
				// Note-based syntax disabled
				mmd_export_token_tree_latex(out, source, t->child, scratch);
			}

			break;

		case PAIR_BRACKET_VARIABLE:
			temp_char = text_inside_pair(source, t);
			temp_char2 = extract_metadata(scratch, temp_char);

			if (temp_char2) {
				mmd_print_string_latex(out, temp_char2);
			} else {
				mmd_export_token_tree_latex(out, source, t->child, scratch);
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
					mmd_export_token_tree_latex(out, source, t->child, scratch);
				} else {
					print_const("\\underline{");
					mmd_export_token_tree_latex(out, source, t->child, scratch);
					print_const("}");
				}
			} else {
				mmd_export_token_tree_latex(out, source, t->child, scratch);
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
					mmd_export_token_tree_latex(out, source, t->child, scratch);
				} else {
					print_const("\\sout{");
					mmd_export_token_tree_latex(out, source, t->child, scratch);
					print_const("}");
				}
			} else {
				mmd_export_token_tree_latex(out, source, t->child, scratch);
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
				print_const("\\cmnote{");
				mmd_export_token_tree_latex(out, source, t->child, scratch);
				print_const("}");
			} else {
				mmd_export_token_tree_latex(out, source, t->child, scratch);
			}

			break;

		case PAIR_CRITIC_HI:

			// Ignore if we're rejecting or accepting
			if ((scratch->extensions & EXT_CRITIC_REJECT) ||
					(scratch->extensions & EXT_CRITIC_ACCEPT)) {
				t->child->type = TEXT_EMPTY;
				t->child->mate->type = TEXT_EMPTY;
				mmd_export_token_tree_latex(out, source, t->child, scratch);
				break;
			}

			if (scratch->extensions & EXT_CRITIC) {
				t->child->type = TEXT_EMPTY;
				t->child->mate->type = TEXT_EMPTY;
				// 'hl' requires 'soul' package
				print_const("\\hl{");
				mmd_export_token_tree_latex(out, source, t->child, scratch);
				print_const("}");
			} else {
				mmd_export_token_tree_latex(out, source, t->child, scratch);
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
					mmd_export_token_tree_latex(out, source, t->child, scratch);
				} else {
					print_const("\\sout{");
					mmd_export_token_tree_latex(out, source, t->child, scratch);
					print_const("}");
				}
			} else {
				mmd_export_token_tree_latex(out, source, t->child, scratch);
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
					mmd_export_token_tree_latex(out, source, t->child, scratch);
				} else {
					print_const("\\underline{");
					mmd_export_token_tree_latex(out, source, t->child, scratch);
					print_const("}");
				}
			} else {
				mmd_export_token_tree_latex(out, source, t->child, scratch);
			}

			break;

		case PAIR_HTML_COMMENT:
			break;

		case PAIR_MATH:
			if (strncmp(&source[t->child->start + t->child->len], "\\begin", 6) != 0) {
				mmd_export_token_latex(out, source, t->child, scratch);
			}

			// Math is raw LaTeX -- use string itself rather than interior tokens
			d_string_append_c_array(out, &(source[t->child->start + t->child->len]), t->child->mate->start - t->child->start - t->child->len);

			if (strncmp(&source[t->child->start + t->child->len], "\\begin", 6) != 0) {
				mmd_export_token_latex(out, source, t->child->mate, scratch);
			}

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
			mmd_export_token_tree_latex(out, source, t->child, scratch);
			break;

		case PAREN_LEFT:
			print_const("(");
			break;

		case PAREN_RIGHT:
			print_const(")");
			break;

		case PIPE:
			for (int i = 0; i < t->len; ++i) {
				print_const("\\textbar{}");
			}

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
				print_const("''");
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
			print_const("\\slash ");
			break;

		case STAR:
			print_token(t);
			break;

		case STRONG_START:
			print_const("\\textbf{");
			break;

		case STRONG_STOP:
			print_const("}");
			break;

		case SUBSCRIPT:
			if (t->mate) {
				(t->start < t->mate->start) ? (print_const("\\textsubscript{")) : (print_const("}"));
			} else if (t->len != 1) {
				print_const("\\textsubscript{");
				mmd_export_token_latex(out, source, t->child, scratch);
				print_const("}");
			} else {
				print_const("\\ensuremath{\\sim}");
			}

			break;

		case SUPERSCRIPT:
			if (t->mate) {
				(t->start < t->mate->start) ? (print_const("\\textsuperscript{")) : (print_const("}"));
			} else if (t->len != 1) {
				print_const("\\textsuperscript{");
				mmd_export_token_latex(out, source, t->child, scratch);
				print_const("}");
			} else {
				print_const("\\^{}");
			}

			break;

		case TABLE_CELL:
			if (t->next && t->next->type == TABLE_DIVIDER) {
				if (t->next->len > 1) {
					printf("\\multicolumn{%lu}{", t->next->len);

					switch (scratch->table_alignment[scratch->table_cell_count]) {
						case 'l':
						case 'L':
							print_const("l}{");
							break;

						case 'r':
						case 'R':
							print_const("r}{");
							break;

						default:
							print_const("c}{");
							break;
					}
				}
			}

			mmd_export_token_tree_latex(out, source, t->child, scratch);

			if (t->next && t->next->type == TABLE_DIVIDER) {
				if (t->next->len > 1) {
					print_const("}");
				}
			}

			if (t->next && t->next->type == TABLE_DIVIDER) {
				t = t->next;

				if (t->next && t->next->type == TABLE_CELL) {
					print_const("&");
					scratch->table_cell_count += t->next->len;
				}
			} else {
				scratch->table_cell_count++;
			}

			break;

		case TABLE_DIVIDER:
			break;

		case TABLE_ROW:
			scratch->table_cell_count = 0;
			mmd_export_token_tree_latex(out, source, t->child, scratch);
			print_const("\\\\\n");
			break;

		case TEXT_BACKSLASH:
			print_const("\\textbackslash{}");
			break;

		case TEXT_EMPTY:
			break;

		case TEXT_HASH:
			print_const("\\#");
			break;

		case TEXT_LINEBREAK:
			if (t->next) {
				print_const("\\\\\n");
				scratch->padded = 1;
			}

			break;

		case TEXT_NL:
			if (t->next) {
				print_char('\n');
			}

			break;

		case TEXT_PERCENT:
			print_const("\\%");
			break;

		case TEXT_BRACE_LEFT:
		case TEXT_BRACE_RIGHT:
			print_const("\\");

		case RAW_FILTER_LEFT:
		case TEXT_NUMBER_POSS_LIST:
		case TEXT_PERIOD:
		case TEXT_PLAIN:
			print_token(t);
			break;

		case TOC:
			print_const("\\{\\{TOC\\}\\}");
			break;

		case UL:
			print_const("\\_");
			break;

		default:
			fprintf(stderr, "Unknown token type: %d\n", t->type);
			token_describe(t, source);
			break;
	}
}


void mmd_export_token_tree_latex(DString * out, const char * source, token * t, scratch_pad * scratch) {

	// Prevent stack overflow with "dangerous" input causing extreme recursion
	if (scratch->recurse_depth == kMaxExportRecursiveDepth) {
		return;
	}

	scratch->recurse_depth++;

	while (t != NULL) {
		if (scratch->skip_token) {
			scratch->skip_token--;
		} else {
			mmd_export_token_latex(out, source, t, scratch);
		}

		t = t->next;
	}

	scratch->recurse_depth--;
}


void mmd_export_token_latex_raw(DString * out, const char * source, token * t, scratch_pad * scratch) {
	if (t == NULL) {
		return;
	}

	switch (t->type) {
		case ESCAPED_CHARACTER:
			print_const("\\");
			print_char(source[t->start + 1]);
//			mmd_print_char_latex(out, source[t->start + 1]);
			break;

		case HTML_ENTITY:
			print_token(t);
			break;

		case SUBSCRIPT:
			if (t->child) {
				print_const("\\ensuremath{\\sim}");
				mmd_export_token_tree_latex_raw(out, source, t->child, scratch);
			} else {
				print_token(t);
			}

			break;

		case SUPERSCRIPT:
			if (t->child) {
				print_const("\\^{}");
				mmd_export_token_tree_latex_raw(out, source, t->child, scratch);
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
				mmd_export_token_tree_latex_raw(out, source, t->child, scratch);
			} else {
				print_token(t);
			}

			break;
	}
}


void mmd_export_token_tree_latex_raw(DString * out, const char * source, token * t, scratch_pad * scratch) {
	while (t != NULL) {
		if (scratch->skip_token) {
			scratch->skip_token--;
		} else {
			mmd_export_token_latex_raw(out, source, t, scratch);
		}

		t = t->next;
	}
}


void mmd_export_token_latex_tt(DString * out, const char * source, token * t, scratch_pad * scratch) {
	if (t == NULL) {
		return;
	}

	switch (t->type) {
		case AMPERSAND:
		case AMPERSAND_LONG:
			print_const("\\&");
			break;

		case ANGLE_LEFT:
			print_const("<");
			break;

		case ANGLE_RIGHT:
			print_const(">");
			break;

		case CRITIC_ADD_OPEN:
			print_const("\\{++");
			break;

		case CRITIC_ADD_CLOSE:
			print_const("++\\}");
			break;

		case CRITIC_COM_OPEN:
			print_const("\\{>>");
			break;

		case CRITIC_COM_CLOSE:
			print_const("<<\\}");
			break;

		case CRITIC_DEL_OPEN:
			print_const("\\{--");
			break;

		case CRITIC_DEL_CLOSE:
			print_const("--\\}");
			break;

		case CRITIC_HI_OPEN:
			print_const("\\{==");
			break;

		case CRITIC_HI_CLOSE:
			print_const("==\\}");
			break;

		case CRITIC_SUB_OPEN:
			print_const("\\{~~");
			break;

		case CRITIC_SUB_DIV:
			print_const("~>");
			break;

		case CRITIC_SUB_CLOSE:
			print_const("~~\\}");
			break;

		case DASH_N:
			if (t->len == 1) {
				print_const("-");
			} else {
				print_const("-{}-");
			}

			break;

		case DASH_M:
			print_const("-{}-{}-");
			break;

		case EMPH_START:
		case EMPH_STOP:
			if (source[t->start] == '_') {
				print_const("\\_");
			} else {
				print_const("*");
			}

			break;

		case ESCAPED_CHARACTER:
			print_const("\\textbackslash{}");
			mmd_print_char_latex(out, source[t->start + 1]);
			break;

		case HTML_ENTITY:
			if (source[t->start + 1] == '#') {
				print_const("\\&\\#");
				d_string_append_c_array(out, &(source[t->start + 2]), t->len - 2);
			} else {
				print_const("\\");
				print_token(t);
			}

			break;

		case CODE_FENCE:
			if (t->next) {
				t->next->type = TEXT_EMPTY;
			}

		case MATH_BRACKET_OPEN:
		case MATH_BRACKET_CLOSE:
		case MATH_PAREN_OPEN:
		case MATH_PAREN_CLOSE:
			print_const("\\textbackslash{}\\textbackslash{}");
			print_char(source[t->start + 2]);
			break;

		case TEXT_EMPTY:
			break;

		case SLASH:
			print_const("\\slash ");
			break;

		case TEXT_BACKSLASH:
			print_const("\\textbackslash{}");
			break;

		case BRACE_DOUBLE_LEFT:
			print_const("\\{\\{");
			break;

		case BRACE_DOUBLE_RIGHT:
			print_const("\\}\\}");
			break;

		case SUBSCRIPT:
			if (t->child) {
				print_const("\\ensuremath{\\sim}");
				mmd_export_token_tree_latex_tt(out, source, t->child, scratch);
			} else {
				print_const("\\ensuremath{\\sim}");
			}

			break;

		case SUPERSCRIPT:
			if (t->child) {
				print_const("\\^{}");
				mmd_export_token_tree_latex_tt(out, source, t->child, scratch);
			} else {
				print_const("\\^{}");
			}

			break;

		case TEXT_BRACE_LEFT:
			print_const("\\{");
			break;

		case TEXT_BRACE_RIGHT:
			print_const("\\}");
			break;

		case TOC:
			print_const("\\{\\{TOC\\}\\}");
			break;

		case UL:
			print_const("\\_");
			break;

		default:
			if (t->child) {
				mmd_export_token_tree_latex_tt(out, source, t->child, scratch);
			} else {
				print_token(t);
			}

			break;
	}
}


void mmd_export_token_tree_latex_tt(DString * out, const char * source, token * t, scratch_pad * scratch) {
	while (t != NULL) {
		if (scratch->skip_token) {
			scratch->skip_token--;
		} else {
			mmd_export_token_latex_tt(out, source, t, scratch);
		}

		t = t->next;
	}
}

int clean_text_sort(fn_holder * a, fn_holder * b) {
	return strcmp(a->note->clean_text, b->note->clean_text);
}



void mmd_define_glossaries_latex(DString * out, const char * source, scratch_pad * scratch) {
	// Iterate through glossary definitions
	fn_holder * f, * f_tmp;

	// Sort glossary entries
	HASH_SORT(scratch->glossary_hash, clean_text_sort);

	char * last_key = NULL;

	HASH_ITER(hh, scratch->glossary_hash, f, f_tmp) {
		if (!last_key || strcmp(last_key, f->note->clean_text) != 0) {
			// Add this glossary definition
			print_const("\\longnewglossaryentry{");
			print(f->note->clean_text);

			print_const("}{name=");
			print(f->note->clean_text);
			print_const("}{");

			mmd_export_token_tree_latex(out, source, f->note->content, scratch);
			print_const("}\n\n");
		}

		last_key = f->note->clean_text;
	}

	// And abbreviations

	HASH_ITER(hh, scratch->abbreviation_hash, f, f_tmp) {
		// Add this abbreviation definition
		print_const("\\newacronym{");
		print(f->note->label_text);
		print_const("}{");
		print(f->note->label_text);
		print_const("}{");
		print(f->note->clean_text);
		print_const("}\n\n");
	}
}


void mmd_start_complete_latex(DString * out, const char * source, scratch_pad * scratch) {
	// Iterate over metadata keys
	meta * m;

	m = extract_meta_from_stack(scratch, "latexleader");

	if (m) {
		printf("\\input{%s}\n", m->value);
	} else {
		m = extract_meta_from_stack(scratch, "latexconfig");

		if (m) {
			printf("\\input{mmd6-%s-leader}\n", m->value);
		}
	}

	for (m = scratch->meta_hash; m != NULL; m = m->hh.next) {
		if (strcmp(m->key, "baseheaderlevel") == 0) {
		} else if (strcmp(m->key, "css") == 0) {
		} else if (strcmp(m->key, "htmlfooter") == 0) {
		} else if (strcmp(m->key, "htmlheader") == 0) {
		} else if (strcmp(m->key, "htmlheaderlevel") == 0) {
		} else if (strcmp(m->key, "language") == 0) {
		} else if (strcmp(m->key, "latexbegin") == 0) {
		} else if (strcmp(m->key, "latexconfig") == 0) {
		} else if (strcmp(m->key, "latexheader") == 0) {
			print(m->value);
			print_char('\n');
		} else if (strcmp(m->key, "latexfooter") == 0) {
		} else if (strcmp(m->key, "latexheaderlevel") == 0) {
		} else if (strcmp(m->key, "latexinput") == 0) {
		} else if (strcmp(m->key, "latexleader") == 0) {
		} else if (strcmp(m->key, "latexmode") == 0) {
		} else if (strcmp(m->key, "mmdfooter") == 0) {
		} else if (strcmp(m->key, "mmdheader") == 0) {
		} else if (strcmp(m->key, "quoteslanguage") == 0) {
		} else if (strcmp(m->key, "title") == 0) {
			print_const("\\def\\mytitle{");
			mmd_print_string_latex(out, m->value);
			print_const("}\n");
		} else if (strcmp(m->key, "latextitle") == 0) {
			print_const("\\def\\latextitle{");
			print(m->value);
			print_const("}\n");
		} else if (strcmp(m->key, "author") == 0) {
			print_const("\\def\\myauthor{");
			mmd_print_string_latex(out, m->value);
			print_const("}\n");
		} else if (strcmp(m->key, "latexauthor") == 0) {
			print_const("\\def\\latexauthor{");
			print(m->value);
			print_const("}\n");
		} else if (strcmp(m->key, "date") == 0) {
			print_const("\\def\\mydate{");
			mmd_print_string_latex(out, m->value);
			print_const("}\n");
		} else if (strcmp(m->key, "copyright") == 0) {
			print_const("\\def\\mycopyright{");
			mmd_print_string_latex(out, m->value);
			print_const("}\n");
		} else if (strcmp(m->key, "bibtex") == 0) {
			print_const("\\def\\bibliocommand{\\bibliography{");
			mmd_print_string_latex(out, m->value);
			print_const("}}\n");
		} else if (strcmp(m->key, "transcludebase") == 0) {
		} else if (strcmp(m->key, "xhtmlheader") == 0) {
		} else if (strcmp(m->key, "xhtmlheaderlevel") == 0) {
		} else {
			print_const("\\def\\");
			mmd_print_string_latex(out, m->key);
			print_const("{");
			mmd_print_string_latex(out, m->value);
			print_const("}\n");
		}
	}

	// Define glossaries in preamble for more flexibility
	mmd_define_glossaries_latex(out, source, scratch);

	m = extract_meta_from_stack(scratch, "latexbegin");

	if (m) {
		printf("\\input{%s}\n", m->value);
	} else {
		m = extract_meta_from_stack(scratch, "latexconfig");

		if (m) {
			printf("\\input{mmd6-%s-begin}\n", m->value);
		}
	}

	scratch->padded = 1;
}


void mmd_end_complete_latex(DString * out, const char * source, scratch_pad * scratch) {
	pad(out, 2, scratch);

	meta * m = extract_meta_from_stack(scratch, "latexfooter");

	if (m) {
		printf("\\input{%s}\n\n", m->value);
	} else {
		m = extract_meta_from_stack(scratch, "latexconfig");

		if (m) {
			printf("\\input{mmd6-%s-footer}\n", m->value);
		}
	}

	print_const("\\end{document}");
	scratch->padded = 0;
}


void mmd_export_citation_list_latex(DString * out, const char * source, scratch_pad * scratch) {
	if (scratch->used_citations->size > 0) {
		footnote * note;
		token * content;

		pad(out, 2, scratch);
		print_const("\\begin{thebibliography}{0}");
		scratch->padded = 0;

		for (int i = 0; i < scratch->used_citations->size; ++i) {
			// Export footnote
			pad(out, 2, scratch);

			note = stack_peek_index(scratch->used_citations, i);
			content = note->content;

			printf("\\bibitem{%s}\n", note->label_text);
			scratch->padded = 6;

			scratch->footnote_para_counter = 0;

			scratch->citation_being_printed = i + 1;

			mmd_export_token_tree_latex(out, source, content, scratch);
		}

		pad(out, 1, scratch);
		print_const("\\end{thebibliography}");
		scratch->padded = 0;
		scratch->citation_being_printed = 0;
	}
}
