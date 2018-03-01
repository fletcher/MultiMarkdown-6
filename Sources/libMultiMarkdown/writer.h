/**

	MultiMarkdown 6 -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file writer.h

	@brief Coordinate conversion of token tree to output formats.


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


#ifndef WRITER_MULTIMARKDOWN_H
#define WRITER_MULTIMARKDOWN_H

#ifdef TEST
	#include "CuTest.h"
#endif

#include "d_string.h"
#include "mmd.h"
#include "stack.h"
#include "token.h"
#include "uthash.h"


#define kMaxExportRecursiveDepth 1000		//!< Maximum recursion depth when exporting token tree -- to prevent stack overflow with "pathologic" input

#define kMaxTableColumns 48					//!< Maximum number of table columns for specifying alignment

typedef struct {
	struct link *		link_hash;
	struct meta *		meta_hash;

	unsigned long		extensions;
	short				output_format;
	short				padded;			//!< How many empty lines at end output buffer
	short				list_is_tight;
	short				close_para;
	short				skip_token;

	short				footnote_para_counter;
	stack *				used_footnotes;
	stack *				inline_footnotes_to_free;
	struct fn_holder *	footnote_hash;
	short				footnote_being_printed;

	int 				random_seed_base;

	stack *				used_citations;
	stack *				inline_citations_to_free;
	struct fn_holder *	citation_hash;
	short				citation_being_printed;
	char *				bibtex_file;

	stack *				used_glossaries;
	stack *				inline_glossaries_to_free;
	struct fn_holder *	glossary_hash;
	short				glossary_being_printed;

	stack *				used_abbreviations;
	stack *				inline_abbreviations_to_free;
	struct fn_holder *	abbreviation_hash;

	short				language;
	short				quotes_lang;

	short				base_header_level;

	stack *				header_stack;

	stack *				outline_stack;

	short				recurse_depth;

	short				in_table_header;
	short				table_column_count;
	short				table_cell_count;
	char 				table_alignment[kMaxTableColumns];

	short				odf_para_type;

	struct asset *		asset_hash;
	short				store_assets;
	short				remember_assets;
} scratch_pad;


struct attr {
	char *				key;
	char *				value;
	struct attr *		next;
};

typedef struct attr attr;

struct link {
	token *				label;
	char * 				label_text;
	char *				clean_text;
	char *				url;
	char *				title;
	attr *				attributes;
	short				flags;
	UT_hash_handle		hh;
};

enum link_flags {
	LINK_INLINE       = 1 << 0,			//!< Inline link, e.g. [foo](#bar)
	LINK_IMPLICIT     = 1 << 1,			//!< Implicit link, e.g. [foo]
	LINK_REFERENCE    = 1 << 2,			//!< Reference definition
	LINK_AUTO         = 1 << 3, 		//!< Automatically generated link (e.g. Headers, tables)
};

typedef struct link link;

struct footnote {
	token *				label;
	char *				label_text;
	char *				clean_text;
	token *				content;
	size_t				count;
	bool				free_para;

	char 				_PADDING[7];	//!< pad struct for alignment
};

typedef struct footnote footnote;

struct fn_holder {
	footnote *			note;
	UT_hash_handle		hh;
};

typedef struct fn_holder fn_holder;

struct meta {
	char *				key;
	char *				value;
	size_t				start;
	UT_hash_handle		hh;
};

typedef struct meta meta;

struct abbr {
	char *				abbr;
	size_t				abbr_len;
	char *				expansion;
	size_t				expansion_len;
	UT_hash_handle		hh;
};

typedef struct abbr abbr;


/// Temporary storage while exporting parse tree to output format
scratch_pad * scratch_pad_new(mmd_engine * e, short format);

void scratch_pad_free(scratch_pad * scratch);


/// Ensure at least num newlines at end of output buffer
void pad(DString * d, short num, scratch_pad * scratch);

link * explicit_link(scratch_pad * scratch, token * label, token * url, const char * source);

/// Find link based on label
link * extract_link_from_stack(scratch_pad * scratch, const char * target);

char * text_inside_pair(const char * source, token * pair);
char * clean_inside_pair(const char * source, token * t, bool lowercase);

void link_free(link * l);
void footnote_free(footnote * f);

char * label_from_token(const char * source, token * t);
char * label_from_header(const char * source, token * t);

void parse_brackets(const char * source, scratch_pad * scratch, token * bracket, link ** link, short * skip_token, bool * free_link);


void print_token_raw(DString * out, const char * source, token * t);

void print_token_tree_raw(DString * out, const char * source, token * t);

char * url_accept(const char * source, size_t start, size_t max_len, size_t * end_pos, bool validate);

void abbreviation_from_bracket(const char * source, scratch_pad * scratch, token * t, short * num);
void citation_from_bracket(const char * source, scratch_pad * scratch, token * t, short * num);
void footnote_from_bracket(const char * source, scratch_pad * scratch, token * t, short * num);
void glossary_from_bracket(const char * source, scratch_pad * scratch, token * t, short * num);

meta * meta_new(const char * source, size_t start, size_t len);
void meta_set_value(meta * m, const char * value);
void meta_free(meta * m);
char * extract_metadata(scratch_pad * scratch, const char * target);
meta * extract_meta_from_stack(scratch_pad * scratch, const char * target);

void read_table_column_alignments(const char * source, token * table, scratch_pad * scratch);

void strip_leading_whitespace(token * chain, const char * source);

bool table_has_caption(token * table);

char * get_fence_language_specifier(token * fence, const char * source);

token * manual_label_from_header(token * h, const char * source);

char * label_from_string(const char * str);

char * clean_string(const char * str, bool lowercase);

short raw_level_for_header(token * header);

void store_asset(scratch_pad * scratch_pad, char * url);
asset * extract_asset(scratch_pad * scratch, char * url);
void asset_free(asset * a);

bool raw_filter_text_matches(char * pattern, short format);
bool raw_filter_matches(token * filter, const char * source, short format);


#endif

