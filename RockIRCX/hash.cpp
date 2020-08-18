/*
** hash.cpp
**
** Implementation for hashing routines, used for high-performance searching of
** strings
*/

#include "hash.h"

int	Hash_Create(HASH_TABLE_STRUCT *pHashTable, int size)
/*
** Hash_Create()
** Creates a new hash table with size specified
** Returns: < 0 on error
*/
{
	int count, hssize = sizeof(HASH_ENTRY_STRUCT*);

	pHashTable->size = size;

	/* Array of pointers has been allocated */
	pHashTable->HashEntry = (HASH_ENTRY_STRUCT**)calloc(size + 1,hssize);

	if (!pHashTable->HashEntry)
	/* Unable to allocate space for new table */
		return -1;

	/* Create hashtab for multiplication */
	pHashTable->hashtab = (unsigned int*)malloc(256 * sizeof(unsigned int));

	if (!pHashTable->hashtab)
	/* Unable to allocate space for new hashtab */
	{
		free(pHashTable->HashEntry);
		return -1;
	}
	for (count = 0; count < 256; count++)
		pHashTable->hashtab[count] = tolower((char)count) * 109;

	return 0;
}
int	Hash_Destroy(HASH_TABLE_STRUCT *pHashTable)
/*
** Destroys the target hash table & deallocates all memory used
** Returns: < 0 on error
*/
{
	int count;

	for (count = pHashTable->size - 1; count >= 0; count--)
	{
		if (pHashTable->HashEntry[count])
		/* Clear this bucket */
		{
			while (pHashTable->HashEntry[count]->chainhead.next)
				LL_Remove(&pHashTable->HashEntry[count]->chainhead,pHashTable->HashEntry[count]->chainhead.next->data);

			free(pHashTable->HashEntry[count]);
			pHashTable->HashEntry[count] = NULL;
		}
	}

	free(pHashTable->HashEntry);
	free(pHashTable->hashtab);

	pHashTable->HashEntry = NULL;
	pHashTable->hashtab = NULL;
	pHashTable->size = 0;

	return 0;
}

int Hash_Add(HASH_TABLE_STRUCT *pHashTable, const char *string, void *data)
/*
** Hash_Add()
** Adds an entry to the specified hash table, creating the key with the string specified
** Once the key is found, it adds the void* specified to the linked list
** Returns: < 0 on failure, or the index of the hash entry
*/
{
	int key = Hash_MakeKey(pHashTable,string);


	if (!pHashTable->HashEntry[key])
		pHashTable->HashEntry[key] = (HASH_ENTRY_STRUCT*)calloc(1,sizeof(HASH_ENTRY_STRUCT));

	if (!LL_Add(&pHashTable->HashEntry[key]->chainhead,data))
	/* Unable to add data to hash table */
		return -1;

	return key;
}
int Hash_Delete(HASH_TABLE_STRUCT *pHashTable, const char *string, void *data)
/*
** Hash_Delete()
** Deletes the specified data from the hash table
*/
{
	unsigned int key = Hash_MakeKey(pHashTable,string);

	if (pHashTable->HashEntry[key])
	/* Delete entry */
	{
		LL_Remove(&pHashTable->HashEntry[key]->chainhead,data);

		if (&pHashTable->HashEntry[key]->chainhead.next == NULL)
		/* Clear hash entry */
		{
			free(pHashTable->HashEntry[key]);
			pHashTable->HashEntry[key] = NULL;
		}
	}
	else
	/* The string does not exist in the table */
		return -1;

	return 0;
}
void Hash_Find(HASH_TABLE_STRUCT *pHashTable, HASH_FIND_STRUCT *pFindInfo, int hashkey)
/*
** Hash_Find()
** Searches the specified hash table using the information supplied in the find structure
** 7/13/04: Hashkey may be optionally supplied(HASH_NOKEY if none) to help save CPU cycles
*/
{
	int key;

	if (hashkey == HASH_NOKEY)
	/* Create key with string */
		key = Hash_MakeKey(pHashTable,pFindInfo->textkey);
	else
		key = hashkey;

	pFindInfo->found = NULL;
	pFindInfo->chainptr = NULL;

	pFindInfo->found = pHashTable->HashEntry[key];

	if (!pFindInfo->found)
		return;
	else if (!pFindInfo->found->chainhead.next)
	{
		pFindInfo->found = NULL;
		return;
	}

	if (pFindInfo->data)
	/* Data specified, return link in which data resides */
		pFindInfo->chainptr = LL_Find(&pHashTable->HashEntry[key]->chainhead,pFindInfo->data);
}

unsigned int Hash_MakeKey(HASH_TABLE_STRUCT *pHashTable, const char *string)
/*
** Hash_MakeKey()
** Makes a hash table key with the string specified, and returns it
*/
{
	unsigned char *name = (unsigned char*)string;
	unsigned char ch;
	unsigned int hash = 1;

	for (; (ch = *name); name++)
	{
		hash <<= 1;
		hash += pHashTable->hashtab[(int)ch];
	}

	hash %= pHashTable->size;
	return (hash);
}