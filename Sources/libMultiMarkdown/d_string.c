/**

	MultiMarkdown 6 -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file d_string.c

	@brief Dynamic string -- refactoring of old GLibFacade.  Provides a string
	"object" that can grow to accomodate any size content that is appended.


	@author	Daniel Jalkut, modified by Fletcher T. Penney and Dan Lowe

	@bug

**/

/*

	Copyright © 2011 Daniel Jalkut.
	Modifications by Fletcher T. Penney, Copyright © 2011-2017 Fletcher T. Penney.
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "d_string.h"


/*
 * The following section came from:
 *
 *	http://lists-archives.org/mingw-users/12649-asprintf-missing-vsnprintf-
 *		behaving-differently-and-_vsncprintf-undefined.html
 *
 * and
 *
 *	http://groups.google.com/group/jansson-users/browse_thread/thread/
 *		76a88d63d9519978/041a7d0570de2d48?lnk=raot
 */

// Some operating systems do not supply vasprintf() -- standardize on this
// replacement from:
//		https://github.com/esp8266/Arduino/issues/1954
int vasprintf(char** strp, const char* fmt, va_list ap) {
	va_list ap2;
	va_copy(ap2, ap);
	char tmp[1];
	int size = vsnprintf(tmp, 1, fmt, ap2);

	if (size <= 0) {
		return size;
	}

	va_end(ap2);
	size += 1;
	*strp = (char*)malloc(size * sizeof(char));
	return vsnprintf(*strp, size, fmt, ap);
}


/* DString */

#define kStringBufferStartingSize 1024					//!< Default size of string buffer capacity
#define kStringBufferGrowthMultiplier 2					//!< Multiply capacity by this factor when more space is needed
#define kStringBufferMaxIncrement 1024 * 1024 * 100		//!< Maximum growth increment when resizing (to limit exponential growth)


/// Create a new dynamic string
DString* d_string_new(const char * startingString) {
	DString* newString = malloc(sizeof(DString));

	if (!newString) {
		return NULL;
	}

	if (startingString == NULL) {
		startingString = "";
	}

	size_t startingBufferSize = kStringBufferStartingSize;
	size_t startingStringSize = strlen(startingString);

	while (startingBufferSize < (startingStringSize + 1)) {
		startingBufferSize *= kStringBufferGrowthMultiplier;
	}

	newString->str = malloc(startingBufferSize);

	if (!newString->str) {
		free(newString);
		return NULL;
	}

	newString->currentStringBufferSize = startingBufferSize;
	strncpy(newString->str, startingString, startingStringSize);
	newString->str[startingStringSize] = '\0';
	newString->currentStringLength = startingStringSize;

	return newString;
}


/// Free dynamic string
char* d_string_free(DString * ripString, bool freeCharacterData) {
	if (ripString == NULL) {
		return NULL;
	}

	char* returnedString = ripString->str;

	if (freeCharacterData) {
		if (ripString->str != NULL) {
			free(ripString->str);
		}

		returnedString = NULL;
	}

	free(ripString);

	return returnedString;
}


/// Ensure that dynamic string has specified capacity
static void ensureStringBufferCanHold(DString * baseString, size_t newStringSize) {
	size_t newBufferSizeNeeded = newStringSize + 1;

	if (newBufferSizeNeeded > baseString->currentStringBufferSize) {
		size_t newBufferSize = baseString->currentStringBufferSize;

		while (newBufferSizeNeeded > newBufferSize) {
			if (newBufferSize > kStringBufferMaxIncrement) {
				newBufferSize += kStringBufferMaxIncrement;
			} else {
				newBufferSize *= kStringBufferGrowthMultiplier;
			}
		}

		char *temp;
		temp = realloc(baseString->str, newBufferSize);

		if (temp == NULL) {
			/* realloc failed */
			fprintf(stderr, "Error reallocating memory for d_string. Current buffer size %lu.\n", baseString->currentStringBufferSize);

			exit(1);
		}

		baseString->str = temp;
		baseString->currentStringBufferSize = newBufferSize;
	}
}


/// Append null-terminated string to end of dynamic string
void d_string_append(DString * baseString, const char * appendedString) {
	size_t appendedStringLength = strlen(appendedString);

	if ((appendedString != NULL) && (appendedStringLength > 0)) {
		size_t newStringLength = baseString->currentStringLength + appendedStringLength;
		ensureStringBufferCanHold(baseString, newStringLength);

		/* We already know where the current string ends, so pass that as the starting address for strncat */
		strncat(baseString->str + baseString->currentStringLength, appendedString, appendedStringLength);
		baseString->currentStringLength = newStringLength;
	}
}


/// Append single character to end of dynamic string
void d_string_append_c(DString * baseString, char appendedCharacter) {
	size_t newSizeNeeded = baseString->currentStringLength + 1;
	ensureStringBufferCanHold(baseString, newSizeNeeded);

	baseString->str[baseString->currentStringLength] = appendedCharacter;
	baseString->currentStringLength++;
	baseString->str[baseString->currentStringLength] = '\0';
}


/// Append array of characters to end of dynamic string
void d_string_append_c_array(DString * baseString, const char * appendedChars, size_t bytes) {
	size_t newSizeNeeded = baseString->currentStringLength + bytes;
	ensureStringBufferCanHold(baseString, newSizeNeeded);

	memcpy(baseString->str + baseString->currentStringLength, appendedChars, bytes);

	baseString->currentStringLength = newSizeNeeded;
	baseString->str[baseString->currentStringLength] = '\0';
}


/// Append to end of dynamic string using format specifier
void d_string_append_printf(DString * baseString, const char * format, ...) {
	va_list args;
	va_start(args, format);

	char* formattedString = NULL;
	vasprintf(&formattedString, format, args);

	if (formattedString != NULL) {
		d_string_append(baseString, formattedString);
		free(formattedString);
	}

	va_end(args);
}


/// Prepend null-terminated string to end of dynamic string
void d_string_prepend(DString * baseString, const char * prependedString) {
	size_t prependedStringLength = strlen(prependedString);

	if ((prependedString != NULL) && (prependedStringLength > 0)) {
		size_t newStringLength = baseString->currentStringLength + prependedStringLength;
		ensureStringBufferCanHold(baseString, newStringLength);

		memmove(baseString->str + prependedStringLength, baseString->str, baseString->currentStringLength);
		strncpy(baseString->str, prependedString, prependedStringLength);
		baseString->currentStringLength = newStringLength;
		baseString->str[baseString->currentStringLength] = '\0';
	}
}


/// Insert null-terminated string inside dynamic string
void d_string_insert(DString * baseString, size_t pos, const char * insertedString) {
	size_t insertedStringLength = strlen(insertedString);

	if ((insertedString != NULL) && (insertedStringLength > 0)) {
		if (pos > baseString->currentStringLength) {
			pos = baseString->currentStringLength;
		}

		size_t newStringLength = baseString->currentStringLength + insertedStringLength;
		ensureStringBufferCanHold(baseString, newStringLength);

		/* Shift following string to 'right' */
		memmove(baseString->str + pos + insertedStringLength, baseString->str + pos, baseString->currentStringLength - pos);
		strncpy(baseString->str + pos, insertedString, insertedStringLength);
		baseString->currentStringLength = newStringLength;
		baseString->str[baseString->currentStringLength] = '\0';
	}
}


/// Insert single character inside dynamic string
void d_string_insert_c(DString * baseString, size_t pos, char insertedCharacter) {
	if (pos > baseString->currentStringLength) {
		pos = baseString->currentStringLength;
	}

	size_t newSizeNeeded = baseString->currentStringLength + 1;
	ensureStringBufferCanHold(baseString, newSizeNeeded);

	/* Shift following string to 'right' */
	memmove(baseString->str + pos + 1, baseString->str + pos, baseString->currentStringLength - pos);

	baseString->str[pos] = insertedCharacter;
	baseString->currentStringLength++;
	baseString->str[baseString->currentStringLength] = '\0';
}


/// Insert inside dynamic string using format specifier
void d_string_insert_printf(DString * baseString, size_t pos, const char * format, ...) {
	va_list args;
	va_start(args, format);

	char* formattedString = NULL;
	vasprintf(&formattedString, format, args);

	if (formattedString != NULL) {
		d_string_insert(baseString, pos, formattedString);
		free(formattedString);
	}

	va_end(args);
}


/// Erase portion of dynamic string
void d_string_erase(DString * baseString, size_t pos, size_t len) {
	if ((pos > baseString->currentStringLength) || (len <= 0)) {
		return;
	}

	if ((pos + len) >= baseString->currentStringLength) {
		len = -1;
	}

	if (len == -1) {
		baseString->currentStringLength = pos;
	} else {
		memmove(baseString->str + pos, baseString->str + pos + len, baseString->currentStringLength - pos - len);
		baseString->currentStringLength -= len;
	}

	baseString->str[baseString->currentStringLength] = '\0';
}

/// Copy a portion of dynamic string
char * d_string_copy_substring(DString * d, size_t start, size_t len) {
	char * result;

	if (len == -1) {
		len = d->currentStringLength - start;
	} else {
		if (start + len > d->currentStringLength) {
			fprintf(stderr, "d_string: Asked to copy invalid substring range.\n");
			fprintf(stderr, "start: %lu  len: %lu  string: %lu\n", start, len,
			        d->currentStringLength);
			return NULL;
		}
	}

	result = malloc(len + 1);
	strncpy(result, &d->str[start], len);
	result[len] = '\0';

	return result;
}


/// Replace occurences of "original" with "replace" inside the specified range
/// Returns the change in overall length
long d_string_replace_text_in_range(DString * d, size_t pos, size_t len, const char * original, const char * replace) {
	long delta = 0;		// Overall change in length

	long len_o = strlen(original);
	long len_r = strlen(replace);
	long change = len_r - len_o;	// Change in length for each replacement

	size_t stop;

	if (len == -1) {
		stop = d->currentStringLength;
	} else {
		stop = pos + len;
	}

	char * match = strstr(&(d->str[pos]), original);

	while (match && (match - d->str < stop)) {
		pos = match - d->str;
		d_string_erase(d, match - d->str, len_o);
		d_string_insert(d, match - d->str, replace);

		delta += change;
		stop += change;
		match = strstr(d->str + pos + len_r, original);
	}

	return delta;
}

