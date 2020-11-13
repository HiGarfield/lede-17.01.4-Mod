#ifndef ARRAY_H_INCLUDED
#define ARRAY_H_INCLUDED

#include "common.h"

typedef struct _Array{

	/* All elements reside here */
	char		*Data;

	/* Length of one element */
	int32_t		DataLength;

	/* How many elements are there in this array. Always non-negative. */
	int32_t		Used;

	/* How many rooms (one room one element) have been allocated.
	 *   An Array, normally, grows up (towards high address). In this case,
	 * `Allocated' is non-negative.
	 *   Typically, `Allocated' will never be less than `Used'. In most cases,
	 * `Allocated' is greater than `Used'. If they are equal, this means there
	 * is no left room for a new element, then we must allocate more
	 * space (realloc) just before adding new elements. The total allocated
	 * space is equal to `Allocated' times `DataLength'.
	 *
	 *   If `Allocated' is negative, the array grows down (towarding lower address,
	 * like stack) and won't allocate new space. One using this should guarantee
	 * there is enough space to hold all elements.
	 */
	int32_t	Allocated;
}Array;

int Array_Init(	__in Array *a,
				__in int DataLength,
				__in int InitialCount,
				__in BOOL GrowsDown,
				__in void *TheFirstAddress);
/* Description:
 *  Initialize an Array.
 * Parameters:
 *  a              	: The Array to be initialized.
 *  DataLength     	: The length of one element.
 *  InitialCount   	: The number of initial allocated rooms.
 *                     	If the array grows down, this parameter will be ignored.
 *  GrowsDown      	: Whether it grows down.
 *  TheFirstAddress : (Only) For growing down array, the address of the first
 *                    element (at the highist address). The address is the head
 *                    address of the element, not the tail address.
 *                      This parameter will be ignored for growing up array.
 * Return value:
 *  0 on success, a non-zero value otherwise.
 */

#define Array_Init_Static(DataLengrh)	{NULL, (DataLengrh), 0, 0}

#define Array_IsEmpty(a_ptr)	(((a_ptr)->Used) == 0)
/* Description:
 *  Check if an Array is empty.
 * Parameters:
 *  a_ptr : Pointer to an Array to be checked.
 */

#define Array_GetUsed(a_ptr)	((a_ptr)->Used)
/* Description:
 *  Get `Used' of an Array.
 * Parameters:
 *  a_ptr : Pointer to an Array to be gotten.
 */

#define Array_GetDataLength(a_ptr)	((a_ptr)->DataLength)

#define Array_GetRawArray(a_ptr)	((a_ptr)->Data)

void *Array_GetBySubscript(__in const Array *a, __in int Subscript);
/* Description:
 *  Get an element by its subscript.
 * Parameters:
 *  a         : The Array to be gotten from.
 *  Subscript : The subscript of the element which will be gotten.
 *                For growing down array, the subscript is still non-negative.
 *              An element at higher address has a smaller subscript while an
 *              element at lower address has a bigger subscript.
 * Return value:
 *  The address of the gotten element. NULL on failure.
 */

void *Array_GetThis(__in Array *a, __in const void *Position);

void *Array_GetNext(__in Array *a, __in const void *Position);

int Array_PushBack(__in Array *a, __in_opt const void *Data, __in_opt void *Boundary);
/* Description:
 *  Add an element to the end of an array. For growing down array, end means the
 *  element at the lowest address in an array.
 * Parameters:
 *  a        : The Array to be added in.
 *  Data     : Data of newly added element. If this parameter is NULL, the newly
 *             added element only occupies a room without any useful data, we
 *             call this kind of element void element.
 *  Boundary : For growing down array, the upper boundary of the array. If there
 *             is no enough space inside the boundary, no new element will be
 *             added, and the function will return failure. If this parameter is
 *             NULL, no bounds checking will be done.
 *               This parameter will be ignored for growing up array.
 * Return value:
 *  The subscript of the newly added element. Or a negative value on failure.
 */

void *Array_SetToSubscript(__in Array *a, __in int Subscript, __in const void *Data);
/* Description:
 *  Set data of a element which has the subscript.
 *    There are two cases.
 *  Case 1 : `Subscript' is less than a->Used
 *    Simply set the data of a element which has the subscript.
 *  Case 2 : `Subscript' is greater than or equal to a->Used
 *    Add some new void elements so as to let `Subscript' be less than a->Used,
 *  then, set the data of a element which has the subscript.
 * Parameters:
 *  a         : The Array of which an element's data is to be set.
 *  Subscript : Subscript of the element to be operated.
 *  Data      : Data to be set to.
 * Return value:
 *  The address of the operated element. Or NULL on failure.
 */

void Array_Sort(Array *a, int (*Compare)(const void *, const void *));

#define Array_Clear(a_ptr)	((a_ptr)->Used = 0)
/* Description:
 *  Remove all elements, but their rooms are still there.
 * Parameters:
 *  a_ptr : Pointer to the Array to be cleared.
 */

void Array_Fill(Array *a, int Num, const void *DataSample);

void Array_Free(__in Array *a);
/* Description:
 *  Free an initialized Array.
 * Parameters:
 *  a : The Array to be freed.
 */

#endif // ARRAY_H_INCLUDED
