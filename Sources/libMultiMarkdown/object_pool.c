/**

	MultiMarkdown 6 -- Lightweight markup processor to produce HTML, LaTeX, and more.

	@file object_pool.c

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

#include <stdlib.h>

#include "object_pool.h"

#define kNumberOfObjects	1024


void pool_add_slab(pool * p) {
	void * slab = malloc(p->object_size * kNumberOfObjects);

	if (slab) {
		stack_push(p->allocated, slab);

		// Next object will come from beginning of this slab
		p->next = slab;

		// Set warning to trigger need for next slab
		p->last = slab + (p->object_size * (kNumberOfObjects));
	}
}


/// Allocate a new object pool
pool * pool_new(short size) {
	pool * p = malloc(sizeof(pool));

	if (p) {
		p->object_size = size;

		p->allocated = stack_new(1024);

		pool_add_slab(p);
	}

	return p;
}


/// Free object pool
void pool_free(pool * p) {
	if (p) {
		pool_drain(p);

		stack_free(p->allocated);

		free(p);
	}
}


/// Drain pool -- free slabs previously allocated
void pool_drain(pool * p) {
	if (p == NULL)
		return;

	void * slab;

	while (p->allocated->size > 0) {
		slab = stack_pop(p->allocated);
		free(slab);
	}

	p->next = NULL;
	p->last = NULL;
}


/// Request memory for a new object from the pool
void * pool_allocate_object(pool * p) {
	void * a = NULL;

	if (p->next == p->last) {
		pool_add_slab(p);
	}

	if (p->next < p->last) {
		a = p->next;

		p->next += (p->object_size);
	}

	return a;
}

