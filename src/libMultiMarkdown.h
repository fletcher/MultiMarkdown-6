/**

	MultiMarkdown 6 -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file libMultiMarkdown.h

	@brief Header file for libMultiMarkdown.


	@author	Fletcher T. Penney
	@bug


	******IMPORTANT******

	If you are using libMultiMarkdown in your own project, you need to either:

	1. Disable kUseObjectPool in `token.h`

	2. Properly manage the `token_pool_init` and `token_pool_free` functions.


	I recommend option #1, unless you absolutely need the best performance for
	long documents.  Doing #2 properly is tricky in any program that can handle
	multiple MMD text strings at overlapping times.

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


#ifndef MMD6_H
#define MMD6_H

#include <stdbool.h>
#include <stdlib.h>


/// typedefs for internal data structures.  If you intend to work with these structures
/// in your own code, you may need to import additional header files.

/// From token.h:
typedef struct token token;

/// From d_string.h:
typedef struct DString DString;

/// From mmd.h
typedef struct mmd_engine mmd_engine;

/// From stack.h
typedef struct stack stack;


/// There are 3 main versions of the primary functions:
///
///	* `mmd_string...` -- start from source text in c string
/// * `mmd_d_string...` -- start from a DString (Useful if you already use DString's for your text)
/// * `mmd_engine...` -- useful when you are processing the same source multiple times


/*
	C string variants
*/

/// Convert OPML string to MMD
DString * mmd_string_convert_opml_to_text(const char * source);


/// Convert ITMZ string to MMD
DString * mmd_string_convert_itmz_to_text(const char * source);


/// Convert MMD text to specified format, with specified extensions, and language
/// Returned char * must be freed
char * mmd_string_convert(const char * source, unsigned long extensions, short format, short language);


/// Convert MMD text to specified format using DString as a container for block of data
/// and length of that block.  Must be used for "complex" output formats such as EPUB.
/// Returned DString * must be freed
DString * mmd_string_convert_to_data(const char * source, unsigned long extensions, short format, short language, const char * directory);


/// Convert MMD text and write results to specified file -- used for "complex" output formats requiring
/// multiple documents (e.g. EPUB)
void mmd_string_convert_to_file(const char * source, unsigned long extensions, short format, short language, const char * directory, const char * filepath);


/// Does the text have metadata?
bool mmd_string_has_metadata(char * source, size_t * end);


/// Return metadata keys, one per line
/// Returned char * must be freed
char * mmd_string_metadata_keys(char * source);


/// Extract desired metadata as string value
/// Returned char * must be freed
char * mmd_string_metavalue_for_key(char * source, const char * key);


/// Insert/replace metadata in string, returning new string
char * mmd_string_update_metavalue_for_key(const char * source, const char * key, const char * value);


/// Grab list of all transcluded files, but we need to know directory to search,
/// as well as the path to the file
/// Returned stack needs to be freed
struct stack * mmd_string_transclusion_manifest(const char * source, const char * search_path, const char * source_path);




/*
	DString variants - DString ("dynamic string") is a mutable string implementation used in this project
*/

/// Convert OPML DString to MMD
DString * mmd_d_string_convert_opml_to_text(DString * source);


/// Convert ITMZ DString to MMD
DString * mmd_d_string_convert_itmz_to_text(DString * source);


/// Convert MMD text to specified format, with specified extensions, and language
/// Returned char * must be freed
char * mmd_d_string_convert(DString * source, unsigned long extensions, short format, short language);


/// Convert MMD text to specified format using DString as a container for block of data
/// and length of that block.  Must be used for "complex" output formats such as EPUB.
/// Returned DString * must be freed
DString * mmd_d_string_convert_to_data(DString * source, unsigned long extensions, short format, short language, const char * directory);


/// Convert MMD text and write results to specified file -- used for "complex" output formats requiring
/// multiple documents (e.g. EPUB)
void mmd_d_string_convert_to_file(DString * source, unsigned long extensions, short format, short language, const char * directory, const char * filepath);


/// Does the text have metadata?
bool mmd_d_string_has_metadata(DString * source, size_t * end);


/// Return metadata keys, one per line
/// Returned char * must be freed
char * mmd_d_string_metadata_keys(DString * source);


/// Extract desired metadata as string value
/// Returned char * must be freed
char * mmd_d_string_metavalue_for_key(DString * source, const char * key);


/// Insert/replace metadata value in DString
void mmd_d_string_update_metavalue_for_key(DString * source, const char * key, const char * value);


/// Grab list of all transcluded files, but we need to know directory to search,
/// as well as the path to the file
/// Returned stack needs to be freed
struct stack * mmd_d_string_transclusion_manifest(DString * source, const char * search_path, const char * source_path);




/*
	MMD Engine variants
*/

/// Create MMD Engine using an existing DString (A new copy is *not* made)
mmd_engine * mmd_engine_create_with_dstring(
	DString 	*	d,
	unsigned long	extensions
);


/// Create MMD Engine using a C string (A private copy of the string will be
/// made.  The one passed here can be freed by the calling function)
mmd_engine * mmd_engine_create_with_string(
	const char *	str,
	unsigned long	extensions
);


/// Reset engine when finished parsing. (Usually not necessary to use this.)
void mmd_engine_reset(mmd_engine * e);


/// Free an existing MMD Engine
void mmd_engine_free(
	mmd_engine *	e,
	bool			freeDString
);


/// Set language and smart quotes language
void mmd_engine_set_language(mmd_engine * e, short language);


/// Access DString directly
DString * mmd_engine_d_string(mmd_engine * e);

/// Return token tree after previous parsing
token * mmd_engine_root(mmd_engine * e);


/// Parse part of the string into a token tree
token * mmd_engine_parse_substring(mmd_engine * e, size_t byte_start, size_t byte_len);


/// Parse the entire string into a token tree
void mmd_engine_parse_string(mmd_engine * e);


/// Export parsed token tree to output format
void mmd_engine_export_token_tree(DString * out, mmd_engine * e, short format);


/// Convert MMD text to specified format, with specified extensions, and language
/// Returned char * must be freed
char * mmd_engine_convert(mmd_engine * e, short format);


/// Convert MMD text and write results to specified file -- used for "complex" output formats requiring
/// multiple documents (e.g. EPUB)
void mmd_engine_convert_to_file(mmd_engine * e, short format, const char * directory, const char * filepath);


/// Convert OPML to text without modifying original engine source
DString  * mmd_engine_convert_opml_to_text(mmd_engine * e);


/// Convert ITMZ to text without modifying original engine source
DString  * mmd_engine_convert_itmz_to_text(mmd_engine * e);


/// Convert MMD text to specified format using DString as a container for block of data
/// and length of that block.  Must be used for "complex" output formats such as EPUB.
/// Returned DString * must be freed
DString * mmd_engine_convert_to_data(mmd_engine * e, short format, const char * directory);


/// Does the text have metadata?
bool mmd_engine_has_metadata(mmd_engine * e, size_t * end);


/// Return metadata keys, one per line
/// Returned char * must be freed
char * mmd_engine_metadata_keys(mmd_engine * e);


/// Extract desired metadata as string value
char * mmd_engine_metavalue_for_key(mmd_engine * e, const char * key);


/// Insert/replace metadata value in mmd_engine
void mmd_engine_update_metavalue_for_key(mmd_engine * e, const char * key, const char * value);


/// Grab list of all transcluded files, but we need to know directory to search,
/// as well as the path to the file
/// Returned stack needs to be freed
struct stack * mmd_engine_transclusion_manifest(mmd_engine * e, const char * search_path, const char * source_path);




/*
	Utility functions
*/

/// Return the version string for this build of libMultiMarkdown
/// The returned `char *` will need to be freed after it is no longer needed.
char * mmd_version(void);


// Read file into memory
DString * scan_file(const char * fname);


/// Recursively transclude source text, given a search directory.
/// Track files to prevent infinite recursive loops
void mmd_transclude_source(DString * source, const char * search_path, const char * source_path, short format, struct stack * parsed, struct stack * manifest);


/// If MMD Header metadata used, insert it into appropriate place
void mmd_prepend_mmd_header(DString * source);


/// If MMD Footer metadata used, insert it into appropriate place
void mmd_append_mmd_footer(DString * source);


/// Accept all CriticMarkup changes in the source string
void mmd_critic_markup_accept(DString * d);


/// Accept all CriticMarkup changes in the specified range
void mmd_critic_markup_accept_range(DString * d, size_t start, size_t len);


/// Reject all CriticMarkup changes in the source string
void mmd_critic_markup_reject(DString * d);


/// Reject all CriticMarkup changes in the specified range
void mmd_critic_markup_reject_range(DString * d, size_t start, size_t len);


/// Token types for parse tree
enum token_types {
	DOC_START_TOKEN = 0,	//!< DOC_START_TOKEN must be type 0

	BLOCK_BLOCKQUOTE = 50,		//!< This must start *after* the largest number in parser.h
	BLOCK_CODE_FENCED,
	BLOCK_CODE_INDENTED,
	BLOCK_DEFLIST,
	BLOCK_DEFINITION,
	BLOCK_DEF_ABBREVIATION,
	BLOCK_DEF_CITATION,
	BLOCK_DEF_GLOSSARY,
	BLOCK_DEF_FOOTNOTE,
	BLOCK_DEF_LINK,
	BLOCK_EMPTY,
	BLOCK_HEADING,				//!< Placeholder for theme cascading
	BLOCK_H1,					//!< Leave H1, H2, etc. in order
	BLOCK_H2,
	BLOCK_H3,
	BLOCK_H4,
	BLOCK_H5,
	BLOCK_H6,
	BLOCK_HR,
	BLOCK_HTML,
	BLOCK_LIST_BULLETED,
	BLOCK_LIST_BULLETED_LOOSE,
	BLOCK_LIST_ENUMERATED,
	BLOCK_LIST_ENUMERATED_LOOSE,
	BLOCK_LIST_ITEM,
	BLOCK_LIST_ITEM_TIGHT,
	BLOCK_META,
	BLOCK_PARA,
	BLOCK_SETEXT_1,
	BLOCK_SETEXT_2,
	BLOCK_TABLE,
	BLOCK_TABLE_HEADER,
	BLOCK_TABLE_SECTION,
	BLOCK_TERM,
	BLOCK_TOC,

	CRITIC_ADD_OPEN,
	CRITIC_ADD_CLOSE,
	CRITIC_DEL_OPEN,
	CRITIC_DEL_CLOSE,
	CRITIC_COM_OPEN,
	CRITIC_COM_CLOSE,
	CRITIC_SUB_OPEN,
	CRITIC_SUB_DIV,
	CRITIC_SUB_DIV_A,
	CRITIC_SUB_DIV_B,
	CRITIC_SUB_CLOSE,
	CRITIC_HI_OPEN,
	CRITIC_HI_CLOSE,

	PAIR_CRITIC_ADD,
	PAIR_CRITIC_DEL,
	PAIR_CRITIC_COM,
	PAIR_CRITIC_SUB_ADD,
	PAIR_CRITIC_SUB_DEL,
	PAIR_CRITIC_HI,

	PAIRS,			//!< Placeholder for theme cascading
	PAIR_ANGLE,
	PAIR_BACKTICK,
	PAIR_BRACKET,
	PAIR_BRACKET_ABBREVIATION,
	PAIR_BRACKET_FOOTNOTE,
	PAIR_BRACKET_GLOSSARY,
	PAIR_BRACKET_CITATION,
	PAIR_BRACKET_IMAGE,
	PAIR_BRACKET_VARIABLE,
	PAIR_BRACE,
	PAIR_EMPH,
	PAIR_MATH,
	PAIR_PAREN,
	PAIR_QUOTE_SINGLE,
	PAIR_QUOTE_DOUBLE,
	PAIR_QUOTE_ALT,
	PAIR_RAW_FILTER,
	PAIR_SUBSCRIPT,
	PAIR_SUPERSCRIPT,
	PAIR_STAR,
	PAIR_STRONG,
	PAIR_UL,
	PAIR_BRACES,

	MARKUP,
	STAR,
	UL,
	EMPH_START,
	EMPH_STOP,
	STRONG_START,
	STRONG_STOP,

	BRACKET_LEFT,
	BRACKET_RIGHT,
	BRACKET_ABBREVIATION_LEFT,
	BRACKET_FOOTNOTE_LEFT,
	BRACKET_GLOSSARY_LEFT,
	BRACKET_CITATION_LEFT,
	BRACKET_IMAGE_LEFT,
	BRACKET_VARIABLE_LEFT,

	PAREN_LEFT,
	PAREN_RIGHT,

	ANGLE_LEFT,
	ANGLE_RIGHT,

	BRACE_DOUBLE_LEFT,
	BRACE_DOUBLE_RIGHT,

	AMPERSAND,
	AMPERSAND_LONG,
	APOSTROPHE,
	BACKTICK,
	CODE_FENCE,
	COLON,
	DASH_M,
	DASH_N,
	ELLIPSIS,
	QUOTE_SINGLE,
	QUOTE_DOUBLE,
	QUOTE_LEFT_SINGLE,
	QUOTE_RIGHT_SINGLE,
	QUOTE_LEFT_DOUBLE,
	QUOTE_RIGHT_DOUBLE,
	QUOTE_RIGHT_ALT,

	ESCAPED_CHARACTER,

	HTML_ENTITY,
	HTML_COMMENT_START,
	HTML_COMMENT_STOP,
	PAIR_HTML_COMMENT,

	MATH_PAREN_OPEN,
	MATH_PAREN_CLOSE,
	MATH_BRACKET_OPEN,
	MATH_BRACKET_CLOSE,
	MATH_DOLLAR_SINGLE,
	MATH_DOLLAR_DOUBLE,

	EQUAL,
	PIPE,
	PLUS,
	SLASH,

	SUPERSCRIPT,
	SUBSCRIPT,

	INDENT_TAB,
	INDENT_SPACE,
	NON_INDENT_SPACE,

	HASH1,							//!< Leave HASH1, HASH2, etc. in order
	HASH2,
	HASH3,
	HASH4,
	HASH5,
	HASH6,
	MARKER_BLOCKQUOTE,
	MARKER_H1,						//!< Leave MARKER_H1, MARKER_H2, etc. in order
	MARKER_H2,
	MARKER_H3,
	MARKER_H4,
	MARKER_H5,
	MARKER_H6,
	MARKER_LIST_BULLET,
	MARKER_LIST_ENUMERATOR,
	MARKER_DEFLIST_COLON,

	TABLE_ROW,
	TABLE_CELL,
	TABLE_DIVIDER,

	TOC,
	TOC_SINGLE,
	TOC_RANGE,

	TEXT_BACKSLASH,
	RAW_FILTER_LEFT,
	TEXT_BRACE_LEFT,
	TEXT_BRACE_RIGHT,
	TEXT_EMPTY,
	TEXT_HASH,
	TEXT_LINEBREAK,
	TEXT_LINEBREAK_SP,
	TEXT_NL,
	TEXT_NL_SP,
	TEXT_NUMBER_POSS_LIST,
	TEXT_PERCENT,
	TEXT_PERIOD,
	TEXT_PLAIN,

	TEXT_REPLACEMENT_CHAR,

	MANUAL_LABEL,
};


/// Define smart typography languages -- first in list is default
enum smart_quotes_language {
	ENGLISH = 0,
	DUTCH,
	FRENCH,
	GERMAN,
	GERMANGUILL,
	SPANISH,
	SWEDISH,
};


enum output_format {
	FORMAT_HTML,
	FORMAT_EPUB,
	FORMAT_LATEX,
	FORMAT_BEAMER,
	FORMAT_MEMOIR,
	FORMAT_FODT,
	FORMAT_ODT,
	FORMAT_TEXTBUNDLE,
	FORMAT_TEXTBUNDLE_COMPRESSED,
	FORMAT_OPML,
	FORMAT_ITMZ,
	FORMAT_MMD,
	FORMAT_HTML_WITH_ASSETS
};


enum parser_extensions {
	EXT_COMPATIBILITY       = 1 << 0,    //!< Markdown compatibility mode
	EXT_COMPLETE            = 1 << 1,    //!< Create complete document
	EXT_SNIPPET             = 1 << 2,    //!< Create snippet only
	EXT_SMART               = 1 << 3,    //!< Enable Smart quotes
	EXT_NOTES               = 1 << 4,    //!< Enable Footnotes
	EXT_NO_LABELS           = 1 << 5,    //!< Don't add anchors to headers, etc.
	EXT_PROCESS_HTML        = 1 << 6,    //!< Process Markdown inside HTML
	EXT_NO_METADATA         = 1 << 7,    //!< Don't parse Metadata
	EXT_OBFUSCATE           = 1 << 8,    //!< Mask email addresses
	EXT_CRITIC              = 1 << 9,    //!< Critic Markup Support
	EXT_CRITIC_ACCEPT       = 1 << 10,   //!< Accept all proposed changes
	EXT_CRITIC_REJECT       = 1 << 11,   //!< Reject all proposed changes
	EXT_RANDOM_FOOT         = 1 << 12,   //!< Use random numbers for footnote links
	EXT_TRANSCLUDE          = 1 << 13,   //!< Perform transclusion(s)
	EXT_PARSE_OPML          = 1 << 14,   //!< Convert from OPML before processing source text
	EXT_PARSE_ITMZ			= 1 << 15,   //!< Convert from ITMZ (iThoughts) before processing source text
	EXT_RANDOM_LABELS		= 1 << 16,   //!< Use random numbers for header labels (unless manually defined)
	EXT_FAKE                = 1 << 31,   //!< 31 is highest number allowed
};


#endif
