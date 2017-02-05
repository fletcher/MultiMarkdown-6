/**

	MultiMarkdown 6 -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file parser.y

	@brief Definition of the parser grammar, processed with lemon to create a parser.

		http://www.hwaci.com/sw/lemon/

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

%token_type { token * }

%extra_argument { mmd_engine * engine }

%fallback LINE_PLAIN LINE_TABLE_SEPARATOR.

%fallback LINE_CONTINUATION LINE_PLAIN LINE_INDENTED_TAB LINE_INDENTED_SPACE.

%fallback LINE_HTML LINE_ATX_1 LINE_ATX_2 LINE_ATX_3 LINE_ATX_4 LINE_ATX_5 LINE_ATX_6 LINE_HR LINE_BLOCKQUOTE LINE_LIST_BULLETED LINE_LIST_ENUMERATED LINE_TABLE LINE_DEF_CITATION LINE_DEF_FOOTNOTE LINE_DEF_LINK LINE_FENCE_BACKTICK LINE_FENCE_BACKTICK_START.

doc					::= blocks(B).								{ engine->root = B; }

blocks(A)			::= blocks(B) block(C).
	{
		strip_line_tokens_from_block(engine, C);
		if (B == NULL) { B = C; C = NULL;}
		A = B;
		token_chain_append(A, C);
		#ifndef NDEBUG
		fprintf(stderr, "Next block %d\n", A->tail->type);
		#endif
	}
blocks(A)			::= block(B).
	{
		strip_line_tokens_from_block(engine, B);
		#ifndef NDEBUG
		fprintf(stderr, "First block %d\n", B->type);
		#endif
		A = B;
	}

	
block(A)			::= para(B).								{ A = token_new_parent(B, BLOCK_PARA); is_para_html(engine, A); }
block(A)			::= indented_code(B).						{ A = token_new_parent(B, BLOCK_CODE_INDENTED); }
block(A)			::= LINE_ATX_1(B).							{ A = token_new_parent(B, BLOCK_H1); if (!(engine->extensions & EXT_NO_LABELS)) stack_push(engine->header_stack, A); }
block(A)			::= LINE_ATX_2(B).							{ A = token_new_parent(B, BLOCK_H2); if (!(engine->extensions & EXT_NO_LABELS)) stack_push(engine->header_stack, A); }
block(A)			::= LINE_ATX_3(B).							{ A = token_new_parent(B, BLOCK_H3); if (!(engine->extensions & EXT_NO_LABELS)) stack_push(engine->header_stack, A); }
block(A)			::= LINE_ATX_4(B).							{ A = token_new_parent(B, BLOCK_H4); if (!(engine->extensions & EXT_NO_LABELS)) stack_push(engine->header_stack, A); }
block(A)			::= LINE_ATX_5(B).							{ A = token_new_parent(B, BLOCK_H5); if (!(engine->extensions & EXT_NO_LABELS)) stack_push(engine->header_stack, A); }
block(A)			::= LINE_ATX_6(B).							{ A = token_new_parent(B, BLOCK_H6); if (!(engine->extensions & EXT_NO_LABELS)) stack_push(engine->header_stack, A); }
block(A)			::= empty(B).								{ A = token_new_parent(B, BLOCK_EMPTY); }
block(A)			::= list_bulleted(B).						{ A = token_new_parent(B, BLOCK_LIST_BULLETED); is_list_loose(A); }
block(A)			::= list_enumerated(B).						{ A = token_new_parent(B, BLOCK_LIST_ENUMERATED); is_list_loose(A); }
block(A)			::= blockquote(B).							{ A = token_new_parent(B, BLOCK_BLOCKQUOTE); recursive_parse_blockquote(engine, A); }
block(A)			::= table(B).								{ A = token_new_parent(B, BLOCK_TABLE); }
block(A)			::= LINE_HR(B).								{ A = token_new_parent(B, BLOCK_HR); }
block(A)			::= def_citation(B).						{ A = token_new_parent(B, BLOCK_DEF_CITATION); stack_push(engine->definition_stack, A); }
block(A)			::= def_footnote(B).						{ A = token_new_parent(B, BLOCK_DEF_FOOTNOTE); stack_push(engine->definition_stack, A); }
block(A)			::= def_link(B).							{ A = token_new_parent(B, BLOCK_DEF_LINK); stack_push(engine->definition_stack, A); }
block(A)			::= html_block(B).							{ A = token_new_parent(B, BLOCK_HTML); }
block(A)			::= fenced_block(B).						{ A = token_new_parent(B, BLOCK_CODE_FENCED); B->child->type = CODE_FENCE; }
block(A)			::= meta_block(B).							{ A = token_new_parent(B, BLOCK_META); }
block(A)			::= LINE_TOC(B).							{ A = token_new_parent(B, BLOCK_TOC); }
block(A)			::= definition_block(B).					{ A = token_new_parent(B, BLOCK_DEFLIST); }

para(A)				::= LINE_PLAIN(B) para_lines(C).			{ A = B; token_chain_append(B, C); }
para				::= LINE_PLAIN.

para_lines(A)		::= para_lines(B) para_line(C).				{ A = B; token_chain_append(B, C); }
para_lines			::= para_line.

para_line 			::= LINE_CONTINUATION.

indented_code(A)	::= indented_code(B) code_line(C).			{ A = B; token_chain_append(B, C); }
indented_code		::= indented_line.

code_line			::= indented_line.
code_line			::= LINE_EMPTY.

indented_line		::= LINE_INDENTED_TAB.
indented_line		::= LINE_INDENTED_SPACE.

empty(A)			::= empty(B) LINE_EMPTY(C).					{ A = B; token_chain_append(B, C); }
empty				::= LINE_EMPTY.

blockquote(A)		::= LINE_BLOCKQUOTE(B) quote_lines(C).		{ A = B; token_chain_append(B, C); }
blockquote			::= LINE_BLOCKQUOTE.

quote_lines(A)		::= quote_lines(B) quote_line(C).			{ A = B; token_chain_append(B, C); }
quote_lines			::= quote_line.

quote_line			::= LINE_BLOCKQUOTE.
quote_line			::= LINE_CONTINUATION.

list_bulleted(A)	::= list_bulleted(B) item_bulleted(C).		{ A = B; token_chain_append(B, C); }
list_bulleted		::=	item_bulleted.

item_bulleted(A)	::= LINE_LIST_BULLETED(B) para_lines(C) cont_blocks(D).	{ token_chain_append(B, C); token_chain_append(B, D); A = token_new_parent(B, BLOCK_LIST_ITEM); recursive_parse_list_item(engine, A); }
item_bulleted(A)	::= LINE_LIST_BULLETED(B) para_lines(C).	{ token_chain_append(B, C); A = token_new_parent(B, BLOCK_LIST_ITEM_TIGHT); recursive_parse_list_item(engine, A); }
item_bulleted(A)	::= LINE_LIST_BULLETED(B) cont_blocks(C).	{ token_chain_append(B, C); A = token_new_parent(B, BLOCK_LIST_ITEM); if (C) {recursive_parse_list_item(engine, A);} }
item_bulleted(A)	::= LINE_LIST_BULLETED(B).					{ A = token_new_parent(B, BLOCK_LIST_ITEM_TIGHT); }

list_enumerated(A)	::= list_enumerated(B) item_enumerated(C).	{ A = B; token_chain_append(B, C); }
list_enumerated		::=	item_enumerated.

item_enumerated(A)	::= LINE_LIST_ENUMERATED(B) para_lines(C) cont_blocks(D).	{ token_chain_append(B, C); token_chain_append(B, D); A = token_new_parent(B, BLOCK_LIST_ITEM); recursive_parse_list_item(engine, A); }
item_enumerated(A)	::= LINE_LIST_ENUMERATED(B) para_lines(C).	{ token_chain_append(B, C); A = token_new_parent(B, BLOCK_LIST_ITEM_TIGHT); recursive_parse_list_item(engine, A); }
item_enumerated(A)	::= LINE_LIST_ENUMERATED(B) cont_blocks(C).	{ token_chain_append(B, C); A = token_new_parent(B, BLOCK_LIST_ITEM); recursive_parse_list_item(engine, A); }
item_enumerated(A)	::= LINE_LIST_ENUMERATED(B).				{ A = token_new_parent(B, BLOCK_LIST_ITEM_TIGHT); }

cont_blocks(A)		::= cont_blocks(B) cont_block(C).			{ A = B; token_chain_append(B, C); }
cont_blocks			::= cont_block.

cont_block(A)		::= empty(B) indented_line(C) para_lines(D).{ A = B; token_chain_append(B, C); token_chain_append(B, D); C->type = LINE_CONTINUATION; }
cont_block(A)		::= empty(B) indented_line(C).				{ A = B; token_chain_append(B, C); C->type = LINE_CONTINUATION; }
cont_block			::= empty.

table(A)			::= table_header(B) table_body(C) LINE_EMPTY(D).	{ A = B; token_chain_append(B, C); token_chain_append(B, D); }
table(A)			::= table_header(B) table_body(C).			{ A = B; token_chain_append(B, C); }

table_header(A)		::= table_section(B) LINE_TABLE_SEPARATOR(C).	{ A = token_new_parent(B, BLOCK_TABLE_HEADER); token_chain_append(B, C); }

table_body(A)		::= table_body(B) LINE_EMPTY(C) table_section(D).	{ A = B; token_chain_append(B, C); token_chain_append(B, token_new_parent(D, BLOCK_TABLE_SECTION)); }
table_body(A)		::= table_section(B).						{ A = token_new_parent(B, BLOCK_TABLE_SECTION); }

table_section(A)	::= table_section(B) LINE_TABLE(C).			{ A = B; token_chain_append(B, C); }
table_section		::= LINE_TABLE.

def_citation(A)		::= LINE_DEF_CITATION(B) para_lines(C) cont_blocks(D).	{ A = B; token_chain_append(B, C); token_chain_append(B, D); }
def_citation(A)		::= LINE_DEF_CITATION(B) para_lines(C).		{ A = B; token_chain_append(B, C); }
def_citation(A)		::= LINE_DEF_CITATION(B) cont_blocks(C).	{ A = B; token_chain_append(B, C); }
def_citation		::= LINE_DEF_CITATION.

def_footnote(A)		::= LINE_DEF_FOOTNOTE(B) para_lines(C) cont_blocks(D).	{ A = B; token_chain_append(B, C); token_chain_append(B, D); }
def_footnote(A)		::= LINE_DEF_FOOTNOTE(B) para_lines(C).		{ A = B; token_chain_append(B, C); }
def_footnote(A)		::= LINE_DEF_FOOTNOTE(B) cont_blocks(C).	{ A = B; token_chain_append(B, C); }
def_footnote		::= LINE_DEF_FOOTNOTE.

def_link(A)			::= LINE_DEF_LINK(B) para_lines(C).			{ A = B; token_chain_append(B, C); }
def_link			::= LINE_DEF_LINK.

html_block(A)		::= LINE_HTML(B) html_block_lines(C).		{ A = B; token_chain_append(B, C); }
html_block			::= LINE_HTML.


html_block_lines(A)	::= html_block_lines(B) html_block_line(C).	{ A = B; token_chain_append(B, C); }
html_block_lines	::= html_block_line.

html_block_line		::= LINE_CONTINUATION.
html_block_line		::= LINE_HTML.

fenced_block(A)		::= LINE_FENCE_BACKTICK(B) fenced_lines(C) LINE_FENCE_BACKTICK(D).		{ A = B; token_chain_append(B, C); token_chain_append(B, D); D->child->type = CODE_FENCE; }
fenced_block(A)		::= LINE_FENCE_BACKTICK(B) fenced_lines(C).								{ A = B; token_chain_append(B, C); }
fenced_block(A)		::= LINE_FENCE_BACKTICK_START(B) fenced_lines(C) LINE_FENCE_BACKTICK(D).	{ A = B; token_chain_append(B, C); token_chain_append(B, D); D->child->type = CODE_FENCE; }
fenced_block(A)		::= LINE_FENCE_BACKTICK_START(B) fenced_lines(C).							{ A = B; token_chain_append(B, C); }


fenced_lines(A)		::= fenced_lines(B) fenced_line(C).			{ A = B; token_chain_append(B, C); }
fenced_lines		::= fenced_line.

fenced_line			::= LINE_CONTINUATION.
fenced_line			::= LINE_EMPTY.


meta_block(A)		::= LINE_META(B) meta_lines(C).				{ A = B; token_chain_append(B, C); }
meta_block			::= LINE_META.

meta_lines(A)		::= meta_lines(B) meta_line(C).				{ A = B; token_chain_append(B, C); }
meta_lines			::= meta_line.

meta_line 			::= LINE_META.
meta_line 			::= LINE_CONTINUATION.

definition_block(A)	::= definition_block(B) empty(C) definition(D).		{ A = B; token_chain_append(B, C); token_chain_append(B, D); }
definition_block(A)	::= definition_block(B) empty(C).				{ A = B; token_chain_append(B, C); }
definition_block	::= definition.

definition(A)		::= para(B) defs(C).						{ A = token_new_parent(B, BLOCK_DEFINITION_GROUP); token_chain_append(B, C); B->type = BLOCK_TERM; }

defs(A)				::= defs(B) def(C).							{ A = B; token_chain_append(B, C); }
defs				::= def.

def(A)				::= LINE_DEFINITION(B) def_lines(C).		{ A = token_new_parent(B, BLOCK_DEFINITION); token_chain_append(B, C); }
def(A)				::= LINE_DEFINITION(B).						{ A = token_new_parent(B, BLOCK_DEFINITION); }

def_lines(A)		::= def_lines(B) LINE_CONTINUATION(C).		{ A = B; token_chain_append(B, C); }
def_lines			::= LINE_CONTINUATION.


// Fallbacks for improper structures
para(A)				::= table_section(B) LINE_EMPTY(C).			{ A = B; token_chain_append(B, C); }
para(A)				::= table_section(B) para_lines(C).			{ A = B; token_chain_append(B, C); }
para(A)				::= table_section(B).						{ A = B; }



//
// Additional Configuration
//

%include {
	#include <assert.h>
	#include <stdio.h>
	#include <stdlib.h>

	#include "libMultiMarkdown.h"
	#include "mmd.h"
	#include "parser.h"
	#include "token.h"
}


// Improved error messages for debugging:
//	http://stackoverflow.com/questions/11705737/expected-token-using-lemon-parser-generator

%syntax_error {
#ifndef NDEBUG
	fprintf(stderr,"Parser syntax error.\n");
	int n = sizeof(yyTokenName) / sizeof(yyTokenName[0]);
	for (int i = 0; i < n; ++i) {
		int a = yy_find_shift_action(yypParser, (YYCODETYPE)i);
		if (a < YYNSTATE + YYNRULE) {
			fprintf(stderr,"expected token: %s\n", yyTokenName[i]);
		}
	}
#endif
}

%parse_failure {
	fprintf(stderr, "Parser failed to successfully parse.\n");
}

