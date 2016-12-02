/**

	Smart String -- Library to abstract smart typing features from MMD Composer

	@file d_string.c

	@brief Dynamic string -- refactoring of old GLibFacade


	@author	Daniel Jalkut, modified by Fletcher T. Penney and Dan Lowe
	@bug	

**/

/*

	Copyright © 2011 Daniel Jalkut.
	Modifications by Fletcher T. Penney, Copyright © 2011-2016 Fletcher T. Penney.
	Modifications by Dan Lowe, Copyright © 2011 Dan Lowe.


	The `c-template` project is released under the MIT License.
	
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
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "d_string.h"


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


/* DString */

#define kStringBufferStartingSize 1024
#define kStringBufferGrowthMultiplier 2

DString* d_string_new(const char *startingString)
{
	DString* newString = malloc(sizeof(DString));

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

char* d_string_free(DString* ripString, bool freeCharacterData)
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

static void ensureStringBufferCanHold(DString* baseString, size_t newStringSize)
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

void d_string_append(DString* baseString, char* appendedString)
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

void d_string_append_c(DString* baseString, char appendedCharacter)
{	
	size_t newSizeNeeded = baseString->currentStringLength + 1;
	ensureStringBufferCanHold(baseString, newSizeNeeded);
	
	baseString->str[baseString->currentStringLength] = appendedCharacter;
	baseString->currentStringLength++;	
	baseString->str[baseString->currentStringLength] = '\0';
}

void d_string_append_c_array(DString *baseString, const char * appendedChars, size_t bytes)
{
	size_t newSizeNeeded = baseString->currentStringLength + bytes;
	ensureStringBufferCanHold(baseString, newSizeNeeded);

	memcpy(baseString->str + baseString->currentStringLength,appendedChars, bytes);

	baseString->currentStringLength = newSizeNeeded;
	baseString->str[baseString->currentStringLength] = '\0';
}

void d_string_append_printf(DString* baseString, char* format, ...)
{
	va_list args;
	va_start(args, format);
	
	char* formattedString = NULL;
	vasprintf(&formattedString, format, args);
	if (formattedString != NULL)
	{
		d_string_append(baseString, formattedString);
		free(formattedString);
	}
	va_end(args);
} 

void d_string_prepend(DString* baseString, char* prependedString)
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

void d_string_insert(DString* baseString, size_t pos, const char * insertedString)
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

void d_string_insert_c(DString* baseString, size_t pos, char insertedCharacter)
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


void d_string_insert_printf(DString* baseString, size_t pos, char* format, ...)
{
	va_list args;
	va_start(args, format);
	
	char* formattedString = NULL;
	vasprintf(&formattedString, format, args);
	if (formattedString != NULL)
	{
		d_string_insert(baseString, pos, formattedString);
		free(formattedString);
	}
	va_end(args);
}

void d_string_erase(DString* baseString, size_t pos, size_t len)
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
