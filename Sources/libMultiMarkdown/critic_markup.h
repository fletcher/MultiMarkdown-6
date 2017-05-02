/**

	MultiMarkdown -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file critic_markup.h

	@brief 


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


#ifndef CRITIC_MARKUP_MULTIMARKDOWN_H
#define CRITIC_MARKUP_MULTIMARKDOWN_H

#include "d_string.h"

enum cm_types {
	CM_ADD_OPEN = 1,		// Can't use type 0
	CM_ADD_CLOSE,

	CM_DEL_OPEN,
	CM_DEL_CLOSE,

	CM_SUB_OPEN,
	CM_SUB_DIV,
	CM_SUB_CLOSE,

	CM_HI_OPEN,
	CM_HI_CLOSE,

	CM_COM_OPEN,
	CM_COM_CLOSE,

	CM_ADD_PAIR,
	CM_DEL_PAIR,
	CM_SUB_PAIR,
	CM_HI_PAIR, 
	CM_COM_PAIR,

	CM_PLAIN_TEXT
};


void mmd_critic_markup_accept(DString * d);

void mmd_critic_markup_reject(DString * d);

#endif
