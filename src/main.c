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
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "argtable3.h"
#include "d_string.h"
#include "i18n.h"
#include "libMultiMarkdown.h"
#include "html.h"
#include "mmd.h"
#include "token.h"
#include "transclude.h"
#include "version.h"

#define kBUFFERSIZE 4096	// How many bytes to read at a time

// argtable structs
struct arg_lit *a_help, *a_version, *a_compatibility, *a_nolabels, *a_batch, *a_accept, *a_reject, *a_full, *a_snippet;
struct arg_str *a_format, *a_lang;
struct arg_file *a_file, *a_o;
struct arg_end *a_end;
struct arg_rem *a_rem1, *a_rem2, *a_rem3, *a_rem4;


DString * stdin_buffer() {
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


static DString * scan_file(const char * fname) {
	/* Read from a file and return a GString *
		`buffer` will need to be freed elsewhere */

	char chunk[kBUFFERSIZE];
	size_t bytes;

	FILE * file;

	if ((file = fopen(fname, "r")) == NULL ) {
		return NULL;
	}

	DString * buffer = d_string_new("");

	while ((bytes = fread(chunk, 1, kBUFFERSIZE, file)) > 0) {
		d_string_append_c_array(buffer, chunk, bytes);
	}

	fclose(file);

	return buffer;
}


/// Given a filename, remove the extension and replace it with a new one.
/// The next extension must include the leading '.', e.g. '.html'
char * filename_with_extension(const char * original, const char * new_extension) {
	char * name_no_ext;
	DString * new_name;

	// Determine output filename without file extension
	name_no_ext = strdup(original);

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


char * mmd_process(DString * buffer, unsigned long extensions, short format, short language) {
	char * result;

	mmd_engine * e = mmd_engine_create_with_dstring(buffer, extensions);

	mmd_engine_set_language(e, language);

	mmd_engine_parse_string(e);

	DString * output = d_string_new("");

	mmd_export_token_tree(output, e, format);

	result = output->str;

	mmd_engine_free(e, false);
	d_string_free(output, false);

	return result;
}


int main(int argc, char** argv) {
	int exitcode = EXIT_SUCCESS;
	char * binname = "multimarkdown";
	short format = FORMAT_HTML;
	short language = LC_EN;

	// Initialize argtable structs
	void *argtable[] = {
		a_help			= arg_lit0(NULL, "help", "display this help and exit"),
		a_version		= arg_lit0(NULL, "version", "display version info and exit"),

		a_rem1			= arg_rem("", ""),

		a_format		= arg_str0("t", "to", "FORMAT", "convert to FORMAT"),
		a_o				= arg_file0("o", "output", "FILE", "send output to FILE"),

		a_batch			= arg_lit0("b", "batch", "process each file separately"),
		a_compatibility	= arg_lit0("c", "compatibility", "Markdown compatibility mode"),
		a_full			= arg_lit0("f", "full", "force a complete document"),
		a_snippet		= arg_lit0("s", "snippet", "force a snippet"),

		a_rem2			= arg_rem("", ""),

		a_accept		= arg_lit0("a", "accept", "accept all CriticMarkup changes"),
		a_reject		= arg_lit0("r", "reject", "reject all CriticMarkup changes"),

		a_rem3			= arg_rem("", ""),

		a_nolabels		= arg_lit0(NULL, "nolabels", "Disable id attributes for headers"),
		
		a_file 			= arg_filen(NULL, NULL, "<FILE>", 0, argc+2, "read input from file(s)"),

		a_rem4			= arg_rem("", ""),

		a_lang			= arg_str0("l", "lang", "LANG", "language localization, LANG = en|es|de"),
		a_end 			= arg_end(20),
	};

	// Set default options
	a_o->filename[0] = "-";		// Default to stdout if no option specified

	int nerrors = arg_parse(argc, argv, argtable);

	// '--help' takes precedence
	if (a_help->count > 0) {
		printf("\n%s v%s\n\n", MULTIMARKDOWN_6_NAME, MULTIMARKDOWN_6_VERSION);
		printf("\tUsage: %s", binname);
		arg_print_syntax(stdout, argtable, "\n\n");
		printf("Options:\n");
		arg_print_glossary(stdout, argtable, "\t%-25s %s\n");
		printf("\n");
		goto exit;
	}

	if (nerrors > 0) {
		// Report errors
		arg_print_errors(stdout, a_end, MULTIMARKDOWN_6_NAME);
		printf("Try '%s --help' for more information.\n", binname);
		exitcode = 1;
		goto exit;
	}

	// '--version' also takes precedence
	if (a_version->count > 0) {
		printf("\nMultiMarkdown 6 v%s\n", MULTIMARKDOWN_6_VERSION);
		printf("%s\n\n", MULTIMARKDOWN_6_COPYRIGHT);
		printf("%s\n", MULTIMARKDOWN_6_LICENSE);
		printf("\n");
		goto exit;
	}


	// Parse options
	unsigned long extensions = EXT_SMART | EXT_NOTES | EXT_CRITIC | EXT_TRANSCLUDE;

	if (a_compatibility->count > 0) {
		// Compatibility mode disables certain features
		// Reset extensions
		extensions = EXT_COMPATIBILITY | EXT_NO_LABELS | EXT_OBFUSCATE;
	}

	if (a_nolabels->count > 0) {
		// Disable header id attributes
		extensions |= EXT_NO_LABELS;
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

	if (a_format->count > 0) {
		if (strcmp(a_format->sval[0], "html") == 0)
			format = FORMAT_HTML;
		else if (strcmp(a_format->sval[0], "latex") == 0)
			format = FORMAT_LATEX;
		else {
			// No valid format found
			fprintf(stderr, "%s: Unknown output format '%s'\n", binname, a_format->sval[0]);
			exitcode = 1;
			goto exit;
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
	char * result;
	FILE * output_stream;
	char * output_filename;

	// Prepare token pool
#ifdef kUseObjectPool
	token_pool_init();
#endif

	// Determine processing mode -- batch/stdin/files??

	if ((a_batch->count) && (a_file->count)) {
		// Batch process 1 or more files
		for (int i = 0; i < a_file->count; ++i)
		{
			buffer = scan_file(a_file->filename[i]);

			if (buffer == NULL) {
				fprintf(stderr, "Error reading file '%s'\n", a_file->filename[i]);
				exitcode = 1;
				goto exit;
			}

			// Append output file extension
			switch (format) {
				case FORMAT_HTML:
					output_filename = filename_with_extension(a_file->filename[i], ".html");
					break;
				case FORMAT_LATEX:
					output_filename = filename_with_extension(a_file->filename[i], ".tex");
					break;
			}

			// Perform transclusion(s)
			if (extensions & EXT_TRANSCLUDE) {
				char * folder = dirname((char *) a_file->filename[i]);

				transclude_source(buffer, folder, format, NULL, NULL);
	
				free(folder);
			}
	
			result = mmd_process(buffer, extensions, format, language);

			if (!(output_stream = fopen(output_filename, "w"))) {
				// Failed to open file
				perror(output_filename);
			} else {
				fputs(result, output_stream);
				fputc('\n', output_stream);
				fclose(output_stream);
			}

			d_string_free(buffer, true);
			free(result);
			free(output_filename);
		}
	} else {
		if (a_file->count) {
			// We have files to process
			buffer = d_string_new("");
			DString * file_buffer;

			// Concatenate all input files
			for (int i = 0; i < a_file->count; ++i)
			{
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

		if ((extensions & EXT_TRANSCLUDE) && (a_file->count == 1)) {
			// Perform transclusion(s)
			char * folder = dirname((char *) a_file->filename[0]);

			transclude_source(buffer, folder, format, NULL, NULL);

			free(folder);
		}

		result = mmd_process(buffer, extensions, format, language);

		// Where does output go?
		if (strcmp(a_o->filename[0], "-") == 0) {
			// direct to stdout
			output_stream = stdout;
		} else if (!(output_stream = fopen(a_o->filename[0], "w"))) {
			perror(a_o->filename[0]);
			free(result);
			d_string_free(buffer, true);
	
			exitcode = 1;
			goto exit;
		}

		fputs(result, output_stream);
		fputc('\n', output_stream);
		
		if (output_stream != stdout)
			fclose(output_stream);
		
		d_string_free(buffer, true);

		free(result);
	}


exit:

	// Clean up token pool
#ifdef kUseObjectPool
	token_pool_free();
#endif

	// Clean up after argtable
	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	return exitcode;
}

