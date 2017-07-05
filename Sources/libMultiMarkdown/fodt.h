/**

	MultiMarkdown -- Lightweight markup processor to produce HTML, odf, and more.

	@file fodt.h

	@brief Convert token tree to Flat OpenDocument (ODF/FODT) output


	@author	Fletcher T. Penney
	@bug	

**/

/*

	Copyright © 2016 - 2017 Fletcher T. Penney.


	The `MultiMarkdown 6` project is released under the MIT License..
	
	GLibFacade.c and GLibFacade.h are from the MultiMarkdown v4 project:
	
		https://github.com/fletcher/MultiMarkdown-4/
	
	MMD 4 is released under both the MIT License and GPL.
	
	
	CuTest is released under the zlib/libpng license. See CuTest.c for the
	text of the license.
	
	
	## The MIT License ##
	
	Permission is hereby granted, free of charge, to any person obtaining
	a copy of this software and associated documentation files (the
	"Software"), to deal in the Software without restriction, including
	without limitation the rights to use, copy, modify, merge, publish,
	distribute, sublicense, and/or sell copies of the Software, and to
	permit persons to whom the Software is furnished to do so, subject to
	the following conditions:
	
	The above copyright notice and this permission notice shall be
	included in all copies or substantial portions of the Software.
	
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
	EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
	CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
	TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
	

*/


#ifndef FODT_MULTIMARKDOWN_H
#define FODT_MULTIMARKDOWN_H


#include "d_string.h"
#include "token.h"
#include "writer.h"

void mmd_export_token_odf(DString * out, const char * source, token * t, scratch_pad * scratch);
void mmd_export_token_tree_odf(DString * out, const char * source, token * t, scratch_pad * scratch);

void mmd_export_token_odf_raw(DString * out, const char * source, token * t, scratch_pad * scratch);
void mmd_export_token_tree_odf_raw(DString * out, const char * source, token * t, scratch_pad * scratch);

void mmd_export_citation_list_odf(DString * out, const char * source, scratch_pad * scratch);
void mmd_export_footnote_list_odf(DString * out, const char * source, scratch_pad * scratch);

void mmd_start_complete_odf(DString * out, const char * source, scratch_pad * scratch);
void mmd_end_complete_odf(DString * out, const char * source, scratch_pad * scratch);

void mmd_export_citation_list_odf(DString * out, const char * source, scratch_pad * scratch);


#endif
