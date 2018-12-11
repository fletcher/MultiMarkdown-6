/**

	Dynamic string -- Lightweight dynamic string implementation.

	@file d_string.h

	@brief Dynamic string -- refactoring of old GLibFacade from MultiMarkdown.
	Provides a string "object" that can grow to accomodate any size content
	that is appended.


	@author	Daniel Jalkut, modified by Fletcher T. Penney and Dan Lowe

	@bug

**/

/*

	Copyright © 2011 Daniel Jalkut.
	Modifications by Fletcher T. Penney, Copyright © 2011-2018 Fletcher T. Penney.
	Modifications by Dan Lowe, Copyright © 2011 Dan Lowe.


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


#ifndef D_STRING_SMART_STRING_H
#define D_STRING_SMART_STRING_H

#include <stdbool.h>
#include <stdlib.h>

/* WE implement minimal mirror implementations of GLib's GString
 * sufficient to cover the functionality required by MultiMarkdown.
 *
 * NOTE: THese are 100% clean, from-scratch implementations using only the
 * GLib function prototype as guide for behavior.
 */


/// Structure for dynamic string
struct DString {
	char * str;                             //!< Pointer to UTF-8 byte stream for string
	unsigned long currentStringBufferSize;  //!< Size of buffer currently allocated
	unsigned long currentStringLength;      //!< Size of current string
};

typedef struct DString DString;


/// Create a new dynamic string
DString * d_string_new(
	const char * startingString             //!< Initial contents for string
);


/// Free dynamic string
char * d_string_free(
	DString * ripString,                    //!< DString to be freed
	bool freeCharacterData                  //!< Should the underlying str be freed as well?
);


/// Append null-terminated string to end of dynamic string
void d_string_append(
	DString * baseString,                   //!< DString to be appended
	const char * appendedString             //!< String to be appended
);


/// Append single character to end of dynamic string
void d_string_append_c(
	DString * baseString,                   //!< DString to be appended
	char appendedCharacter                  //!< Character to append
);


/// Append array of characters to end of dynamic string
void d_string_append_c_array(
	DString * baseString,                   //!< DString to be appended
	const char * appendedChars,             //!< String to be appended
	size_t bytes                            //!< Number of bytes to append
);


/// Append to end of dynamic string using format specifier
void d_string_append_printf(
	DString * baseString,                   //!< DString to be appended
	const char * format,                    //!< Format specifier for appending
	...                                     //!< Arguments for format specifier
);


/// Prepend null-terminated string to end of dynamic string
void d_string_prepend(
	DString * baseString,                   //!< DString to be appended
	const char * prependedString            //!< String to be prepended
);


/// Insert null-terminated string inside dynamic string
void d_string_insert(
	DString * baseString,                   //!< DString to be appended
	size_t pos,                             //!< Offset at which to insert string
	const char * insertedString             //!< String to be inserted
);


/// Insert single character inside dynamic string
void d_string_insert_c(
	DString * baseString,                   //!< DString to be appended
	size_t pos,                             //!< Offset at which to insert string
	char insertedCharacter                  //!< Character to insert
);


/// Insert inside dynamic string using format specifier
void d_string_insert_printf(
	DString * baseString,                   //!< DString to be appended
	size_t pos,                             //!< Offset at which to insert string
	const char * format,                    //!< Format specifier for appending
	...                                     //!< Arguments for format specifier
);


/// Erase portion of dynamic string
void d_string_erase(
	DString * baseString,                   //!< DString to be appended
	size_t pos,                             //!< Offset at which to erase portion of string
	size_t len                              //!< Character to append
);

/// Copy a portion of dynamic string
char * d_string_copy_substring(
	DString * d,                            //!< DString to copy
	size_t start,                           //!< Start position for copy
	size_t len                              //!< How many characters(bytes) to copy
);

/// Replace occurences of "original" with "replace" inside the specified range
/// Returns the change in overall length
long d_string_replace_text_in_range(
	DString * d,
	size_t pos,
	size_t len,
	const char * original,
	const char * replace
);

#endif
