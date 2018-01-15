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

#ifdef TEST
	#include "CuTest.h"
#endif


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

	#if (defined(_WIN32) || defined(__WIN32__))
	char *tmp = NULL;
	int size = vsnprintf(tmp, 0, fmt, ap2);
	#else
	char tmp[1];
	int size = vsnprintf(tmp, 1, fmt, ap2);
	#endif

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


#ifdef TEST
void Test_d_string_new(CuTest* tc) {
	char * test = "foo";

	DString * result = d_string_new(test);

	CuAssertIntEquals(tc, 3, result->currentStringLength);
	CuAssertIntEquals(tc, kStringBufferStartingSize, result->currentStringBufferSize);
	CuAssertStrEquals(tc, test, result->str);
	CuAssertIntEquals(tc, '\0', result->str[strlen(test)]);

	d_string_free(result, true);

	result = d_string_new(NULL);

	CuAssertIntEquals(tc, 0, result->currentStringLength);
	CuAssertIntEquals(tc, kStringBufferStartingSize, result->currentStringBufferSize);
	CuAssertStrEquals(tc, "", result->str);
	CuAssertIntEquals(tc, '\0', 0);

	d_string_free(result, true);
}
#endif


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
	if (baseString) {
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
}


#ifdef TEST
void Test_ensureStringBufferCanHold(CuTest* tc) {
	char * test = "foo";

	DString * result = d_string_new(test);

	ensureStringBufferCanHold(result, 1024);
	CuAssertIntEquals(tc, 2048, result->currentStringBufferSize);

	ensureStringBufferCanHold(result, 1024);
	CuAssertIntEquals(tc, 2048, result->currentStringBufferSize);

	/* This becomes 0 after we add 1 for the '\0' */
	ensureStringBufferCanHold(result, -1);
	CuAssertIntEquals(tc, 2048, result->currentStringBufferSize);

	ensureStringBufferCanHold(result, 1024 * 1024 - 1);
	CuAssertIntEquals(tc, 1024 * 1024, result->currentStringBufferSize);

	ensureStringBufferCanHold(result, 1024 * 1024 - 1);
	CuAssertIntEquals(tc, 1024 * 1024, result->currentStringBufferSize);

	ensureStringBufferCanHold(NULL, 1024);

	d_string_free(result, true);
}
#endif


/// Append null-terminated string to end of dynamic string
void d_string_append(DString * baseString, const char * appendedString) {
	if (baseString && appendedString) {
		size_t appendedStringLength = strlen(appendedString);

		if (appendedStringLength > 0) {
			size_t newStringLength = baseString->currentStringLength + appendedStringLength;
			ensureStringBufferCanHold(baseString, newStringLength);

			/* We already know where the current string ends, so pass that as the starting address for strncat */
			strncat(baseString->str + baseString->currentStringLength, appendedString, appendedStringLength);
			baseString->currentStringLength = newStringLength;
		}
	}
}


#ifdef TEST
void Test_d_string_append(CuTest* tc) {
	char * test = "foo";

	DString * result = d_string_new(test);

	d_string_append(result, "bar");
	CuAssertStrEquals(tc, "foobar", result->str);

	d_string_append(result, "");
	CuAssertStrEquals(tc, "foobar", result->str);

	d_string_append(result, NULL);
	CuAssertStrEquals(tc, "foobar", result->str);

	d_string_append(NULL, "foo");

	d_string_free(result, true);
}
#endif


/// Append single character to end of dynamic string
void d_string_append_c(DString * baseString, char appendedCharacter) {
	if (baseString && appendedCharacter) {
		size_t newSizeNeeded = baseString->currentStringLength + 1;
		ensureStringBufferCanHold(baseString, newSizeNeeded);

		baseString->str[baseString->currentStringLength] = appendedCharacter;
		baseString->currentStringLength++;
		baseString->str[baseString->currentStringLength] = '\0';
	}
}


#ifdef TEST
void Test_d_string_append_c(CuTest* tc) {
	char * test = "foo";

	DString * result = d_string_new(test);

	d_string_append_c(result, 'z');
	CuAssertStrEquals(tc, "fooz", result->str);
	CuAssertIntEquals(tc, 4, result->currentStringLength);

	d_string_append_c(result, 0);
	CuAssertStrEquals(tc, "fooz", result->str);
	CuAssertIntEquals(tc, 4, result->currentStringLength);

	d_string_append_c(NULL, 'f');

	d_string_free(result, true);
}
#endif


/// Append array of characters to end of dynamic string
void d_string_append_c_array(DString * baseString, const char * appendedChars, size_t bytes) {
	if (baseString && appendedChars) {
		if (bytes == -1) {
			// This is the same as regular append
			d_string_append(baseString, appendedChars);
		} else {
			if (appendedChars) {
				size_t newSizeNeeded = baseString->currentStringLength + bytes;
				ensureStringBufferCanHold(baseString, newSizeNeeded);

				memcpy(baseString->str + baseString->currentStringLength, appendedChars, bytes);

				baseString->currentStringLength = newSizeNeeded;
				baseString->str[baseString->currentStringLength] = '\0';
			}
		}
	}
}


#ifdef TEST
void Test_d_string_append_c_array(CuTest* tc) {
	char * test = "foo";

	DString * result = d_string_new(test);

	d_string_append_c_array(result, "bar", 3);
	CuAssertStrEquals(tc, "foobar", result->str);
	CuAssertIntEquals(tc, 6, result->currentStringLength);

	d_string_append_c_array(result, "baz", -1);
	CuAssertStrEquals(tc, "foobarbaz", result->str);
	CuAssertIntEquals(tc, 9, result->currentStringLength);

	d_string_append_c_array(result, NULL, 0);
	CuAssertStrEquals(tc, "foobarbaz", result->str);
	CuAssertIntEquals(tc, 9, result->currentStringLength);

	d_string_append_c_array(result, NULL, -1);
	CuAssertStrEquals(tc, "foobarbaz", result->str);
	CuAssertIntEquals(tc, 9, result->currentStringLength);

	d_string_append_c_array(NULL, "foo", -1);

	d_string_free(result, true);
}
#endif


/// Append to end of dynamic string using format specifier
void d_string_append_printf(DString * baseString, const char * format, ...) {
	if (baseString && format) {
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
}


#ifdef TEST
void Test_d_string_append_printf(CuTest* tc) {
	char * test = "foo";

	DString * result = d_string_new(test);

	d_string_append_printf(result, "%dbar%d", 5, 7);
	CuAssertStrEquals(tc, "foo5bar7", result->str);
	CuAssertIntEquals(tc, 8, result->currentStringLength);

	d_string_append_printf(result, NULL);
	CuAssertStrEquals(tc, "foo5bar7", result->str);
	CuAssertIntEquals(tc, 8, result->currentStringLength);

	d_string_append_printf(result, NULL, 5, 7);
	CuAssertStrEquals(tc, "foo5bar7", result->str);
	CuAssertIntEquals(tc, 8, result->currentStringLength);

	d_string_append_printf(NULL, "foo");

	d_string_free(result, true);
}
#endif


/// Prepend null-terminated string to end of dynamic string
void d_string_prepend(DString * baseString, const char * prependedString) {
	if (baseString && prependedString) {
		size_t prependedStringLength = strlen(prependedString);

		if (prependedStringLength > 0) {
			size_t newStringLength = baseString->currentStringLength + prependedStringLength;
			ensureStringBufferCanHold(baseString, newStringLength);

			memmove(baseString->str + prependedStringLength, baseString->str, baseString->currentStringLength);
			strncpy(baseString->str, prependedString, prependedStringLength);
			baseString->currentStringLength = newStringLength;
			baseString->str[baseString->currentStringLength] = '\0';
		}
	}
}


#ifdef TEST
void Test_d_string_prepend(CuTest* tc) {
	char * test = "foo";

	DString * result = d_string_new(test);

	d_string_prepend(result, "bar");
	CuAssertStrEquals(tc, "barfoo", result->str);
	CuAssertIntEquals(tc, 6, result->currentStringLength);

	d_string_prepend(result, NULL);
	CuAssertStrEquals(tc, "barfoo", result->str);
	CuAssertIntEquals(tc, 6, result->currentStringLength);

	d_string_prepend(NULL, "bar");

	d_string_free(result, true);
}
#endif


/// Insert null-terminated string inside dynamic string
void d_string_insert(DString * baseString, size_t pos, const char * insertedString) {
	if (baseString && insertedString) {
		size_t insertedStringLength = strlen(insertedString);

		if (insertedStringLength > 0) {
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
}


#ifdef TEST
void Test_d_string_insert(CuTest* tc) {
	char * test = "foo";

	DString * result = d_string_new(test);

	d_string_insert(result, 2, "bar");
	CuAssertStrEquals(tc, "fobaro", result->str);
	CuAssertIntEquals(tc, 6, result->currentStringLength);

	d_string_insert(result, -1, "bar");
	CuAssertStrEquals(tc, "fobarobar", result->str);
	CuAssertIntEquals(tc, 9, result->currentStringLength);

	d_string_insert(result, -1, NULL);
	CuAssertStrEquals(tc, "fobarobar", result->str);
	CuAssertIntEquals(tc, 9, result->currentStringLength);

	d_string_insert(NULL, 0, NULL);

	d_string_free(result, true);
}
#endif


/// Insert single character inside dynamic string
void d_string_insert_c(DString * baseString, size_t pos, char insertedCharacter) {
	if (baseString && insertedCharacter) {
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
}


#ifdef TEST
void Test_d_string_insert_c(CuTest* tc) {
	char * test = "foo";

	DString * result = d_string_new(test);

	d_string_insert_c(result, 2, 'b');
	CuAssertStrEquals(tc, "fobo", result->str);
	CuAssertIntEquals(tc, 4, result->currentStringLength);

	d_string_insert_c(result, -1, 'z');
	CuAssertStrEquals(tc, "foboz", result->str);
	CuAssertIntEquals(tc, 5, result->currentStringLength);

	d_string_insert_c(result, 3, 0);
	CuAssertStrEquals(tc, "foboz", result->str);
	CuAssertIntEquals(tc, 5, result->currentStringLength);

	d_string_insert_c(NULL, 0, 0);

	d_string_free(result, true);
}
#endif


/// Insert inside dynamic string using format specifier
void d_string_insert_printf(DString * baseString, size_t pos, const char * format, ...) {
	if (baseString && format) {
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
}


#ifdef TEST
void Test_d_string_insert_printf(CuTest* tc) {
	char * test = "foo";

	DString * result = d_string_new(test);

	d_string_insert_printf(result, 2, "%dbar%d", 5, 7);
	CuAssertStrEquals(tc, "fo5bar7o", result->str);
	CuAssertIntEquals(tc, 8, result->currentStringLength);

	d_string_insert_printf(result, -1, "z", 5, 7);
	CuAssertStrEquals(tc, "fo5bar7oz", result->str);
	CuAssertIntEquals(tc, 9, result->currentStringLength);

	d_string_insert_printf(NULL, 0, NULL);

	d_string_free(result, true);
}
#endif


/// Erase portion of dynamic string
void d_string_erase(DString * baseString, size_t pos, size_t len) {
	if (baseString) {
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
}


#ifdef TEST
void Test_d_string_erase(CuTest* tc) {
	char * test = "foobar";

	DString * result = d_string_new(test);

	d_string_erase(result, 2, 1);
	CuAssertStrEquals(tc, "fobar", result->str);
	CuAssertIntEquals(tc, 5, result->currentStringLength);

	d_string_erase(result, -1, -1);
	CuAssertStrEquals(tc, "fobar", result->str);
	CuAssertIntEquals(tc, 5, result->currentStringLength);

	d_string_erase(result, 2, -1);
	CuAssertStrEquals(tc, "fo", result->str);
	CuAssertIntEquals(tc, 2, result->currentStringLength);

	d_string_erase(NULL, 0, 0);

	d_string_free(result, true);
}
#endif


/// Copy a portion of dynamic string
char * d_string_copy_substring(DString * d, size_t start, size_t len) {
	if (d) {
		char * result;

		if ((len == -1) && (start < d->currentStringLength)) {
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
	} else {
		return NULL;
	}
}


#ifdef TEST
void Test_d_string_copy_substring(CuTest* tc) {
	char * test = "foobar";

	DString * result = d_string_new(test);

	test = d_string_copy_substring(result, 0, -1);
	CuAssertStrEquals(tc, "foobar", test);
	free(test);

	test = d_string_copy_substring(result, 2, 3);
	CuAssertStrEquals(tc, "oba", test);
	free(test);

	test = d_string_copy_substring(result, 8, 2);
	CuAssertStrEquals(tc, NULL, test);
	free(test);

	test = d_string_copy_substring(result, -1, -1);
	CuAssertStrEquals(tc, NULL, test);
	free(test);

	test = d_string_copy_substring(NULL, -1, -1);
	CuAssertStrEquals(tc, NULL, test);

	d_string_free(result, true);
}
#endif


/// Replace occurences of "original" with "replace" inside the specified range
/// Returns the change in overall length
long d_string_replace_text_in_range(DString * d, size_t pos, size_t len, const char * original, const char * replace) {
	if (d && original && replace) {
		long delta = 0;		// Overall change in length

		long len_o = strlen(original);
		long len_r = strlen(replace);
		long change = len_r - len_o;	// Change in length for each replacement

		size_t stop;

		if (len == -1) {
			stop = d->currentStringLength;
		} else {
			stop = pos + len;

			if (stop > d->currentStringLength) {
				stop = d->currentStringLength;
			}
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
	} else {
		return 0;
	}
}


#ifdef TEST
void Test_d_string_replace_text_in_range(CuTest* tc) {
	char * test = "foobarfoobarfoo";
	long delta = 0;

	DString * result = d_string_new(test);

	delta = d_string_replace_text_in_range(result, 100, 3, "foo", "zapz");
	CuAssertIntEquals(tc, 15, result->currentStringLength);
	CuAssertStrEquals(tc, "foobarfoobarfoo", result->str);
	CuAssertIntEquals(tc, delta, 0);

	delta = d_string_replace_text_in_range(result, 0, 3, "foo", "zapz");
	CuAssertIntEquals(tc, 16, result->currentStringLength);
	CuAssertStrEquals(tc, "zapzbarfoobarfoo", result->str);
	CuAssertIntEquals(tc, delta, 1);

	delta = d_string_replace_text_in_range(result, 0, 100, "foo", "zapz");
	CuAssertIntEquals(tc, 18, result->currentStringLength);
	CuAssertStrEquals(tc, "zapzbarzapzbarzapz", result->str);
	CuAssertIntEquals(tc, delta, 2);

	delta = d_string_replace_text_in_range(result, 0, -1, NULL, "zap");
	CuAssertIntEquals(tc, 18, result->currentStringLength);
	CuAssertStrEquals(tc, "zapzbarzapzbarzapz", result->str);
	CuAssertIntEquals(tc, delta, 0);

	d_string_replace_text_in_range(result, 0, -1, "foo", NULL);
	CuAssertIntEquals(tc, 18, result->currentStringLength);
	CuAssertStrEquals(tc, "zapzbarzapzbarzapz", result->str);

	d_string_replace_text_in_range(NULL, 0, -1, "foo", NULL);

	d_string_free(result, true);
}
#endif
