/**

	MultiMarkdown -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file textbundle.c

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
#include <sys/stat.h>

#ifdef USE_CURL
	#include <curl/curl.h>
#endif

#include "file.h"
#include "miniz.h"
#include "textbundle.h"
#include "writer.h"
#include "zip.h"


char * textbundle_info_json(void) {
	DString * info = d_string_new("");

	d_string_append(info, "{\n");
	d_string_append(info, "\t\"version\": 2,\n");
	d_string_append(info, "\t\"type\": \"net.daringfireball.markdown\",\n");
	d_string_append(info, "\t\"transient\": false,\n");
	d_string_append(info, "\t\"creatorIdentifier\": \"net.multimarkdown\"\n");
	d_string_append(info, "}");

	char * result = info->str;
	d_string_free(info, false);
	return result;
}


static bool add_asset_from_file(mz_zip_archive * pZip, asset * a, const char * destination, const char * directory) {
	if (!directory) {
		return false;
	}

	char * path = path_from_dir_base(directory, a->url);
	mz_bool status;
	bool result = false;

	DString * buffer = scan_file(path);

	if (buffer && buffer->currentStringLength > 0) {
		status = mz_zip_writer_add_mem(pZip, destination, buffer->str, buffer->currentStringLength, MZ_BEST_COMPRESSION);

		if (!status) {
			fprintf(stderr, "Error adding asset '%s' to zip.\n", destination);
		}

		d_string_free(buffer, true);
		result = true;
	}

	free(path);

	return result;
}


#ifdef USE_CURL
// Dynamic buffer for downloading files in memory
// Based on https://curl.haxx.se/libcurl/c/getinmemory.html

struct MemoryStruct {
	char * memory;
	size_t size;
};


static size_t write_memory(void * contents, size_t size, size_t nmemb, void * userp) {
	size_t realsize = size * nmemb;
	struct MemoryStruct * mem = (struct MemoryStruct *)userp;

	mem->memory = realloc(mem->memory, mem->size + realsize + 1);

	if (mem->memory == NULL) {
		// Out of memory
		fprintf(stderr, "Out of memory\n");
		return 0;
	}

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

// Add assets to zipfile using libcurl
static void add_assets(mz_zip_archive * pZip, mmd_engine * e, const char * directory) {
	asset * a, * a_tmp;

	if (e->asset_hash) {
		CURL * curl;
		CURLcode res;

		struct MemoryStruct chunk;
		chunk.memory = malloc(1);
		chunk.size = 0;

		char destination[100] = "assets/";
		destination[43] = '\0';

		mz_bool status;

		curl_global_init(CURL_GLOBAL_ALL);
		curl = curl_easy_init();

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

		HASH_ITER(hh, e->asset_hash, a, a_tmp) {
			curl_easy_setopt(curl, CURLOPT_URL, a->url);
			res = curl_easy_perform(curl);

			memcpy(&destination[7], a->asset_path, 36);

			if (res != CURLE_OK) {
				// Attempt to add asset from local file
				if (!add_asset_from_file(pZip, a, destination, directory)) {
					fprintf(stderr, "Unable to store '%s' in EPUB\n", a->url);
				}
			} else {
				// Store downloaded file in zip
				status = mz_zip_writer_add_mem(pZip, destination, chunk.memory, chunk.size, MZ_BEST_COMPRESSION);

				if (!status) {
					fprintf(stderr, "Error adding asset '%s' to zip as '%s'.\n", a->asset_path, destination);
				}
			}
		}
	}
}

#else
// Add local assets only (libcurl not available)
static void add_assets(mz_zip_archive * pZip, mmd_engine * e, const char * directory) {
	asset * a, * a_tmp;

	if (e->asset_hash) {

		char destination[100] = "assets/";
		destination[43] = '\0';

		mz_bool status;

		HASH_ITER(hh, e->asset_hash, a, a_tmp) {

			memcpy(&destination[7], a->asset_path, 36);

			// Attempt to add asset from local file
			if (!add_asset_from_file(pZip, a, destination, directory)) {
				fprintf(stderr, "Unable to store '%s' in EPUB\n", a->url);
			}
		}
	}
}
#endif


void traverse_for_images(token * t, DString * text, mmd_engine * e, long * offset, char * destination, char * url) {
	asset * a;
	char * clean;
	link * l;

	while (t) {
		switch (t->type) {
			case PAIR_BRACKET_IMAGE:
				if (t->next && t->next->type == PAIR_PAREN) {
					t = t->next;

					memcpy(url, &text->str[t->start + *offset + 1], t->len - 2);
					url[t->len - 2] = '\0';
					clean = clean_string(url, false);

					HASH_FIND_STR(e->asset_hash, clean, a);

					if (a) {
						// Replace url with asset path
						memcpy(&destination[7], a->asset_path, 36);
						* offset += d_string_replace_text_in_range(text, t->start + *offset, t->len, clean, destination);
					}

					free(clean);
				}

				break;

			case BLOCK_EMPTY:

				// Is this a link definition?
				for (int i = 0; i < e->definition_stack->size; ++i) {
					if (t == stack_peek_index(e->definition_stack, i)) {
						// Find matching link
						for (int j = 0; j < e->link_stack->size; ++j) {
							l = stack_peek_index(e->link_stack, j);

							if (l->label->start == t->child->start) {
								// This is a match
								HASH_FIND_STR(e->asset_hash, l->url, a);

								if (a) {
									memcpy(&destination[7], a->asset_path, 36);
									* offset += d_string_replace_text_in_range(text, t->start + *offset, t->len, l->url, destination);
								}
							}
						}
					}
				}

				break;

			default:
				if (t->child) {
					traverse_for_images(t->child, text, e, offset, destination, url);
				}

				break;
		}

		t = t->next;
	}
}


void sub_asset_paths(DString * text, mmd_engine * e) {
	long offset = 0;
	asset * a;
	token * t = e->root->child;

	char destination[100] = "assets/";
	destination[43] = '\0';

	// Is there CSS metadata?
	if (e->metadata_stack) {
		if (e->metadata_stack->size > 0) {
			meta * m;

			for (int i = 0; i < e->metadata_stack->size; ++i) {
				m = stack_peek_index(e->metadata_stack, i);

				if (strcmp("css", m->key) == 0) {
					// Get METADATA range
					t = e->root->child;
					token_skip_until_type(&t, BLOCK_META);

					// Substitute inside metadata block
					HASH_FIND_STR(e->asset_hash, m->value, a);

					if (a) {
						memcpy(&destination[7], a->asset_path, 36);
						offset += d_string_replace_text_in_range(text, t->start, t->len, m->value, destination);
					}
				}
			}
		}
	}


	// Find images
	char url[1000] = "";

	// Travel parse tree for images and image reference definitions
	traverse_for_images(t, text, e, &offset, &destination[0], &url[0]);
}


DString * textbundle_create(const char * body, mmd_engine * e, const char * directory) {
	DString * result = d_string_new("");
	scratch_pad * scratch = scratch_pad_new(e, FORMAT_TEXTBUNDLE_COMPRESSED);

	mz_bool status;
	char * data;
	size_t len;

	mz_zip_archive zip;
	zip_new_archive(&zip);


	// Add info json
	data = textbundle_info_json();
	len = strlen(data);
	status = mz_zip_writer_add_mem(&zip, "info.json", data, len, MZ_BEST_COMPRESSION);
	free(data);

	if (!status) {
		fprintf(stderr, "Error adding JSON info to zip.\n");
	}

	// Create directories
	status = mz_zip_writer_add_mem(&zip, "assets/", NULL, 0, MZ_NO_COMPRESSION);

	if (!status) {
		fprintf(stderr, "Error adding assets directory to zip.\n");
	}

	// Add main document
	DString * temp = d_string_new(e->dstr->str);

	sub_asset_paths(temp, e);

	len = temp->currentStringLength;
	status = mz_zip_writer_add_mem(&zip, "text.markdown", temp->str, len, MZ_BEST_COMPRESSION);

	if (!status) {
		fprintf(stderr, "Error adding content to zip.\n");
	}

	// Add html version document
	len = strlen(body);
	status = mz_zip_writer_add_mem(&zip, "text.html", body, len, MZ_BEST_COMPRESSION);

	if (!status) {
		fprintf(stderr, "Error adding content to zip.\n");
	}

	// Add assets
	add_assets(&zip, e, directory);

	scratch_pad_free(scratch);

	// Finalize zip archive and extract data
	free(result->str);

	status = mz_zip_writer_finalize_heap_archive(&zip, (void **) & (result->str), (size_t *) & (result->currentStringLength));

	if (!status) {
		fprintf(stderr, "Error finalizing zip.\n");
	}

	return result;
}



// Use the miniz library to create a zip archive for the TEXTBUNDLE_COMPRESSED document
void textbundle_write_wrapper(const char * filepath, const char * body, mmd_engine * e, const char * directory) {
	FILE * output_stream;

	DString * result = textbundle_create(body, e, directory);

	if (!(output_stream = fopen(filepath, "w"))) {
		// Failed to open file
		perror(filepath);
	} else {
		fwrite(&(result->str), result->currentStringLength, 1, output_stream);
		fclose(output_stream);
	}

	d_string_free(result, true);
}
