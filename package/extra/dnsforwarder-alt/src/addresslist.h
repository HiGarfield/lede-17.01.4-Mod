/* Address List
 *
 */

#ifndef ADDRESSLIST_H_INCLUDED
#define ADDRESSLIST_H_INCLUDED

#include "array.h"
#include "common.h"

typedef struct _AddressList {

	/* An array of `Address_Type' */
	Array		AddressList;

	/* The `Counter' is used by `AddressList_Advance' and `AddressList_GetOne',
	 * see them.
	 */
	uint32_t	Counter;

} AddressList;


int AddressList_Init(__in AddressList *a);
/* Description:
 *  Initialize an AddressList.
 * Parameters:
 *  a                : The AddressList to be initialized.
 * Return value:
 *  0 on success, a non-zero value otherwise.
 */

int AddressList_Add(__in	AddressList		*a,
					__in	Address_Type	*Addr);
/* Description:
 *  Add an address in the form of `Address_Type' to an AddressList.
 * Parameters:
 *  a      : The AddressList to be added in.
 *  Addr   : The added adress, which is a pointer to a `struct sockaddr_in'
 *           or `struct sockaddr_in6'.
 * Return value:
 *  0 on success, a non-zero value otherwise.
 */

sa_family_t AddressList_ConvertFromString(__out Address_Type    *Out,
                                          __in  const char      *Addr_Port,
                                          __in  int             DefaultPort
                                          );

int AddressList_Add_From_String(__in	AddressList	*a,
								__in	const char	*Addr_Port,
								__in	int			DefaultPort
								);
/* Description:
 *  Add an address in text to an AddressList.
 * Parameters:
 *  a           :  The AddressList to be added in.
 *  Addr_Port   : A string in the form of IP:Port, which will be interpreted
 *                to a typical address struct and added to the AddressList.
 *                  `Port' and the colon just before it can be omitted,
 *                in this case, the port will be assumed to be `DefaultPort'.
 *                  An IPv6 IP should be enclosed in square bracket,
 *                like [2001:a5::1], in order not to be confused with :Port.
 *                The full IPv6:Port is like [2001:a5::1]:80 .
 *  DefaultPort : The port used when `:Port' is absent.
 * Return value:
 *  0 on success, a non-zero value otherwise.
 */

int AddressList_Advance(__in AddressList *a);
/* Description:
 *  Increase a->Counter by 1 .
 * Return value:
 *  The a->Counter before it increased.
 */

struct sockaddr *AddressList_GetOneBySubscript(__in			AddressList	*a,
											   __out_opt	sa_family_t	*family,
											   __in			int			Subscript);

struct sockaddr *AddressList_GetOne(__in		AddressList	*a,
									__out_opt	sa_family_t	*family);
/* Description:
 *  Fetch an address from an AddressList. See the implementation for details.
 * Parameters:
 *  a      : The AddressList fetched from.
 *  family : A pointer to a `sa_family_t' which will be assigned to the family
 *           of the returned the address. This parameter can be NULL.
 * Return value:
 *  The pointer to the fetched address.
 */

#define AddressList_GetNumberOfAddresses(a_ptr)	( Array_GetUsed(&((a_ptr)->AddressList)) )

/* You should free the return value and *families when they are no longer needed. */
struct sockaddr **AddressList_GetPtrListOfFamily(AddressList *a, sa_family_t family);
struct sockaddr **AddressList_GetPtrList(AddressList *a, sa_family_t **families);

#define AddressList_Free(a_ptr)	(Array_Free(&((a_ptr)->AddressList)))
/* Description:
 *  Free an initialized AddressList.
 * Return value:
 *  Apparently.
 */

#endif // ADDRESSLIST_H_INCLUDED
