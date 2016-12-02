/**

	Smart String -- Library to abstract smart typing features from MMD Composer

	@file d_string.h

	@brief Dynamic string -- refactoring of old GLibFacade


	@author	Daniel Jalkut, modified by Fletcher T. Penney and Dan Lowe
	@bug	

**/

/*

	Copyright © 2011 Daniel Jalkut.
	Modifications by Fletcher T. Penney, Copyright © 2011-2016 Fletcher T. Penney.
	Modifications by Dan Lowe, Copyright © 2011 Dan Lowe.


	The `c-template` project is released under the MIT License.
	
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


#ifndef D_STRING_SMART_STRING_H
#define D_STRING_SMART_STRING_H

#include <stdbool.h>

/* WE implement minimal mirror implementations of GLib's GString  
 * sufficient to cover the functionality required by MultiMarkdown.
 *
 * NOTE: THese are 100% clean, from-scratch implementations using only the 
 * GLib function prototype as guide for behavior.
 */

typedef struct 
{	
	/* Current UTF8 byte stream this string represents */
	char* str;

	/* Where in the str buffer will we add new characters */
	/* or append new strings? */
	unsigned long currentStringBufferSize;
	unsigned long currentStringLength;
} DString;

DString* d_string_new(const char *startingString);

char* d_string_free(DString* ripString, bool freeCharacterData);

void d_string_append_c(DString* baseString, char appendedCharacter);
void d_string_append_c_array(DString *baseString, const char * appendedChars, size_t bytes);
void d_string_append(DString* baseString, char *appendedString);

void d_string_prepend(DString* baseString, char* prependedString);

void d_string_append_printf(DString* baseString, char* format, ...);

void d_string_insert(DString* baseString, size_t pos, const char * insertedString);
void d_string_insert_c(DString* baseString, size_t pos, char insertedCharacter);
void d_string_insert_printf(DString* baseString, size_t pos, char* format, ...);

void d_string_erase(DString* baseString, size_t pos, size_t len);


#endif
