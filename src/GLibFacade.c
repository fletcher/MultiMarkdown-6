/*
 *	GLibFacade.c
 *	MultiMarkdown
 *	
 *	Created by Daniel Jalkut on 7/26/11.
 *  Modified by Fletcher T. Penney 9/15/11 - 5/6/16.
 *		Changes Copyright 2011-2016
 *  Modified by Dan Lowe on 1/3/12.
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

#include "GLibFacade.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/*
 * The following section came from:
 *
 *	http://lists-archives.org/mingw-users/12649-asprintf-missing-vsnprintf-
 *		behaving-differently-and-_vsncprintf-undefined.html
 *
 * and
 *
 *	http://groups.google.com/group/jansson-users/browse_thread/thread/
 *		76a88d63d9519978/041a7d0570de2d48?lnk=raot
 */

/* Solaris and Windows do not provide vasprintf() or asprintf(). */
#if defined(__WIN32) || (defined(__SVR4) && defined(__sun))
int vasprintf( char **sptr, char *fmt, va_list argv ) 
{ 
    int wanted = vsnprintf( *sptr = NULL, 0, fmt, argv ); 
    if( (wanted > 0) && ((*sptr = malloc( 1 + wanted )) != NULL) ) 
        return vsprintf( *sptr, fmt, argv ); 
 
    return wanted; 
} 
 
int asprintf( char **sptr, char *fmt, ... ) 
{ 
    int retval; 
    va_list argv; 
    va_start( argv, fmt ); 
    retval = vasprintf( sptr, fmt, argv ); 
    va_end( argv ); 
    return retval; 
} 
#endif


/* GString */

#define kStringBufferStartingSize 1024
#define kStringBufferGrowthMultiplier 2

GString* g_string_new(const char *startingString)
{
	GString* newString = malloc(sizeof(GString));

	if (startingString == NULL) startingString = "";

	size_t startingBufferSize = kStringBufferStartingSize;
	size_t startingStringSize = strlen(startingString);
	while (startingBufferSize < (startingStringSize + 1))
	{
		startingBufferSize *= kStringBufferGrowthMultiplier;
	}
	
	newString->str = malloc(startingBufferSize);
	newString->currentStringBufferSize = startingBufferSize;
	strncpy(newString->str, startingString, startingStringSize);
	newString->str[startingStringSize] = '\0';
	newString->currentStringLength = startingStringSize;
	
	return newString;
}

char* g_string_free(GString* ripString, bool freeCharacterData)
{	
	if (ripString == NULL)
		return NULL;
	
	char* returnedString = ripString->str;
	if (freeCharacterData)
	{
		if (ripString->str != NULL)
		{
			free(ripString->str);
		}
		returnedString = NULL;
	}
	
	free(ripString);
	
	return returnedString;
}

static void ensureStringBufferCanHold(GString* baseString, size_t newStringSize)
{
	size_t newBufferSizeNeeded = newStringSize + 1;
	if (newBufferSizeNeeded > baseString->currentStringBufferSize)
	{
		size_t newBufferSize = baseString->currentStringBufferSize;	

		while (newBufferSizeNeeded > newBufferSize)
		{
			newBufferSize *= kStringBufferGrowthMultiplier;
		}
		
        char *temp;
        temp = realloc(baseString->str, newBufferSize);
        
        if (temp == NULL) {
            /* realloc failed */
            fprintf(stderr, "error reallocating memory\n");

            exit(1);
        }
		baseString->str = temp;
		baseString->currentStringBufferSize = newBufferSize;
	}
}

void g_string_append(GString* baseString, char* appendedString)
{
	if ((appendedString != NULL) && (strlen(appendedString) > 0))
	{
		size_t appendedStringLength = strlen(appendedString);
		size_t newStringLength = baseString->currentStringLength + appendedStringLength;
		ensureStringBufferCanHold(baseString, newStringLength);

		/* We already know where the current string ends, so pass that as the starting address for strncat */
		strncat(baseString->str + baseString->currentStringLength, appendedString, appendedStringLength);
		baseString->currentStringLength = newStringLength;
	}
}

void g_string_append_c(GString* baseString, char appendedCharacter)
{	
	size_t newSizeNeeded = baseString->currentStringLength + 1;
	ensureStringBufferCanHold(baseString, newSizeNeeded);
	
	baseString->str[baseString->currentStringLength] = appendedCharacter;
	baseString->currentStringLength++;	
	baseString->str[baseString->currentStringLength] = '\0';
}

void g_string_append_c_array(GString *baseString, char * appendedChars, size_t bytes)
{
	size_t newSizeNeeded = baseString->currentStringLength + bytes;
	ensureStringBufferCanHold(baseString, newSizeNeeded);

	memcpy(baseString->str + baseString->currentStringLength,appendedChars, bytes);

	baseString->currentStringLength = newSizeNeeded;
	baseString->str[baseString->currentStringLength] = '\0';
}

void g_string_append_printf(GString* baseString, char* format, ...)
{
	va_list args;
	va_start(args, format);
	
	char* formattedString = NULL;
	vasprintf(&formattedString, format, args);
	if (formattedString != NULL)
	{
		g_string_append(baseString, formattedString);
		free(formattedString);
	}
	va_end(args);
} 

void g_string_prepend(GString* baseString, char* prependedString)
{
	if ((prependedString != NULL) && (strlen(prependedString) > 0))
	{
		size_t prependedStringLength = strlen(prependedString);
		size_t newStringLength = baseString->currentStringLength + prependedStringLength;
		ensureStringBufferCanHold(baseString, newStringLength);

		memmove(baseString->str + prependedStringLength, baseString->str, baseString->currentStringLength);
		strncpy(baseString->str, prependedString, prependedStringLength);
		baseString->currentStringLength = newStringLength;
		baseString->str[baseString->currentStringLength] = '\0';
	}
}

void g_string_insert(GString* baseString, size_t pos, char * insertedString)
{
	if ((insertedString != NULL) && (strlen(insertedString) > 0))
	{
		if (pos > baseString->currentStringLength)
			pos = baseString->currentStringLength;
		
		size_t insertedStringLength = strlen(insertedString);
		size_t newStringLength = baseString->currentStringLength + insertedStringLength;
		ensureStringBufferCanHold(baseString, newStringLength);
		
		/* Shift following string to 'right' */
		memmove(baseString->str + pos + insertedStringLength, baseString->str + pos, baseString->currentStringLength - pos);
		strncpy(baseString->str + pos, insertedString, insertedStringLength);
		baseString->currentStringLength = newStringLength;
		baseString->str[baseString->currentStringLength] = '\0';
	}
}

void g_string_insert_c(GString* baseString, size_t pos, char insertedCharacter)
{	
	if (pos > baseString->currentStringLength)
		pos = baseString->currentStringLength;
	
	size_t newSizeNeeded = baseString->currentStringLength + 1;
	ensureStringBufferCanHold(baseString, newSizeNeeded);
	
	/* Shift following string to 'right' */
	memmove(baseString->str + pos + 1, baseString->str + pos, baseString->currentStringLength - pos);
	
	baseString->str[pos] = insertedCharacter;
	baseString->currentStringLength++;	
	baseString->str[baseString->currentStringLength] = '\0';
}


void g_string_insert_printf(GString* baseString, size_t pos, char* format, ...)
{
	va_list args;
	va_start(args, format);
	
	char* formattedString = NULL;
	vasprintf(&formattedString, format, args);
	if (formattedString != NULL)
	{
		g_string_insert(baseString, pos, formattedString);
		free(formattedString);
	}
	va_end(args);
}

void g_string_erase(GString* baseString, size_t pos, size_t len)
{
	if ((pos > baseString->currentStringLength) || (len <= 0))
		return;
	
	if ((pos + len) >= baseString->currentStringLength) 
		len = -1;
	
	if (len == -1) {
		baseString->currentStringLength = pos;
	} else {
		memmove(baseString->str + pos, baseString->str + pos + len, baseString->currentStringLength - pos - len);
		baseString->currentStringLength -= len;
	}
	baseString->str[baseString->currentStringLength] = '\0';
}

/* GSList */

void g_slist_free(GSList* ripList)
{
	GSList* thisListItem = ripList;
	while (thisListItem != NULL)
	{
		GSList* nextItem = thisListItem->next;
		
		/* I guess we don't release the data? Non-retained memory management is hard... let's figure it out later. */
		free(thisListItem);
		
		thisListItem = nextItem;
	}
}

/* Currently only used for markdown_output.c endnotes printing */
GSList* g_slist_reverse(GSList* theList)
{	
	GSList* lastNodeSeen = NULL;
	
	/* Iterate the list items, tacking them on to our new reversed List as we find them */
	GSList* listWalker = theList;
	while (listWalker != NULL)
	{
		GSList* nextNode = listWalker->next;
		listWalker->next = lastNodeSeen;
		lastNodeSeen = listWalker;
		listWalker = nextNode;
	}
	
	return lastNodeSeen;
}

GSList* g_slist_prepend(GSList* targetElement, void* newElementData)
{
	GSList* newElement = malloc(sizeof(GSList));
	newElement->data = newElementData;
	newElement->next = targetElement;
	return newElement;
}

