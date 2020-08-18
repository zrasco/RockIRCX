
/*
** lookup.cpp
**
** Contains all routines for asynchronous hostname lookups
*/

#include "lookup.h"
#include "ll.h"

LINKED_LIST_STRUCT cache_head;

void Lookup_Init()
/*
** Lookup_Init()
** This function initializes the DNS cache, it MUST be called at the beginning
*/
{
	cache_head.data = NULL;
	cache_head.next = NULL;
}

int Lookup_Perform(unsigned int ip, CLIENT_STRUCT *client)
/*
** Lookup_Perform()
** The function will perform a hostname lookup with the specified IP address, it works
** like so:
**
** Step 1: The function is called from the server, with the client pending a lookup
** Step 2: The cache table is searched for the specified IP, and one of the following
** will occur:
**		a) The IP is found on a cache entry pending a lookup, so the client address
**			is added to the list of pending clients on the cache structure.
**			Return value: LOOKUP_PENDING
**		b) The IP is found on a cache entry with a completed lookup, the client's
**			hostname and status are changed
**			Return value: LOOKUP_COMPLETE
*/
{
	LINKED_LIST_STRUCT *seeker = &cache_head;
	LOOKUP_CACHE_STRUCT *lookup;

	while (seeker->next)
	/* Search through cache entries for a possible match of IP address */
	{
		seeker = seeker->next;

		lookup = (LOOKUP_CACHE_STRUCT*)seeker->data;

		if (lookup->ip == ip)
		/* Match for IP found in cache */
		{
			if (lookup->completed)
			/* This hostname is cached */
			{
				Client_SendToOne(client,FALSE,":%s NOTICE AUTH :*** Hostname lookup completed(cached)...",SettingsInfo.isGeneral.servername);
                strcpy(client->hostname,lookup->hostname);
				client->state = CLIENT_STATE_REGISTERING;

				return LOOKUP_COMPLETE;
			}
			else
			/* The lookup is pending a blocking call to gethostbyaddr() */
			{
				LL_Add(&lookup->chead,client);
				return LOOKUP_PENDING;
			}
		}
	}

	/* If we're here, the IP is not in the cache; 
	add a new entry and return pending status... */
	Lookup_AddToCache(ip,client);

	return LOOKUP_PENDING;
}

void Lookup_AddToCache(unsigned int ip, CLIENT_STRUCT *client)
/*
** Lookup_AddToCache()
** This function will add a lookup to the table of pending lookups, it works like so:
** 1) Add a cache structure to the linked list
** 2) Create a new thread to perform the lookup
*/
{
	LINKED_LIST_STRUCT *curr = &cache_head;
	LOOKUP_CACHE_STRUCT *lookup;

	while (curr->next)
	/* Make sure no matches are on the way */
	{
		curr = curr->next;

		if (((LOOKUP_CACHE_STRUCT*)(curr->data))->ip == ip)
			return;
	}
	
	lookup = (LOOKUP_CACHE_STRUCT*)calloc(1,sizeof(LOOKUP_CACHE_STRUCT));
	lookup->ip = ip;
	LL_Add(&lookup->chead,(void*)client);

	LL_Add(&cache_head,lookup);

	lookup->tHandle = (HANDLE)_beginthreadex(NULL,0,(unsigned int(__stdcall*)(void*))Lookup_Thread,lookup,0,&lookup->tID);
	CloseHandle(lookup->tHandle);
}

void Lookup_RemoveFromCache(unsigned int ip)
/*
** Lookup_RemoveFromCache()
** Removes the specified IP address from the DNS cache, no questions asked
** NOTE: DON'T use this during a lookup!!!!
*/
{
	LINKED_LIST_STRUCT *temp = &cache_head, *curr = &cache_head;
	LOOKUP_CACHE_STRUCT *lookup;

	while (curr->next)
	{
		curr = curr->next;

		if ((lookup = (LOOKUP_CACHE_STRUCT*)(curr->data))->ip == ip)
		/* Target found */
		{
			/* Free lookup information */
			while (lookup->chead.next)
				LL_Remove(&lookup->chead,lookup->chead.next->data);
			free(lookup);
			lookup = NULL;

			return;
		}

		temp = curr;
	}
}

int Lookup_Thread(void *param)
/*
** Lookup_Thread()
** Performs lookup work in its own thread, therefore allowing the server to continue service
*/
{
	char ip[32];
	LOOKUP_CACHE_STRUCT *lookup = (LOOKUP_CACHE_STRUCT*)param;
	LPHOSTENT lphostent;
	in_addr in;

	if (lookup->hostname[0])
	/* This is a hostname lookup(used to connect to outbound servers only) */
	{
		lphostent = gethostbyname(lookup->hostname);

		if (lphostent == NULL)
		/* Unable to resolve */
			lookup->error = WSAGetLastError();
		else
		/* Resolved successfully */
		{
			/* Get first h_addr from list */
			in = *((LPIN_ADDR)*lphostent->h_addr_list);
			lookup->ip = in.s_addr;
		}
	}
	else
	/* Regular IP lookup */
	{

		in.s_addr = lookup->ip;

		strcpy(ip,inet_ntoa(in));

		if ((lphostent = gethostbyaddr((char*)&in.s_addr,4,PF_INET)) == NULL)
		/* Unable to resolve */
		{
			strcpy(lookup->hostname,ip);
			lookup->error = WSAGetLastError();
		}
		else
		{
			if (strlen(lphostent->h_addr) > 63)
			/* The hostname is too long, use IP address instead */        
				strcpy(lookup->hostname,inet_ntoa(in));
			else
			/* Use hostname */
				strcpy(lookup->hostname,lphostent->h_name);
		}
	}

	lookup->completed = TRUE;

	return 0;
}

void Lookup_Cycle(INFOSTRUCTSETTINGS *is)
/*
** Lookup_Cycle()
** This is the function performed every pass by the main socket procedure, it:
** 1) Assigns the correct clients the correct lookup information
** 2) Manages the lookup cache, removing old entries
*/
{
	LINKED_LIST_STRUCT *temp = &cache_head, *curr = &cache_head;
	LOOKUP_CACHE_STRUCT *lookup;
	CLIENT_STRUCT *client;
	time_t cachetab[] = { 1, 60, 3600 }, cachetime, currtime = 0;

	if (curr->next)
	/* Only call time() when we have to */
		currtime = time(NULL);

	while (curr->next)
	{
		curr = curr->next;

		lookup = (LOOKUP_CACHE_STRUCT*)curr->data;

		if (lookup->completed && lookup->chead.next)
		/* The lookup has been completed but the clients haven't been updated */
		{
			while (lookup->chead.next)
			/* Update all clients */
			{
				client = (CLIENT_STRUCT*)lookup->chead.next->data;

				Client_SendToOne(client,FALSE,":%s NOTICE AUTH :*** Hostname lookup completed...",SettingsInfo.isGeneral.servername);

				if (!lookup->chead.next)
				{
					_asm int 3
				}

				strcpy(client->hostname,lookup->hostname);
				client->state = CLIENT_STATE_REGISTERING;

				LL_Remove(&lookup->chead,lookup->chead.next->data);
			}

			cachetime = is->isGeneral.dns_cache ? is->isGeneral.dns_cachetime * cachetab[is->isGeneral.dns_cachetimeduration] : 0;
			lookup->expiration = currtime + cachetime;
		}

		/* Check for cache expiries */
		if (lookup->expiration && lookup->expiration <= currtime || scs.signal == SIGNAL_ENDTHREAD)
		{
			Lookup_RemoveFromCache(lookup->ip);
			LL_Remove(&cache_head,curr->data);
			curr = temp;
		}
		temp = curr;
	}
}