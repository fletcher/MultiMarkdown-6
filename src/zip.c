/**

	MultiMarkdown -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file zip.c

	@brief Common routines for zip-based file formats


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

#include "d_string.h"
#include "zip.h"

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>


// Windows deprecated mkdir()
// Fix per internet searches and modified by @f8ttyc8t (<https://github.com/f8ttyc8t>)
#if (defined(_WIN32) || defined(__WIN32__))
	// Let compiler know where to find _mkdir()
	#include  <direct.h>
	#define mkdir(A, B) _mkdir(A)
#endif


// Create new zip archive
void zip_new_archive(mz_zip_archive * pZip) {
	memset(pZip, 0, sizeof(mz_zip_archive));

	mz_bool status;

	status = mz_zip_writer_init_heap(pZip, 0, 0);

	if (!status) {
		fprintf(stderr, "mz_zip_writer_init_heap() failed.\n");
	}
}


// Unzip archive to specified file path
mz_bool unzip_archive_to_path(mz_zip_archive * pZip, const char * path) {
	// Ensure folder 'path' exists

	DIR * dir = opendir(path);
	mz_bool status;

	if (!dir) {
		// path is not an existing directory

		if (access(path, F_OK) == 0) {
			// path is an existing file
			fprintf(stderr, "'%s' is an existing file.\n", path);
			return -1;
		} else {
			// path doesn't exist - create directory
			mkdir(path, 0755);
		}
	}

	dir = opendir(path);

	if (dir) {
		// Directory 'path' exists

		// Remember current working directory
		// Apparently PATH_MAX doesn't actually mean anything, so pick a big number
		char cwd[4096 + 1];
		getcwd(cwd, sizeof(cwd));

		// Move into the desired directory
		chdir(path);

		int file_count = mz_zip_reader_get_num_files(pZip);

		mz_zip_archive_file_stat pStat;

		for (int i = 0; i < file_count; ++i) {
			mz_zip_reader_file_stat(pZip, i, &pStat);

			if (pStat.m_is_directory) {
				// Create the directory
				mkdir(pStat.m_filename, 0755);
			} else {
				status = mz_zip_reader_extract_to_file(pZip, i, pStat.m_filename, 0);

				if (!status) {
					fprintf(stderr, "Error extracting file from zip archive.\n");
					return status;
				}
			}
		}

		// Return to prior working directory
		chdir(cwd);
	}


	return 0;
}


// Unzip archive (as plain binary data) to specified file path
mz_bool unzip_data_to_path(const void * data, size_t size, const char * path) {
	mz_zip_archive * pZip = malloc(sizeof(mz_zip_archive));
	memset(pZip, 0, sizeof(mz_zip_archive));

	mz_bool status = mz_zip_reader_init_mem(pZip, data, size, 0);

	if (!status) {
		fprintf(stderr, "mz_zip_reader_init_mem() failed.\n");
		mz_zip_reader_end(pZip);
		return status;
	}

	status = unzip_archive_to_path(pZip, path);
	mz_zip_reader_end(pZip);
	free(pZip);
	return status;
}


// Extract single file from archive
mz_bool unzip_file_from_archive(mz_zip_archive * pZip, const char * filename, DString * destination) {
	mz_uint32 index;
	mz_zip_archive_file_stat pStat;
	mz_bool status;

	int result = mz_zip_reader_locate_file_v2(pZip, filename, NULL, 0, &index);

	if (result == -1) {
		// Failed
		return 0;
	}

	mz_zip_reader_file_stat(pZip, index, &pStat);
	unsigned long long size = pStat.m_uncomp_size + 1;				// Allow for null terminator in case this is text

	if (destination->currentStringBufferSize < size) {
		// Buffer to small
		free(destination->str);
		destination->str = malloc((unsigned long)size);
		destination->currentStringBufferSize = (size_t)size;
	}

	status = mz_zip_reader_extract_to_mem(pZip, index, destination->str, destination->currentStringBufferSize, 0);
	destination->currentStringLength = (size_t)size - 1;
	destination->str[destination->currentStringLength] = '\0';

	if (!status) {
		fprintf(stderr, "mz_zip_reader_extract_to_mem() failed.\n");
	}

	return status;
}


// Extract single file from archive
mz_bool unzip_file_from_data(const void * data, size_t size, const char * filename, DString * destination) {
	mz_zip_archive * pZip = malloc(sizeof(mz_zip_archive));
	memset(pZip, 0, sizeof(mz_zip_archive));

	mz_bool status = mz_zip_reader_init_mem(pZip, data, size, 0);

	if (!status) {
		fprintf(stderr, "mz_zip_reader_init_mem() failed.\n");
		mz_zip_reader_end(pZip);
		return status;
	}

	status =  mz_zip_validate_archive(pZip, 0);

	if (!status) {
		fprintf(stderr, "mz_zip_validate_archive failed.\n");
		mz_zip_reader_end(pZip);
		return status;
	}

	status = unzip_file_from_archive(pZip, filename, destination);
	mz_zip_reader_end(pZip);
	free(pZip);
	return status;
}
