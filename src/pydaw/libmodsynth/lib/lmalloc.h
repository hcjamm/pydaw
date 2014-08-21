/*
This file is part of the PyDAW project, Copyright PyDAW Team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef LMALLOC_H
#define	LMALLOC_H

#include <malloc.h>
#include <stdlib.h>
#include <hugetlbfs.h>

#ifdef	__cplusplus
extern "C" {
#endif


void lmalloc(void**, size_t);

#ifdef	__cplusplus
}
#endif

/* void * lmalloc(size_t a_size)
 *
 * Custom memory allocator
 */
void lmalloc(void ** a_ptr, size_t a_size)
{
    assert(posix_memalign(a_ptr, 16, a_size) == 0);
}

#endif	/* LMALLOC_H */

