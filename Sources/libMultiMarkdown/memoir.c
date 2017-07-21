/**

	MultiMarkdown 6 -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file memoir.c

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
#include "memoir.h"

#define print(x) d_string_append(out, x)
#define print_const(x) d_string_append_c_array(out, x, sizeof(x) - 1)
#define print_char(x) d_string_append_c(out, x)
#define printf(...) d_string_append_printf(out, __VA_ARGS__)
#define print_token(t) d_string_append_c_array(out, &(source[t->start]), t->len)
#define print_localized(x) mmd_print_localized_char_latex(out, x, scratch)


void mmd_export_token_memoir(DString * out, const char * source, token * t, scratch_pad * scratch) {
	if (t == NULL)
		return;

	char *	temp_char	= NULL;

	switch (t->type) {
		case DOC_START_TOKEN:
			mmd_export_token_tree_memoir(out, source, t->child, scratch);
			break;
		case BLOCK_CODE_FENCED:
			pad(out, 2, scratch);

			temp_char = get_fence_language_specifier(t->child->child, source);
			if (temp_char) {
				printf("\\begin{adjustwidth}{2.5em}{2.5em}\n\\begin{lstlisting}[language=%s]\n", temp_char);
			} else {
				print_const("\\begin{adjustwidth}{2.5em}{2.5em}\n\\begin{verbatim}\n");
			}

			mmd_export_token_tree_latex_raw(out, source, t->child->next, scratch);

			if (temp_char) {
				print_const("\\end{lstlisting}\n\\end{adjustwidth}");
				free(temp_char);
			} else {
				print_const("\\end{verbatim}\n\\end{adjustwidth}");
			}
			scratch->padded = 0;
			break;
		case BLOCK_CODE_INDENTED:
			pad(out, 2, scratch);
			print_const("\\begin{adjustwidth}{2.5em}{2.5em}\\begin{verbatim}\n");
			mmd_export_token_tree_latex_raw(out, source, t->child, scratch);
			print_const("\\end{verbatim}\n\\end{adjustwidth}");
			scratch->padded = 0;
			break;
		default:
			// Default to LaTeX behavior
			mmd_export_token_latex(out, source, t, scratch);
	}
}


void mmd_export_token_tree_memoir(DString * out, const char * source, token * t, scratch_pad * scratch) {

	// Prevent stack overflow with "dangerous" input causing extreme recursion
	if (scratch->recurse_depth == kMaxExportRecursiveDepth) {
		return;
	}

	scratch->recurse_depth++;

	while (t != NULL) {
		if (scratch->skip_token) {
			scratch->skip_token--;
		} else {
			mmd_export_token_memoir(out, source, t, scratch);
		}

		t = t->next;
	}

	scratch->recurse_depth--;
}

