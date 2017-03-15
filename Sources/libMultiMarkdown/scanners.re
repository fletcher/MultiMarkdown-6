/**

	MultiMarkdown 6 -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file scanners.c

	@brief After text has been tokenized, there are still some constructs that are best
	interpreted using regular expressions.


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

#include <stdlib.h>

#include "scanners.h"

/*!re2c

	re2c:define:YYCTYPE = "char";
	re2c:define:YYCURSOR = c;
	re2c:define:YYMARKER = marker;
	re2c:define:YYCTXMARKER = marker;
	re2c:yyfill:enable = 0;

	nl 			= ( '\n' | '\r' '\n'?);
	sp 			= [ \t]*;
	spnl		= sp (nl sp)?;
	non_indent	= ' '{0,3};
	nl_eof		= nl | '\x00';

	email		= 'mailto:'? [-A-Za-z0-9+_./!%~$]+ '@' [^ \t\n\r\x00>]+;

	url			= [A-Za-z\-]+ '://' [^ \t\n\r\x00>]+;

	name		= [A-Za-z_:] [A-Za-z0-9_.:-]*;
	quoted_d	= '"' [^"\n\r\x00]* '"';
	quoted_s	= "'" [^'\n\r\x00]* "'";
	quoted_p	= "(" [^)\n\r\x00]* ")";
	unquoted	= [\.A-Za-z0-9]+;

	value		= (quoted_d | quoted_s | unquoted);
	attr		= spnl name '=' sp value;
	attributes	= (attr)+;
	title		= (quoted_d | quoted_s | quoted_p);

	label		= [^\]\n\r\x00]* [^\]\n\r\x00\\];
	finish_line	= [^\n\r\x00]+;

	ref_abbr	= non_indent '[>' label ']' ':' finish_line;

	ref_citation	= non_indent '[#' label ']' ':' finish_line;

	ref_foot	= non_indent '[^' label ']' ':' finish_line;

	ref_glossary	= non_indent '[?' label ']' ':' finish_line;

	ref_link	= non_indent '[' label ']' ':' finish_line;

	destination	= ('<' [^ \t\n\r\x00>]* '>') | [^ \t\n\r\x00]+;

	ref_link_no_attributes	= non_indent '[' label ']' ':' spnl destination sp (nl_eof | (nl? (title) sp) nl_eof);

	tag_name	= [A-Za-z] [A-Za-z0-9\-]*;

	tag_start	= '<' tag_name attributes? sp '>';

	tag_empty	= '<' tag_name attributes? sp '/>';

	tag_end		= '</' tag_name sp '>';

	// We limit comments to exclude '>' character to minimize backtracking
	comment		= [^>\-\x00] [^>\x00]*;

	tag_comment	= '<!--' comment '-->';

	html		= tag_start | tag_empty | tag_end | tag_comment;

	html_line	= html sp nl;

//	http://www.cs.sfu.ca/CourseCentral/165/sbrown1/wdgxhtml10/block.html
//	https://developer.mozilla.org/en-US/docs/Web/HTML/Block-level_elements

	block_tag	= 'address' | 'article' | 'aside' | 'blockquote' | 'canvas' | 'center' | 'dd' |
					'dir' | 'div' | 'dl' | 'dt' | 'fieldset' | 'figcaption' | 'figure' |
					'footer' | 'form' | 'frameset' | 'h1' | 'h2' | 'h3' | 'h4' | 'h5' | 'h6' |
					'header' | 'hgroup' | 'hr' | 'isindex' | 'li' | 'main' | 'menu' | 'nav' |
					'noframes' | 'noscript' | 'ol' | 'output' | 'p' | 'pre' | 'section' |
					'table' | 'tbody' | 'td' | 'tfoot' | 'th' | 'thead' | 'tr' | 'ul' | 'video';

	html_block	= '<' '/'? block_tag attributes? '/'? '>';

	fence_start	= non_indent [`~]{3,} [^`'\n\r\x00]+ nl_eof;

	fence_end	= non_indent [`~]{3,} sp nl_eof;

	meta_key	= [A-Za-z0-9] [A-Za-z0-9_ \t\-\.]*;

	meta_value	= [^\n\r\x00]+;

	meta_line	= meta_key sp ':' meta_value nl_eof;	// meta_line can't match url above

	definition	= non_indent ':' sp [^\n\r\x00];

	table_separator	= (('|' [:\-= \t|+]*) | ([:\-= \t+]+ '|' [:\-= \t|+]*)) nl_eof;

	align		= [\-=]+;
	align_left	= sp ':' align sp ('|' | nl_eof);
	align_right	= sp align ':' sp ('|' | nl_eof);
	align_center	= sp ':' align ':' sp ('|' | nl_eof);
	align_wrap_left		= sp ':' align '+' sp ('|' | nl_eof);
	align_wrap_right	= sp align ':' '+' sp ('|' | nl_eof);
	align_wrap_center	= sp ':' align ':' '+' sp ('|' | nl_eof);

	setext_1	= non_indent '='{2,} nl_eof;

	setext_2	= non_indent '-'{2,} nl_eof;

	atx			= '#'+ [ \t]+ [^ \t\n\r\x00];
*/


size_t scan_spnl(const char * c) {
	const char * start = c;

/*!re2c
	spnl 		{ return (size_t)( c - start ); }
	.?			{ return 0; }
*/	
}


size_t scan_key(const char * c) {
	const char * start = c;

/*!re2c
	name 		{ return (size_t)( c - start ); }
	.?			{ return 0; }
*/	
}


size_t scan_value(const char * c) {
	const char * marker = NULL;
	const char * start = c;

/*!re2c
	value 		{ return (size_t)( c - start ); }
	.?			{ return 0; }
*/	
}


size_t scan_attr(const char * c) {
	const char * marker = NULL;
	const char * start = c;

/*!re2c
	attr		{ return (size_t)( c - start ); }
	.?			{ return 0; }
*/	
}


size_t scan_attributes(const char * c) {
	const char * marker = NULL;
	const char * start = c;

/*!re2c
	attributes	{ return (size_t)( c - start ); }
	.?			{ return 0; }
*/	
}


size_t scan_email(const char * c) {
	const char * marker = NULL;
	const char * start = c;

/*!re2c
	email		{ return (size_t)( c - start ); }
	.?			{ return 0; }
*/	
}


size_t scan_url(const char * c) {
	const char * marker = NULL;
	const char * start = c;

/*!re2c
	email		{ return (size_t)( c - start ); }
	url			{ return (size_t)( c - start ); }
	.?			{ return 0; }
*/	
}


size_t scan_ref_abbreviation(const char * c) {
	const char * marker = NULL;
	const char * start = c;

/*!re2c
	ref_abbr	{ return (size_t)( c - start ); }
	.?			{ return 0; }
*/	
}


size_t scan_ref_citation(const char * c) {
	const char * marker = NULL;
	const char * start = c;

/*!re2c
	ref_citation	{ return (size_t)( c - start ); }
	.?			{ return 0; }
*/	
}


size_t scan_ref_foot(const char * c) {
	const char * marker = NULL;
	const char * start = c;

/*!re2c
	ref_foot	{ return (size_t)( c - start ); }
	.?			{ return 0; }
*/	
}


size_t scan_ref_glossary(const char * c) {
	const char * marker = NULL;
	const char * start = c;

/*!re2c
	ref_glossary	{ return (size_t)( c - start ); }
	.?			{ return 0; }
*/	
}


size_t scan_ref_link_no_attributes(const char * c) {
	const char * marker = NULL;
	const char * start = c;

/*!re2c
	ref_link_no_attributes	{ return (size_t)( c - start ); }
	.?			{ return 0; }
*/	
}


size_t scan_ref_link(const char * c) {
	const char * marker = NULL;
	const char * start = c;

/*!re2c
	ref_link	{ return (size_t)( c - start ); }
	.?			{ return 0; }
*/	
}


size_t scan_html(const char * c) {
	const char * marker = NULL;
	const char * start = c;

/*!re2c
	html 		{ return (size_t)( c - start ); }
	.?			{ return 0; }
*/	
}


size_t scan_html_comment(const char * c) {
	const char * marker = NULL;
	const char * start = c;

/*!re2c
	tag_comment	{ return (size_t)( c - start ); }
	.?			{ return 0; }
*/	
}


size_t scan_html_block(const char * c) {
	const char * marker = NULL;
	const char * start = c;

/*!re2c
	html_block	{ return (size_t)( c - start ); }
	.?			{ return 0; }
*/	
}


size_t scan_html_line(const char * c) {
	const char * marker = NULL;
	const char * start = c;

/*!re2c
	html_line	{ return (size_t)( c - start ); }
	.?			{ return 0; }
*/	
}


size_t scan_fence_start(const char * c) {
	const char * marker = NULL;
	const char * start = c;

/*!re2c
	fence_start	{ return (size_t)( c - start ); }
	.?			{ return 0; }
*/	
}


size_t scan_fence_end(const char * c) {
	const char * marker = NULL;
	const char * start = c;

/*!re2c
	fence_end	{ return (size_t)( c - start ); }
	.?			{ return 0; }
*/	
}


size_t scan_meta_line(const char * c) {
	const char * marker = NULL;
	const char * start = c;

/*!re2c
	meta_line	{ return (size_t)( c - start ); }
	.?			{ return 0; }
*/	
}


size_t scan_meta_key(const char * c) {
	const char * start = c;

/*!re2c
	meta_key	{ return (size_t)( c - start ); }
	.?			{ return 0; }
*/	
}


size_t scan_definition(const char * c) {
	const char * marker = NULL;
	const char * start = c;

/*!re2c
	definition	{ return (size_t)( c - start ); }
	.?			{ return 0; }
*/	
}


size_t scan_table_separator(const char * c) {
	const char * marker = NULL;
	const char * start = c;

/*!re2c
	table_separator	{ return (size_t)( c - start ); }
	.?			{ return 0; }
*/	
}


size_t scan_alignment_string(const char * c) {
	const char * marker = NULL;

/*!re2c
	align_left		{ return ALIGN_LEFT; }
	align_right		{ return ALIGN_RIGHT; }
	align_center	{ return ALIGN_CENTER; }

	align_wrap_left		{ return ALIGN_WRAP | ALIGN_LEFT; }
	align_wrap_right	{ return ALIGN_WRAP | ALIGN_RIGHT; }
	align_wrap_center	{ return ALIGN_WRAP | ALIGN_CENTER; }

	.?			{ return 0; }
*/	
}


size_t scan_destination(const char * c) {
	const char * start = c;

/*!re2c
	destination	{ return (size_t)( c - start ); }
	.?			{ return 0; }
*/	
}


size_t scan_title(const char * c) {
	const char * marker = NULL;
	const char * start = c;

/*!re2c
	title	{ return (size_t)( c - start ); }
	.?			{ return 0; }
*/	
}

size_t scan_setext(const char * c) {
	const char * marker = NULL;
	const char * start = c;

/*!re2c
	setext_1	{ return (size_t)( c - start ); }
	setext_2	{ return (size_t)( c - start ); }
	.?			{ return 0; }
*/	
}

size_t scan_atx(const char * c) {
	const char * marker = NULL;
	const char * start = c;

/*!re2c
	atx			{ return (size_t)( c - start ); }
	.?			{ return 0; }
*/	
}


#ifdef TEST
void Test_scan_url(CuTest* tc) {
	int url_len;

	url_len = (int) scan_url("mailto:foo@bar.com");
	CuAssertIntEquals(tc, 18, url_len);
	url_len = (int) scan_email("mailto:foo@bar.com");
	CuAssertIntEquals(tc, 18, url_len);

	url_len = (int) scan_url("http://test.com/");
	CuAssertIntEquals(tc, 16, url_len);
	url_len = (int) scan_email("mailto:foo@bar.com");
	CuAssertIntEquals(tc, 18, url_len);

	url_len = (int) scan_url("foo@bar.com  ");
	CuAssertIntEquals(tc, 11, url_len);
	url_len = (int) scan_email("mailto:foo@bar.com");
	CuAssertIntEquals(tc, 18, url_len);
}
#endif

