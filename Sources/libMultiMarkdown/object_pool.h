/**

	MultiMarkdown 6 -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file object_pool.h

	@brief Allocate memory for "objects" in large slabs, rather than one at a time. Improves
	performance when generating large numbers of small chunks of memory, as the expense of
	allocating memory in larger units.  Could cause difficulty in extreme low memory situations.


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


#ifndef OBJECT_POOL_MULTIMARKDOWN_H
#define OBJECT_POOL_MULTIMARKDOWN_H

#include "stack.h"


/// Structure for an object allocator pool
struct pool {
	stack *			allocated;		//!< Stack of pointers to slabs that have been allocated
	void *			next;			//!< Pointer to next available memory for allocation
	void *			last;			//!< Pointer to end of available memory
	short			object_size;	//!< Size of individual objects to be allocated

	char 			_PADDING[6];	//!< pad struct for alignment
};

typedef struct pool pool;


/// Allocate a new object pool
pool * pool_new(
	short size 						//!< How big are the objects to be allocated
);


/// Free object pool
void pool_free(
	pool * p						//!< Pool to be freed
);


/// Drain pool -- free slabs previously allocated
void pool_drain(
	pool * p						//!< Pool to be drained
);


/// Request memory for a new object from the pool
void * pool_allocate_object(
	pool * p						//!< Pool to be used for allocation
);


#endif
