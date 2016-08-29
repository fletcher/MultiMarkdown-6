/*
 *	GLibFacade.h
 *	MultiMarkdown
 *	
 *	Created by Daniel Jalkut on 7/26/11.
 *	Modified by Fletcher T. Penney 9/15/11 - 5/6/16.
 *		Changes Copyright 2011-2016
 *	Modified by Dan Lowe on 1/3/12.
 *
 *	License for original code by Daniel Jalkut:
 *	
 *	Copyright 2011 Daniel Jalkut. All rights reserved.
 *	
 *	Permission is hereby granted, free of charge, to any person obtaining a copy of
 *	this software and associated documentation files (the “Software”), to deal in
 *	the Software without restriction, including without limitation the rights to
 *	use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 *	of the Software, and to permit persons to whom the Software is furnished to do
 *	so, subject to the following conditions:
 *	
 *	The above copyright notice and this permission notice shall be included in all
 *	copies or substantial portions of the Software.
 *	
 *	THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 */

#ifndef __MARKDOWN_GLIB_FACADE__
#define __MARKDOWN_GLIB_FACADE__

/* peg_markdown uses the link symbol for its own purposes */
#define link MARKDOWN_LINK_IGNORED
#include <unistd.h>
#undef link

#include <stdbool.h>
#include <ctype.h>

typedef int gboolean;
typedef char gchar;

/* This style of bool is used in shared source code */
#if !defined(FALSE)
#define FALSE false
#endif
#if !defined(TRUE)
#define TRUE true
#endif

/* WE implement minimal mirror implementations of GLib's GString and GSList 
 * sufficient to cover the functionality required by MultiMarkdown.
 *
 * NOTE: THese are 100% clean, from-scratch implementations using only the 
 * GLib function prototype as guide for behavior.
 */

typedef struct 
{	
	/* Current UTF8 byte stream this string represents */
	char* str;

	/* Where in the str buffer will we add new characters */
	/* or append new strings? */
	unsigned long currentStringBufferSize;
	unsigned long currentStringLength;
} GString;

GString* g_string_new(const char *startingString);
char* g_string_free(GString* ripString, bool freeCharacterData);

void g_string_append_c(GString* baseString, char appendedCharacter);
void g_string_append_c_array(GString *baseString, char * appendedChars, size_t bytes);
void g_string_append(GString* baseString, char *appendedString);

void g_string_prepend(GString* baseString, char* prependedString);

void g_string_append_printf(GString* baseString, char* format, ...);

void g_string_insert(GString* baseString, size_t pos, char * insertedString);
void g_string_insert_c(GString* baseString, size_t pos, char insertedCharacter);
void g_string_insert_printf(GString* baseString, size_t pos, char* format, ...);

void g_string_erase(GString* baseString, size_t pos, size_t len);

/* Just implement a very simple singly linked list. */

typedef struct _GSList
{
	void* data;	
	struct _GSList* next;
} GSList;

void g_slist_free(GSList* ripList);
GSList* g_slist_prepend(GSList* targetElement, void* newElementData);
GSList* g_slist_reverse(GSList* theList);

#endif
