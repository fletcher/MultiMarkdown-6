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
#include "file.h"
#include "libMultiMarkdown.h"
#include "transclude.h"


/// strdup() not available on all platforms
static char * my_strdup(const char * source) {
	char * result = malloc(strlen(source) + 1);

	if (result) {
		strcpy(result, source);
	}

	return result;
}


/// Recursively transclude source text, given a search directory.
/// Track files to prevent infinite recursive loops
void mmd_transclude_source(DString * source, const char * search_path, const char * source_path, short format, stack * parsed, stack * manifest) {
	DString * file_path;
	DString * buffer;

	// Ensure search_folder is tidied up
	char * search_folder = path_from_dir_base(search_path, NULL);
	char * source_folder;
	char * source_file;

	split_path_file(&source_folder, &source_file, source_path);

	char * start, * stop;
	char text[1100];

	char * temp;

	size_t offset = 0;
	size_t last_match;

	mmd_engine * e = mmd_engine_create_with_dstring(source, EXT_TRANSCLUDE);

	if (mmd_engine_has_metadata(e, &offset)) {

		temp = mmd_engine_metavalue_for_key(e, "transclude base");

		if (temp) {
			// The new file overrides the search path
			free(search_folder);

			// Calculate new search path relative to source document
			search_folder = path_from_dir_base(source_folder, temp);
		}
	}

	free(source_folder);
	free(source_file);

	mmd_engine_free(e, false);

	if (search_folder == NULL) {
		// We don't have anywhere to search, so nothing to do
		goto exit;
	}

	// Make sure we use a parse tree for children
	stack * parse_stack = parsed;

	if (parsed == NULL) {
		// Create temporary stack
		parse_stack = stack_new(0);
	}

	// Remember where we currently are in the stack
	size_t stack_depth = parse_stack->size;

	// Iterate through source text, looking for `{{foo}}`

	start = strstr(&source->str[offset], "{{");

	while (start != NULL) {
		stop = strstr(start, "}}");

		if (stop == NULL) {
			break;
		}

		// Remember insertion point
		last_match = start - source->str;

		// Ensure we have a reasonable match -- cap at 1000 characters
		if (stop - start < 1000) {
			// Grab text
			strncpy(text, start + 2, stop - start - 2);
			text[stop - start - 2] = '\0';

			// Is this just {{TOC}}
			if (strcmp("TOC", text) == 0) {
				start = strstr(stop, "{{");
				continue;
			}

			// Is this an absolute path or relative path?
			if (is_separator(text[0])) {
				// Absolute path
				file_path = d_string_new(text);
			} else {
				// Relative path
				file_path = d_string_new(search_folder);

				// Ensure that search_folder ends in separator
				add_trailing_sep(file_path);

				d_string_append(file_path, text);
			}

			// Adjust file wildcard extension for output format
			// e.g. `foo.*`
			if ((format != FORMAT_MMD) && strncmp(&text[stop - start - 4], ".*", 2) == 0) {
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

					case FORMAT_FODT:
						d_string_append(file_path, ".fodt");
						break;

					default:
						d_string_append(file_path, ".txt");
						break;

				}
			}

			// Prevent infinite recursive loops
			for (int i = 0; i < stack_depth; ++i) {
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

				for (int i = 0; i < manifest->size; ++i) {
					temp = stack_peek_index(manifest, i);

					if (strcmp(file_path->str, temp) == 0) {
						// Already on manifest, don't duplicate
						add = false;
					}
				}

				// Add path to manifest
				if (add) {
					stack_push(manifest, my_strdup(file_path->str));
				}
			}

			// Read the file
			buffer = scan_file(file_path->str);

			// Substitue buffer for transclusion token
			if (buffer) {
				// Erase transclusion token from current source
				d_string_erase(source, start - source->str, 2 + stop - start);

				// Recursively check this file for transclusions
				mmd_transclude_source(buffer, search_folder, file_path->str, format, parse_stack, manifest);

				// Strip metadata from buffer now that we have parsed it
				e = mmd_engine_create_with_dstring(buffer, EXT_TRANSCLUDE);

				if (mmd_engine_has_metadata(e, &offset)) {
					d_string_erase(buffer, 0, offset);
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
	} else {
		// Reset stack depth
		parse_stack->size = stack_depth;
	}

	free(search_folder);
}



/// If MMD Header metadata used, insert it into appropriate place
void mmd_prepend_mmd_header(DString * source) {
	size_t end;

	if (mmd_d_string_has_metadata(source, &end)) {
		char * meta = mmd_d_string_metavalue_for_key(source, "mmdheader");

		if (meta) {
			d_string_insert(source, end, "\n\n");
			d_string_insert(source, end + 2, meta);
			free(meta);
		}
	}
}


/// If MMD Footer metadata used, insert it into appropriate place
void mmd_append_mmd_footer(DString * source) {
	char * meta = mmd_d_string_metavalue_for_key(source, "mmdfooter");

	if (meta) {
		d_string_append(source, "\n\n");
		d_string_append(source, meta);

		free(meta);
	}
}

