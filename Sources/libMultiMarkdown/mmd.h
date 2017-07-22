/**

	MultiMarkdown 6 -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file mmd.h

	@brief Create MMD parsing engine


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


#ifndef MMD_MULTIMARKDOWN_H
#define MMD_MULTIMARKDOWN_H

#include "d_string.h"
#include "libMultiMarkdown.h"
#include "stack.h"
#include "token.h"
#include "token_pairs.h"
#include "uthash.h"


#define kMaxParseRecursiveDepth 1000		//!< Maximum recursion depth when parsing -- to prevent stack overflow with "pathologic" input


struct mmd_engine {
	DString *				dstr;
	token *					root;
	unsigned long			extensions;
	unsigned short			recurse_depth;

	bool					allow_meta;

	token_pair_engine *		pairings1;
	token_pair_engine *		pairings2;
	token_pair_engine *		pairings3;
	token_pair_engine *		pairings4;

	stack *					abbreviation_stack;
	stack *					citation_stack;
	stack *					definition_stack;
	stack *					footnote_stack;
	stack *					glossary_stack;
	stack *					header_stack;
	stack *					link_stack;
	stack *					metadata_stack;
	stack *					table_stack;

	short					language;
	short					quotes_lang;

	struct asset *			asset_hash;
};


/// Expose routines to lemon parser
void recursive_parse_indent(mmd_engine * e, token * block);
void recursive_parse_list_item(mmd_engine * e, token * block);
void recursive_parse_blockquote(mmd_engine * e, token * block);
void strip_line_tokens_from_block(mmd_engine * e, token * block);
void is_para_html(mmd_engine * e, token * block);


void is_list_loose(token * list);


struct asset {
	char *				url;
	char *				asset_path;
	UT_hash_handle		hh;
};

typedef struct asset asset;


#endif
