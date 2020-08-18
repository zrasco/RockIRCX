/*
** ll.h
**
** Contains header information for all linked list information & functions
*/

#ifndef __LL_H__
#define __LL_H__

#include <stdlib.h>
#include <crtdbg.h>

#define LINKED_LIST_STARTNODES 500

/* Generic linked list */
typedef struct LINKED_LIST_STRUCT
{
	void *data;									/* Any data type, just cast it */

	LINKED_LIST_STRUCT *next;					/* Pointer to next */
} LINKED_LIST_STRUCT;

/* Function prototypes */
LINKED_LIST_STRUCT			*LL_Add(LINKED_LIST_STRUCT *head, void *data);
LINKED_LIST_STRUCT			*LL_AddNoCheck(LINKED_LIST_STRUCT *head, void *data);
LINKED_LIST_STRUCT			*LL_Find(LINKED_LIST_STRUCT *head, void *data);
void						LL_Clear(LINKED_LIST_STRUCT *head);
void						LL_ClearAndFree(LINKED_LIST_STRUCT *head);
void						*LL_Remove(LINKED_LIST_STRUCT *head, void *data);

void						LL_Init();
void						LL_Destroy();
LINKED_LIST_STRUCT			*LLNode_Alloc();
void						LLNode_Free(LINKED_LIST_STRUCT *llTarget);
#endif		/* __LL_H__ */