/*
** ll.cpp
**
** Contains all routines for the generic linked list functions
*/

#include "ll.h"

int	g_nOnFreelist = 0;
int g_nInUse = 0;
LINKED_LIST_STRUCT llFreelist;

LINKED_LIST_STRUCT *LL_Add(LINKED_LIST_STRUCT *head, void *data)
/*
** LL_Add()
** Adds an entry to the linked list, returns NULL if unable to do so
*/
{
	LINKED_LIST_STRUCT *curr = head;

	while (curr->next)
	{
		curr = curr->next;

		if (curr->data == data)
			return NULL;
	}

	/* Create new entry and add it */
	curr->next = LLNode_Alloc();
	curr = curr->next;

	if (curr)
	{
		curr->data = data;
		curr->next = NULL;
	}
	else
	/* Unable to create new entry */
		return NULL;

	return curr;
}

LINKED_LIST_STRUCT *LL_AddNoCheck(LINKED_LIST_STRUCT *head, void *data)
/*
** LL_AddNoCheck()
** Adds an entry to the linked list, but does not care about duplicate data.
** Returns NULL if error
*/
{
	LINKED_LIST_STRUCT *curr = head;

	while (curr->next)
		curr = curr->next;

	/* Create new entry and add it */
	curr->next = LLNode_Alloc();
	curr = curr->next;

	if (curr)
	{
		curr->data = data;
		curr->next = NULL;
	}
	else
	/* Unable to malloc() new entry */
		return NULL;

	return curr;
}

LINKED_LIST_STRUCT *LL_Find(LINKED_LIST_STRUCT *head, void *data)
/*
** LL_Find()
** Searches through specified linked list and returns pointer to link w/data
*/
{
	LINKED_LIST_STRUCT *curr = head;

	while (curr->next)
	{
		curr = curr->next;

		if (curr->data == data)
			return curr;
	}

	return NULL;
}

void LL_Clear(LINKED_LIST_STRUCT *head)
/*
** LL_Clear()
** Clears and deallocates a linked list
*/
{
	LINKED_LIST_STRUCT *temp;

	while (head->next)
	/* Keep clearin 'em */
	{
		temp = head->next->next;
		LLNode_Free(head->next);
		head->next = temp;
	}
}
void LL_ClearAndFree(LINKED_LIST_STRUCT *head)
/*
** LL_ClearAndFree()
** Clears and deallocates a linked list, as well as deallocating the data fields in each list
*/
{
	LINKED_LIST_STRUCT *temp;

	while (head->next)
	/* Keep clearin 'em */
	{
		temp = head->next->next;
		LLNode_Free((LINKED_LIST_STRUCT*)head->next->data);
		LLNode_Free(head->next);
		head->next = temp;
	}
}

void *LL_Remove(LINKED_LIST_STRUCT *head, void *data)
/*
** LL_Remove()
** Removes an entry from the linked list, and returns pointer to data(for further cleanup)
*/
{
	LINKED_LIST_STRUCT *temp = head, *curr = head;
	void *retval;

	if (head)
	{
		while (curr->next)
		{
			curr = curr->next;

			if (curr->data == data)
			/* Match found */
			{
				retval = curr->data;
				temp->next = curr->next;
				LLNode_Free(curr);

				return retval;
			}

			temp = curr;
		}
	}

	return NULL;
}

void LL_Init()
{
	int nCount;
	LINKED_LIST_STRUCT *llPtr = &llFreelist;

	llFreelist.data = NULL;

	for (nCount = 0; nCount < LINKED_LIST_STARTNODES; nCount++)
	/* Create list nodes */
	{
		llPtr->next = (LINKED_LIST_STRUCT*)malloc(sizeof(LINKED_LIST_STRUCT));

		if (llPtr->next)
			llPtr = llPtr->next;

		g_nOnFreelist++;
		g_nInUse++;
	}

	llPtr->next = NULL;
}

void LL_Destroy()
{
	LINKED_LIST_STRUCT *llPtr;

	while (llFreelist.next)
	{
		llPtr = llFreelist.next;
		llFreelist.next = llFreelist.next->next;
		free(llPtr);
	}
}

LINKED_LIST_STRUCT *LLNode_Alloc()
/*
** LLNode_Alloc()
** Assigns new linked list node from free list
*/
{
	LINKED_LIST_STRUCT *llPtr = llFreelist.next;

	if (llPtr)
	/* Get from free list */
	{
		llFreelist.next = llFreelist.next->next;
		g_nOnFreelist--;
	}
	else
	/* Allocate new one */
		llPtr = (LINKED_LIST_STRUCT*)malloc(sizeof(LINKED_LIST_STRUCT));

	if (llPtr)
	{
		llPtr->data = NULL;
		llPtr->next = NULL;
	}

	return llPtr;
}

void LLNode_Free(LINKED_LIST_STRUCT *llTarget)
{
	LINKED_LIST_STRUCT *llPtr = llFreelist.next;

	llFreelist.next = llTarget;
	llTarget->next = llPtr;

	g_nOnFreelist++;
}