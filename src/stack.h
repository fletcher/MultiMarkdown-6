/**

	libCoreUtilities -- Reusable component libraries

	@file stack.h

	@brief Create a dynamic array that stores pointers in a LIFO order.


	@author	Fletcher T. Penney
	@bug

 **/

/*

	Copyright © 2016-2020 Fletcher T. Penney.


	MIT License

	Copyright (c) 2016-2020 Fletcher T. Penney

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.

*/


#ifndef STACK_SMART_STRING_H
#define STACK_SMART_STRING_H

#include <stdlib.h>

/// Structure for a stack
struct stack {
	size_t		size;				//!< Number of objects currently in stack
	size_t		capacity;			//!< Total current capacity for stack
	void 	**	element;			//!< Array of pointers to objects in stack
};

typedef struct stack stack;


/// Create a new stack with dynamic storage with an
/// initial capacity (0 to use default capacity)
stack * stack_new(
	int startingSize				//!< Default capacity for stack
);


/// Free the stack
void stack_free(
	stack * s						//!< Stack to be freed
);


/// Add a new pointer to the stack
void stack_push(
	stack * s,						//!< Stack to use
	void * element					//!< Pointer to push onto stack
);


/// Pop the top pointer off the stack and return it
void * stack_pop(
	stack * s						//!< Stack to examine
);


/// Peek at the top pointer on the stack (but don't remove it from stack)
void * stack_peek(
	stack * s						//!< Stack to examine
);


/// Peek at a specific index in the stack
void * stack_peek_index(
	stack * s,						//!< Stack to examine
	size_t index					//!< Index to peek at (0 is first pointer on stack)
);


/// Sort stack based on sort_function
void stack_sort(stack * s, void * compare_function);

#endif
