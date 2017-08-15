/**

	MultiMarkdown 6 -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file stack.c

	@brief Create a dynamic array that stores pointers in a LIFO order.


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

#include <stdlib.h>
#include <string.h>

#include "stack.h"

#define kStackStartingSize 64


/// Create a new stack with dynamic storage with an
/// initial capacity (0 to use default capacity)
stack * stack_new(int startingSize) {
	stack * s = malloc(sizeof(stack));

	if (s) {
		if (startingSize <= 0) {
			startingSize = kStackStartingSize;
		}

		s->element = malloc(sizeof(void *) * startingSize);

		if (!s->element) {
			free(s);
			return NULL;
		}

		s->size = 0;
		s->capacity = startingSize;
	}

	return s;
}


/// Free the stack
void stack_free(stack * s) {
	free(s->element);
	free(s);
}


/// Add a new pointer to the stack
void stack_push(stack * s, void * element) {
	if (s->size == s->capacity) {
		s->capacity *= 2;
		s->element = realloc(s->element, s->capacity * sizeof(void *));
	}

	s->element[s->size++] = element;
}


/// Pop the top item off the stack
void * stack_pop(stack * s) {
	void * last = stack_peek(s);

	if (s->size != 0) {
		s->size--;
	}

	return last;
}


/// Peek at the top item on the stack
void * stack_peek(stack * s) {
	if (s->size == 0) {
		return NULL;
	}

	return s->element[(s->size) - 1];
}


/// Peek at a specific index in the stack
void * stack_peek_index(stack * s, size_t index) {
	if (index >= s->size) {
		return NULL;
	}

	return s->element[index];
}


/// Sort array using specified compare_function
void stack_sort(stack * s, void * compare_function) {
	qsort(s->element, s->size, sizeof(void *), compare_function);
}

