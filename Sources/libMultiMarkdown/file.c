/**

	MultiMarkdown -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file file.c

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

	uthash library:
		Copyright (c) 2005-2016, Troy D. Hanson

		Licensed under Revised BSD license

	miniz library:
		Copyright 2013-2014 RAD Game Tools and Valve Software
		Copyright 2010-2014 Rich Geldreich and Tenacious Software LLC

		Licensed under the MIT license

	argtable3 library:
		Copyright (C) 1998-2001,2003-2011,2013 Stewart Heitmann
		<sheitmann@users.sourceforge.net>
		All rights reserved.

		Licensed under the Revised BSD License


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


	## Revised BSD License ##

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are
	met:
	    * Redistributions of source code must retain the above copyright
	      notice, this list of conditions and the following disclaimer.
	    * Redistributions in binary form must reproduce the above
	      copyright notice, this list of conditions and the following
	      disclaimer in the documentation and/or other materials provided
	      with the distribution.
	    * Neither the name of the <organization> nor the
	      names of its contributors may be used to endorse or promote
	      products derived from this software without specific prior
	      written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT
	HOLDER> BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES LOSS OF USE, DATA, OR
	PROFITS OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
	LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "d_string.h"
#include "file.h"

#if defined(__WIN32)
	#include <windows.h>
#endif

#define kBUFFERSIZE 4096	// How many bytes to read at a time


/// Scan file into a DString
DString * scan_file(const char * fname) {
	/* Read from stdin and return a DString *
		`buffer` will need to be freed elsewhere */

	char chunk[kBUFFERSIZE];
	size_t bytes;

	FILE * file;

	#if defined(__WIN32)
	int wchars_num = MultiByteToWideChar(CP_UTF8, 0, fname, -1, NULL, 0);
	wchar_t wstr[wchars_num];
	MultiByteToWideChar(CP_UTF8, 0, fname, -1, wstr, wchars_num);

	if ((file = _wfopen(wstr, L"rb")) == NULL) {
	#else

	if ((file = fopen(fname, "r")) == NULL ) {
	#endif

		return NULL;
	}

	DString * buffer = d_string_new("");

	while ((bytes = fread(chunk, 1, kBUFFERSIZE, file)) > 0) {
		d_string_append_c_array(buffer, chunk, bytes);

		if (buffer->currentStringLength <= kBUFFERSIZE) {
			// Strip BOM
			if (strncmp(buffer->str, "\xef\xbb\xbf", 3) == 0) {
				d_string_erase(buffer, 0, 3);
			}
		}
	}

	fclose(file);

	return buffer;
}


/// Scan from stdin into a DString
DString * stdin_buffer(void) {
	/* Read from stdin and return a GString *
		`buffer` will need to be freed elsewhere */

	char chunk[kBUFFERSIZE];
	size_t bytes;

	DString * buffer = d_string_new("");

	while ((bytes = fread(chunk, 1, kBUFFERSIZE, stdin)) > 0) {
		d_string_append_c_array(buffer, chunk, bytes);
	}

	fclose(stdin);

	return buffer;
}


/// Windows can use either `\` or `/` as a separator -- thanks to t-beckmann on github
///	for suggesting a fix for this.
bool is_separator(char c) {
	#if defined(__WIN32)
	return c == '\\' || c == '/';
	#else
	return c == '/';
	#endif
}


#ifdef TEST
void Test_is_separator(CuTest* tc) {
	char * test = "a/\\";

	#if defined(__WIN32)
	CuAssertIntEquals(tc, false, is_separator(test[0]));
	CuAssertIntEquals(tc, true, is_separator(test[1]));
	CuAssertIntEquals(tc, true, is_separator(test[2]));
	#else
	CuAssertIntEquals(tc, false, is_separator(test[0]));
	CuAssertIntEquals(tc, true, is_separator(test[1]));
	CuAssertIntEquals(tc, false, is_separator(test[2]));
	#endif
}
#endif


/// Ensure that path ends in separator
void add_trailing_sep(DString * path) {
	#if defined(__WIN32)
	char sep = '\\';
	#else
	char sep = '/';
	#endif

	// Ensure that folder ends in separator
	if ((path->currentStringLength == 0) || (!is_separator(path->str[path->currentStringLength - 1]))) {
		d_string_append_c(path, sep);
	}
}


/// strndup not available on all platforms
static char * my_strndup(const char * source, size_t n) {
	if (source == NULL) {
		return NULL;
	}

	size_t len = 0;
	char * result;
	const char * test = source;

	// strlen is too slow if strlen(source) >> n
	for (len = 0; len < n; ++len) {
		if (*test == '\0') {
			break;
		}

		test++;
	}

	result = malloc(len + 1);

	if (result) {
		memcpy(result, source, len);
		result[len] = '\0';
	}

	return result;
}


/// strdup() not available on all platforms
static char * my_strdup(const char * source) {
	if (source == NULL) {
		return NULL;
	}

	char * result = malloc(strlen(source) + 1);

	if (result) {
		strcpy(result, source);
	}

	return result;
}


/// Combine directory and base filename to create a full path */
char * path_from_dir_base(const char * dir, const char * base) {
	if (!dir && !base) {
		return NULL;
	}

	DString * path = NULL;
	char * result = NULL;

	if ((base != NULL) && (is_separator(base[0]))) {
		// We have an absolute path
		return my_strdup(base);
	}

	// We have a directory and relative path
	path = d_string_new(dir);

	// Ensure that folder ends in separator
	add_trailing_sep(path);

	// Append filename (if present)
	if (base) {
		d_string_append(path, base);
	}

	result = path->str;
	d_string_free(path, false);

	return result;
}


#ifdef TEST
void Test_path_from_dir_base(CuTest* tc) {
	char dir[10] = "/foo";
	char base[10] = "bar";

	char * path = path_from_dir_base(dir, base);

	#if defined(__WIN32)
	CuAssertStrEquals(tc, "/foo\\bar", path);
	#else
	CuAssertStrEquals(tc, "/foo/bar", path);
	#endif

	free(path);
	strcpy(base, "/bar");

	path = path_from_dir_base(dir, base);

	CuAssertStrEquals(tc, "/bar", path);

	free(path);

	path = path_from_dir_base(NULL, NULL);
	CuAssertStrEquals(tc, NULL, path);
}
#endif


/// Separate filename and directory from a full path
///
/// See http://stackoverflow.com/questions/1575278/function-to-split-a-filepath-into-path-and-file
void split_path_file(char ** dir, char ** file, const char * path) {
	const char * slash = path, * next;

	#if defined(__WIN32)
	const char sep[] = "\\/";	// Windows allows either variant
	#else
	const char sep[] = "/";
	#endif

	while ((next = strpbrk(slash + 1, sep))) {
		slash = next;
	}

	if (path != slash) {
		slash++;
	}

	if (dir) {
		*dir = my_strndup(path, slash - path);
	}

	if (file) {
		*file = my_strdup(slash);
	}
}


#ifdef TEST
void Test_split_path_file(CuTest* tc) {
	char * dir, * file;

	char * path = "/foo/bar.txt";
	split_path_file(&dir, &file, path);

	CuAssertStrEquals(tc, "/foo/", dir);
	CuAssertStrEquals(tc, "bar.txt", file);

	path = "\\foo\\bar.txt";
	split_path_file(&dir, &file, path);

	#if defined(__WIN32)
	CuAssertStrEquals(tc, "\\foo\\", dir);
	CuAssertStrEquals(tc, "bar.txt", file);
	#else
	CuAssertStrEquals(tc, "", dir);
	CuAssertStrEquals(tc, "\\foo\\bar.txt", file);
	#endif
}
#endif


// Windows does not know realpath(), so we need a "windows port"
// Fix by @f8ttyc8t (<https://github.com/f8ttyc8t>)
#if (defined(_WIN32) || defined(__WIN32__))
// Let compiler know where to find GetFullPathName()
#include <windows.h>

char *realpath(const char *path, char *resolved_path) {
	DWORD  retval = 0;
	DWORD  dwBufSize = 0; // Just in case MAX_PATH differs from PATH_MAX
	TCHAR  *buffer = NULL;

	if (resolved_path == NULL) {
		// realpath allocates appropiate bytes if resolved_path is null. This is to mimic realpath behavior
		dwBufSize = PATH_MAX; // Use windows PATH_MAX constant, because we are in Windows context now.
		buffer = (char*)malloc(dwBufSize);

		if (buffer == NULL) {
			return NULL; // some really weird is going on...
		}
	} else {
		dwBufSize = MAX_PATH;  // buffer has been allocated using MAX_PATH earlier
		buffer = resolved_path;
	}

	retval = GetFullPathName(path, dwBufSize, buffer, NULL);

	if (retval == 0) {
		return NULL;
		printf("Failed to GetFullPathName()\n");
	}

	return buffer;
}
#endif


// Convert argument to absolute path
char * absolute_path_for_argument(const char * arg) {
	char * result = NULL;
	#ifdef PATH_MAX
	// If PATH_MAX defined, use it
	char absolute[PATH_MAX + 1];
	realpath(arg, absolute);
	result = my_strdup(absolute);
	#else
	// If undefined, then we *should* be able to use a NULL pointer to allocate
	result = realpath(arg, NULL);
	#endif

	return result;
}

