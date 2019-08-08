/**

	MultiMarkdown 6 -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file main.c

	@brief Create command-line frontend for libMultiMarkdown.


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

#include <ctype.h>
#ifndef DJGPP
#include <libgen.h>
#endif
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "argtable3.h"
#include "d_string.h"
#include "file.h"
#include "i18n.h"
#include "libMultiMarkdown.h"
#include "token.h"
#include "uuid.h"
#include "version.h"
#include "zip.h"

#define kBUFFERSIZE 4096	// How many bytes to read at a time

// argtable structs
struct arg_lit * a_help, *a_version, *a_compatibility, *a_nolabels, *a_batch,
		   *a_accept, *a_reject, *a_full, *a_snippet, *a_random, *a_unique, *a_meta,
		   *a_notransclude, *a_nosmart, *a_opml, *a_itmz;
struct arg_str * a_format, *a_lang, *a_extract;
struct arg_file * a_file, *a_o;
struct arg_end * a_end;
struct arg_rem * a_rem1, *a_rem2, *a_rem3, *a_rem4, *a_rem5, *a_rem6;


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


/// Given a filename, remove the extension and replace it with a new one.
/// The next extension must include the leading '.', e.g. '.html'
char * filename_with_extension(const char * original, const char * new_extension) {
	char * name_no_ext;
	DString * new_name;

	// Determine output filename without file extension
	name_no_ext = my_strdup(original);

	if (strrchr(name_no_ext, '.') != NULL) {
		long count = strrchr(name_no_ext, '.') - name_no_ext;

		if (count != 0) {
			name_no_ext[count] = '\0';
		}
	}

	new_name = d_string_new(name_no_ext);
	free(name_no_ext);

	d_string_append(new_name, new_extension);

	name_no_ext = new_name->str;

	d_string_free(new_name, false);

	return name_no_ext;
}


int main(int argc, char ** argv) {
	int exitcode = EXIT_SUCCESS;
	char * binname = "multimarkdown";
	short format = FORMAT_HTML;
	short language = LC_EN;

	// Initialize argtable structs
	void * argtable[] = {
		a_help			= arg_lit0(NULL, "help", "display this help and exit"),
		a_version		= arg_lit0(NULL, "version", "display version info and exit"),

		a_rem1			= arg_rem("", ""),

		a_batch			= arg_lit0("b", "batch", "process each file separately"),
		a_full			= arg_lit0("f", "full", "force a complete document"),
		a_snippet		= arg_lit0("s", "snippet", "force a snippet"),
		a_compatibility	= arg_lit0("c", "compatibility", "Markdown compatibility mode"),
		a_random		= arg_lit0(NULL, "random", "use random numbers for footnote anchors"),
		a_unique		= arg_lit0(NULL, "unique", "use random numbers for header labels unless manually specified"),
		a_nosmart		= arg_lit0(NULL, "nosmart", "Disable smart typography"),
		a_nolabels		= arg_lit0(NULL, "nolabels", "Disable id attributes for headers"),
		a_notransclude	= arg_lit0(NULL, "notransclude", "Disable file transclusion"),
		a_opml			= arg_lit0(NULL, "opml", "Convert OPML source to plain text before processing"),
		a_itmz			= arg_lit0(NULL, "itmz", "Convert ITMZ (iThoughts) source to plain text before processing"),

		a_rem2			= arg_rem("", ""),

		a_format		= arg_str0("t", "to", "FORMAT", "convert to FORMAT, FORMAT = html|latex|beamer|memoir|mmd|odt|fodt|epub|opml|itmz|bundle|bundlezip"),
		a_o				= arg_file0("o", "output", "FILE", "send output to FILE"),

		a_rem3			= arg_rem("", ""),

		a_accept		= arg_lit0("a", "accept", "accept all CriticMarkup changes"),
		a_reject		= arg_lit0("r", "reject", "reject all CriticMarkup changes"),

		a_rem4			= arg_rem("", ""),

		a_lang			= arg_str0("l", "lang", "LANG", "language/smart quote localization, LANG = en|es|de|fr|he|nl|sv"),

		a_rem5			= arg_rem("", ""),

		a_meta			= arg_lit0("m", "metadata-keys", "list all metadata keys"),
		a_extract		= arg_str0("e", "extract", "KEY", "extract specified metadata key"),

		a_rem6			= arg_rem("", ""),

		a_file 			= arg_filen(NULL, NULL, "<FILE>", 0, argc + 2, "read input from file(s) -- use stdin if no files given"),

		a_end 			= arg_end(20),
	};

	// Set default options
	a_o->filename[0] = "-";		// Default to stdout if no option specified

	int nerrors = arg_parse(argc, argv, argtable);

	// '--help' takes precedence
	if (a_help->count > 0) {
		printf("\n%s v%s -- %s\n\n", MULTIMARKDOWN_NAME, MULTIMARKDOWN_VERSION, MULTIMARKDOWN_COPYRIGHT);
		printf("\tUsage: %s", binname);
		arg_print_syntax(stdout, argtable, "\n\n");
		printf("Options:\n");
		arg_print_glossary(stdout, argtable, "\t%-25s %s\n");
		printf("\n");
		goto exit2;
	}

	if (nerrors > 0) {
		// Report errors
		arg_print_errors(stdout, a_end, MULTIMARKDOWN_NAME);
		printf("Try '%s --help' for more information.\n", binname);
		exitcode = 1;
		goto exit2;
	}

	// '--version' also takes precedence
	if (a_version->count > 0) {
		printf("\nMultiMarkdown 6 v%s\n", MULTIMARKDOWN_VERSION);
		printf("%s\n\n", MULTIMARKDOWN_COPYRIGHT);
		printf("%s\n", MULTIMARKDOWN_LICENSE);
		printf("\n");
		goto exit2;
	}


	// Parse options
	unsigned long extensions = EXT_SMART | EXT_NOTES | EXT_CRITIC | EXT_TRANSCLUDE;

	if (a_compatibility->count > 0) {
		// Compatibility mode disables certain features
		// Reset extensions
		extensions = EXT_COMPATIBILITY | EXT_NO_LABELS | EXT_OBFUSCATE | EXT_NO_METADATA;
	}

	if (a_nosmart->count > 0) {
		// Disable smart typography
		extensions &= ~EXT_SMART;
	}

	if (a_nolabels->count > 0) {
		// Disable header id attributes
		extensions |= EXT_NO_LABELS;
	}

	if (a_notransclude->count > 0) {
		// Disable file transclusion
		extensions &= ~EXT_TRANSCLUDE;
	}

	if (a_opml->count > 0) {
		// Attempt to convert from OPML
		extensions |= EXT_PARSE_OPML;
	} else if (a_itmz->count > 0) {
		// Attempt to convert from ITMZ
		extensions |=  EXT_PARSE_ITMZ;
	}

	if (a_accept->count > 0) {
		// Accept CriticMarkup changes
		extensions |= EXT_CRITIC_ACCEPT | EXT_CRITIC;
	}

	if (a_reject->count > 0) {
		// Reject CriticMarkup changes
		extensions |= EXT_CRITIC_REJECT | EXT_CRITIC;
	}

	if (a_reject->count && a_accept->count) {
		// Old options that don't apply now, so change them
		extensions &= ~(EXT_CRITIC_REJECT | EXT_CRITIC_ACCEPT);
	}

	if (a_full->count > 0) {
		// Force complete document
		extensions |= EXT_COMPLETE;
	}

	if (a_snippet->count > 0) {
		// Force snippet
		extensions |= EXT_SNIPPET;
	}

	if (a_random->count > 0) {
		// Use random anchors
		extensions |= EXT_RANDOM_FOOT;
	}

	if (a_unique->count > 0) {
		// Use random header labels
		extensions |= EXT_RANDOM_LABELS;
	}

	if (a_format->count > 0) {
		if (strcmp(a_format->sval[0], "html") == 0) {
			format = FORMAT_HTML;
		} else if (strcmp(a_format->sval[0], "latex") == 0) {
			format = FORMAT_LATEX;
		} else if (strcmp(a_format->sval[0], "beamer") == 0) {
			format = FORMAT_BEAMER;
		} else if (strcmp(a_format->sval[0], "memoir") == 0) {
			format = FORMAT_MEMOIR;
		} else if (strcmp(a_format->sval[0], "mmd") == 0) {
			format = FORMAT_MMD;
		} else if (strcmp(a_format->sval[0], "odt") == 0) {
			format = FORMAT_ODT;
		} else if (strcmp(a_format->sval[0], "fodt") == 0) {
			format = FORMAT_FODT;
		} else if (strcmp(a_format->sval[0], "epub") == 0) {
			format = FORMAT_EPUB;
		} else if (strcmp(a_format->sval[0], "bundle") == 0) {
			format = FORMAT_TEXTBUNDLE;
		} else if (strcmp(a_format->sval[0], "bundlezip") == 0) {
			format = FORMAT_TEXTBUNDLE_COMPRESSED;
		} else if (strcmp(a_format->sval[0], "opml") == 0) {
			format = FORMAT_OPML;
		} else if (strcmp(a_format->sval[0], "itmz") == 0) {
			format = FORMAT_ITMZ;
		} else {
			// No valid format found
			fprintf(stderr, "%s: Unknown output format '%s'\n", binname, a_format->sval[0]);
			exitcode = 1;
			goto exit2;
		}
	}

	if (a_lang->count > 0) {
		language = LANG_FROM_STR(a_lang->sval[0]);
	}

	// Determine input
	if (a_file->count == 0) {
		// Read from stdin
	} else {
		// Read from files
	}

	DString * buffer = NULL;
	DString * result = NULL;
	char * char_result = NULL;
	FILE * output_stream;
	char * output_filename;

	// Increment counter and prepare token pool
#ifdef kUseObjectPool
	token_pool_init();
#endif

	// Seed random numbers
	custom_seed_rand();

	// Determine processing mode -- batch/stdin/files??

	if ((a_batch->count) && (a_file->count)) {
		// Batch process 1 or more files
		for (int i = 0; i < a_file->count; ++i) {

			buffer = scan_file(a_file->filename[i]);

			if (buffer == NULL) {
				fprintf(stderr, "Error reading file '%s'\n", a_file->filename[i]);
				exitcode = 1;
				goto exit2;
			}

			// Append output file extension
			switch (format) {
				case FORMAT_HTML:
					output_filename = filename_with_extension(a_file->filename[i], ".html");
					break;

				case FORMAT_LATEX:
				case FORMAT_BEAMER:
				case FORMAT_MEMOIR:
					output_filename = filename_with_extension(a_file->filename[i], ".tex");
					break;

				case FORMAT_FODT:
					output_filename = filename_with_extension(a_file->filename[i], ".fodt");
					break;

				case FORMAT_ODT:
					output_filename = filename_with_extension(a_file->filename[i], ".odt");
					break;

				case FORMAT_MMD:
					output_filename = filename_with_extension(a_file->filename[i], ".mmdtext");
					break;

				case FORMAT_EPUB:
					output_filename = filename_with_extension(a_file->filename[i], ".epub");
					break;

				case FORMAT_TEXTBUNDLE:
					output_filename = filename_with_extension(a_file->filename[i], ".textbundle");
					break;

				case FORMAT_TEXTBUNDLE_COMPRESSED:
					output_filename = filename_with_extension(a_file->filename[i], ".textpack");
					break;

				case FORMAT_OPML:
					output_filename = filename_with_extension(a_file->filename[i], ".opml");
					break;

				case FORMAT_ITMZ:
					output_filename = filename_with_extension(a_file->filename[i], ".itmz");
					break;
			}

			// Perform transclusion(s)
			char * folder = dirname((char *) a_file->filename[i]);

			if (!(extensions & EXT_COMPATIBILITY)) {
				mmd_prepend_mmd_header(buffer);
				mmd_append_mmd_footer(buffer);
			}

			if (extensions & EXT_TRANSCLUDE) {
				mmd_transclude_source(buffer, folder, a_file->filename[i], format, NULL, NULL);

				// Don't free folder -- owned by dirname
			}

			// Perform block level CriticMarkup?
			if (extensions & EXT_CRITIC_ACCEPT) {
				mmd_critic_markup_accept(buffer);
			}

			if (extensions & EXT_CRITIC_REJECT) {
				mmd_critic_markup_reject(buffer);
			}

			// Increment counter and prepare token pool
#ifdef kUseObjectPool
			token_pool_init();
#endif

			if (a_meta->count > 0) {
				// List metadata keys
				char_result = mmd_string_metadata_keys(buffer->str);

				if (char_result) {
					fputs(char_result, stdout);

					free(char_result);
				}
			} else if (a_extract->count > 0) {
				// Extract metadata key
				const char * query = a_extract->sval[0];

				char_result = mmd_string_metavalue_for_key(buffer->str, query);

				if (char_result) {
					fputs(char_result, stdout);
					fputc('\n', stdout);

					free(char_result);
				}
			} else {
				// Regular processing

				result = mmd_d_string_convert_to_data(buffer, extensions, format, language, folder);

				if (FORMAT_TEXTBUNDLE == format) {
					unzip_data_to_path(result->str, result->currentStringLength, output_filename);
				} else {
					if (!(output_stream = fopen(output_filename, "wb"))) {
						// Failed to open file
						perror(output_filename);
					} else {
						fwrite(result->str, result->currentStringLength, 1, output_stream);
						fclose(output_stream);
					}
				}

				d_string_free(result, true);
			}

			d_string_free(buffer, true);
			free(output_filename);

			// Decrement counter and drain
#ifdef kUseObjectPool
			token_pool_drain();
#endif
		}
	} else {
		if (a_file->count) {
			// We have files to process
			buffer = d_string_new("");
			DString * file_buffer;

			// Concatenate all input files
			for (int i = 0; i < a_file->count; ++i) {
				file_buffer = scan_file(a_file->filename[i]);

				if (file_buffer == NULL) {
					fprintf(stderr, "Error reading file '%s'\n", a_file->filename[i]);
					exitcode = 1;
					goto exit;
				}

				d_string_append_c_array(buffer, file_buffer->str, file_buffer->currentStringLength);
				d_string_free(file_buffer, true);
			}
		} else {
			// Obtain input from stdin
			buffer = stdin_buffer();
		}

		char * folder = NULL;

		if (!(extensions & EXT_COMPATIBILITY)) {
			mmd_prepend_mmd_header(buffer);
			mmd_append_mmd_footer(buffer);
		}

		if ((extensions & EXT_TRANSCLUDE) && (a_file->count == 1)) {
			// Perform transclusion(s)

			// Convert to absolute path for first file to enable proper path resolution
#ifdef PATH_MAX
			// If PATH_MAX defined, use it
			char absolute[PATH_MAX + 1];
			realpath(a_file->filename[0], absolute);
			folder = dirname((char *) a_file->filename[0]);

			mmd_transclude_source(buffer, folder, absolute, format, NULL, NULL);
#else
			// If undefined, then we *should* be able to use a NULL pointer to allocate
			char * absolute = realpath(a_file->filename[0], NULL);
			folder = dirname((char *) a_file->filename[0]);
			mmd_transclude_source(buffer, folder, absolute, format, NULL, NULL);
			free(absolute);
#endif
			// Don't free folder -- owned by dirname
		}

		if (a_file->count == 1) {
			// Must do this after realpath, b/c on some OS's (e.g. Travis-CI linux)
			// this truncates a_file->filename[0]
			folder = dirname((char *) a_file->filename[0]);
		}

		// Perform block level CriticMarkup?
		if (extensions & EXT_CRITIC_ACCEPT) {
			mmd_critic_markup_accept(buffer);
		}

		if (extensions & EXT_CRITIC_REJECT) {
			mmd_critic_markup_reject(buffer);
		}

		if (a_meta->count > 0) {
			// List metadata keys
			char_result = mmd_string_metadata_keys(buffer->str);

			if (char_result) {
				fputs(char_result, stdout);

				free(char_result);
			}
		} else if (a_extract->count > 0) {
			// Extract metadata key
			const char * query = a_extract->sval[0];

			char_result = mmd_string_metavalue_for_key(buffer->str, query);

			if (char_result) {
				fputs(char_result, stdout);
				fputc('\n', stdout);

				free(char_result);
			}
		} else {
			// Regular processing

			result = mmd_d_string_convert_to_data(buffer, extensions, format, language, folder);

			// Where does output go?
			if (strcmp(a_o->filename[0], "-") == 0) {
				// direct to stdout
				output_stream = stdout;
			} else if (!(output_stream = fopen(a_o->filename[0], "wb"))) {
				perror(a_o->filename[0]);
				free(result);
				d_string_free(buffer, true);

				exitcode = 1;
				goto exit;
			}

			fwrite(result->str, result->currentStringLength, 1, output_stream);

			if (output_stream != stdout) {
				fclose(output_stream);
			}

			d_string_free(result, true);
		}

		d_string_free(buffer, true);
	}


exit:

#ifdef kUseObjectPool
	// Decrement counter and clean up token pool
	token_pool_drain();

	token_pool_free();
#endif

exit2:

	// Clean up after argtable
	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	return exitcode;
}

