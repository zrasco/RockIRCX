/*
** hash.h
**
** Contains all declarations and header information for hash table routines used
** by the RockIRCX server
**
** The hash keys are created by using the ascii characters in the specified strings,
** then the keys are mapped to array locations; the general idea is a fast search for
** nicknames.
** This hash table is entirely string-based...
*/

#ifndef __HASH_H__
#define __HASH_H__

#include <stdlib.h>
#include <ctype.h>
#include "ll.h"

/* Definitions */
#define HASH_NOKEY -1

typedef struct HASH_ENTRY_STRUCT
/* Hash table entry structure */
{
	LINKED_LIST_STRUCT	chainhead;				/* Beginning of chain(for collisions) */
} HASH_ENTRY_STRUCT;

typedef struct HASH_TABLE_STRUCT
/* Hash table structure */
{
	HASH_ENTRY_STRUCT	**HashEntry;				/* Array of entries */
	int					size;						/* Size of hash table */
	unsigned int		*hashtab;					/* Multiplication table */
} HASH_TABLE_STRUCT;

typedef struct HASH_FIND_STRUCT
/* Hash table work structure used for finds */
{
	/* Supplied: */
	const char				*textkey;			/* Text used for table key */
	void					*data;				/* Pointer to user-defined data, */
												/* NULL causes chainptr to = NULL as well */

	/* Returns: */
	LINKED_LIST_STRUCT		*chainptr;			/* Pointer to chain link w/ found data */
    HASH_ENTRY_STRUCT		*found;				/* NULL if not found */
} HASH_FIND_STRUCT;


/* Function prototypes */
int				Hash_Create(HASH_TABLE_STRUCT *pHashTable, int size);
int				Hash_Destroy(HASH_TABLE_STRUCT *pHashTable);
int				Hash_Add(HASH_TABLE_STRUCT *pHashTable, const char *string, void *data);
int				Hash_Delete(HASH_TABLE_STRUCT *pHashTable, const char *string, void *data);
void			Hash_Find(HASH_TABLE_STRUCT *pHashTable, HASH_FIND_STRUCT *pFindInfo, int hashkey);
unsigned int	Hash_MakeKey(HASH_TABLE_STRUCT *pHashTable, const char *string);

#endif		/* __HASH_H__ */

