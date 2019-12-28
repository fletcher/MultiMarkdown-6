/**

	MultiMarkdown 6 -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file beamer.c

	@brief


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


#include "latex.h"
#include "beamer.h"
#include "parser.h"
#include "stack.h"

#define print(x) d_string_append(out, x)
#define print_const(x) d_string_append_c_array(out, x, sizeof(x) - 1)
#define print_char(x) d_string_append_c(out, x)
#define printf(...) d_string_append_printf(out, __VA_ARGS__)
#define print_token(t) d_string_append_c_array(out, &(source[t->start]), t->len)
#define print_localized(x) mmd_print_localized_char_latex(out, x, scratch)


void mmd_outline_add_beamer(DString * out, token * current, scratch_pad * scratch) {
	token * t;
	short level;		// Header level we are adding
	short t_level;
	stack * s = scratch->outline_stack;

	if (current) {
		switch (current->type) {
			case BLOCK_SETEXT_1:
				level = 1;
				break;

			case BLOCK_SETEXT_2:
				level = 2;
				break;

			default:
				level = 1 + current->type - BLOCK_H1;
		}

		level += scratch->base_header_level - 1;
	} else {
		level = 0;
	}

	if (s->size) {
		t = stack_peek(s);

		while (t) {
			switch (t->type) {
				case BLOCK_SETEXT_1:
					t_level = 1;
					break;

				case BLOCK_SETEXT_2:
					t_level = 2;
					break;

				default:
					t_level = 1 + t->type - BLOCK_H1;
			}

			t_level += scratch->base_header_level - 1;

			if (t_level >= level) {
				// Close out level
				switch (t_level) {
					case 3:
						pad(out, 1, scratch);
						print_const("\\end{frame}\n\n");
						scratch->padded = 2;
						break;

					case 4:
						pad(out, 1, scratch);
						print_const("}\n\n");
						scratch->padded = 2;
						break;
				}

				stack_pop(s);
				t = stack_peek(s);
			} else {
				// Nothing to close
				t = NULL;
			}
		}
	}


	// Add current level to stack and open
	switch (level) {
		case 3:
			pad(out, 2, scratch);
			print_const("\\begin{frame}[fragile]\n");
			scratch->padded = 1;
			stack_push(s, current);
			break;

		case 4:
			pad(out, 2, scratch);
			print_const("\\mode<article>{");
			scratch->padded = 0;
			stack_push(s, current);
			break;
	}
}


void mmd_export_token_beamer(DString * out, const char * source, token * t, scratch_pad * scratch) {
	if (t == NULL) {
		return;
	}

	short	temp_short;
	char *	temp_char	= NULL;
	token *	temp_token	= NULL;

	switch (t->type) {
		case DOC_START_TOKEN:
			mmd_export_token_tree_beamer(out, source, t->child, scratch);
			break;

		case BLOCK_CODE_FENCED:
			pad(out, 2, scratch);

			temp_char = get_fence_language_specifier(t->child->child, source);

			if (temp_char) {
				if (strncmp("{=", temp_char, 2) == 0) {
					// Raw source
					if (raw_filter_text_matches(temp_char, FORMAT_BEAMER)) {
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
							if (t->child->next) {
								d_string_append_c_array(out, &source[t->child->start + t->child->len], t->start + t->len - t->child->next->start);
							}

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

		case BLOCK_H1:
		case BLOCK_H2:
		case BLOCK_H3:
		case BLOCK_H4:
		case BLOCK_H5:
		case BLOCK_H6:
		case BLOCK_SETEXT_1:
		case BLOCK_SETEXT_2:
			pad(out, 2, scratch);

			mmd_outline_add_beamer(out, t, scratch);

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
					print_const("\\section{");
					break;

				case 3:
					print_const("\\frametitle{");
					break;

				default:
					print_const("\\emph{");
					break;
			}

			mmd_export_token_tree_beamer(out, source, t->child, scratch);
			trim_trailing_whitespace_d_string(out);

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

				if (temp_char) {
					free(temp_char);
				}
			}

			scratch->padded = 0;
			break;

		default:
			// Default to LaTeX behavior
			mmd_export_token_latex(out, source, t, scratch);
	}
}


void mmd_export_token_tree_beamer(DString * out, const char * source, token * t, scratch_pad * scratch) {

	// Prevent stack overflow with "dangerous" input causing extreme recursion
	if (scratch->recurse_depth == kMaxExportRecursiveDepth) {
		return;
	}

	scratch->recurse_depth++;

	while (t != NULL) {
		if (scratch->skip_token) {
			scratch->skip_token--;
		} else {
			mmd_export_token_beamer(out, source, t, scratch);
		}

		t = t->next;
	}

	scratch->recurse_depth--;
}


void mmd_export_citation_list_beamer(DString * out, const char * source, scratch_pad * scratch) {
	if (scratch->used_citations->size > 0) {
		footnote * note;
		token * content;

		pad(out, 2, scratch);
		print_const("\\part{Bibliography}\n\\begin{frame}[allowframebreaks]\n\\frametitle{Bibliography}\n\\def\\newblock{}\n\\begin{thebibliography}{0}");
		scratch->padded = 0;

		for (int i = 0; i < scratch->used_citations->size; ++i) {
			// Export footnote
			pad(out, 2, scratch);

			note = stack_peek_index(scratch->used_citations, i);

			printf("\\bibitem{%s}\n", note->label_text);
			scratch->padded = 6;

			scratch->footnote_para_counter = 0;

			content = note->content;
			scratch->citation_being_printed = i + 1;

			mmd_export_token_tree_latex(out, source, content, scratch);
		}

		pad(out, 1, scratch);
		print_const("\\end{thebibliography}\n\\end{frame}");
		scratch->padded = 0;
		scratch->citation_being_printed = 0;
	}
}


void mmd_end_complete_beamer(DString * out, const char * source, scratch_pad * scratch) {
	pad(out, 2, scratch);

	print_const("\\mode<all>\n");
	meta * m = extract_meta_from_stack(scratch, "latexfooter");

	if (m) {
		printf("\\input{%s}\n\n", m->value);
	}

	print_const("\\end{document}");
	print_const("\\mode*\n");
	scratch->padded = 0;
}
