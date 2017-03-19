/**

	MultiMarkdown 6 -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file transclude.c

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
#include <string.h>

#include "d_string.h"
#include "libMultiMarkdown.h"
#include "transclude.h"

#if defined(__WIN32)
#include <windows.h>
#endif

#define kBUFFERSIZE 4096	// How many bytes to read at a time


/// strndup not available on all platforms
static char * my_strndup(const char * source, size_t n) {
	size_t len = 0;
	char * result;
	const char * test = source;

	// strlen is too slow if strlen(source) >> n
	for (len = 0; len < n; ++len)
	{
		if (test == '\0')
			break;

		test++;
	}

	result = malloc(len + 1);

	if (result) {
		memcpy(result, source, len);
		result[len] = '\0';
	}
	
	return result;
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


void add_trailing_sep(DString * path) {
#if defined(__WIN32)
	char sep = '\\';
#else
	char sep = '/';
#endif

	// Ensure that folder ends in separator
	if (!is_separator(path->str[path->currentStringLength - 1])) {
		d_string_append_c(path, sep);
	}
}

/// Combine directory and base filename to create a full path */
char * path_from_dir_base(const char * dir, const char * base) {
	if (!dir && !base)
		return NULL;


	DString * path = NULL;
	char * result = NULL;

	if ((base != NULL) && (is_separator(base[0]))) {
		// We have an absolute path
		path = d_string_new(base);
	} else {
		// We have a directory and relative path
		path = d_string_new(dir);

		// Ensure that folder ends in separator
		add_trailing_sep(path);

		// Append filename (if present)
		if (base)
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
void split_path_file(char ** dir, char ** file, char * path) {
    char * slash = path, * next;

#if defined(__WIN32)
	const char sep[] = "\\/";	// Windows allows either variant
#else
	const char sep[] = "/";
#endif

    while ((next = strpbrk(slash + 1, sep)))
    	slash = next;
    
    if (path != slash)
    	slash++;

    *dir = my_strndup(path, slash - path);
    *file = strdup(slash);
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


DString * scan_file(const char * fname) {
	/* Read from a file and return a DString *
		`buffer` will need to be freed elsewhere */

	char chunk[kBUFFERSIZE];
	size_t bytes;

	FILE * file;

#if defined(__WIN32)
	int wchars_num = MultiByteToWideChar(CP_UTF8, 0, fname, -1, NULL, 0);
	wchar_t wstr[wchars_num];
	MultiByteToWideChar(CP_UTF8, 0, fname, -1, wstr, wchars_num);

	if ((file = _wfopen(wstr, L"r")) == NULL) {
#else
	if ((file = fopen(fname, "r")) == NULL ) {
#endif

		return NULL;
	}

	DString * buffer = d_string_new("");

	while ((bytes = fread(chunk, 1, kBUFFERSIZE, file)) > 0) {
		d_string_append_c_array(buffer, chunk, bytes);
	}

	fclose(file);

	return buffer;
}


/// Recursively transclude source text, given a search directory.
/// Track files to prevent infinite recursive loops
void transclude_source(DString * source, const char * dir, short format, stack * parsed, stack * manifest) {
	DString * file_path;
	DString * buffer;

	// Ensure folder is tidied up
	char * folder = path_from_dir_base(dir, NULL);

	char * start, * stop;
	char text[1100];

	char * temp;

	size_t offset;
	size_t last_match;

	// TODO: Does this source have metadata that overrides the search directory?
	mmd_engine * e = mmd_engine_create_with_dstring(source, EXT_TRANSCLUDE);
	if (mmd_has_metadata(e, &offset)) {

		temp = metavalue_for_key(e, "transclude base");

		if (temp) {
			free(folder);

			folder = path_from_dir_base(dir, temp);
		}
	}

	mmd_engine_free(e, false);

	if (folder == NULL) {
		// We don't have anywhere to search, so nothing to do
		goto exit;
	}

	// Make sure we use a parse tree for children
	stack * parse_stack = parsed;

	if (parsed == NULL) {
		// Create temporary stack
		parse_stack = stack_new(0);
	}

	// Iterate through source text, looking for `{{foo}}`

	start = strstr(source->str, "{{");

	while (start != NULL) {
		stop = strstr(start, "}}");

		if (stop == NULL)
			break;

		// Remember insertion point
		last_match = start - source->str;

		// Ensure we have a reasonable match -- cap at 1000 characters
		if (stop - start < 1000) {
			// Grab text
			strncpy(text, start + 2, stop - start - 2);
			text[stop - start - 2] = '\0';

			// Is this just {{TOC}}
			if (strcmp("TOC",text) == 0) {
				start = strstr(stop, "{{");
				continue;
			}

			// Is this an absolute path or relative path?
			if (is_separator(text[0])) {
				// Absolute path
				file_path = d_string_new(text);
			} else {
				// Relative path
				file_path = d_string_new(folder);

				// Ensure that folder ends in separator
				add_trailing_sep(file_path);

				d_string_append(file_path, text);
			}

			// Adjust file wildcard extension for output format
			// e.g. `foo.*`
			if (format && strncmp(&text[stop - start - 4], ".*", 2) == 0) {
				// Trim '.*'
				d_string_erase(file_path, file_path->currentStringLength - 2, 2);

				switch (format) {
					case FORMAT_HTML:
						d_string_append(file_path, ".html");
						break;
					case FORMAT_LATEX:
					case FORMAT_BEAMER:
					case FORMAT_MEMOIR:
						d_string_append(file_path, ".tex");
						break;
					default:
						d_string_append(file_path, ".txt");
						break;

				}
			}

			// Prevent infinite recursive loops
			for (int i = 0; i < parse_stack->size; ++i)
			{
				temp = stack_peek_index(parse_stack, i);
				if (strcmp(file_path->str, temp) == 0) {
					// We have parsed this file already, don't recurse infinitely
					last_match += 2;
					goto finish_file;
				}
			}

			// Add this file to stack
			stack_push(parse_stack, file_path->str);

			// Add file to the manifest?
			if (manifest) {
				bool add = true;

				for (int i = 0; i < manifest->size; ++i)
				{
					temp = stack_peek_index(manifest, i);
					if (strcmp(file_path->str, temp) == 0) {
						// Already on manifest, don't duplicate
						add = false;
					}
				}

				// Add path to manifest
				if (add)
					stack_push(manifest, strdup(file_path->str));
			}

			// Read the file
			buffer = scan_file(file_path->str);

			// Substitue buffer for transclusion token
			if (buffer) {
				// Erase transclusion token from current source
				d_string_erase(source, start - source->str, 2 + stop - start);

				// Recursively check this file for transclusions
				transclude_source(buffer, folder, format, parse_stack, manifest);

				// Strip metadata from buffer now that we have parsed it
				e = mmd_engine_create_with_dstring(buffer, EXT_TRANSCLUDE);
				
				if (mmd_has_metadata(e, &offset)) {
					d_string_erase(buffer, 0, offset);
				} else {
					// Do we need to strip BOM?
					if (strncmp(buffer->str, "\xef\xbb\xbf",3) == 0)
					d_string_erase(buffer, 0, 3);
				}

				mmd_engine_free(e, false);

				// Insert file text -- this may cause d_string to reallocate the
				// character buffer, meaning start/stop are no longer valid
				d_string_insert(source, start - source->str, buffer->str);

				// Shift search point
				last_match += buffer->currentStringLength;

				d_string_free(buffer, true);
			} else {
				// Skip over marker
				last_match += 2;
			}

			// Remove this file from stack
			stack_pop(parse_stack);

			finish_file:
			d_string_free(file_path, true);

		} else {
			// Match was too long to be reasonable file name
			// Skip over marker
			last_match += 2;
		}

		start = strstr(source->str + last_match, "{{");
	}

	exit:

	if (parsed == NULL) {
		// Free temp stack
		stack_free(parse_stack);
	}

	free(folder);
}


