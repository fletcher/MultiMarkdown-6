/**

	MultiMarkdown 6 -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file scanners.h

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


#ifndef SCANNERS_MULTIMARKDOWN_H
#define SCANNERS_MULTIMARKDOWN_H

#ifdef TEST
#include "CuTest.h"
#endif

size_t scan_attr(const char * c);
size_t scan_attributes(const char * c);
size_t scan_email(const char * c);
size_t scan_fence_start(const char * c);
size_t scan_fence_end(const char * c);
size_t scan_html(const char * c);
size_t scan_html_block(const char * c);
size_t scan_html_line(const char * c);
size_t scan_key(const char * c);
size_t scan_ref_citation(const char * c);
size_t scan_ref_foot(const char * c);
size_t scan_ref_link(const char * c);
size_t scan_ref_link_no_attributes(const char * c);
size_t scan_spnl(const char * c);
size_t scan_url(const char * c);
size_t scan_value(const char * c);

#endif
