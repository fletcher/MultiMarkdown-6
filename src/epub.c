/**

	MultiMarkdown -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file epub.c

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

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#ifdef __APPLE__
	#include "TargetConditionals.h"
	#if TARGET_IPHONE_SIMULATOR
		// iOS Simulator
		#undef USE_CURL
	#elif TARGET_OS_IPHONE
		// iOS device
		#undef USE_CURL
	#elif TARGET_OS_MAC
		// Other kinds of Mac OS
	#else
		#error "Unknown Apple platform"
	#endif
#endif

#ifdef USE_CURL
	#include <curl/curl.h>
#endif

#include "epub.h"
#include "file.h"
#include "html.h"
#include "i18n.h"
#include "miniz.h"
#include "stack.h"
#include "uuid.h"
#include "writer.h"
#include "zip.h"

#define print(x) d_string_append(out, x)
#define print_const(x) d_string_append_c_array(out, x, sizeof(x) - 1)
#define print_char(x) d_string_append_c(out, x)
#define printf(...) d_string_append_printf(out, __VA_ARGS__)
#define print_token(t) d_string_append_c_array(out, &(source[t->start]), t->len)
#define print_localized(x) mmd_print_localized_char_html(out, x, scratch)


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


char * epub_mimetype(void) {
	return my_strdup("application/epub+zip");
}


char * epub_container_xml(void) {
	DString * container = d_string_new("");

	d_string_append(container, "<?xml version=\"1.0\"?>\n");
	d_string_append(container, "<container version=\"1.0\" xmlns=\"urn:oasis:names:tc:opendocument:xmlns:container\">\n");
	d_string_append(container, "<rootfiles>\n");
	d_string_append(container, "<rootfile full-path=\"OEBPS/main.opf\" media-type=\"application/oebps-package+xml\" />\n");
	d_string_append(container, "</rootfiles>\n");
	d_string_append(container, "</container>\n");

	char * result = container->str;
	d_string_free(container, false);
	return result;
}


char * epub_package_document(scratch_pad * scratch) {
	DString * out = d_string_new("");

	meta * m;

	d_string_append(out, "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"no\"?>\n");
	d_string_append(out, "<package xmlns=\"http://www.idpf.org/2007/opf\" version=\"3.0\" unique-identifier=\"pub-id\">\n");


	// Metadata
	d_string_append(out, "<metadata xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n");

	// Identifier
	HASH_FIND_STR(scratch->meta_hash, "uuid", m);

	if (m) {
		print_const("<dc:identifier id=\"pub-id\">urn:uuid:");
		mmd_print_string_html(out, m->value, false, false);
		print_const("</dc:identifier>\n");
	} else {
		print_const("<dc:identifier id=\"pub-id\">urn:uuid:");

		char * id = uuid_new();
		print(id);
		print_const("</dc:identifier>\n");
		free(id);
	}

	// Title
	HASH_FIND_STR(scratch->meta_hash, "title", m);

	if (m) {
		print_const("<dc:title>");
		mmd_print_string_html(out, m->value, false, false);
		print_const("</dc:title>\n");
	} else {
		print_const("<dc:title>Untitled</dc:title>\n");
	}

	// Author
	HASH_FIND_STR(scratch->meta_hash, "author", m);

	if (m) {
		print_const("<dc:creator>");
		mmd_print_string_html(out, m->value, false, false);
		print_const("</dc:creator>\n");
	}


	// Language
	HASH_FIND_STR(scratch->meta_hash, "language", m);

	if (m) {
		print_const("<dc:language>");
		mmd_print_string_html(out, m->value, false, false);
		print_const("</dc:language>\n");
	} else {
		switch (scratch->language) {
			case LC_ES:
				print_const("<dc:language>es</dc:language>\n");
				break;

			case LC_DE:
				print_const("<dc:language>de</dc:language>\n");
				break;

			case LC_FR:
				print_const("<dc:language>fr</dc:language>\n");
				break;

			case LC_NL:
				print_const("<dc:language>nl</dc:language>\n");
				break;

			case LC_SV:
				print_const("<dc:language>sv</dc:language>\n");
				break;

			case LC_HE:
				print_const("<dc:language>he</dc:language>\n");
				break;

			default:
				print_const("<dc:language>en</dc:language>\n");
		}
	}

	// Date
	HASH_FIND_STR(scratch->meta_hash, "date", m);

	if (m) {
		print_const("<meta property=\"dcterms:modified\">");
		mmd_print_string_html(out, m->value, false, false);
		print_const("</meta>\n");
	} else {
		time_t t = time(NULL);
		struct tm * today = localtime(&t);

		d_string_append_printf(out, "<meta property=\"dcterms:modified\">%d-%02d-%02d</meta>\n",
							   today->tm_year + 1900, today->tm_mon + 1, today->tm_mday);
	}

	d_string_append(out, "</metadata>\n");


	// Manifest
	d_string_append(out, "<manifest>\n");
	d_string_append(out, "<item id=\"nav\" href=\"nav.xhtml\" properties=\"nav\" media-type=\"application/xhtml+xml\"/>\n");
	d_string_append(out, "<item id=\"main\" href=\"main.xhtml\" media-type=\"application/xhtml+xml\"/>\n");
	d_string_append(out, "</manifest>\n");

	// Spine
	d_string_append(out, "<spine>\n");
	d_string_append(out, "<itemref idref=\"main\"/>");
	d_string_append(out, "</spine>\n");

	d_string_append(out, "</package>\n");

	char * result = out->str;
	d_string_free(out, false);
	return result;
}


void epub_export_nav_entry(DString * out, const char * source, scratch_pad * scratch, size_t * counter, short level) {
	token * entry, * next;
	short entry_level, next_level;
	char * temp_char;

	print_const("\n<ol>\n");

	// Iterate over tokens
	while (*counter < scratch->header_stack->size) {
		// Get token for header
		entry = stack_peek_index(scratch->header_stack, *counter);
		entry_level = raw_level_for_header(entry);

		if (entry_level >= level) {
			// This entry is a direct descendant of the parent
			scratch->label_counter = (int) * counter;
			temp_char = label_from_header(source, entry, scratch);
			printf("<li><a href=\"main.xhtml#%s\">", temp_char);
			mmd_export_token_tree_html(out, source, entry->child, scratch);
			print_const("</a>");

			if (*counter < scratch->header_stack->size - 1) {
				next = stack_peek_index(scratch->header_stack, *counter + 1);
				next_level = next->type - BLOCK_H1 + 1;

				if (next_level > entry_level) {
					// This entry has children
					(*counter)++;
					epub_export_nav_entry(out, source, scratch, counter, entry_level + 1);
				}
			}

			print_const("</li>\n");
			free(temp_char);
		} else if (entry_level < level ) {
			// If entry < level, exit this level
			// Decrement counter first, so that we can test it again later
			(*counter)--;
			break;
		}

		// Increment counter
		(*counter)++;
	}

	print_const("</ol>\n");
}


void epub_export_nav(DString * out, mmd_engine * e, scratch_pad * scratch) {
	size_t counter = 0;

	int old_label_counter = scratch->label_counter;

	epub_export_nav_entry(out, e->dstr->str, scratch, &counter, 0);

	scratch->label_counter = old_label_counter;
}


char * epub_nav(mmd_engine * e, scratch_pad * scratch) {
	meta * temp;

	DString * out = d_string_new("");

	d_string_append(out, "<!DOCTYPE html>\n<html xmlns=\"http://www.w3.org/1999/xhtml\" xmlns:epub=\"http://www.idpf.org/2007/ops\">\n");

	d_string_append(out, "<head>\n<title>");
	HASH_FIND_STR(scratch->meta_hash, "title", temp);

	if (temp) {
		mmd_print_string_html(out, temp->value, false, false);
	} else {
		print_const("Untitled");
	}

	print_const("</title>\n</head>\n");

	print_const("<body>\n<nav epub:type=\"toc\">\n");
	print_const("<h2>Table of Contents</h2>\n");

	epub_export_nav(out, e, scratch);

	print_const("</nav>\n</body>\n</html>\n");

	char * result = out->str;
	d_string_free(out, false);
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
			fprintf(stderr, "Error adding asset to zip.\n");
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

		char destination[100] = "OEBPS/assets/";
		destination[49] = '\0';

		mz_bool status;

		curl_global_init(CURL_GLOBAL_ALL);
		curl = curl_easy_init();

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

		HASH_ITER(hh, e->asset_hash, a, a_tmp) {
			curl_easy_setopt(curl, CURLOPT_URL, a->url);
			res = curl_easy_perform(curl);

			memcpy(&destination[13], a->asset_path, 36);

			if (res != CURLE_OK) {
				// Attempt to add asset from local file
				if (!add_asset_from_file(pZip, a, destination, directory)) {
					fprintf(stderr, "Unable to store '%s' in EPUB\n", a->url);
				}
			} else {
				// Store downloaded file in zip
				status = mz_zip_writer_add_mem(pZip, destination, chunk.memory, chunk.size, MZ_BEST_COMPRESSION);

				if (!status) {
					fprintf(stderr, "Error adding asset to zip.\n");
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

		char destination[100] = "OEBPS/assets/";
		destination[49] = '\0';

		HASH_ITER(hh, e->asset_hash, a, a_tmp) {

			memcpy(&destination[13], a->asset_path, 36);

			// Attempt to add asset from local file
			if (!add_asset_from_file(pZip, a, destination, directory)) {
				fprintf(stderr, "Unable to store '%s' in EPUB\n", a->url);
			}
		}
	}
}
#endif


// Use the miniz library to create a zip archive for the EPUB document
void epub_write_wrapper(const char * filepath, DString * body, mmd_engine * e, const char * directory) {
	FILE * output_stream;

	DString * result = epub_create(body, e, directory);

	if (!(output_stream = fopen(filepath, "w"))) {
		// Failed to open file
		perror(filepath);
	} else {
		fwrite(&(result->str), result->currentStringLength, 1, output_stream);
		fclose(output_stream);
	}

	d_string_free(result, true);
}


DString * epub_create(DString * body, mmd_engine * e, const char * directory) {
	DString * result = d_string_new("");
	scratch_pad * scratch = scratch_pad_new(e, FORMAT_EPUB);
	scratch->random_seed_base_labels = e->random_seed_base_labels;

	mz_bool status;
	char * data;
	size_t len;

	mz_zip_archive zip;
	zip_new_archive(&zip);

	// Add mimetype
	data = epub_mimetype();
	len = strlen(data);
	status = mz_zip_writer_add_mem(&zip, "mimetype", data, len, MZ_BEST_COMPRESSION);
	free(data);

	if (!status) {
		fprintf(stderr, "Error adding asset to zip.\n");
	}

	// Create directories
	status = mz_zip_writer_add_mem(&zip, "OEBPS/", NULL, 0, MZ_NO_COMPRESSION);

	if (!status) {
		fprintf(stderr, "Error adding asset to zip.\n");
	}

	status = mz_zip_writer_add_mem(&zip, "META-INF/", NULL, 0, MZ_NO_COMPRESSION);

	if (!status) {
		fprintf(stderr, "Error adding asset to zip.\n");
	}

	// Add container
	data = epub_container_xml();
	len = strlen(data);
	status = mz_zip_writer_add_mem(&zip, "META-INF/container.xml", data, len, MZ_BEST_COMPRESSION);
	free(data);

	if (!status) {
		fprintf(stderr, "Error adding asset to zip.\n");
	}

	// Add package
	data = epub_package_document(scratch);
	len = strlen(data);
	status = mz_zip_writer_add_mem(&zip, "OEBPS/main.opf", data, len, MZ_BEST_COMPRESSION);
	free(data);

	if (!status) {
		fprintf(stderr, "Error adding asset to zip.\n");
	}

	// Add nav
	data = epub_nav(e, scratch);
	len = strlen(data);
	status = mz_zip_writer_add_mem(&zip, "OEBPS/nav.xhtml", data, len, MZ_BEST_COMPRESSION);
	free(data);

	if (!status) {
		fprintf(stderr, "Error adding asset to zip.\n");
	}

	// Add main document
	status = mz_zip_writer_add_mem(&zip, "OEBPS/main.xhtml", body->str, body->currentStringLength, MZ_BEST_COMPRESSION);

	if (!status) {
		fprintf(stderr, "Error adding asset to zip.\n");
	}

	// Add assets
	add_assets(&zip, e, directory);

	scratch_pad_free(scratch);

	// Finalize zip archive and extract data
	free(result->str);

	status = mz_zip_writer_finalize_heap_archive(&zip, (void **) & (result->str), (size_t *) & (result->currentStringLength));

	if (!status) {
		fprintf(stderr, "Error finalizing zip archive.\n");
	}

	return result;
}


