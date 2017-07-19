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

%fallback LINE_HR LINE_SETEXT_1 LINE_SETEXT_2 LINE_YAML.

//%fallback LINE_PLAIN LINE_TABLE_SEPARATOR.

%fallback LINE_CONTINUATION LINE_PLAIN LINE_INDENTED_TAB LINE_INDENTED_SPACE LINE_TABLE LINE_TABLE_SEPARATOR.

%fallback LINE_FALLBACK LINE_HTML LINE_ATX_1 LINE_ATX_2 LINE_ATX_3 LINE_ATX_4 LINE_ATX_5 LINE_ATX_6
LINE_BLOCKQUOTE LINE_LIST_BULLETED LINE_LIST_ENUMERATED LINE_DEF_ABBREVIATION LINE_DEF_CITATION
LINE_DEF_FOOTNOTE LINE_DEF_GLOSSARY LINE_DEF_LINK LINE_TOC LINE_DEFINITION LINE_META.

%fallback LINE_BACKTICK LINE_FENCE_BACKTICK_3 LINE_FENCE_BACKTICK_4 LINE_FENCE_BACKTICK_5
LINE_FENCE_BACKTICK_START_3 LINE_FENCE_BACKTICK_START_4 LINE_FENCE_BACKTICK_START_5.


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
		engine->root = B;	// In case the first block is metadata and we just want to know if it exists
		strip_line_tokens_from_block(engine, B);
		#ifndef NDEBUG
		fprintf(stderr, "First block %d\n", B->type);
		#endif
		A = B;
	}


// Blocks

// Single line blocks

block(A)			::= LINE_ATX_1(B).			{ A = token_new_parent(B, BLOCK_H1); stack_push(engine->header_stack, A); }
block(A)			::= LINE_ATX_2(B).			{ A = token_new_parent(B, BLOCK_H2); stack_push(engine->header_stack, A); }
block(A)			::= LINE_ATX_3(B).			{ A = token_new_parent(B, BLOCK_H3); stack_push(engine->header_stack, A); }
block(A)			::= LINE_ATX_4(B).			{ A = token_new_parent(B, BLOCK_H4); stack_push(engine->header_stack, A); }
block(A)			::= LINE_ATX_5(B).			{ A = token_new_parent(B, BLOCK_H5); stack_push(engine->header_stack, A); }
block(A)			::= LINE_ATX_6(B).			{ A = token_new_parent(B, BLOCK_H6); stack_push(engine->header_stack, A); }

block(A)			::= LINE_HR(B).				{ A = token_new_parent(B, BLOCK_HR); }
block(A)			::= LINE_YAML(B).			{ A = token_new_parent(B, BLOCK_HR); }

block(A)			::= LINE_TOC(B).			{ A = token_new_parent(B, BLOCK_TOC); }


// Multi-line blocks

block(A)			::= blockquote(B).			{ A = token_new_parent(B, BLOCK_BLOCKQUOTE); recursive_parse_blockquote(engine, A); }
block(A)			::= def_abbreviation(B).	{ A = token_new_parent(B, BLOCK_DEF_ABBREVIATION); stack_push(engine->definition_stack, A); }
block(A)			::= def_citation(B).		{ A = token_new_parent(B, BLOCK_DEF_CITATION); stack_push(engine->definition_stack, A); }
block(A)			::= def_footnote(B).		{ A = token_new_parent(B, BLOCK_DEF_FOOTNOTE); stack_push(engine->definition_stack, A); recursive_parse_indent(engine, A); }
block(A)			::= def_glossary(B).		{ A = token_new_parent(B, BLOCK_DEF_GLOSSARY); stack_push(engine->definition_stack, A); recursive_parse_indent(engine, A); }
block(A)			::= def_link(B).			{ A = token_new_parent(B, BLOCK_DEF_LINK); stack_push(engine->definition_stack, A); }
block(A)			::= definition_block(B).	{ A = token_new_parent(B, BLOCK_DEFLIST); }
block(A)			::= empty(B).				{ A = token_new_parent(B, BLOCK_EMPTY); }
block(A)			::= fenced_block(B).		{ A = token_new_parent(B, BLOCK_CODE_FENCED); B->child->type = CODE_FENCE; }
block(A)			::= html_block(B).			{ A = token_new_parent(B, BLOCK_HTML); }
block(A)			::= html_com_block(B).		{ A = token_new_parent(B, BLOCK_HTML); }
block(A)			::= indented_code(B).		{ A = token_new_parent(B, BLOCK_CODE_INDENTED); }
block(A)			::= list_bullet(B).			{ A = token_new_parent(B, BLOCK_LIST_BULLETED); is_list_loose(A); }
block(A)			::= list_enum(B).			{ A = token_new_parent(B, BLOCK_LIST_ENUMERATED); is_list_loose(A); }
block(A)			::= meta_block(B).			{ A = token_new_parent(B, BLOCK_META); }
block(A)			::= meta_block(B) LINE_SETEXT_2(C).	{ A = token_new_parent(B, BLOCK_META); token_chain_append(A, C); }
block(A)			::= para(B).				{ A = token_new_parent(B, BLOCK_PARA); is_para_html(engine, A); }
block(A)			::= setext_1(B).			{ A = token_new_parent(B, BLOCK_SETEXT_1); stack_push(engine->header_stack, A); }
block(A)			::= setext_2(B).			{ A = token_new_parent(B, BLOCK_SETEXT_2); stack_push(engine->header_stack, A); }
block(A)			::= table(B).				{ A = token_new_parent(B, BLOCK_TABLE); stack_push(engine->table_stack, A); }


// Reusable components

// A "chunk" allows you to grab all regular lines before the next empty line.
// For example, to grab all lines after the first line in a paragraph.
// You will want to specify a certain line type to start a chunk, but
// this can grab the remaining lines, if any.

chunk(A)			::= chunk(B) chunk_line(C).					{ A = B; token_chain_append(B, C); }
chunk				::= chunk_line.

chunk_line			::= LINE_CONTINUATION.
chunk_line			::= LINE_STOP_COMMENT.


// A "nested chunk" is useful when a chunk can also include following blocks
// that have an extra level of indention to indicate that they belong together.
// For example, a list item can include multiple paragraphs, even other lists.
// This structure is also used in footnotes and definitions.

nested_chunks(A)	::= nested_chunks(B) nested_chunk(C).		{ A = B; token_chain_append(B, C); }
nested_chunks		::= nested_chunk.

nested_chunk(A)		::= empty(B) indented_line(C) chunk(D).		{ A = B; token_chain_append(B, C); token_chain_append(B, D); C->type = LINE_CONTINUATION; }
nested_chunk(A)		::= empty(B) indented_line(C).				{ A = B; token_chain_append(B, C); C->type = LINE_CONTINUATION; }
nested_chunk		::= empty.

indented_line		::= LINE_INDENTED_TAB.
indented_line		::= LINE_INDENTED_SPACE.


// Shortcut for optional chunk
//opt_chunk			::= chunk.
//opt_chunk			::= .


// Shortcut for "extended chunk" (chunk with following blocks)
ext_chunk(A)		::= chunk(B) nested_chunks(C).				{ A = B; token_chain_append(B, C); }


// Shortcut for optionally extended chunk
opt_ext_chunk(A)	::= chunk(B) nested_chunks(C).				{ A = B; token_chain_append(B, C); }
opt_ext_chunk		::= chunk.


// Shortcut for anything that falls into the extended chunk pattern
tail				::= opt_ext_chunk.
tail				::= nested_chunks.
//tail 				::= empty.


// Blockquotes
blockquote(A)		::= blockquote(B) quote_line(C).			{ A = B; token_chain_append(B, C); }
blockquote 			::= LINE_BLOCKQUOTE.

quote_line			::= LINE_BLOCKQUOTE.
quote_line			::= LINE_CONTINUATION.


// Reference definitions
def_citation(A)		::= LINE_DEF_CITATION(B) tail(C).			{ A = B; token_chain_append(B, C); }
def_citation		::= LINE_DEF_CITATION.

def_footnote(A)		::= LINE_DEF_FOOTNOTE(B) tail(C).			{ A = B; token_chain_append(B, C); }
def_footnote		::= LINE_DEF_FOOTNOTE.

def_glossary(A)		::= LINE_DEF_GLOSSARY(B) tail(C).			{ A = B; token_chain_append(B, C); }
def_glossary		::= LINE_DEF_GLOSSARY.

def_link(A)			::= LINE_DEF_LINK(B) chunk(C).				{ A = B; token_chain_append(B, C); }
def_link			::= LINE_DEF_LINK.

def_abbreviation(A)	::= LINE_DEF_ABBREVIATION(B) chunk(C).		{ A = B; token_chain_append(B, C); }
def_abbreviation	::= LINE_DEF_ABBREVIATION.


// Definition lists
// Lemon's LALR(1) parser can't properly allow for detecting consecutive definition blocks and concatenating them,
// because 'para defs para' could be the beginning of the next definition, OR the next regular para.
// We have to bundle them when exporting, if desired.
definition_block(A)	::= para(B) defs(C).						{ A = B; token_chain_append(B, C); B->type = BLOCK_TERM; }

defs(A)				::= defs(B) def(C).							{ A = B; token_chain_append(B, C); }
defs				::= def.									

def(A)				::= LINE_DEFINITION(B) tail(C).				{ A = token_new_parent(B, BLOCK_DEFINITION); token_chain_append(B, C); recursive_parse_indent(engine, A); }
def(A)				::= LINE_DEFINITION(B).						{ A = token_new_parent(B, BLOCK_DEFINITION);  }


// Empty lines
empty(A)			::= empty(B) LINE_EMPTY(C).					{ A = B; token_chain_append(B, C); }
empty				::= LINE_EMPTY.


// Fenced code blocks

fenced_block(A)		::= fenced_3(B) LINE_FENCE_BACKTICK_3(C).	{ A = B; token_chain_append(B, C); C->child->type = CODE_FENCE; }
fenced_block(A)		::= fenced_3(B) LINE_FENCE_BACKTICK_4(C).	{ A = B; token_chain_append(B, C); C->child->type = CODE_FENCE; }
fenced_block(A)		::= fenced_3(B) LINE_FENCE_BACKTICK_5(C).	{ A = B; token_chain_append(B, C); C->child->type = CODE_FENCE; }
fenced_block		::= fenced_3.

fenced_3(A)			::= fenced_3(B) fenced_line(C).				{ A = B; token_chain_append(B, C); }
fenced_3			::= LINE_FENCE_BACKTICK_3.
fenced_3			::= LINE_FENCE_BACKTICK_START_3.


fenced_block(A)		::= fenced_4(B) LINE_FENCE_BACKTICK_4(C).	{ A = B; token_chain_append(B, C); C->child->type = CODE_FENCE; }
fenced_block(A)		::= fenced_4(B) LINE_FENCE_BACKTICK_5(C).	{ A = B; token_chain_append(B, C); C->child->type = CODE_FENCE; }
fenced_block		::= fenced_4.

fenced_4(A)			::= fenced_4(B) fenced_line(C).				{ A = B; token_chain_append(B, C); }
fenced_4(A)			::= fenced_4(B) LINE_FENCE_BACKTICK_3(C).	{ A = B; token_chain_append(B, C); }
fenced_4(A)			::= fenced_4(B) LINE_FENCE_BACKTICK_START_3(C).	{ A = B; token_chain_append(B, C); }
fenced_4			::= LINE_FENCE_BACKTICK_4.
fenced_4			::= LINE_FENCE_BACKTICK_START_4.


fenced_block(A)		::= fenced_5(B) LINE_FENCE_BACKTICK_5(C).	{ A = B; token_chain_append(B, C); C->child->type = CODE_FENCE; }
fenced_block		::= fenced_5.

fenced_5(A)			::= fenced_5(B) fenced_line(C).				{ A = B; token_chain_append(B, C); }
fenced_5(A)			::= fenced_5(B) LINE_FENCE_BACKTICK_3(C).	{ A = B; token_chain_append(B, C); }
fenced_5(A)			::= fenced_5(B) LINE_FENCE_BACKTICK_START_3(C).	{ A = B; token_chain_append(B, C); }
fenced_5(A)			::= fenced_5(B) LINE_FENCE_BACKTICK_4(C).	{ A = B; token_chain_append(B, C); }
fenced_5(A)			::= fenced_5(B) LINE_FENCE_BACKTICK_START_4(C).	{ A = B; token_chain_append(B, C); }
fenced_5			::= LINE_FENCE_BACKTICK_5.
fenced_5			::= LINE_FENCE_BACKTICK_START_5.

fenced_line			::= LINE_CONTINUATION.
fenced_line			::= LINE_EMPTY.
fenced_line			::= LINE_FALLBACK.
fenced_line			::= LINE_HR.
fenced_line			::= LINE_HTML.
fenced_line			::= LINE_START_COMMENT.
fenced_line			::= LINE_STOP_COMMENT.


// HTML
html_block(A)		::= html_block(B) html_line(C).				{ A = B; token_chain_append(B, C); }
html_block			::= LINE_HTML.

html_line			::= LINE_CONTINUATION.
html_line			::= LINE_FALLBACK.
html_line			::= LINE_HR.
html_line			::= LINE_HTML.


// HTML Comment
html_com_block(A)	::= html_comment(B) LINE_STOP_COMMENT(C).	{ A = B; token_chain_append(B, C); }
html_com_block 		::= html_comment.

html_comment(A)		::= html_comment(B) comment_line(C).		{ A = B; token_chain_append(B, C); }
html_comment		::= LINE_START_COMMENT.

comment_line		::= LINE_CONTINUATION.
comment_line		::= LINE_EMPTY.
comment_line		::= LINE_FALLBACK.
comment_line		::= LINE_HR.
comment_line		::= LINE_HTML.


// Indented code blocks
indented_code(A)	::= indented_code(B) indented_line(C).		{ A = B; token_chain_append(B, C); }
indented_code(A)	::= indented_code(B) LINE_EMPTY(C).			{ A = B; token_chain_append(B, C); }
indented_code		::= indented_line.


// Bulleted lists
list_bullet(A)		::= list_bullet(B) item_bullet(C).			{ A = B; token_chain_append(B, C); }
list_bullet			::=	item_bullet.

item_bullet(A)		::= LINE_LIST_BULLETED(B) ext_chunk(C).		{ A = token_new_parent(B, BLOCK_LIST_ITEM); token_chain_append(B, C); recursive_parse_list_item(engine, A); }
item_bullet(A)		::= LINE_LIST_BULLETED(B) chunk(C).			{ A = token_new_parent(B, BLOCK_LIST_ITEM_TIGHT); token_chain_append(B, C); recursive_parse_list_item(engine, A); }
item_bullet(A)		::= LINE_LIST_BULLETED(B) nested_chunks(C).	{ A = token_new_parent(B, BLOCK_LIST_ITEM); token_chain_append(B, C); recursive_parse_list_item(engine, A); }
item_bullet(A)		::= LINE_LIST_BULLETED(B).					{ A = token_new_parent(B, BLOCK_LIST_ITEM_TIGHT); }


// Enumerated lists
list_enum(A)		::= list_enum(B) item_enum(C).				{ A = B; token_chain_append(B, C); }
list_enum			::=	item_enum.

item_enum(A)		::= LINE_LIST_ENUMERATED(B) ext_chunk(C).	{ A = token_new_parent(B, BLOCK_LIST_ITEM); token_chain_append(B, C); recursive_parse_list_item(engine, A); }
item_enum(A)		::= LINE_LIST_ENUMERATED(B) chunk(C).		{ A = token_new_parent(B, BLOCK_LIST_ITEM_TIGHT); token_chain_append(B, C); recursive_parse_list_item(engine, A); }
item_enum(A)		::= LINE_LIST_ENUMERATED(B) nested_chunks(C).	{ A = token_new_parent(B, BLOCK_LIST_ITEM); token_chain_append(B, C); recursive_parse_list_item(engine, A); }
item_enum(A)		::= LINE_LIST_ENUMERATED(B).				{ A = token_new_parent(B, BLOCK_LIST_ITEM_TIGHT); }


// Metadata
meta_block(A)		::= meta_block(B) meta_line(C).				{ A = B; token_chain_append(B, C); }
meta_block 			::= LINE_META.
meta_block(A)		::= LINE_YAML(B) LINE_META(C).				{ A = B; token_chain_append(B, C); }

meta_line 			::= LINE_META.
meta_line 			::= LINE_CONTINUATION.


// Paragraphs
para(A)				::= LINE_PLAIN(B) chunk(C).					{ A = B; token_chain_append(B, C); }
para				::= LINE_PLAIN.
para				::= LINE_STOP_COMMENT.


// Setext headers
setext_1(A)			::= para(B) LINE_SETEXT_1(C).				{ A = B; token_chain_append(B, C); }
setext_2(A)			::= para(B) LINE_SETEXT_2(C).				{ A = B; token_chain_append(B, C); }


// Tables
table(A)			::= table_header(B) table_body(C).			{ A = B; token_chain_append(B, C); }
table				::= table_header.

table_header(A)		::= header_rows(B) LINE_TABLE_SEPARATOR(C).	{ A = token_new_parent(B, BLOCK_TABLE_HEADER); token_chain_append(B, C); }

header_rows(A)		::= header_rows(B) LINE_TABLE(C).			{ A = B; token_chain_append(B, C); }
header_rows			::= LINE_TABLE.

table_body(A)		::= table_body(B) table_section(C).			{ A = B; token_chain_append(B, C); }
table_body			::= table_section.

table_section(A)	::= all_rows(B) LINE_EMPTY(C).				{ A = token_new_parent(B, BLOCK_TABLE_SECTION); token_chain_append(B, C); }
table_section(A)	::= all_rows(B).							{ A = token_new_parent(B, BLOCK_TABLE_SECTION); }

all_rows(A)			::= all_rows(B) row(C).						{ A = B; token_chain_append(B, C); }
all_rows			::= row.

row					::= header_rows.
row					::= LINE_TABLE_SEPARATOR.


// Fallbacks for improper structures
para(A)				::= all_rows(B).							{ A = B; }
para				::= defs.


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

