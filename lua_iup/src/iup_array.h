/** \file
 * \brief Simple expandable array
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUP_ARRAY_H 
#define __IUP_ARRAY_H

#ifdef __cplusplus
extern "C"
{
#endif

/** \defgroup iarray Simple Array
 * \par
 * Expandable array using a simple pointer.
 * \par
 * See \ref iup_array.h
 * \ingroup util */

typedef struct _Iarray Iarray;

/** Creates an array with an initial room for elements, and the element size.
 * The array count starts at 0. And the maximum number of elements starts at the given count.
 * The maximum number of elements is increased by the start count, every time it needs more memory.
 * Must call \ref iupArrayInc to proper allocates memory.
 * \ingroup iarray */
Iarray* iupArrayCreate(int start_count, int elem_size);

/** Destroys the array.
 * \ingroup iarray */
void iupArrayDestroy(Iarray* iarray);

/** Returns the pointer that contains the array.
 * \ingroup iarray */
void* iupArrayGetData(Iarray* iarray);

/** Increments the number of elements in the array.
 * The array count starts at 0. 
 * If the maximum number of elements is reached, the memory allocated is increased by the initial start count.
 * Returns the pointer that contains the array.
 * \ingroup iarray */
void* iupArrayInc(Iarray* iarray);

/** Increments the number of elements in the array by a given count.
 * The array count starts at 0. 
 * If the maximum number of elements is reached, the memory allocated is increased by the given count.
 * Returns the pointer that contains the array.
 * \ingroup iarray */
void* iupArrayAdd(Iarray* iarray, int new_count);

/** Returns the actual number of elements in the array.
 * \ingroup iarray */
int iupArrayCount(Iarray* iarray);



#ifdef __cplusplus
}
#endif

#endif
