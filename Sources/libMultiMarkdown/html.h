/**

	MultiMarkdown 6 -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file html.h

	@brief Convert token tree to HTML output.


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


#ifndef HTML_MULTIMARKDOWN_H
#define HTML_MULTIMARKDOWN_H

#include "d_string.h"
#include "token.h"
#include "writer.h"

void mmd_export_token_html(DString * out, const char * source, token * t, size_t offset, scratch_pad * scratch);
void mmd_export_token_tree_html(DString * out, const char * source, token * t, size_t offset, scratch_pad * scratch);


void mmd_export_token_html_raw(DString * out, const char * source, token * t, size_t offset, scratch_pad * scratch);
void mmd_export_token_tree_html_raw(DString * out, const char * source, token * t, size_t offset, scratch_pad * scratch);

void mmd_export_citation_list_html(DString * out, const char * source, scratch_pad * scratch);
void mmd_export_footnote_list_html(DString * out, const char * source, scratch_pad * scratch);
void mmd_export_glossary_list_html(DString * out, const char * source, scratch_pad * scratch);

void mmd_start_complete_html(DString * out, const char * source, scratch_pad * scratch);
void mmd_end_complete_html(DString * out, const char * source, scratch_pad * scratch);


#endif
