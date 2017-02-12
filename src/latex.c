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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>	

#include "char.h"
#include "i18n.h"
#include "latex.h"
#include "parser.h"
#include "scanners.h"


#define print(x) d_string_append(out, x)
#define print_char(x) d_string_append_c(out, x)
#define printf(...) d_string_append_printf(out, __VA_ARGS__)
#define print_token(t) d_string_append_c_array(out, &(source[t->start]), t->len)
#define print_localized(x) mmd_print_localized_char_latex(out, x, scratch)


void mmd_print_char_latex(DString * out, char c) {
	switch (c) {
		case '\\':
			print("\\textbackslash{}");
			break;
		case '~':
			print("\\ensuremath{\\sim}");
			break;
		case '/':
			print("\\slash ");
			break;
		case '^':
			print("\\^{}");
			break;
		case '<':
		case '>':
			print_char('$');
			print_char(c);
			print_char('$');
			break;
		case '|':
			print("\\textbar{}");
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
    if (str == NULL)
        return;
    
	while (*str != '\0') {
		mmd_print_char_latex(out, *str);
		str++;
	}
}


void mmd_print_localized_char_latex(DString * out, unsigned short type, scratch_pad * scratch) {
	switch (type) {
		case DASH_N:
			print("--");
			break;
		case DASH_M:
			print("---");
			break;
		case ELLIPSIS:
			print("{\\ldots}");
			break;
		case APOSTROPHE:
			print("'");
			break;
		case QUOTE_LEFT_SINGLE:
			switch (scratch->quotes_lang) {
				case SWEDISH:
					print("'");
					break;
				case FRENCH:
					print("'");
					break;
				case GERMAN:
					print("‚");
					break;
				case GERMANGUILL:
					print("›");
					break;
				default:
					print("`");
				}
			break;
		case QUOTE_RIGHT_SINGLE:
			switch (scratch->quotes_lang) {
				case GERMAN:
					print("`");
					break;
				case GERMANGUILL:
					print("‹");
					break;
				default:
					print("'");
				}
			break;
		case QUOTE_LEFT_DOUBLE:
			switch (scratch->quotes_lang) {
				case DUTCH:
				case GERMAN:
					print("„");
					break;
				case GERMANGUILL:
					print("»");
					break;
				case FRENCH:
					print("«");
					break;
				case SWEDISH:
					print("''");
					break;
				default:
					print("``");
				}
			break;
		case QUOTE_RIGHT_DOUBLE:
			switch (scratch->quotes_lang) {
				case GERMAN:
					print("``");
					break;
				case GERMANGUILL:
					print("«");
					break;
				case FRENCH:
					print("»");
					break;
				case SWEDISH:
				case DUTCH:
				default:
					print("''");
				}
			break;
	}
}



void mmd_export_link_latex(DString * out, const char * source, token * text, link * link, scratch_pad * scratch) {
	attr * a = link->attributes;
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
					print(" (");
					printf("\\autoref{%s}", &(link->url)[1]);
					print(")");
				}

				free(temp_char);
			} else {
				printf("\\autoref{%s}", &(link->url)[1]);
			}
			return;
		} else {
			printf("\\href{%s}", link->url);			
		}
	} else
		print("\\href{}");

//	if (link->title && link->title[0] != '\0') {
//		print(" title=\"");
//		mmd_print_string_latex(out, link->title);
//		print("\"");
//	}

//	while (a) {
//		print(" ");
//		print(a->key);
//		print("=\"");
//		print(a->value);
//		print("\"");
//		a = a->next;
//	}

	print("{");

	// If we're printing contents of bracket as text, then ensure we include it all
	if (text && text->child && text->child->len > 1) {
		text->child->next->start--;
		text->child->next->len++;
	}
	
	mmd_export_token_tree_latex(out, source, text->child, scratch);

	print("}");

	// Reprint as footnote for printed copies
	printf("\\footnote{\\href{%s}{", link->url);
	mmd_print_string_latex(out, link->url);
	print("}}");
}


void mmd_export_image_latex(DString * out, const char * source, token * text, link * link, scratch_pad * scratch, bool is_figure) {
	attr * a = link->attributes;

	// Compatibility mode doesn't allow figures
	if (scratch->extensions & EXT_COMPATIBILITY)
		is_figure = false;

	if (is_figure) {
		print("\\begin{figure}[htbp]\n\\centering\n");
		scratch->close_para = false;
	}

	print("\\includegraphics[keepaspectratio");

//	if (text) {
//		print(" alt=\"");
//		print_token_tree_raw(out, source, text->child);
//		print("\"");
//	}
//
//	if (link->title && link->title[0] != '\0')
//		printf(" title=\"%s\"", link->title);
//
//	while (a) {
//		print(" ");
//		print(a->key);
//		print("=\"");
//		print(a->value);
//		print("\"");
//		a = a->next;
//	}


	if (link->url)
		printf("]{%s}", link->url);
	else
		print("]{}");


	if (is_figure) {
		print("\n");
		if (text) {
			print("\\caption{");
			mmd_export_token_tree_latex(out, source, text->child, scratch);
			print("}\n");
		}
		if (link->label) {
			// \todo: Need to decide on approach to id's
			char * label = label_from_token(source, link->label);
			printf("\\label{%s}\n", label);
			free(label);
		}

		print("\\end{figure}");
	}
}


void mmd_export_token_latex(DString * out, const char * source, token * t, scratch_pad * scratch) {
	if (t == NULL)
		return;

	short	temp_short;
	short	temp_short2;
	link *	temp_link	= NULL;
	char *	temp_char	= NULL;
	char *	temp_char2	= NULL;
	bool	temp_bool	= 0;
	token *	temp_token	= NULL;
	footnote * temp_note = NULL;

	switch (t->type) {
		case AMPERSAND:
		case AMPERSAND_LONG:
			print("\\&");
			break;
		case ANGLE_LEFT:
			print("$<$");
			break;
		case ANGLE_RIGHT:
			print("$>$");
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
			print("\\begin{quote}\n");
			scratch->padded = 2;
			mmd_export_token_tree_latex(out, source, t->child, scratch);
			pad(out, 1, scratch);
			print("\\end{quote}");
			scratch->padded = 0;
			break;
		case BLOCK_CODE_FENCED:
			pad(out, 2, scratch);

			temp_char = get_fence_language_specifier(t->child->child, source);
			if (temp_char) {
				printf("\\begin{lstlisting}[language=%s]\n", temp_char);
			} else {
				print("\\begin{verbatim}\n");
			}

			mmd_export_token_tree_latex_raw(out, source, t->child->next, scratch);

			if (temp_char) {
				print("\\end{lstlisting}");
				free(temp_char);
			} else {
				print("\\end{verbatim}");
			}
			scratch->padded = 0;
			break;
		case BLOCK_CODE_INDENTED:
			pad(out, 2, scratch);
			print("\\begin{verbatim}\n");
			mmd_export_token_tree_latex_raw(out, source, t->child, scratch);
			print("\\end{verbatim}");
			scratch->padded = 0;
			break;
		case BLOCK_DEFINITION:
			pad(out, 2, scratch);

			temp_short = scratch->list_is_tight;
			if (!(t->child->next && (t->child->next->type == BLOCK_EMPTY) && t->child->next->next))
				scratch->list_is_tight = true;

			mmd_export_token_tree_latex(out, source, t->child, scratch);
			scratch->padded = 0;

			scratch->list_is_tight = temp_short;
			break;
		case BLOCK_DEFLIST:
			pad(out, 2, scratch);

			// Group consecutive definition lists into a single list.
			// lemon's LALR(1) parser can't properly handle this (to my understanding).

			if (!(t->prev && (t->prev->type == BLOCK_DEFLIST)))
				print("\\begin{description}\n");
	
			scratch->padded = 2;

			mmd_export_token_tree_latex(out, source, t->child, scratch);
			pad(out, 1, scratch);

			if (!(t->next && (t->next->type == BLOCK_DEFLIST)))
				print("\\end{description}\n");

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
					print("\\part{");
					break;
				case 2:
					print("\\chapter{");
					break;
				case 3:
					print("\\section{");
					break;
				case 4:
					print("\\subsection{");
					break;
				case 5:
					print("\\subsubsection{");
					break;
				case 6:
					print("\\paragraph{");
					break;
				case 7:
					print("\\subparagraph{");
					break;
			}

			mmd_export_token_tree_latex(out, source, t->child, scratch);

			if (scratch->extensions & EXT_NO_LABELS) {
				print("}");
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
			print("\\begin{center}\\rule{3in}{0.4pt}\\end{center}");
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
			print("\\begin{itemize}");
			scratch->padded = 1;
			mmd_export_token_tree_latex(out, source, t->child, scratch);
			pad(out, 2, scratch);
			print("\\end{itemize}");
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
			print("\\begin{enumerate}");
			scratch->padded = 1;
			mmd_export_token_tree_latex(out, source, t->child, scratch);
			pad(out, 2, scratch);
			print("\\end{enumerate}");
			scratch->padded = 0;
			scratch->list_is_tight = temp_short;
			break;
		case BLOCK_LIST_ITEM:
			pad(out, 2, scratch);
			print("\\item ");
			scratch->padded = 2;
			mmd_export_token_tree_latex(out, source, t->child, scratch);
			scratch->padded = 0;
			break;
		case BLOCK_LIST_ITEM_TIGHT:
			pad(out, 2, scratch);
			print("\\item ");
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
		case BLOCK_TERM:
			pad(out, 2, scratch);
			print("\\item[");
			mmd_export_token_tree_latex(out, source, t->child, scratch);
			print("]");
			scratch->padded = 0;
			break;
		case BRACE_DOUBLE_LEFT:
			print("\\{\\{");
			break;
		case BRACE_DOUBLE_RIGHT:
			print("\\}\\}");
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
		case CODE_FENCE:
			break;
		case COLON:
			print(":");
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
			print("\\emph{");
			break;
		case EMPH_STOP:
			print("}");
			break;
		case EQUAL:
			print("=");
			break;
		case ESCAPED_CHARACTER:
			mmd_print_char_latex(out, source[t->start + 1]);
			break;
		case HASH1:
		case HASH2:
		case HASH3:
		case HASH4:
		case HASH5:
		case HASH6:
			for (int i = 0; i < t->len; ++i)
			{
				print_char('\\');
				print_char('#');
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
		case MANUAL_LABEL:
			break;
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
			print("\\[");
			break;
		case MATH_BRACKET_CLOSE:
			print("\\]");
			break;
		case MATH_DOLLAR_SINGLE:
			print("$");
			break;
		case MATH_DOLLAR_DOUBLE:
			print("$$");
			break;
		case MATH_PAREN_OPEN:
			print("\\(");
			break;
		case MATH_PAREN_CLOSE:
			print("\\)");
			break;
		case NON_INDENT_SPACE:
			print_char(' ');
			break;
		case PAIR_ANGLE:
			temp_char = url_accept(source, t->start + 1, t->len - 2, NULL, true);

			if (temp_char) {
				if (scan_email(temp_char)) {
					print("\\href{mailto:");
					print(temp_char);
				} else {
					print("\\href{");
					print(temp_char);
				}
				print("}{");
				mmd_print_string_latex(out, temp_char);
				print("}");
			} else if (scan_html(&source[t->start])) {
				print_token(t);
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
			print("\\texttt{");
			mmd_export_token_tree_latex_raw(out, source, t->child, scratch);
			print("}");
			break;
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

					if (temp_token && temp_token->type == TEXT_NL)
						temp_token = temp_token->next;

					if (temp_token && temp_token->type == TEXT_LINEBREAK)
						temp_token = temp_token->next;

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
		case PAIR_BRACKET_CITATION:
			parse_citation:
			temp_bool = true;

			if (t->type == PAIR_BRACKET) {
				// This is a locator for subsequent citation
				temp_char = text_inside_pair(source, t);
				temp_char2 = label_from_string(temp_char);

				if (strcmp(temp_char2, "notcited") == 0) {
					free(temp_char2);
					free(temp_char);
					temp_char = strdup("");
					temp_bool = false;
				}

				if (temp_char[0] == '\0')
					temp_char2 = strdup("");
				else
					temp_char2 = strdup(", ");


				// Process the actual citation
				t = t->next;
			} else {
				temp_char = strdup("");
				temp_char2 = strdup("");
			}

			if (scratch->extensions & EXT_NOTES) {
				citation_from_bracket(source, scratch, t, &temp_short);

				if (temp_bool) {
					if (temp_short < scratch->used_citations->size) {
						// Re-using previous citation
						printf("\\citation{reuse");

						printf("}");
					} else {
						// This is a new citation
						printf("\\citation{");

						printf("}");
					}
				}

				if (t->prev && (t->prev->type == PAIR_BRACKET)) {
					// Skip citation on next pass
					scratch->skip_token = 1;
				}
			} else {
				// Footnotes disabled
				mmd_export_token_tree_latex(out, source, t->child, scratch);
			}

			free(temp_char);
			free(temp_char2);
			break;
		case PAIR_BRACKET_FOOTNOTE:
			if (scratch->extensions & EXT_NOTES) {
				footnote_from_bracket(source, scratch, t, &temp_short);

				if (temp_short < scratch->used_footnotes->size) {
					// Re-using previous footnote
					printf("\\footnote{reuse");

					printf("}");
				} else {
					// This is a new footnote
					printf("\\footnote{");
					temp_note = stack_peek_index(scratch->used_footnotes, temp_short - 1);

					mmd_export_token_tree_latex(out, source, temp_note->content, scratch);
					printf("}");
				}
			} else {
				// Footnotes disabled
				mmd_export_token_tree_latex(out, source, t->child, scratch);
			}
			break;
		case PAIR_BRACKET_VARIABLE:
			temp_char = text_inside_pair(source, t);
			temp_char2 = extract_metadata(scratch, temp_char);

			if (temp_char2)
				mmd_print_string_latex(out, temp_char2);
			else
				mmd_export_token_tree_latex(out, source, t->child, scratch);

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
					mmd_export_token_tree_latex(out, source, t->child, scratch);
				} else {
					print("\\underline{");
					mmd_export_token_tree_latex(out, source, t->child, scratch);
					print("}");
				}
			} else {
				mmd_export_token_tree_latex(out, source, t->child, scratch);				
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
					mmd_export_token_tree_latex(out, source, t->child, scratch);
				} else {
					print("\\sout{");
					mmd_export_token_tree_latex(out, source, t->child, scratch);
					print("}");
				}
			} else {
				mmd_export_token_tree_latex(out, source, t->child, scratch);				
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
				print("\\todo{");
				mmd_export_token_tree_latex(out, source, t->child, scratch);
				print("}");
			} else {
				mmd_export_token_tree_latex(out, source, t->child, scratch);
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
				// 'hl' requires 'soul' package
				print("\\hl{");
				mmd_export_token_tree_latex(out, source, t->child, scratch);
				print("}");
			} else {
				mmd_export_token_tree_latex(out, source, t->child, scratch);
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
					mmd_export_token_tree_latex(out, source, t->child, scratch);
				} else {
					print("\\sout{");
					mmd_export_token_tree_latex(out, source, t->child, scratch);
					print("}");
				}
			} else {
				mmd_export_token_tree_latex(out, source, t->child, scratch);
			}
			break;
		case PAIR_CRITIC_SUB_ADD:
			if ((scratch->extensions & EXT_CRITIC) &&
				(t->prev->type == PAIR_CRITIC_SUB_DEL)) {
				t->child->type = TEXT_EMPTY;
				t->child->mate->type = TEXT_EMPTY;
				if (scratch->extensions & EXT_CRITIC_REJECT) {

				} else if (scratch->extensions & EXT_CRITIC_ACCEPT) {
					mmd_export_token_tree_latex(out, source, t->child, scratch);
				} else {
					print("\\underline{");
					mmd_export_token_tree_latex(out, source, t->child, scratch);
					print("}");
				}
			} else {
				mmd_export_token_tree_latex(out, source, t->child, scratch);
			}
			break;
		case PAIR_MATH:
			// Math is raw LaTeX -- use string itself
			mmd_export_token_latex(out, source, t->child, scratch);
			d_string_append_c_array(out, &(source[t->child->start + t->child->len]), t->child->mate->start - t->child->start - t->child->len);
			mmd_export_token_latex(out, source, t->child->mate, scratch);
			break;			
		case PAIR_PAREN:
		case PAIR_QUOTE_DOUBLE:
		case PAIR_QUOTE_SINGLE:
		case PAIR_STAR:
		case PAIR_UL:
			mmd_export_token_tree_latex(out, source, t->child, scratch);
			break;
		case PAREN_LEFT:
			print("(");
			break;
		case PAREN_RIGHT:
			print(")");
			break;
		case PIPE:
			print("\\textbar{}");
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
		case SLASH:
			print("\\slash ");
			break;
		case STAR:
			print_token(t);
			break;
		case STRONG_START:
			print("\\textbf{");
			break;
		case STRONG_STOP:
			print("}");
			break;
		case SUBSCRIPT:
			if (t->mate) {
				(t->start < t->mate->start) ? (print("\\textsubscript{")) : (print("}"));
			} else if (t->len != 1) {
				print("\\textsubscript{");
				mmd_export_token_latex(out, source, t->child, scratch);
				print("}");
			} else {
				print("\\ensuremath{\\sim}");
			}
			break;
		case SUPERSCRIPT:
			if (t->mate) {
				(t->start < t->mate->start) ? (print("\\textsuperscript{")) : (print("}"));
			} else if (t->len != 1) {
				print("\\textsuperscript{");
				mmd_export_token_latex(out, source, t->child, scratch);
				print("}");
			} else {
				print("\\^{}");
			}	
			break;
		case TEXT_EMPTY:
			break;
		case TEXT_HASH:
			print("\\#");
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
	if (t == NULL)
		return;

	switch (t->type) {
		case AMPERSAND:
			print("\\&");
			break;
		case AMPERSAND_LONG:
			print("\\textbackslash{}&");
			break;
		case ANGLE_LEFT:
			print("$<$");
			break;
		case ANGLE_RIGHT:
			print("$>$");
			break;
		case DASH_N:
			if (t->len == 1) {
				print("-");
			} else {
				print("-{}-");
			}
			break;
		case DASH_M:
			print("-{}-{}-");
			break;
		case ESCAPED_CHARACTER:
			print("\\textbackslash{}");
			mmd_print_char_latex(out, source[t->start + 1]);
			break;
		case CODE_FENCE:
			if (t->next)
				t->next->type = TEXT_EMPTY;
		case TEXT_EMPTY:
			break;
		default:
			if (t->child)
				mmd_export_token_tree_latex_raw(out, source, t->child, scratch);
			else
				print_token(t);
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


void mmd_start_complete_latex(DString * out, const char * source, scratch_pad * scratch) {

}


void mmd_end_complete_latex(DString * out, const char * source, scratch_pad * scratch) {

}

