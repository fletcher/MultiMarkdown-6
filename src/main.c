/*

	main.c -- Template main()

	Copyright © 2015 Fletcher T. Penney.


	This program is free software you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation either version 2 of the License, or
	(at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
	
*/

#include <stdio.h>
#include <stdlib.h>

#include "GLibFacade.h"

#define kBUFFERSIZE 4096	// How many bytes to read at a time

GString * stdin_buffer() {
	/* Read from stdin and return a GString *
		`buffer` will need to be freed elsewhere */

	char chunk[kBUFFERSIZE];
	size_t bytes;

	GString * buffer = g_string_new("");

    while ((bytes = fread(chunk, 1, kBUFFERSIZE, stdin)) > 0) {
    	g_string_append_c_array(buffer, chunk, bytes);
    }

	fclose(stdin);

	return buffer;
}

GString * scan_file(char * fname) {
	/* Read from stdin and return a GString *
		`buffer` will need to be freed elsewhere */

	char chunk[kBUFFERSIZE];
	size_t bytes;

	FILE * file;

	if ((file = fopen(fname, "r")) == NULL ) {
		return NULL;
	}

	GString * buffer = g_string_new("");

    while ((bytes = fread(chunk, 1, kBUFFERSIZE, file)) > 0) {
    	g_string_append_c_array(buffer, chunk, bytes);
    }

	fclose(file);

	return buffer;
}

int main( int argc, char** argv ) {
	/* Make your program do whatever you want */
}
