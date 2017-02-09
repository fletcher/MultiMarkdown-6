/**

	MultiMarkdown 6 -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file parser.y

	@brief Use a cleaned copy of the parser grammar to allow "fuzz" testing with
	combinations of every line type to find combinations that can't be parsed.
	NOTE: This does not mean that each of these combinations is properly parsed, 
	simply that it does not cause the parser to fail.  Use the test suite to ensure
	that input text is properly being parsed.


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



//
// Language grammar here
//

%token_type { int }


// Manually copy fallbacks from `parser.y` here

%fallback LINE_HR LINE_SETEXT_1 LINE_SETEXT_2.

%fallback LINE_PLAIN LINE_TABLE_SEPARATOR.

%fallback LINE_CONTINUATION LINE_PLAIN LINE_INDENTED_TAB LINE_INDENTED_SPACE  LINE_TABLE.

%fallback LINE_HTML LINE_ATX_1 LINE_ATX_2 LINE_ATX_3 LINE_ATX_4 LINE_ATX_5 LINE_ATX_6 LINE_HR LINE_BLOCKQUOTE LINE_LIST_BULLETED LINE_LIST_ENUMERATED LINE_DEF_CITATION LINE_DEF_FOOTNOTE LINE_DEF_LINK LINE_FENCE_BACKTICK LINE_FENCE_BACKTICK_START.


// Copy clean grammar via `lemon -g parser.y` here

doc ::= blocks.
blocks ::= blocks block.
blocks ::= block.
block ::= LINE_ATX_1.
block ::= LINE_ATX_2.
block ::= LINE_ATX_3.
block ::= LINE_ATX_4.
block ::= LINE_ATX_5.
block ::= LINE_ATX_6.
block ::= LINE_HR.
block ::= LINE_TOC.
block ::= blockquote.
block ::= def_citation.
block ::= def_footnote.
block ::= def_link.
block ::= definition_block.
block ::= empty.
block ::= fenced_block.
block ::= html_block.
block ::= indented_code.
block ::= list_bullet.
block ::= list_enum.
block ::= meta_block.
block ::= para.
block ::= setext_1.
block ::= setext_2.
block ::= table.
chunk ::= chunk chunk_line.
nested_chunks ::= nested_chunks nested_chunk.
nested_chunk ::= empty indented_line chunk.
nested_chunk ::= empty indented_line.
ext_chunk ::= chunk nested_chunks.
opt_ext_chunk ::= chunk nested_chunks.
blockquote ::= blockquote quote_line.
def_citation ::= LINE_DEF_CITATION tail.
def_footnote ::= LINE_DEF_FOOTNOTE tail.
def_link ::= LINE_DEF_LINK chunk.
definition_block ::= para defs.
defs ::= defs def.
def ::= LINE_DEFINITION tail.
def ::= LINE_DEFINITION.
empty ::= empty LINE_EMPTY.
fenced_block ::= fenced_3 LINE_FENCE_BACKTICK_3.
fenced_block ::= fenced_3 LINE_FENCE_BACKTICK_4.
fenced_block ::= fenced_3 LINE_FENCE_BACKTICK_5.
fenced_3 ::= fenced_3 fenced_line.
fenced_block ::= fenced_4 LINE_FENCE_BACKTICK_4.
fenced_block ::= fenced_4 LINE_FENCE_BACKTICK_5.
fenced_4 ::= fenced_4 fenced_line.
fenced_4 ::= fenced_4 LINE_FENCE_BACKTICK_3.
fenced_4 ::= fenced_4 LINE_FENCE_BACKTICK_START_3.
fenced_block ::= fenced_5 LINE_FENCE_BACKTICK_5.
fenced_5 ::= fenced_5 fenced_line.
fenced_5 ::= fenced_5 LINE_FENCE_BACKTICK_3.
fenced_5 ::= fenced_5 LINE_FENCE_BACKTICK_START_3.
fenced_5 ::= fenced_5 LINE_FENCE_BACKTICK_4.
fenced_5 ::= fenced_5 LINE_FENCE_BACKTICK_START_4.
html_block ::= html_block html_line.
indented_code ::= indented_code indented_line.
indented_code ::= indented_code LINE_EMPTY.
list_bullet ::= list_bullet item_bullet.
item_bullet ::= LINE_LIST_BULLETED ext_chunk.
item_bullet ::= LINE_LIST_BULLETED chunk.
item_bullet ::= LINE_LIST_BULLETED nested_chunks.
item_bullet ::= LINE_LIST_BULLETED.
list_enum ::= list_enum item_enum.
item_enum ::= LINE_LIST_ENUMERATED ext_chunk.
item_enum ::= LINE_LIST_ENUMERATED chunk.
item_enum ::= LINE_LIST_ENUMERATED nested_chunks.
item_enum ::= LINE_LIST_ENUMERATED.
meta_block ::= meta_block meta_line.
para ::= LINE_PLAIN chunk.
setext_1 ::= para LINE_SETEXT_1.
setext_2 ::= para LINE_SETEXT_2.
table ::= table_header table_body.
table_header ::= header_rows LINE_TABLE_SEPARATOR.
header_rows ::= header_rows LINE_TABLE.
table_body ::= table_body table_section.
table_section ::= all_rows LINE_EMPTY.
table_section ::= all_rows.
all_rows ::= all_rows row.
para ::= all_rows.
chunk ::= chunk_line.
chunk_line ::= LINE_CONTINUATION.
nested_chunks ::= nested_chunk.
nested_chunk ::= empty.
indented_line ::= LINE_INDENTED_TAB.
indented_line ::= LINE_INDENTED_SPACE.
opt_ext_chunk ::= chunk.
tail ::= opt_ext_chunk.
tail ::= nested_chunks.
blockquote ::= LINE_BLOCKQUOTE.
quote_line ::= LINE_BLOCKQUOTE.
quote_line ::= LINE_CONTINUATION.
def_citation ::= LINE_DEF_CITATION.
def_footnote ::= LINE_DEF_FOOTNOTE.
def_link ::= LINE_DEF_LINK.
defs ::= def.
empty ::= LINE_EMPTY.
fenced_block ::= fenced_3.
fenced_3 ::= LINE_FENCE_BACKTICK_3.
fenced_3 ::= LINE_FENCE_BACKTICK_START_3.
fenced_block ::= fenced_4.
fenced_4 ::= LINE_FENCE_BACKTICK_4.
fenced_4 ::= LINE_FENCE_BACKTICK_START_4.
fenced_block ::= fenced_5.
fenced_5 ::= LINE_FENCE_BACKTICK_5.
fenced_5 ::= LINE_FENCE_BACKTICK_START_5.
fenced_line ::= LINE_CONTINUATION.
fenced_line ::= LINE_EMPTY.
fenced_line ::= LINE_SETEXT_1.
fenced_line ::= LINE_SETEXT_2.
html_block ::= LINE_HTML.
html_line ::= LINE_CONTINUATION.
html_line ::= LINE_HTML.
indented_code ::= indented_line.
list_bullet ::= item_bullet.
list_enum ::= item_enum.
meta_block ::= LINE_META.
meta_line ::= LINE_META.
meta_line ::= LINE_CONTINUATION.
para ::= LINE_PLAIN.
table ::= table_header.
header_rows ::= LINE_TABLE.
table_body ::= table_section.
all_rows ::= row.
row ::= header_rows.
row ::= LINE_TABLE_SEPARATOR.
para ::= defs.


//
// Additional Configuration
//

%include {
	#include <assert.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include "parser_test.h"

	// Basic parser function declarations
	void * ParseAlloc();
	void Parse();
	void ParseFree();
	void ParseTrace();

	#define kMaxToken 34

	int i,j,k,l,m, n;

int main(int argc, char** argv) {

	void* pParser = ParseAlloc (malloc);		// Create a parser (for lemon)

#ifndef NDEBUG
	ParseTrace(stderr, "parser >>");
#endif


	fprintf(stderr, "Single line tests\n");

	i = 0;
	j = 0;
	k = 0;
	l = 0;
	m = 0;
	n = 0;

	for (i = 1; i <= kMaxToken; ++i)
	{
		// LINE_CONTINUATION can't be the first line
		if (i == LINE_CONTINUATION)
			break;

		Parse(pParser, i, NULL);

		Parse(pParser, 0, NULL);
	}
	
	fprintf(stderr, "\nDouble line tests\n");

	i = 0;
	j = 0;
	k = 0;
	l = 0;
	m = 0;

	for (i = 1; i <= kMaxToken; ++i)
	{
		// LINE_CONTINUATION can't be the first line
		if (i == LINE_CONTINUATION)
			break;

		for (j = 1; j <= kMaxToken; ++j) {
			
			Parse(pParser, i, NULL);
			Parse(pParser, j, NULL);

			Parse(pParser, 0, NULL);
		}
	}

	fprintf(stderr, "\nTriple line tests\n");

	i = 0;
	j = 0;
	k = 0;
	l = 0;
	m = 0;

	for (i = 1; i <= kMaxToken; ++i)
	{
		// LINE_CONTINUATION can't be the first line
		if (i == LINE_CONTINUATION)
			break;
		
		for (j = 1; j <= kMaxToken; ++j) {

			for (k = 1; k <= kMaxToken; ++k) {
				
				Parse(pParser, i, NULL);
				Parse(pParser, j, NULL);
				Parse(pParser, k, NULL);

				Parse(pParser, 0, NULL);
			}
		}
	}

//	return(0);

	fprintf(stderr, "\nQuad line tests\n");

	i = 0;
	j = 0;
	k = 0;
	l = 0;
	m = 0;

	for (i = 1; i <= kMaxToken; ++i)
	{
		// LINE_CONTINUATION can't be the first line
		if (i == LINE_CONTINUATION)
			break;
		
		for (j = 1; j <= kMaxToken; ++j) {

			for (k = 1; k <= kMaxToken; ++k) {

				for (l = 1; l <= kMaxToken; ++l) {
					
					Parse(pParser, i, NULL);
					Parse(pParser, j, NULL);
					Parse(pParser, k, NULL);
					Parse(pParser, l, NULL);

					Parse(pParser, 0, NULL);
				}
			}
		}
	}

//	return(0);

	fprintf(stderr, "\nFive line tests\n");

	i = 0;
	j = 0;
	k = 0;
	l = 0;
	m = 0;

	for (i = 1; i <= kMaxToken; ++i)
	{
		// LINE_CONTINUATION can't be the first line
		if (i == LINE_CONTINUATION)
			break;
		
		for (j = 1; j <= kMaxToken; ++j) {

			for (k = 1; k <= kMaxToken; ++k) {

				for (l = 1; l <= kMaxToken; ++l) {

					for (m = 1; m <= kMaxToken; ++m) {	

						Parse(pParser, i, NULL);
						Parse(pParser, j, NULL);
						Parse(pParser, k, NULL);
						Parse(pParser, l, NULL);
						Parse(pParser, m, NULL);

						Parse(pParser, 0, NULL);
					}
				}
			}
		}
	}

	return(0);

	// Six cycles takes quite a while

	fprintf(stderr, "\nSix line tests\n");

	i = 0;
	j = 0;
	k = 0;
	l = 0;
	m = 0;
	n = 0;

	for (i = 1; i <= kMaxToken; ++i)
	{
		// LINE_CONTINUATION can't be the first line
		if (i == LINE_CONTINUATION)
			break;

		fprintf(stderr, "%d\n", i);
		
		for (j = 1; j <= kMaxToken; ++j) {

			for (k = 1; k <= kMaxToken; ++k) {

				for (l = 1; l <= kMaxToken; ++l) {

					for (m = 1; m <= kMaxToken; ++m) {	

						for (n = 1; n <= kMaxToken; ++n) {	
							Parse(pParser, i, NULL);
							Parse(pParser, j, NULL);
							Parse(pParser, k, NULL);
							Parse(pParser, l, NULL);
							Parse(pParser, m, NULL);
							Parse(pParser, n, NULL);

							Parse(pParser, 0, NULL);
						}
					}
				}
			}
		}
	}
}

}


// Improved error messages for debugging:
//	http://stackoverflow.com/questions/11705737/expected-token-using-lemon-parser-generator

%syntax_error {
#ifndef NDEBUG
	fprintf(stderr,"Parser syntax error.\n");
	int n = sizeof(yyTokenName) / sizeof(yyTokenName[0]);
	for (i = 0; i < n; ++i) {
		int a = yy_find_shift_action(yypParser, (YYCODETYPE)i);
		if (a < YYNSTATE + YYNRULE) {
			fprintf(stderr,"expected token: %s\n", yyTokenName[i]);
		}
	}
#endif
}

%parse_failure {
	fprintf(stderr, "%d:%d:%d:%d:%d:%d Parser failed to successfully parse.\n", i, j, k, l, m, n);
}

