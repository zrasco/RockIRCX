/*
** settings.cpp
** Contains functions for modifying different settings loaded
*/

#include "settings.h"

#ifdef APP_ROCKIRCXRA

int Binding_Add(char* address, unsigned short port)
/*
** Binding_Add()
** This function adds a binding to the bindings list
** Return values: -1 if ip/port combo exists, otherwise 0
** NOTE: This function to be used for remote admin utility only!
*/
{
	char buffer[256];
	HWND binding = GetSubItemHandle(IDC_LISTBINDINGS);

	sprintf(buffer,"%s:%d",address,port);

	if (SendMessage(binding,LB_FINDSTRING,-1,(LPARAM)buffer) != LB_ERR)
	{
		return -1;
	}
	else
	{
		SendMessage(binding,LB_ADDSTRING,0,(LPARAM)buffer);
	}

	return 0;
}

int Binding_Delete(int index)
/*
** Binding_Delete()
** Deletes specified binding based on the index value
** NOTE: This function to be used for remote admin utility only!
*/
{
	SendMessage(GetSubItemHandle(IDC_LISTBINDINGS),LB_DELETESTRING,index,0);

	return 0;
}

#endif	/* APP_ROCKIRCXRA */

char *SettingsFileName(char *szBuf)
/*
** SettingsFileName()
** Represented as <computername>.dat
*/
{
	char szComputerName[256];
	DWORD dwSize = 256;

	GetComputerName(szComputerName,&dwSize); szComputerName[dwSize] = 0;
	sprintf(szBuf,"%s.dat",szComputerName);
	return szBuf;
}

int	BindingBuf_Add(char **bindingbuf, unsigned short *numbindings , char* address, unsigned short port)
/*
** BindingBuf_Add()
** This function adds a binding to the bindings list
** Return values: -1 if ip/port combo exists, otherwise 0
** NOTE: This function to be used for RockIRCX server only!
*/
{
	char *bb = *bindingbuf;
	unsigned short nb = (*numbindings);
	int iAddr = inet_addr(address), count, offset = nb * 6;

	for (count = 0; count < nb; count++)
	/* Check for existing binding of same ip/port */
		if (iAddr == (*((unsigned long*)&bb[count * 6])) && (unsigned)bb[(count * 6) + 4] == htons(port))
			return -1;

	bb = (char*)realloc(bb,offset + 6);

	/* Add binding, attach pointer, and increment number of bindings */
	(*((unsigned long*)&bb[offset])) = htonl(iAddr);
	(*((unsigned short*)&bb[offset + 4])) = htons(port);
	*bindingbuf = bb;
	(*numbindings)++;

	return 0;
}

int	BindingBuf_Delete(char*, unsigned short*, int)
/*
** Binding_Delete()
** Deletes specified binding based on the index value
** NOTE: This function to be used for server only!
*/
{
	return 0;
}

BOOL BindingBuf_GetInfo(const char *bindingbuf, unsigned short num, char* ipaddr, unsigned short* port)
/*
** BindingBuffer_GetInfo()
** Returns info to specified pointers on binding IP/port
*/
{
	unsigned short lport, offset = num * (SIZE_LONG + SIZE_SHORT);
	struct in_addr in;

	GetLong(bindingbuf,offset,&in.s_addr); in.s_addr = ntohl(in.s_addr);

	/* Get IP address */
	strcpy(ipaddr,inet_ntoa(in));

	/* And port */
	GetShort(bindingbuf,offset + SIZE_LONG,&lport); lport = ntohs(lport);
	*port = lport;

	return TRUE;
}

int Account_Add(AccountList* accthead, AccountList* newacct)
/*
** Account_Add()
** Adds an account to the linked list of accounts
** Returns: 0 if successful, -1 if Account already exists
*/
{
	AccountList* curr = accthead;

	/* Set limits on string data, in case we have an elite h4x0r with my window handle */
	newacct->username[31] = 0; newacct->hostmask[31] = 0; newacct->password[31] = 0;

	while (curr->next != NULL)
	{
		curr = curr->next;

		if (strcmp(curr->username,newacct->username) == 0)
		/* Account exists */
			return -1;
	}

	curr->next = (AccountList*)calloc(1,sizeof(AccountList));

	curr = curr->next;

	curr->level = newacct->level;
	curr->logins = newacct->logins;
	strcpy(curr->username,newacct->username);
	strcpy(curr->hostmask,newacct->hostmask);
	strcpy(curr->password,newacct->password);
	curr->next = NULL;

	return 0;
}
void Account_Delete(AccountList* accthead, AccountList* target)
/*
** Account_Delete()
** Deletes specified account from linked list
*/
{
    AccountList* curr = accthead;

	while (curr->next != NULL)
	{

		if (curr->next == target)
		/* Found match, delete */
		{
			AccountList* temp = curr->next;

			curr->next = curr->next->next;

			free(temp);
			
			return;
		}
		else
			curr = curr->next;
	}
}

AccountList* Account_Find(AccountList* accthead, char* username)
/*
** Account_Find()
** Searches through account lists by username and finds the right one
** Returns: Pointer to matching account list structure
*/
{
	AccountList* retval = NULL;
	AccountList* curr = accthead;

	while (curr->next != NULL)
	{
		curr = curr->next;

		if (strcmp(curr->username,username) == 0)
		{
			retval = curr;
			break;
		}
	}

	return retval;
}

int Account_Modify(AccountList* accthead, AccountList* dest, AccountList* srcinfo)
/*
** Account_Modify()
** Modifies account information for dest, getting the information from src
** Returns: 0 if successful, -1 if trying to modify to already existing account
*/
{
	AccountList* curr = accthead;

	/* Set limits on string data, in case we have an elite h4x0r with my window handle */
	srcinfo->username[31] = 0; srcinfo->hostmask[31] = 0; srcinfo->password[31] = 0;

	/* Now, search through and determine if account exists elsewhere */
	while (curr->next != NULL)
	{
		curr = curr->next;

		if ((strcmp(curr->username,srcinfo->username) == 0) && curr != dest)
		/* Trying to modify to account with same username */
			return -1;
	}

	strcpy(dest->username,srcinfo->username);
	strcpy(dest->hostmask,srcinfo->hostmask);
	strcpy(dest->password,srcinfo->password);
	dest->level = srcinfo->level;
	dest->logins = 0;

    return 0;
}

int	Account_GetTotal(AccountList* accthead, unsigned char level)
/*
** Account_GetTotal()
** Gets the total number of accounts
*/
{
	AccountList* curr = accthead;
	int retval = 0;

	while (curr->next != NULL)
	{
		curr = curr->next;

		if (curr->level == level)
			retval++;
	}

	return retval;
}

int Filter_Add(FilterList* filterhead, char* word, unsigned int type)
/*
** Filter_Add()
** Adds a filter to the filter list
** Returns: -1 if filter currently exists with type specified
*/
{
	FilterList* curr = filterhead;


	while (curr->next != NULL)
	{
		curr = curr->next;

		if ((strcmp(curr->word,word) == 0))
		/* Word exists */
		{
			if (curr->type == type)
				return -1;
			else
			{
				curr->type |= type;
				return 0;
			}
		}
	}

	curr->next = (FilterList*)calloc(sizeof(FilterList),1);
	
	/* Create new filter in chain */
	curr = curr->next;
	strcpy(curr->word,word);
	curr->type = type;

	return 0;
}

int Filter_Modify(FilterList* filterhead, FilterList* target, char* newword, unsigned int newtype)
/*
** Filter_Modify()
** Modifies the target filter information structure with new data supplied
** Returns: -1 if filter with specified word already exists
*/
{
	FilterList* curr = filterhead, *temp = Filter_Find(filterhead, newword);

	if (target == temp)
	/* Modifying the same filter */
	{
		target->type = newtype;
		return 0;
	}
	else
	{
		while (curr->next != NULL)
		{
			curr = curr->next;

			if (strcmp(curr->word,newword) == 0)
			/* Attempting to modify to currently used filter */
				return -1;
		}
	}

	strcpy(target->word,newword);
	target->type = newtype;

	return 0;
}
void Filter_Delete(FilterList* filterhead, char* word, unsigned int type)
/*
** Filter_Delete()
** Deletes filter with word, or removes specified type
*/
{
	FilterList* curr = filterhead, *temp;

	while (curr->next != NULL)
	{
		temp = curr;
		curr = curr->next;

		if (strcmp(curr->word,word) == 0)
		/* Word found, XOR the resulting type */
		{
			curr->type ^= type;

			if (!curr->type)
			/* No remaining types, remove from list */
			{

				temp->next = curr->next;

				free(curr);
				
				return;
			}
		}
	}
}

FilterList* Filter_Find(FilterList* filterhead, char* word)
/*
** Filter_Find()
** Finds and returns filter structure pointer with specified word
** Returns: NULL if none found
*/
{
	FilterList *curr = filterhead, *retval = NULL;

	while (curr->next != NULL)
	{
		curr = curr->next;

		if (strcmp(curr->word,word) == 0)
		/* Found matching filter */
		{
			retval = curr;
			break;
		}
	}

	return retval;
}


int Banlist_Add(BanList* banhead, char* hostmask, char* reason, BOOL bNetworkBan, BOOL bException, unsigned int expiration, unsigned char exptype)
/*
** BanList_Add()
** Adds a ban/exception entry with the info specified
** Returns: -1 if ban/exception with specified mask currently exists
*/
{
	BanList* curr = banhead;

	/* Look through existing list and see if ban/exception exists */
	while (curr->next != NULL)
	{
		curr = curr->next;

		if ((strcmp(curr->hostmask,hostmask) == 0) && curr->bException == bException)
		/* It exists */
			return -1;
	}

	curr->next = (BanList*)calloc(1,sizeof(BanList));

	curr = curr->next;

	curr->hostmask = (char*)malloc(strlen(hostmask) + 1);
	strcpy(curr->hostmask,hostmask);
	
	if (reason != NULL)
	{
		curr->reason = (char*)malloc(strlen(reason) + 1);
		strcpy(curr->reason,reason);
	}

	curr->bException = bException;
	curr->bNetworkBan = bNetworkBan;
	curr->expiration = expiration;
	curr->exptype = exptype;
	curr->next = NULL;

	return 0;
}

int Banlist_Modify(BanList* banhead, BanList* target, char* hostmask, char* reason, BOOL bNetworkBan, unsigned int expiration, unsigned char exptype)
/*
** Banlist_Modify()
** Modifies targeted ban/exception information structure
** Returns: -1 if trying to modify to ban/exception with existing hostmask
*/
{
	BanList* curr = banhead;

	/* Now, search through and determine if ban exists */
	while (curr->next != NULL)
	{
		curr = curr->next;

		if ((strcmp(curr->hostmask,hostmask) == 0) && curr->bException == target->bException && curr != target)
		/* Trying to modify to ban/exception with same username */
			return -1;
	}

	target->hostmask = (char*)realloc(target->hostmask,strlen(hostmask) + 1);
	strcpy(target->hostmask,hostmask);
	target->bNetworkBan = bNetworkBan;
	target->expiration = expiration;
	target->exptype = exptype;
	
	if (reason != NULL)
	{
		if (target->reason)
            target->reason = (char*)realloc(target->reason,strlen(reason) + 1);
		else
			target->reason = (char*)malloc(strlen(reason) + 1);

		strcpy(target->reason,reason);
	}
	else if (target->reason != NULL)
	/* Free memory for reason */
	{
		free(target->reason);
		target->reason = NULL;
	}

	return 0;
}
void Banlist_Delete(BanList* banhead, BanList* target)
/*
** Banlist_Delete()
** Deletes specified ban/exception information structure
*/

{
    BanList* curr = banhead;

	while (curr->next != NULL)
	{
		if (curr->next == target)
		/* Found match, delete */
		{
			BanList* temp = curr->next;

			curr->next = curr->next->next;

			free(temp->hostmask);
			free(temp->reason);
			free(temp);
			
			return;
		}
		else
			curr = curr->next;
	}
}
BanList* Banlist_Find(BanList* banhead, char* hostmask, BOOL bException)
/*
** Banlist_Find()
** Finds ban/exception list information structure and returns pointer
** Returns: NULL if not found
*/
{
	BanList* retval = NULL;
	BanList* curr = banhead;

	while (curr->next != NULL)
	{
		curr = curr->next;

		if (strcmp(curr->hostmask,hostmask) == 0 && curr->bException == bException)
		{
			retval = curr;
			break;
		}
	}

	return retval;
}

int RCL_Add(RegChanList* RCLHead, RegChanList* RCLData)
/*
** RCL_Add()
** Adds a channel to the list of registered channels
** Returns: 0 if successful, -1 if entry exists
*/
{
	RegChanList *curr = RCLHead;

	while (curr->next != NULL)
	{
		curr = curr->next;

		if (strcmp(curr->name,RCLData->name) == 0)
		/* Channel entry already exists */
			return -1;
	}

	curr->next = (RegChanList*)calloc(1,sizeof(RegChanList));
	curr = curr->next;

	/* General data */
	strcpy(curr->name,RCLData->name);
	strcpy(curr->subject,RCLData->subject);

	if (RCLData->topic)
	{
		CREATEPTRFROMSTRING(curr->topic,RCLData->topic);
	}
	else
	{
		CREATEPTRFROMSTRING(curr->topic,"");
	}

	if (RCLData->onjoin)
	{
		CREATEPTRFROMSTRING(curr->onjoin,RCLData->onjoin);
	}
	else
	{
		CREATEPTRFROMSTRING(curr->onjoin,"");
	}

	if (RCLData->onpart)
	{
		CREATEPTRFROMSTRING(curr->onpart,RCLData->onpart);
	}
	else
	{
		CREATEPTRFROMSTRING(curr->onpart,"");
	}

	/* Access data */
	curr->accesshead = RCLData->accesshead;
	strcpy(curr->ownerkey,RCLData->ownerkey);
	strcpy(curr->hostkey,RCLData->hostkey);
	strcpy(curr->memberkey,RCLData->memberkey);

	/* Mode data */
	curr->visibility = RCLData->visibility;
	curr->limit = RCLData->limit;
	curr->modeflags = RCLData->modeflags;

	curr->next = NULL;

	return 0;
}

RegChanList* RCL_Find(RegChanList* RCLHead, char* name)
/*
** RCL_Find()
** Finds a registered channel entry by using the name
** Returns: Pointer to entry, or NULL if entry does not exist
*/
{
	RegChanList *curr = RCLHead, *retval = NULL;

	while (curr->next != NULL)
	{
		curr = curr->next;

		if (strcmp(curr->name,name) == 0)
		{
			retval = curr;
			break;
		}
	}

	return retval;
}

int	RCL_Modify(RegChanList* RCLHead, RegChanList* target, RegChanList* RCLData)
/*
** RCL_Modify()
** Modifies targeted registered channel entry
** Returns: 0 if successful, -1 if attempting to rename to existing entry
*/
{
	RegChanList* curr;

	if ((curr = RCL_Find(RCLHead, RCLData->name)) != NULL && curr != target)
	/* Entry exists elsewhere */
		return -1;

	/* General data */
	strcpy(target->name,RCLData->name);
	strcpy(target->subject,RCLData->subject);
	MODIFYPTRFROMSTRING(target->topic,RCLData->topic);
	MODIFYPTRFROMSTRING(target->onjoin,RCLData->onjoin);
	MODIFYPTRFROMSTRING(target->onpart,RCLData->onpart);

	/* Access data */
	target->accesshead = RCLData->accesshead;
	strcpy(target->ownerkey,RCLData->ownerkey);
	strcpy(target->hostkey,RCLData->hostkey);
	strcpy(target->memberkey,RCLData->memberkey);

	/* Mode data */
	target->visibility = RCLData->visibility;
	target->limit = RCLData->limit;
	target->modeflags = RCLData->modeflags;

	return 0;
}

void RCL_Delete(RegChanList* RCLHead, RegChanList *target)
/*
** RCL_Delete()
** Deletes a registered channel entry
*/
{
	RegChanList *curr = RCLHead, *temp = RCLHead;

	while (curr->next != NULL)
	{
		curr = curr->next;

		if (curr == target)
		{
			temp->next = curr->next;

			while (curr->accesshead.next != NULL)
				RCL_DeleteAccess(curr,curr->accesshead.next);

			free(curr->topic);
			free(curr->onjoin);
			free(curr->onpart);
			free(curr);

			return;

		}

		temp = curr;
	}
}



int	RCL_AddAccess(RegChanList* RCLTarget, AccessList* aData)
/*
** RCL_AddAccess()
** Adds an access entry to the specified registered channel entry
** Returns: 0 if successful, -1 if entry exists
*/
{
	AccessList *curr = &RCLTarget->accesshead;

	while (curr->next != NULL)
	{
		curr = curr->next;

		if (strcmp(curr->hostmask,aData->hostmask) == 0)
		/* Access entry exists */
			return -1;
	}

	curr->next = (AccessList*)calloc(1,sizeof(AccessList));
	curr = curr->next;

	strcpy(curr->hostmask,aData->hostmask);
	if (aData->reason != NULL)
	{
        CREATEPTRFROMSTRING(curr->reason,aData->reason);
	}
	else
		curr->reason = NULL;

	curr->expire = aData->expire;
	curr->exptype = aData->exptype;
	curr->type = aData->type;

	curr->next = NULL;

	return 0;
}
AccessList*	RCL_FindAccess(RegChanList* RCLTarget, char* hostmask)
/*
** RCL_FindAccess()
** Finds specified access entry using target registered channel entry & hostmask
** Returns: Pointer to found entry, or NULL if it doesn't exist
*/
{
	AccessList *curr = &RCLTarget->accesshead, *retval = NULL;

	while (curr->next != NULL)
	{
		curr = curr->next;

		if (strcmp(curr->hostmask,hostmask) == 0)
		/* Entry found */
		{
			retval = curr;
			break;
		}
	}

	return retval;
}
int	RCL_ModifyAccess(RegChanList* RCLTarget, AccessList* aTarget, AccessList* aData)
/*
** RCL_ModifyAccess()
** Modifies the specified access entry of the specified registered channel entry
** Returns: 0 if successful, -1 if entry already exists under same hostmask
*/
{
	AccessList *curr;

	if ((curr = RCL_FindAccess(RCLTarget,aData->hostmask)) != NULL && curr != aTarget)
	/* Attempting to modify entry existing elsewhere in list */
		return -1;

	/* Change entry data */
	strcpy(aTarget->hostmask,aData->hostmask);
	if (aData->reason)
	{
		MODIFYPTRFROMSTRING(aTarget->reason,aData->reason);
	}
	else
	{
		free(aTarget->reason);
		aTarget->reason = NULL;
	}

	aTarget->type = aData->type;
	aTarget->expire = aData->expire;
	aTarget->exptype = aData->exptype;

	return 0;
}
void RCL_DeleteAccess(RegChanList* RCLTarget, AccessList* aTarget)
/*
** RCL_DeleteAccess()
** Deletes the specified access entry from the list of specified registered channel
*/
{
	AccessList *curr = &RCLTarget->accesshead, *temp = &RCLTarget->accesshead;

	while (curr->next != NULL)
	{
		curr = curr->next;

		if (curr == aTarget)
		{
			temp->next = curr->next;

			if (curr->reason != NULL)
				free(curr->reason);
			free(curr);

			return;
		}

		temp = curr;
	}
}

int	Server_Add(ServerList* serverhead, ServerList* sData)
/*
** Server_Add()
** Adds a server to the inbound/outbound server list
** Returns: -1 if server name/port combo exists
*/
{
	ServerList *curr = serverhead;

	while (curr->next != NULL)
	{
		curr = curr->next;

		if (strcmp(curr->name,sData->name) == 0)
		/* Server exists in list */
			return -1;
	}

	curr->next = (ServerList*)calloc(1,sizeof(ServerList));
	curr = curr->next;

	strcpy(curr->name,sData->name);
	strcpy(curr->hostmask,sData->hostmask);
	strcpy(curr->password,sData->password);
	curr->ping_frequency = sData->ping_frequency;
	curr->ping_response = sData->ping_response;
	curr->encryption = sData->encryption;

	/* If port == 0, server is inbound */
	curr->port = sData->port;

	curr->next = NULL;

	return 0;

}

ServerList*	Server_Find(ServerList* serverhead, char* name)
/*
** Server_Find()
** Goes through list of servers with name/port combo
** Returns: Pointer to result, or NULL if not found
*/
{
	ServerList *curr = serverhead, *retval = NULL;

	while (curr->next)
	{
		curr = curr->next;

		if (strcmp(curr->name,name) == 0)
		/* Found entry */
		{
			retval = curr;
			break;
		}
	}

	return retval;
}
int	Server_Modify(ServerList* serverhead, ServerList* target, ServerList* sData)
/*
** Server_Modify()
** Modifies target server with information specified
** Returns: 0 if successful, -1 if entry exists with same name
*/
{
	ServerList *curr = Server_Find(serverhead, sData->name);
	if (curr && curr != target)
	/* Server is already in list */
		return -1;

	strcpy(target->name,sData->name);
	strcpy(target->hostmask,sData->hostmask);
	strcpy(target->password,sData->password);
	target->ping_frequency = sData->ping_frequency;
	target->ping_response = sData->ping_response;
	target->encryption = sData->encryption;

	/* If port == 0, server is inbound */
	target->port = sData->port;

	return 0;
}
void Server_Delete(ServerList* serverhead, ServerList* target)
/*
** Server_Delete()
** Goes through server list and deletes target
*/
{
	ServerList *curr = serverhead, *temp = serverhead;

	while (curr->next != NULL)
	{
		curr = curr->next;

		if (curr == target)
		{
			temp->next = curr->next;
			free(curr);

			return;

		}

		temp = curr;
	}
}

SpoofedList* Spoofed_Add(SpoofedList* spoofedhead, SpoofedList* sData)
/*
** Spoofed_Add()
** Adds a spoofed host to the list of detected hosts
** Returns: Pointer to added entry
*/
{
	SpoofedList *curr = spoofedhead;

	while (curr->next)
	{
		curr = curr->next;

		if (strcmp(curr->hostname,sData->hostname) == 0)
		/* Host already is in table */
		{
			register int count;
			register BOOL bAdd = TRUE;

			for (count = 0; curr->port[count] != 0; count++)
			/* If port specified not in list, add it */
			{
				if (curr->port[count] == sData->port[0])
				/* Duplicate entry exists here */
					return curr;
			}

			if (bAdd)
			/* Add port to entry */
			{
				curr->port[count] = sData->port[0];
				curr->port[++count] = 0;
				return curr;
			}
		}
	}

	curr->next = (SpoofedList*)calloc(1,sizeof(SpoofedList));
	curr = curr->next;

	CREATEPTRFROMSTRING(curr->hostname,sData->hostname);
	curr->port[0] = sData->port[0];
	curr->next = NULL;

	return curr;
}

SpoofedList* Spoofed_Find(SpoofedList* spoofedhead, char* hostname)
/*
** Spoofed_Find()
** Goes through list of spooed hosts and returns pointer to entry with matching hostname
*/
{
	SpoofedList *curr = spoofedhead, *retval = NULL;

	while (curr->next)
	{
		curr = curr->next;

		if (strcmp(curr->hostname,hostname) == 0)
		/* Matching entry found */
		{
            retval = curr;
			break;
		}
	}

	return retval;
}

void Spoofed_Delete(SpoofedList* spoofedhead, SpoofedList* target)
/*
** Spoofed_Delete()
** Deletes a spoofed host entry from the list
*/
{
	SpoofedList *curr = spoofedhead, *temp = spoofedhead;

	while (curr->next != NULL)
	{
		curr = curr->next;

		if (curr == target)
		{
			temp->next = curr->next;

            free(curr->hostname);
			free(curr);

			return;
		}

		temp = curr;
	}
}

void SpoofedLogin_Add(SpoofedList* sTarget, char *hostmask, char *server, unsigned short port, unsigned long logintime)
/*
** SpoofedLogin_Add()
** Adds a spoofed host login entry to the specified spoofed host entry
** Returns: 0 if successful, -1 if entry already exists
*/
{
	SpoofedLoginLog *curr = &sTarget->loginhead, *temp = &sTarget->loginhead;

	while (curr->next)
	{
		curr = curr->next;

		if (logintime < curr->time)
		/* Insert entry in previous link */
		{
			SpoofedLoginLog *inc_curr = curr;
			temp->next = (SpoofedLoginLog*)calloc(1,sizeof(SpoofedLoginLog));
			temp = temp->next;

			temp->index = curr->index++;

			/* Increment indexes of entire list */
            while (inc_curr->next != NULL)
			{
				inc_curr = inc_curr->next;
				inc_curr->index++;
			}

			temp->time = logintime;
			temp->port = port;
			CREATEPTRFROMSTRING(temp->server,server);
			CREATEPTRFROMSTRING(temp->hostmask,hostmask);
			temp->next = curr;

			return;
		}

		temp = temp->next;
	}

	/* Insert at end */
	curr->next = (SpoofedLoginLog*)calloc(1,sizeof(SpoofedLoginLog));
	temp = curr; curr = curr->next;

	curr->index = (sTarget->loginhead.next == curr) ? 0 : temp->index + 1;
	curr->time = logintime;
	curr->port = port;
	CREATEPTRFROMSTRING(curr->server,server);
	CREATEPTRFROMSTRING(curr->hostmask,hostmask);
	curr->next = NULL;
}

void SpoofedLogin_Report(SpoofedList *sTarget, SpoofedLoginLog *curr, char *targetbuf)
/*
** SpoofedLogin_Report()
** Fills target buffer with report information about passed index number
*/
{
	char buf[128], temp[128];
	time_t tm = curr->time;

	/* TEMP: Supress warning C4100(Why is sTarget being passed again?) */
	SpoofedList *sTarget2 = sTarget;

	strcpy(buf,ctime(&tm));
	buf[strlen(buf) - 1] = 0;
	ConvertTime(buf,temp);

	sprintf(targetbuf,"Connection from %s to %s:%d at %s.",curr->hostmask,curr->server,curr->port,temp);
}

SpoofedLoginLog* SpoofedLogin_Find(SpoofedList* sTarget, unsigned short index)
/*
** SpoofedLogin_Find()
** Returns pointer to login info structure based on passed index value
*/
{
	SpoofedLoginLog *curr = &sTarget->loginhead, *retval = NULL;

	while (curr->next)
	{
		curr = curr->next;

		if (curr->index == index)
		/* Match found */
		{
			retval = curr;
			break;
		}
	}

	return retval;
}

void SpoofedLogin_DeleteList(SpoofedList* sTarget)
/*
** SpoofedLogin_Delete()
** Clears list of logins by specified spoofed host
*/
{
	SpoofedLoginLog *temp;

    while (temp = sTarget->loginhead.next)
	{
		sTarget->loginhead.next = temp->next;

		free(temp->server);
		free(temp->hostmask);
		free(temp);
	}
}

unsigned int SpoofedLogin_Logins(SpoofedList *sTarget)
/*
** SpoofedList_Logins()
** Returns number of logins for target spoofed host
*/
{
	int count = 0;
	SpoofedLoginLog *curr = &sTarget->loginhead;

	while (curr->next)
	{
		curr = curr->next;

		if (curr->server)
			count++;
	}

	return count;
}

unsigned int SpoofedLogin_Last(SpoofedList* sTarget)
/*
** SpoofedLogin_Last()
** Returns calendar last attempted login by spoofed host
*/
{
	unsigned int last = 0;
	SpoofedLoginLog *curr = &sTarget->loginhead;

	while (curr->next)
	{
		curr = curr->next;

		if (curr->time > last)
			last = curr->time;
	}

	return curr->time;
}


unsigned int IS_CreateBuffer(const void *listhead, char **targetbuf, int type)
/*
** IS_CreateBuffer()
** Creates a buffer of the specified type and returns length of buffer
** NOTE: buffer should be a either a NULL pointer or an invalid one
** NOTE 2: ALL numeric values must be converted to network byte order before
** entering the buffer(using htons() or htonl())
*/
{
	unsigned int index = 0, length = 0;
	char *buffer;

	buffer = (char*)calloc(1,4096);

	switch (type)
	{
		case BUFFER_TYPE_ACCOUNT:
		{
			AccountList *curr = (AccountList*)listhead;
			char buf[4][256];
			unsigned short total = 0, buflen = 0, count;

			/* Save first four bytes for total length & # of accounts */
			index = SIZE_SHORT * 2;

			while (curr->next)
			{
				curr = curr->next;
				total++;

				/* Get all info */
				strcpy(buf[0],curr->username);
				strcpy(buf[1],curr->hostmask);
				strcpy(buf[2],curr->password);

				for (count = 0; count < 3; count++)
				/* Add all of our information to the buffer */
				{
					buflen = (unsigned short)strlen(buf[count]);
					InsertShort(buffer,index,htons(buflen));
					index += SIZE_SHORT; length += SIZE_SHORT;
					memcpy(&buffer[index],&buf[count][0],buflen);
					index += buflen; length += buflen;
				}

				buffer[index++] = curr->level; length++;
				InsertShort(buffer,index,htons(curr->logins));
				index+= SIZE_SHORT; length += SIZE_SHORT;
			}

			if (total)
			{
				length = index;

				/* Bytes 0-1 == Next X bytes is account buffer data */
				InsertShort(buffer,0,htons(index - SIZE_SHORT));

				/* Bytes 2-3 == total # of entries */
                InsertShort(buffer,SIZE_SHORT,htons(total));
			}
		}
		break;
		case BUFFER_TYPE_BAN:
		{
			BanList *curr = (BanList*)listhead;
			unsigned short total = 0, buflen = 0;

			/* Save first four bytes for total length & # of ban/exceptions */
			index = SIZE_SHORT * 2;

			while (curr->next)
			{
				curr = curr->next;
				total++;

				/* Add all of our information to the buffer */

				/* Insert exception flag at beginning */
				buffer[index++] = (char)curr->bException;
				length++;

				/* Add hostmask */
				buflen = (unsigned short)strlen(curr->hostmask);
				InsertShort(buffer,index,htons(buflen));
				index += SIZE_SHORT; length += SIZE_SHORT;
				memcpy(&buffer[index],&curr->hostmask[0],buflen);
				index += buflen; length += buflen;

				if (!curr->bException)
				/* Add ban information */
				{
					/* This ban a network ban? */
					buffer[index++] = (char)curr->bNetworkBan;
					length++;

					if (curr->reason)
					/* There IS a reason for the madness! */
					{
						buflen = (unsigned short)strlen(curr->reason);
						InsertShort(buffer,index,htons(buflen));
						index += SIZE_SHORT; length += SIZE_SHORT;
						memcpy(&buffer[index],&curr->reason[0],buflen);
						index += buflen; length += buflen;
					}
					else
					/* No reason */
					{
						InsertShort(buffer,index,0);
                        index += SIZE_SHORT; length += SIZE_SHORT;
					}

					/* Insert expiration time & type */
					InsertLong(buffer,index,htonl(curr->expiration));
					index += SIZE_LONG; length += SIZE_LONG;

					buffer[index++] = curr->exptype;
					length++;					
				}
			}

			if (total != 0)
			{
				length = index;

				/* Bytes 0-1 == Next X bytes is ban buffer data */
				InsertShort(buffer,0,htons(index - SIZE_SHORT));

				/* Bytes 2-3 == total # of entries */
                InsertShort(buffer,SIZE_SHORT,htons(total));
			}
		}
		break;
		case BUFFER_TYPE_RCL:
		{
			RegChanList *curr = (RegChanList*)listhead;
			AccessList *acurr;
			char buf[8][256];
			unsigned short atotalpos, atotal = 0, total = 0, buflen = 0, count;

			/* Save first four bytes for total length & # of channels */
			index = SIZE_SHORT * 2;

			while (curr->next)
			{
				curr = curr->next;
				acurr = &curr->accesshead; atotal = 0; 
				total++;

				/* Get all info */
				strcpy(buf[0],curr->name);
				strcpy(buf[1],curr->subject);
				strcpy(buf[2],curr->topic);
				strcpy(buf[3],curr->onjoin);
				strcpy(buf[4],curr->onpart);
				strcpy(buf[5],curr->ownerkey);
				strcpy(buf[6],curr->hostkey);
				strcpy(buf[7],curr->memberkey);


				for (count = 0; count < 8; count++)
				/* Add all of our information to the buffer */
				{
					buflen = (unsigned short)strlen(buf[count]);
					InsertShort(buffer,index,htons(buflen));
					index += SIZE_SHORT; length += SIZE_SHORT;
					memcpy(&buffer[index],&buf[count][0],buflen);
					index += buflen; length += buflen;
				}

				/* Add mode info */
				buffer[index++] = curr->visibility; length++;
				InsertLong(buffer,index,htonl(curr->limit));
				index+= SIZE_LONG; length += SIZE_LONG;
				buffer[index++] = curr->modeflags; length++;

				/* Add access info */
				atotalpos = (short)index;
				index += SIZE_SHORT; length += SIZE_SHORT;

				while (acurr->next)
				/* Loop through access list and add to buffer */
				{
					acurr = acurr->next;
					atotal++;

					/* Add hostmask */
					buflen = (unsigned short)strlen(acurr->hostmask);
					InsertShort(buffer,index,htons(buflen));
					index += SIZE_SHORT; length += SIZE_SHORT;
					memcpy(&buffer[index],&acurr->hostmask[0],buflen);
					index += buflen; length += buflen;

					/* ...type */
					buffer[index++] = acurr->type; length++;

					/* reason */
					if (acurr->reason)
					{
						buflen = (unsigned short)strlen(acurr->reason);
					}
					else
					{
						buflen = 0;
					}

					InsertShort(buffer,index,htons(buflen));
					index += SIZE_SHORT; length += SIZE_SHORT;

					if (buflen)
					/* Add reason */
					{
						memcpy(&buffer[index],&acurr->reason[0],buflen);
						index += buflen; length += buflen;
					}

					/* And last, expiration info */
					InsertLong(buffer,index,htonl(acurr->expire));
					index += SIZE_LONG; length += SIZE_LONG;

					buffer[index++] = acurr->exptype;
					length++;
				}

				/* Insert # of access entries */
				InsertShort(buffer,atotalpos,htons(atotal));
			}

			if (total)
			{
				length = index;

				/* Bytes 0-1 == Next X bytes is channel buffer data */
				InsertShort(buffer,0,htons(index - SIZE_SHORT));

				/* Bytes 2-3 == total # of registered channels */
                InsertShort(buffer,SIZE_SHORT,htons(total));
			}

		}
		break;
		case BUFFER_TYPE_SERVER:
		{
			ServerList *curr = (ServerList*)listhead;
			char encryption, buf[4][256];
			unsigned short total = 0, buflen = 0, count;

			/* Save first five bytes for total length, # of servers, and network type */
			index = (SIZE_SHORT * 2);

			while (curr->next)
			{
				curr = curr->next;
				total++;

				/* Get all info */
				strcpy(buf[0],curr->name);
				strcpy(buf[1],curr->hostmask);
				strcpy(buf[2],curr->password);

				encryption = curr->encryption;

				for (count = 0; count < 3; count++)
				/* Add all of our information to the buffer */
				{
					buflen = (unsigned short)strlen(buf[count]);
					InsertShort(buffer,index,htons(buflen));
					index += SIZE_SHORT; length += SIZE_SHORT;
					memcpy(&buffer[index],&buf[count][0],buflen);
					index += buflen; length += buflen;
				}

				/* Insert encryption type */
				buffer[index++] = curr->encryption;

				/* Insert port, 0 if inbound */
				InsertShort(buffer,index,htons(curr->port));
				index+= SIZE_SHORT; length += SIZE_SHORT;

				if (!curr->port)
				{
					InsertShort(buffer,index,htons(curr->ping_frequency));
					index+= SIZE_SHORT; length += SIZE_SHORT;
					InsertShort(buffer,index,htons(curr->ping_response));
					index+= SIZE_SHORT; length += SIZE_SHORT;
				}
			}

			if (total)
			{
				length = index;

				/* Bytes 0-1 == Next X bytes is server buffer data */
				InsertShort(buffer,0,htons(index - SIZE_SHORT));

				/* Bytes 2-3 == total # of servers */
                InsertShort(buffer,SIZE_SHORT,htons(total));
			}
		}
		break;
		case BUFFER_TYPE_SPOOFED:
		{
			SpoofedList *curr = (SpoofedList*)listhead;
			SpoofedLoginLog *scurr;
			unsigned short count, subtotal, pos, total = 0, buflen = 0;

			/* Save first four bytes for total length & # spoofed hosts */
			index = SIZE_SHORT * 2;

			while (curr->next)
			{
				curr = curr->next;
				total++;

				/* Store hostname */
				buflen = (unsigned short)strlen(curr->hostname);
				InsertShort(buffer,index,htons(buflen));
				index += SIZE_SHORT; length += SIZE_SHORT;
				memcpy(&buffer[index],&curr->hostname[0],buflen);
				index += buflen; length += buflen;

				/* Store # of ports & ports */
				subtotal = 0;
				pos = (short)index;
				index += SIZE_SHORT; length += SIZE_SHORT;

				for (count = 0; curr->port[count]; count++)
				/* Add each port into buffer */
				{
					subtotal++;
					InsertShort(buffer,index,htons(curr->port[count]));
					index += SIZE_SHORT; length += SIZE_SHORT;
				}

				InsertShort(buffer,pos,htons(subtotal));

				/* And last, login attempts information */
				scurr = (SpoofedLoginLog*)&curr->loginhead;

				pos = (short)index;
				subtotal = 0;
				index += SIZE_SHORT; length += SIZE_SHORT;

				while (scurr->next)
				{
					scurr = scurr->next;
					subtotal++;

					/* Store hostmask */
					buflen = (unsigned short)strlen(scurr->hostmask);
					InsertShort(buffer,index,htons(buflen));
					index += SIZE_SHORT; length += SIZE_SHORT;
					memcpy(&buffer[index],&scurr->hostmask[0],buflen);
					index += buflen; length += buflen;

					/* Server name too */
					buflen = (unsigned short)strlen(scurr->server);
					InsertShort(buffer,index,htons(buflen));
					index += SIZE_SHORT; length += SIZE_SHORT;
					memcpy(&buffer[index],&scurr->server[0],buflen);
					index += buflen; length += buflen;

					/* Server port */
					InsertShort(buffer,index,htons(scurr->port));
					index += SIZE_SHORT; length += SIZE_SHORT;

					/* ...and calender time of login */
					InsertLong(buffer,index,htonl(scurr->time));
					index += SIZE_LONG; length += SIZE_LONG;

				}

				/* Store total login attempts */
				InsertShort(buffer,pos,htons(subtotal));
			}

			if (total)
			{
				length = index;

				/* Bytes 0-1 == Next X bytes is spoofed buffer data */
				InsertShort(buffer,0,htons(index - SIZE_SHORT));

				/* Bytes 2-3 == total # of spoofed hosts */
                InsertShort(buffer,SIZE_SHORT,htons(total));
			}
		}
		break;
		case BUFFER_TYPE_FILTER:
		{
			FilterList *curr = (FilterList*)listhead;
			unsigned short buflen = 0, total = 0;

			/* First 4 bytes are total length & # of filtered words */
			index = SIZE_SHORT * 2;

			while (curr->next)
			{
				curr = curr->next;
				total++;

				/* Insert word */
				buflen = (unsigned short)strlen(curr->word);
				InsertShort(buffer,index,htons(buflen));
				index += SIZE_SHORT; length += SIZE_SHORT;
				memcpy(&buffer[index],&curr->word[0],buflen);
				index += buflen; length += buflen;

				/* ...and type of filter */
				buffer[index++] = curr->type;
				length++;
			}

			if (total)
			{
				length = index;

				/* Bytes 0-1 == Next X bytes is filter data */
				InsertShort(buffer,0,htons(index - SIZE_SHORT));

				/* Bytes 2-3 == total # of filtered words */
                InsertShort(buffer,SIZE_SHORT,htons(total));
			}
		}
		break;
		case BUFFER_TYPE_ACCESS:
		/* NOTE: RCL we'll be working with is passed as the list head */
		{
			RegChanList *RCLTarget = (RegChanList*)listhead;
			AccessList *acurr = &RCLTarget->accesshead;
			unsigned short atotal = 0, buflen = 0, total = 0;

			/* First 4 bytes will be used as the length of the buffer & # of entries */
			index = SIZE_SHORT * 2;

			while (acurr->next)
			/* Loop through access list and add to buffer */
			{
				acurr = acurr->next;
				atotal++;

				/* Add hostmask */
				buflen = (unsigned short)strlen(acurr->hostmask);
				InsertShort(buffer,index,htons(buflen));
				index += SIZE_SHORT; length += SIZE_SHORT;
				memcpy(&buffer[index],&acurr->hostmask[0],buflen);
				index += buflen; length += buflen;

				/* ...type */
				buffer[index++] = acurr->type; length++;

				/* reason */
				if (acurr->reason)
					buflen = (unsigned short)strlen(acurr->reason);
				else
					buflen = 0;

				InsertShort(buffer,index,htons(buflen));
				index += SIZE_SHORT; length += SIZE_SHORT;

				if (buflen)
				/* Add reason */
				{
					memcpy(&buffer[index],&acurr->reason[0],buflen);
					index += buflen; length += buflen;
				}

				/* And last, expiration info */
				InsertLong(buffer,index,htonl(acurr->expire));
				index += SIZE_LONG; length += SIZE_LONG;

				buffer[index++] = acurr->exptype;
				length++;
			}

			/* Insert length of access data */
			InsertShort(buffer,0,htons(index - SIZE_SHORT));

			/* Insert # of access entries */
			InsertShort(buffer,SIZE_SHORT,htons(atotal));

			/* Make length 4 bytes longer to make up for cutting short our index */
			length += SIZE_SHORT + SIZE_SHORT;
		}
		break;
	}

	if (length == 0)
	/* No work was done here, free up pointer */
	{
		free(buffer);
		buffer = NULL;
	}
	else
	{
		buffer = (char*)realloc(buffer,length);
	}

	*targetbuf = (buffer == NULL) ? NULL : buffer;

	//*targetbuf = (char*)malloc(length);
	//memcpy(targetbuf,buffer,length);
	//free(buffer);

	return length;
}

BOOL IS_CreateList(void* listhead, const char* buffer, int type)
/*
** IS_CreateList()
** Takes the target buffer(created by IS_CreateBuffer()), and creates a linked list
** Returns: TRUE if successful
*/
{
	unsigned short len, count, index = 4, buflen = ntohs(*((unsigned short*)&buffer[0])), total = ntohs(*((unsigned short*)&buffer[2]));

	if (type == BUFFER_TYPE_ACCESS)
		index = 0;

	switch (type)
	{
		case BUFFER_TYPE_ACCOUNT:
		{
			AccountList aData, *accthead = (AccountList*)listhead;

			for (count = 0; count < total; count++)
			/* Loop through buffer and get information */
			{
				/* Get username */
				GetShort(buffer,index,&len); len = ntohs(len); index += SIZE_SHORT;
				memcpy(aData.username,&buffer[index],len); aData.username[len] = 0; index += len;

				/* Hostmask */
				GetShort(buffer,index,&len); len = ntohs(len); index += SIZE_SHORT;
				memcpy(aData.hostmask,&buffer[index],len); aData.hostmask[len] = 0; index += len;

				/* Password */
				GetShort(buffer,index,&len); len = ntohs(len); index += SIZE_SHORT;
				memcpy(aData.password,&buffer[index],len); aData.password[len] = 0; index += len;

				/* Level */
				aData.level = buffer[index++];

				/* And logins */
				GetShort(buffer,index,&aData.logins); aData.logins = ntohs(aData.logins); index += SIZE_SHORT;

				Account_Add(accthead,&aData);
			}
		}
		break;
		case BUFFER_TYPE_BAN:
		{
			BanList *banhead = (BanList*)listhead;
			BOOL bException, bNetworkBan;
			char exptype, reason[512], hostmask[256];
			unsigned long expire;

			for (count = 0; count < total; count++)
			/* Loop through buffer and get information */
			{
				bNetworkBan = FALSE;
				expire = 0;
				exptype = 0;
				reason[0] = 0;

				/* This an exception or a ban? */
				bException = (BOOL)buffer[index++];

				/* Get hostmask */
				GetShort(buffer,index,&len); len = ntohs(len); index += SIZE_SHORT;
				memcpy(hostmask,&buffer[index],len); hostmask[len] = 0; index += len;

				if (!bException)
				/* Its a ban, get remaining information */
				{
					bNetworkBan = (BOOL)buffer[index++];

					GetShort(buffer,index,&len); len = ntohs(len); index += SIZE_SHORT;

					if (len)
					/* There is a reason for this ban */
					{
						memcpy(&reason[0],&buffer[index],len); reason[len] = 0; index += len;
					}

					/* Get expiration time & type */
					GetLong(buffer,index,&expire); expire = ntohl(expire); index += SIZE_LONG;
					exptype = buffer[index++];
				}

				Banlist_Add(banhead,hostmask,reason[0] == 0 ? NULL : reason,bNetworkBan,bException, expire, exptype);
			}
		}
		break;
		case BUFFER_TYPE_RCL:
		{
			RegChanList RCLData, *RCLHead = (RegChanList*)listhead;
			AccessList aData;
			char reason[512], topic[512], onjoin[512], onpart[512];
			unsigned short count1, atotal;

			for (count = 0; count < total; count++)
			/* Loop through buffer and get information */
			{
				memset(&RCLData,0,sizeof(RegChanList));
				topic[0] = 0; onjoin[0] = 0; onpart[0] = 0;
				RCLData.topic = topic; RCLData.onjoin = onjoin; RCLData.onpart = onpart;

				/* Get channel name */
				GetShort(buffer,index,&len); len = ntohs(len); index += SIZE_SHORT;
				memcpy(RCLData.name,&buffer[index],len); RCLData.name[len] = 0; index += len;

				/* Subject */
				GetShort(buffer,index,&len); len = ntohs(len); index += SIZE_SHORT;
				if (len)
				{
					memcpy(RCLData.subject,&buffer[index],len); RCLData.subject[len] = 0; index += len;
				}

				/* Topic */
				GetShort(buffer,index,&len); len = ntohs(len); index += SIZE_SHORT;
				if (len)
				{
					memcpy(RCLData.topic,&buffer[index],len); RCLData.topic[len] = 0; index += len;
				}

				/* Onjoin */
				GetShort(buffer,index,&len); len = ntohs(len); index += SIZE_SHORT;
				if (len)
				{
					memcpy(RCLData.onjoin,&buffer[index],len); RCLData.onjoin[len] = 0; index += len;
				}

				/* Onpart */
				GetShort(buffer,index,&len); len = ntohs(len); index += SIZE_SHORT;
				if (len)
				{
					memcpy(RCLData.onpart,&buffer[index],len); RCLData.onpart[len] = 0; index += len;
				}

				/* Ownerkey */
				GetShort(buffer,index,&len); len = ntohs(len); index += SIZE_SHORT;
				if (len)
				{
					memcpy(RCLData.ownerkey,&buffer[index],len); RCLData.ownerkey[len] = 0; index += len;
				}

				/* Hostkey */
				GetShort(buffer,index,&len); len = ntohs(len); index += SIZE_SHORT;
				if (len)
				{
					memcpy(RCLData.hostkey,&buffer[index],len); RCLData.hostkey[len] = 0; index += len;
				}

				/* Memberkey */
				GetShort(buffer,index,&len); len = ntohs(len); index += SIZE_SHORT;
				if (len)
				{
					memcpy(RCLData.memberkey,&buffer[index],len); RCLData.memberkey[len] = 0; index += len;
				}

				/* Channel visibility */
				RCLData.visibility = buffer[index++];

				/* Limit */
				GetLong(buffer,index,&RCLData.limit); RCLData.limit = ntohl(RCLData.limit); index += SIZE_LONG;

				/* And mode flag */
				RCLData.modeflags = buffer[index++];

				/* Access entries */
				GetShort(buffer,index,&atotal); atotal = htons(atotal); index += SIZE_SHORT;
				aData.reason = reason;

				for (count1 = 0; count1 < atotal; count1++)
				/* Channel has permanent access entries */
				{
					reason[0] = 0;
					aData.expire = 0;
					aData.exptype = 0;

					/* Get hostmask */
					GetShort(buffer,index,&len); len = ntohs(len); index += SIZE_SHORT;
					memcpy(aData.hostmask,&buffer[index],len); aData.hostmask[len] = 0; index += len;

					/* Type */
					aData.type = buffer[index++];
                    
					/* Reason */
					GetShort(buffer,index,&len); len = ntohs(len); index += SIZE_SHORT;
					if (len)
					{
						memcpy(aData.reason,&buffer[index],len); aData.reason[len] = 0; index += len;
					}

					/* And expiration info */
					GetLong(buffer,index,&aData.expire); aData.expire = ntohl(aData.expire); index += SIZE_LONG;
					aData.exptype = buffer[index++];

					/* Add access entry to channel */
					RCL_AddAccess(&RCLData,&aData);
				}

				/* Add channel to list */
				RCL_Add(RCLHead,&RCLData);
			}
		}
		break;
		case BUFFER_TYPE_SERVER:
		{
			ServerList sData, *serverhead = (ServerList*)listhead;
			unsigned short count;

			for (count = 0; count < total; count++)
			/* Get information from buffer */
			{
				memset(&sData,0,sizeof(ServerList));

				/* Get name */
				GetShort(buffer,index,&len); len = ntohs(len); index += SIZE_SHORT;
				memcpy(sData.name,&buffer[index],len); sData.name[len] = 0; index += len;

				/* Hostmask */
				GetShort(buffer,index,&len); len = ntohs(len); index += SIZE_SHORT;
				memcpy(sData.hostmask,&buffer[index],len); sData.hostmask[len] = 0; index += len;

				/* Password */
				GetShort(buffer,index,&len); len = ntohs(len); index += SIZE_SHORT;
				memcpy(sData.password,&buffer[index],len); sData.password[len] = 0; index += len;

				/* Encryption */
				sData.encryption = buffer[index++];

				/* ...And port */
				GetShort(buffer,index,&sData.port); sData.port = ntohs(sData.port); index += SIZE_SHORT;

				if (!sData.port)
				/* Inbound server */
				{
					GetShort(buffer,index,&sData.ping_frequency); sData.ping_frequency = ntohs(sData.ping_frequency); index += SIZE_SHORT;
					GetShort(buffer,index,&sData.ping_response); sData.ping_response = ntohs(sData.ping_response); index += SIZE_SHORT;
				}

				Server_Add(serverhead,&sData);
			}
		}
		break;
		case BUFFER_TYPE_SPOOFED:
		{
			SpoofedList sData, *curr, *spoofedhead = (SpoofedList*)listhead;
			SpoofedLoginLog slData;
			unsigned short port, count, count1, subtotal;
			char hostmask[256], server[128];

			for (count = 0; count < total; count++)
			/* Get spoofed hosts & login information */
			{
				memset(&sData,0,sizeof(SpoofedLoginLog));

				/* Hostname */
				GetShort(buffer,index,&len); len = ntohs(len); index += SIZE_SHORT;
				memcpy(sData.hostname,&buffer[index],len); sData.hostname[len] = 0; index += len;

				/* # of ports */
				GetShort(buffer,index,&subtotal); subtotal = ntohs(subtotal); index += SIZE_SHORT;

				for (count1 = 0; count1 < subtotal; count1++)
				/* Get ports */
				{
					GetShort(buffer,index,&port); port = ntohs(port); index += SIZE_SHORT;
					curr = Spoofed_Add(spoofedhead,&sData);
				}

				/* # of login attempts */
				GetShort(buffer,index,&subtotal); subtotal = ntohs(subtotal); index += SIZE_SHORT;

				for (count1 = 0; count1 < subtotal; count1++)
				/* Login attempts */
				{
					memset(&slData,0,sizeof(SpoofedLoginLog));
					slData.hostmask = hostmask;
					slData.server = server;
					slData.index = 0;
					slData.port = 0;
					slData.time = 0;

					/* Logged hostmask */
					GetShort(buffer,index,&len); len = ntohs(len); index += SIZE_SHORT;
					memcpy(slData.hostmask,&buffer[index],len); slData.hostmask[len] = 0; index += len;

					/* Server name */
					GetShort(buffer,index,&len); len = ntohs(len); index += SIZE_SHORT;
					memcpy(slData.server,&buffer[index],len); slData.server[len] = 0; index += len;

					/* Server port */
					GetShort(buffer,index,&slData.port); slData.port = ntohs(slData.port); index += SIZE_SHORT;

					/* And calendar time of login */
					GetLong(buffer,index,&slData.time); slData.time = ntohl(slData.time); index += SIZE_LONG;

					/* TODO: Possible bug here? should it be spoofedhead instead of curr? */
					SpoofedLogin_Add(curr,slData.hostmask,slData.server,slData.port,slData.time);
				}
			}
		}
		break;
		case BUFFER_TYPE_FILTER:
		{
			FilterList *filterhead = (FilterList*)listhead;
			unsigned short count;
			char word[128];
			unsigned char type;

			for (count = 0; count < total; count++)
			/* Get all filters */
			{
				/* Get word */
				GetShort(buffer,index,&len); len = ntohs(len); index += SIZE_SHORT;
				memcpy(word,&buffer[index],len); word[len] = 0; index += len;

				/* And type */
				type = buffer[index++];

				Filter_Add(filterhead,word,type);
			}
		}
		break;
		case BUFFER_TYPE_ACCESS:
		/* NOTE: RCL we're working with is passed as the list head */
		{
			AccessList aData;
			RegChanList *RCLTarget = (RegChanList*)listhead;
			unsigned short count, atotal;
			char reason[128];

			aData.reason = reason;

			/* Length of access entries(We can skip this) */
			index += SIZE_SHORT;

			/* Number of access entries */
			GetShort(buffer,index,&atotal); atotal = htons(atotal); index += SIZE_SHORT;

			/* Clear old list */
			while (RCLTarget->accesshead.next)
				RCL_DeleteAccess(RCLTarget,RCLTarget->accesshead.next);

			for (count = 0; count < atotal; count++)
			/* Get access entries from buffer */
			{
				reason[0] = 0;
				aData.expire = 0;
				aData.exptype = 0;

				/* Get hostmask */
				GetShort(buffer,index,&len); len = ntohs(len); index += SIZE_SHORT;
				memcpy(aData.hostmask,&buffer[index],len); aData.hostmask[len] = 0; index += len;

				/* Type */
				aData.type = buffer[index++];
                
				/* Reason */
				GetShort(buffer,index,&len); len = ntohs(len); index += SIZE_SHORT;
				if (len)
				{
					memcpy(aData.reason,&buffer[index],len); aData.reason[len] = 0; index += len;
				}

				/* And expiration info */
				GetLong(buffer,index,&aData.expire); aData.expire = ntohl(aData.expire); index += SIZE_LONG;
				aData.exptype = buffer[index++];

				/* Add access entry to channel */
				RCL_AddAccess(RCLTarget,&aData);
			}
		}
		break;

	}

	return TRUE;
}

void IS_CreateBlank(LPINFOSTRUCTSETTINGS is)
/*
** IS_CreateBlank()
** Creates a blank settings information structure
*/
{
	memset(is,0,sizeof(INFOSTRUCTSETTINGS));

	/* Channel information */
	is->isChannel.defaultlimit = 50;

	/* General information */
	BindingBuf_Add(&is->isGeneral.bindingbuf,&is->isGeneral.bindingnum,"0.0.0.0",6668);
	strcpy(is->isGeneral.servername,"RockIRCX_Default");
	is->isGeneral.serverid = 1;
	strcpy(is->isGeneral.serverdescription,"RockIRCX default server description");
	strcpy(is->isGeneral.networkname,"my.network.rocks");
	strcpy(is->isGeneral.adminloc1,"Default admin location #1");
	strcpy(is->isGeneral.adminloc2,"Default admin location #2");
	strcpy(is->isGeneral.adminemail,"IDidntChangeMyDefaultEmail@MySettings.com");

	/* I/O control */
	is->isIOControl.listeningsockets = 1;
	is->isIOControl.totaldescriptors = 2;

	/* Security settings */
	is->isSecurity.RAPort = 6932;
	strcpy(is->isSecurity.RAPassword,"default");
	is->isSecurity.max_access = 30;
	is->isSecurity.max_chanlen = 64;
	is->isSecurity.max_msglen = 512;
	is->isSecurity.max_nicklen = 32;
	is->isSecurity.max_topiclen = 384;

	/* Status settings */
	is->isStatus.localmax = 1000;
	is->isStatus.globalmax = 5000;
	is->isStatus.status = SETTINGS_STATUS_RUNNING;
	is->isStatus.uptime = 0;

	/* User settings */
	is->isUser.max_perip = 3;
	is->isUser.max_recvq = 1024;
	is->isUser.max_sendq = 1024 * 64;
	is->isUser.maxchannels = 10;
	is->isUser.nickdelay = 90;
	is->isUser.ping_duration = 90;
	is->isUser.ping_response = 90;

}

unsigned int IS_SaveToBuffer(char **buffer, LPINFOSTRUCTSETTINGS is)
/*
** IS_SaveToFile()
** Saves the specified settings structure to the specified file
** Returns: FALSE if failed, TRUE if successful
*/
{
	char tempbuf[65535], *filebuf = NULL;
	unsigned short len_curr = 0;
	unsigned long t_index, index = 0, len_total = 0;

	/* Account information */
	if (is->isAccounts.acctbuffer)
	/* Copy account buffer */
	{
		GetShort(is->isAccounts.acctbuffer,0,&len_curr); len_curr = ntohs(len_curr);
		len_total += len_curr += SIZE_SHORT;
		filebuf = (char*)realloc(filebuf,len_total);
		memcpy(filebuf,is->isAccounts.acctbuffer,len_curr); index += len_curr;
	}
	else
	/* No accounts, write 2 NUL bytes at beginning of file */
	{
		len_total += SIZE_SHORT;
		filebuf = (char*)realloc(filebuf,len_total);
		InsertShort(filebuf,index,0); index += SIZE_SHORT;
	}

	/* Ban information */
	if (is->isBans.banbuffer)
	/* Copy ban buffer */
	{
		GetShort(is->isBans.banbuffer,0,&len_curr); len_curr = ntohs(len_curr);
		len_total += len_curr += SIZE_SHORT;
		filebuf = (char*)realloc(filebuf,len_total);
		memcpy(&filebuf[index],is->isBans.banbuffer,len_curr); index += len_curr;
	}
	else
	/* No bans, write 2 NUL bytes */
	{
		len_total += SIZE_SHORT;
		filebuf = (char*)realloc(filebuf,len_total);
		InsertShort(filebuf,index,0); index += SIZE_SHORT;
	}

	/* Channel info */
	len_total += SIZE_LONG + 1;
	filebuf = (char*)realloc(filebuf,len_total);
	InsertLong(filebuf,index,htonl(is->isChannel.defaultlimit)); index += SIZE_LONG;
	filebuf[index++] = is->isChannel.modeflag;

	if (is->isChannel.RCLbuffer)
	/* Copy RCL buffer */
	{
		GetShort(is->isChannel.RCLbuffer,0,&len_curr); len_curr = ntohs(len_curr);
		len_total += len_curr += SIZE_SHORT;
		filebuf = (char*)realloc(filebuf,len_total);
		memcpy(&filebuf[index],is->isChannel.RCLbuffer,len_curr); index += len_curr;
	}
	else
	/* No registered channels, write 2 NUL bytes */
	{
		len_total += SIZE_SHORT;
		filebuf = (char*)realloc(filebuf,len_total);
		InsertShort(filebuf,index,0); index += SIZE_SHORT;
	}

	/* Filtering enabled? */
	len_total += sizeof(char);
	filebuf = (char*)realloc(filebuf,len_total);
	filebuf[index++] = is->isFilter.enabled;

	/* Filter information */
	if (is->isFilter.filterbuffer)
	/* Copy filter buffer */
	{
		GetShort(is->isFilter.filterbuffer,0,&len_curr); len_curr = ntohs(len_curr);
		len_total += len_curr += SIZE_SHORT;
		filebuf = (char*)realloc(filebuf,len_total);
		memcpy(&filebuf[index],is->isFilter.filterbuffer,len_curr); index += len_curr;
	}
	else
	/* No filtered words, write 2 NUL bytes */
	{
		len_total += SIZE_SHORT;
		filebuf = (char*)realloc(filebuf,len_total);
		InsertShort(filebuf,index,0); index += SIZE_SHORT;
	}

	/* General information */
	memset(tempbuf,0,sizeof(tempbuf));
	t_index = 0;

	/* Network name */
	len_curr = (unsigned short)strlen(is->isGeneral.networkname);
	InsertShort(tempbuf,t_index,htons(len_curr)); t_index += SIZE_SHORT;
	memcpy(&tempbuf[t_index],is->isGeneral.networkname,len_curr); t_index += len_curr;

	/* Server name */
	len_curr = (unsigned short)strlen(is->isGeneral.servername);
	InsertShort(tempbuf,t_index,htons(len_curr)); t_index += SIZE_SHORT;
	memcpy(&tempbuf[t_index],is->isGeneral.servername,len_curr); t_index += len_curr;

	/* Server ID # */
	InsertLong(tempbuf,t_index,htonl(is->isGeneral.serverid)); t_index += SIZE_LONG;

	/* Server description */
	len_curr = (unsigned short)strlen(is->isGeneral.serverdescription);
	InsertShort(tempbuf,t_index,htons(len_curr)); t_index += SIZE_SHORT;
	memcpy(&tempbuf[t_index],is->isGeneral.serverdescription,len_curr); t_index += len_curr;

	/* Binding buffer */
	len_curr = is->isGeneral.bindingnum * 6;
	InsertShort(tempbuf,t_index,htons(is->isGeneral.bindingnum)); t_index += SIZE_SHORT;
	memcpy(&tempbuf[t_index],is->isGeneral.bindingbuf,len_curr); t_index += len_curr;

	/* Admin location #1 */
	len_curr = (unsigned short)strlen(is->isGeneral.adminloc1);
	InsertShort(tempbuf,t_index,htons(len_curr)); t_index += SIZE_SHORT;
	memcpy(&tempbuf[t_index],is->isGeneral.adminloc1,len_curr); t_index += len_curr;

	/* Admin location #2 */
	len_curr = (unsigned short)strlen(is->isGeneral.adminloc2);
	InsertShort(tempbuf,t_index,htons(len_curr)); t_index += SIZE_SHORT;
	memcpy(&tempbuf[t_index],is->isGeneral.adminloc2,len_curr); t_index += len_curr;

	/* Admin email address */
	len_curr = (unsigned short)strlen(is->isGeneral.adminemail);
	InsertShort(tempbuf,t_index,htons(len_curr)); t_index += SIZE_SHORT;
	memcpy(&tempbuf[t_index],is->isGeneral.adminemail,len_curr); t_index += len_curr;

	/* DNS lookups enabled? */
	tempbuf[t_index++] = (char)is->isGeneral.dns_lookup;

	/* Cache lookups? DNS cache time & type */
	tempbuf[t_index++] = (char)is->isGeneral.dns_cache;
	InsertLong(tempbuf,t_index,htonl(is->isGeneral.dns_cachetime)); t_index += SIZE_LONG;
	tempbuf[t_index++] = (char)is->isGeneral.dns_cachetimeduration;

	/* IP/DNS mask type */
	tempbuf[t_index++] = (char)is->isGeneral.dns_masktype;

	/* Copy over temporary buffer before MOTD buffer */
	len_total += t_index;
	filebuf = (char*)realloc(filebuf,len_total);
	memcpy(&filebuf[index],tempbuf,t_index); index += t_index;

	if (is->isGeneral.motdbuffer)
	/* Insert MOTD
	#1 Get length of MOTD
	#2 Add length of MOTD + length of a short int(for size of MOTD) to total length
	#3 Re-allocate file buffer
	#4 Insert length of MOTD(in network byte-order) before MOTD itself
	#5 Insert MOTD
	*/
	{
		len_curr = (unsigned short)strlen(is->isGeneral.motdbuffer);
		len_total += len_curr + SIZE_SHORT;
		filebuf = (char*)realloc(filebuf,len_total);
		InsertShort(filebuf,index,htons(len_curr)); index += SIZE_SHORT;
		memcpy(&filebuf[index],is->isGeneral.motdbuffer,len_curr); index += len_curr;
	}
	else
	/* No MOTD, insert 2 NUL's */
	{
		len_total += SIZE_SHORT;
		filebuf = (char*)realloc(filebuf,len_total);
		InsertShort(filebuf,index,0); index += SIZE_SHORT;
	}

	/* 
	** I/O control stuff
	**
	** Total buffer expansion size: (SIZE_LONG * 4) + SIZE_SHORT
	**
	** Total number of items to be inserted: 5
	**
	** Buffer items:
	** #1 - Total server data recieved (long int)
	** #2 - Total server data sent (long int)
	** #3 - Number of listening sockets (short int)
	** #4 - Total number of connections (long int)
	** #5 - Total number of socket descriptors in use by program (long int)
	*/
	len_total += (SIZE_LONG * 4) + SIZE_SHORT;
	filebuf = (char*)realloc(filebuf,len_total);

	InsertLong(filebuf,index,htonl(is->isIOControl.datarecvd)); index += SIZE_LONG;
	InsertLong(filebuf,index,htonl(is->isIOControl.datasent)); index += SIZE_LONG;
	InsertShort(filebuf,index,htons(is->isIOControl.listeningsockets)); index += SIZE_SHORT;
	InsertLong(filebuf,index,htonl(is->isIOControl.totalconnections)); index += SIZE_LONG;
	InsertLong(filebuf,index,htonl(is->isIOControl.totaldescriptors)); index += SIZE_LONG;

	/*
	** Security
	**
	** Total buffer expansion size: (sizeof(char) * 4) + (SIZE_SHORT * 5)
	** + RA password + length of spoofed host buffer
	**
	** Total number of items to be inserted: 10
	** Buffer items:
	** #1 - Prevent nickname changes in channel? (char)
	** #2 - Show PART's as QUIT's? (char)
	** #3 - Maximum access entries per channel (short int)
	** #4 - Spoof protection flag (char)
	** #5 - Consequence for spoofed hosts (char)
	** #6 - Maximum nickname length (short int)
	** #7 - Maximum channel name length (short int)
	** #8 - Maximum topic length (short int)
	** #9 - Maximum message length (short int)
	** #10 - Remote administration port(short int)
	** #11 - Remote administration password
	** #12 - Spoofed host buffer
	*/
	memset(tempbuf,0,sizeof(tempbuf));
	t_index = 0;

	tempbuf[t_index++] = (char)is->isSecurity.preventnickchginchan;
	tempbuf[t_index++] = (char)is->isSecurity.showquitsasparts;
	InsertShort(tempbuf,t_index,htons(is->isSecurity.max_access)); t_index += SIZE_SHORT;
	tempbuf[t_index++] = is->isSecurity.spoofflag;
	tempbuf[t_index++] = is->isSecurity.spoofconsequence;
	InsertShort(tempbuf,t_index,htons(is->isSecurity.max_nicklen)); t_index += SIZE_SHORT;
	InsertShort(tempbuf,t_index,htons(is->isSecurity.max_chanlen)); t_index += SIZE_SHORT;
	InsertShort(tempbuf,t_index,htons(is->isSecurity.max_topiclen)); t_index += SIZE_SHORT;
	InsertShort(tempbuf,t_index,htons(is->isSecurity.max_msglen)); t_index += SIZE_SHORT;
	InsertShort(tempbuf,t_index,htons(is->isSecurity.RAPort)); t_index += SIZE_SHORT;

	/* Remote administration password */
	len_curr = (unsigned short)strlen(is->isSecurity.RAPassword);
	InsertShort(tempbuf,t_index,htons(len_curr)); t_index += SIZE_SHORT;
	memcpy(&tempbuf[t_index],is->isSecurity.RAPassword,len_curr); t_index += len_curr;

	/* Copy over temporary buffer before spoofed buffer */
	len_total += t_index;
	filebuf = (char*)realloc(filebuf,len_total);
	memcpy(&filebuf[index],tempbuf,t_index); index += t_index;

	if (is->isSecurity.spoofedbuffer)
	/* Spoofed hosts entries */
	{
		GetShort(is->isSecurity.spoofedbuffer,0,&len_curr); len_curr = ntohs(len_curr);
		len_total += len_curr += SIZE_SHORT;
		filebuf = (char*)realloc(filebuf,len_total);
		memcpy(&filebuf[index],is->isSecurity.spoofedbuffer,len_curr); index += len_curr;
	}
	else
	/* No spoofed hosts, use 2 NUL's */
	{
		len_total += SIZE_SHORT;
		filebuf = (char*)realloc(filebuf,len_total);
		InsertShort(filebuf,index,0); index += SIZE_SHORT;
	}

	/*
	** Server linkage information
	**
	** Total buffer expansion size: size of server buffer
	**
	** Total items to be inserted: 2
	**
	** Buffer items
	** #1 - Type of network
	** #2 - Server link buffer
	*/

	len_total++;
	filebuf = (char*)realloc(filebuf,len_total);
	filebuf[index++] = is->isServers.networktype;

	if (is->isServers.serverbuffer)
	/* Copy server buffer */
	{
		GetShort(is->isServers.serverbuffer,0,&len_curr); len_curr = ntohs(len_curr);
		len_total += len_curr += SIZE_SHORT;
		filebuf = (char*)realloc(filebuf,len_total);
		memcpy(&filebuf[index],is->isServers.serverbuffer,len_curr); index += len_curr;
	}
	else
	/* No server linkage information, write 2 NUL bytes */
	{
		len_total += SIZE_SHORT;
		filebuf = (char*)realloc(filebuf,len_total);
		InsertShort(filebuf,index,0); index += SIZE_SHORT;
	}

	/*
	** Status information
	**
	** Total buffer expansion size: (SIZE_LONG * 9) + (SIZE_SHORT * 3) + sizeof(char)
	**
	** Total items to be inserted: 13
	**
	** Buffer items:
	** #1 - Local users (long)
	** #2 - Maximum local users (long)
	** #3 - Global users (long)
	** #4 - Maximum global users (long)
	** #5 - Status state (char)
	** #6 - Server uptime (long)
	** #7 - Administrators logged in (short)
	** #8 - IRCops logged in (short)
	** #9 - Power users logged in (short)
	** #10 - Local dynamic channels (long)
	** #11 - Global dynamic channels (long)
	** #12 - Local registered channels (long)
	** #13 - Global registered channels (long)
	*/
	memset(tempbuf,0,sizeof(tempbuf));
	t_index = 0;

#ifndef APP_ROCKIRCXRA

	LINKED_LIST_STRUCT *llPtr;
	CLIENT_STRUCT *client;

	is->isStatus.localusers = scs.lusers.nLocalUsers;
	is->isStatus.globalusers = scs.lusers.nGlobalUsers;
	is->isStatus.localchannels = scs.lusers.nChannels;
	is->isStatus.globalchannels = scs.lusers.nChannels;
	
	llPtr = &(scs.llUserHead[1]);

	is->isStatus.admins = 0;
	is->isStatus.ircops = 0;
	is->isStatus.powerusers = 0;

	while (llPtr->next)
	{
		llPtr = llPtr->next;

		client = (CLIENT_STRUCT*)llPtr->data;

		if (client->flags & CLIENT_FLAG_ADMIN)
			is->isStatus.admins++;
		else if (client->flags & CLIENT_FLAG_SYSOP)
			is->isStatus.ircops++;
		else if (client->flags & CLIENT_FLAG_POWERUSER)
			is->isStatus.powerusers++;
	}

#endif /* APP_ROCKIRCXRA */

	InsertLong(tempbuf,t_index,htonl(is->isStatus.localusers)); t_index += SIZE_LONG;
	InsertLong(tempbuf,t_index,htonl(is->isStatus.localmax)); t_index += SIZE_LONG;
	InsertLong(tempbuf,t_index,htonl(is->isStatus.globalusers)); t_index += SIZE_LONG;
	InsertLong(tempbuf,t_index,htonl(is->isStatus.globalmax)); t_index += SIZE_LONG;
	tempbuf[t_index++] = is->isStatus.status;
	InsertLong(tempbuf,t_index,htonl(is->isStatus.uptime)); t_index += SIZE_LONG;
	InsertShort(tempbuf,t_index,htons(is->isStatus.admins)); t_index += SIZE_SHORT;
	InsertShort(tempbuf,t_index,htons(is->isStatus.ircops)); t_index += SIZE_SHORT;
	InsertShort(tempbuf,t_index,htons(is->isStatus.powerusers)); t_index += SIZE_SHORT;

	InsertLong(tempbuf,t_index,htonl(is->isStatus.localchannels)); t_index += SIZE_LONG;
	InsertLong(tempbuf,t_index,htonl(is->isStatus.globalchannels)); t_index += SIZE_LONG;
	InsertLong(tempbuf,t_index,htonl(is->isStatus.localregchannels)); t_index += SIZE_LONG;
	InsertLong(tempbuf,t_index,htonl(is->isStatus.globalregchannels)); t_index += SIZE_LONG;

	/* Copy over temporary buffer */
	len_total += t_index;
	filebuf = (char*)realloc(filebuf,len_total);
	memcpy(&filebuf[index],tempbuf,t_index); index += t_index;

	/*
	** User information
	**
	** Total buffer expansion size: (sizeof(char) * 2) + (SIZE_SHORT * 6) + (SIZE_LONG * 3)
	** + length of autojoin data string
	**
	** Total items to be inserted: 12
	**
	** Buffer items:
	** #1 - Can users create dynamic channels? (char)
	** #2 - Can users join dynamic channels? (char)
	** #3 - Maximum channels per user (short)
	** #4 - Length of autojoin data(0 if no autojoin data) (short)
	** #5 - Autojoin data (string)
	** #6 - Maximum users per IP address (short)
	** #7 - Maximum RecvQ size (long)
	** #8 - Maximum SendQ size (long)
	** #9 - Ping duration (short)
	** #10 - Ping response time (short)
	** #11 - Minimum delay time per nickname change, in seconds (short)
	** #12 - Forced default message delay, in miliseconds (long)
	*/

	memset(tempbuf,0,sizeof(tempbuf));
	t_index = 0;

	tempbuf[t_index++] = (char)is->isUser.dyn_create;
	tempbuf[t_index++] = (char)is->isUser.dyn_join;
	InsertLong(tempbuf,t_index,htonl(is->isUser.maxchannels)); t_index += SIZE_LONG;

	if (is->isUser.autojoinbuffer)
	{
		len_curr = (unsigned short)strlen(is->isUser.autojoinbuffer);
		InsertShort(tempbuf,t_index,htons(len_curr)); t_index += SIZE_SHORT;
		memcpy(&tempbuf[t_index],is->isUser.autojoinbuffer,len_curr); t_index += len_curr;
	}
	else
	{
		InsertShort(tempbuf,t_index,0); t_index += SIZE_SHORT;
	}

	InsertShort(tempbuf,t_index,htons(is->isUser.max_perip)); t_index += SIZE_SHORT;
	InsertLong(tempbuf,t_index,htonl(is->isUser.max_recvq)); t_index += SIZE_LONG;
	InsertLong(tempbuf,t_index,htonl(is->isUser.max_sendq)); t_index += SIZE_LONG;
	InsertShort(tempbuf,t_index,htons(is->isUser.ping_duration)); t_index += SIZE_SHORT;
	InsertShort(tempbuf,t_index,htons(is->isUser.ping_response)); t_index += SIZE_SHORT;
	InsertShort(tempbuf,t_index,htons(is->isUser.nickdelay)); t_index += SIZE_SHORT;
	InsertLong(tempbuf,t_index,htonl(is->isUser.msgdelay)); t_index += SIZE_LONG;

	/* Copy over temporary buffer */
	len_total += t_index;
	filebuf = (char*)realloc(filebuf,len_total);
	memcpy(&filebuf[index],tempbuf,t_index); index += t_index;

	/* Assign referenced buffer to new buffer */
	*buffer = filebuf;
	return index;
}

BOOL IS_LoadFromBuffer(char *buffer, LPINFOSTRUCTSETTINGS is)
/*
** IS_LoadFromBuffer()
** Loads the specified buffer and fills in the specified settings structure
** Returns: FALSE if failed, TRUE if successful
*/
{
	unsigned long index = 0;
	unsigned short len_curr;

	/* Start with no info */
	memset(is,0,sizeof(*is));

	/* Get length of account information */
	GetShort(buffer,index,&len_curr); len_curr = ntohs(len_curr); index += SIZE_SHORT;

	if (len_curr)
	/* Copy over account buffer & create list of accounts */
	{
		len_curr += SIZE_SHORT;
		is->isAccounts.acctbuffer = (char*)malloc(len_curr);
		memcpy(is->isAccounts.acctbuffer,&buffer[index - SIZE_SHORT],len_curr);
		index += len_curr - SIZE_SHORT;

		IS_CreateList(&is->isAccounts.accthead,is->isAccounts.acctbuffer,BUFFER_TYPE_ACCOUNT);
	}

	/* Get length of ban information */
	GetShort(buffer,index,&len_curr); len_curr = ntohs(len_curr); index += SIZE_SHORT;

	if (len_curr)
	/* Copy over ban buffer & create list of bans/exceptions */
	{
		len_curr += SIZE_SHORT;
		is->isBans.banbuffer = (char*)malloc(len_curr);
		memcpy(is->isBans.banbuffer,&buffer[index - SIZE_SHORT],len_curr);
		index += len_curr - SIZE_SHORT;

		IS_CreateList(&is->isBans.banhead,is->isBans.banbuffer,BUFFER_TYPE_BAN);
	}

	/* Copy channel info */
	GetLong(buffer,index,&is->isChannel.defaultlimit); is->isChannel.defaultlimit = ntohl(is->isChannel.defaultlimit); index += SIZE_LONG;
	is->isChannel.modeflag = buffer[index++];

	/* Get length of RCL information */
	GetShort(buffer,index,&len_curr); len_curr = ntohs(len_curr); index += SIZE_SHORT;

	if (len_curr)
	/* Buffer contains registered channels */
	{
		len_curr += SIZE_SHORT;
		is->isChannel.RCLbuffer = (char*)malloc(len_curr);
		memcpy(is->isChannel.RCLbuffer,&buffer[index - SIZE_SHORT],len_curr);
		index += len_curr - SIZE_SHORT;

		IS_CreateList(&is->isChannel.RCLHead,is->isChannel.RCLbuffer,BUFFER_TYPE_RCL);
	}

	/* Filtering enabled? */
	is->isFilter.enabled = buffer[index++];

	/* Get length of filter information */
	GetShort(buffer,index,&len_curr); len_curr = ntohs(len_curr); index += SIZE_SHORT;

	if (len_curr)
	/* Buffer contains filtered words */
	{
		len_curr += SIZE_SHORT;
		is->isFilter.filterbuffer = (char*)malloc(len_curr);
		memcpy(is->isFilter.filterbuffer,&buffer[index - SIZE_SHORT],len_curr);
		index += len_curr - SIZE_SHORT;

		IS_CreateList(&is->isFilter.filterhead,is->isFilter.filterbuffer,BUFFER_TYPE_FILTER);
	}

	/* Network name */
	GetShort(buffer,index,&len_curr); len_curr = ntohs(len_curr); index += SIZE_SHORT;

	if (len_curr)
	{
		lstrcpyn(is->isGeneral.networkname,&buffer[index],len_curr + 1); index += len_curr;
	}

	/* Server name */
	GetShort(buffer,index,&len_curr); len_curr = ntohs(len_curr); index += SIZE_SHORT;

	if (len_curr)
	{
		lstrcpyn(is->isGeneral.servername,&buffer[index],len_curr + 1); index += len_curr;
	}

	/* Get server ID # */
	GetLong(buffer,index,&is->isGeneral.serverid); is->isGeneral.serverid = ntohl(is->isGeneral.serverid); index += SIZE_LONG;

	/* Server description */
	GetShort(buffer,index,&len_curr); len_curr = ntohs(len_curr); index += SIZE_SHORT;

	if (len_curr)
	{
		lstrcpyn(is->isGeneral.serverdescription,&buffer[index],len_curr + 1); index += len_curr;
	}

	/* Binding information */
	GetShort(buffer,index,&is->isGeneral.bindingnum); is->isGeneral.bindingnum = ntohs(is->isGeneral.bindingnum); index += SIZE_SHORT;

	if (is->isGeneral.bindingnum)
	{
		is->isGeneral.bindingbuf = (char*)malloc(is->isGeneral.bindingnum * 6);
		memcpy(is->isGeneral.bindingbuf,&buffer[index],is->isGeneral.bindingnum * 6); index += is->isGeneral.bindingnum * 6;
	}

	/* Admin location #1 */
	GetShort(buffer,index,&len_curr); len_curr = ntohs(len_curr); index += SIZE_SHORT;

	if (len_curr)
	{
		lstrcpyn(is->isGeneral.adminloc1,&buffer[index],len_curr + 1); index += len_curr;
	}

	/* Admin location #2 */
	GetShort(buffer,index,&len_curr); len_curr = ntohs(len_curr); index += SIZE_SHORT;

	if (len_curr)
	{
		lstrcpyn(is->isGeneral.adminloc2,&buffer[index],len_curr + 1); index += len_curr;
	}

	/* Admin email address */
	GetShort(buffer,index,&len_curr); len_curr = ntohs(len_curr); index += SIZE_SHORT;

	if (len_curr)
	{
		lstrcpyn(is->isGeneral.adminemail,&buffer[index],len_curr + 1); index += len_curr;
	}

	/* DNS lookups enabled? */
	is->isGeneral.dns_lookup = (BOOL)buffer[index++];

	/* Cache lookups? DNS cache time & type */
	is->isGeneral.dns_cache = (BOOL)buffer[index++];
	GetLong(buffer,index,&is->isGeneral.dns_cachetime); is->isGeneral.dns_cachetime = ntohl(is->isGeneral.dns_cachetime); index += SIZE_LONG;
	is->isGeneral.dns_cachetimeduration = (BOOL)buffer[index++];

	/* IP/DNS mask type */
	is->isGeneral.dns_masktype = (BOOL)buffer[index++];

	/* Get MOTD buffer */
	GetShort(buffer,index,&len_curr); len_curr = ntohs(len_curr); index += SIZE_SHORT;

	if (len_curr)
	{
		is->isGeneral.motdbuffer = (char*)malloc(len_curr + 1);

		lstrcpyn(is->isGeneral.motdbuffer,&buffer[index],len_curr + 1); index += len_curr;
	}

	/* I/O control information (only load info from server to remote admin side) */

#ifdef APP_ROCKIRCXRA
    GetLong(buffer,index,&is->isIOControl.datarecvd); is->isIOControl.datarecvd = ntohl(is->isIOControl.datarecvd); index += SIZE_LONG;
	GetLong(buffer,index,&is->isIOControl.datasent); is->isIOControl.datasent = ntohl(is->isIOControl.datasent); index += SIZE_LONG;
	GetShort(buffer,index,&is->isIOControl.listeningsockets); is->isIOControl.listeningsockets = ntohs(is->isIOControl.listeningsockets); index += SIZE_SHORT;
	GetLong(buffer,index,&is->isIOControl.totalconnections); is->isIOControl.totalconnections = ntohl(is->isIOControl.totalconnections); index += SIZE_LONG;
	GetLong(buffer,index,&is->isIOControl.totaldescriptors); is->isIOControl.totaldescriptors = ntohl(is->isIOControl.totaldescriptors); index += SIZE_LONG;
#else
	index += (SIZE_LONG * 4);
	index += SIZE_SHORT;

#endif

	/* Security information */

	is->isSecurity.preventnickchginchan = (BOOL)buffer[index++];
	is->isSecurity.showquitsasparts = (BOOL)buffer[index++];
	GetShort(buffer,index,&is->isSecurity.max_access); is->isSecurity.max_access = ntohs(is->isSecurity.max_access); index += SIZE_SHORT;
	is->isSecurity.spoofflag = buffer[index++];
	is->isSecurity.spoofconsequence = buffer[index++];
	GetShort(buffer,index,&is->isSecurity.max_nicklen); is->isSecurity.max_nicklen = ntohs(is->isSecurity.max_nicklen); index += SIZE_SHORT;
	GetShort(buffer,index,&is->isSecurity.max_chanlen); is->isSecurity.max_chanlen = ntohs(is->isSecurity.max_chanlen); index += SIZE_SHORT;
	GetShort(buffer,index,&is->isSecurity.max_topiclen); is->isSecurity.max_topiclen = ntohs(is->isSecurity.max_topiclen); index += SIZE_SHORT;
	GetShort(buffer,index,&is->isSecurity.max_msglen); is->isSecurity.max_msglen = ntohs(is->isSecurity.max_msglen); index += SIZE_SHORT;
	GetShort(buffer,index,&is->isSecurity.RAPort); is->isSecurity.RAPort = ntohs(is->isSecurity.RAPort); index += SIZE_SHORT;

	/* Remote administration password */
	GetShort(buffer,index,&len_curr); len_curr = ntohs(len_curr); index += SIZE_SHORT;

	if (len_curr)
	{
		lstrcpyn(is->isSecurity.RAPassword,&buffer[index],len_curr + 1); index += len_curr;
	}

	/* Spoofed hosts? */
	GetShort(buffer,index,&len_curr); len_curr = ntohs(len_curr); index += SIZE_SHORT;

	if (len_curr)
	{
		len_curr += SIZE_SHORT;
		is->isSecurity.spoofedbuffer = (char*)malloc(len_curr);
		memcpy(is->isSecurity.spoofedbuffer,&buffer[index - SIZE_SHORT],len_curr);
		index += len_curr - SIZE_SHORT;

		IS_CreateList(&is->isSecurity.spoofedhead,is->isSecurity.spoofedbuffer,BUFFER_TYPE_SPOOFED);
	}

	/* Linked servers */
	is->isServers.networktype = buffer[index++];

	GetShort(buffer,index,&len_curr); len_curr = ntohs(len_curr); index += SIZE_SHORT;

	if (len_curr)
	{
		len_curr += SIZE_SHORT;
		is->isServers.serverbuffer = (char*)malloc(len_curr);
		memcpy(is->isServers.serverbuffer,&buffer[index - SIZE_SHORT],len_curr);
		index += len_curr - SIZE_SHORT;

		IS_CreateList(&is->isServers.serverhead,is->isServers.serverbuffer,BUFFER_TYPE_SERVER);
	}

	/* Status information */
	GetLong(buffer,index,&is->isStatus.localusers); is->isStatus.localusers = ntohl(is->isStatus.localusers); index += SIZE_LONG;
	GetLong(buffer,index,&is->isStatus.localmax); is->isStatus.localmax = ntohl(is->isStatus.localmax); index += SIZE_LONG;
	GetLong(buffer,index,&is->isStatus.globalusers); is->isStatus.globalusers = ntohl(is->isStatus.globalusers); index += SIZE_LONG;
	GetLong(buffer,index,&is->isStatus.globalmax); is->isStatus.globalmax = ntohl(is->isStatus.globalmax); index += SIZE_LONG;
	is->isStatus.status = buffer[index++];
	GetLong(buffer,index,&is->isStatus.uptime); is->isStatus.uptime = ntohl(is->isStatus.uptime); index += SIZE_LONG;
	GetShort(buffer,index,&is->isStatus.admins); is->isStatus.admins = ntohs(is->isStatus.admins); index += SIZE_SHORT;
	GetShort(buffer,index,&is->isStatus.ircops); is->isStatus.ircops = ntohs(is->isStatus.ircops); index += SIZE_SHORT;
	GetShort(buffer,index,&is->isStatus.powerusers); is->isStatus.powerusers = ntohs(is->isStatus.powerusers); index += SIZE_SHORT;
	GetLong(buffer,index,&is->isStatus.localchannels); is->isStatus.localchannels = ntohl(is->isStatus.localchannels); index += SIZE_LONG;
	GetLong(buffer,index,&is->isStatus.globalchannels); is->isStatus.globalchannels = ntohl(is->isStatus.globalchannels); index += SIZE_LONG;
	GetLong(buffer,index,&is->isStatus.localregchannels); is->isStatus.localregchannels = ntohl(is->isStatus.localregchannels); index += SIZE_LONG;
	GetLong(buffer,index,&is->isStatus.globalregchannels); is->isStatus.globalregchannels = ntohl(is->isStatus.globalregchannels); index += SIZE_LONG;

	/* User information */
	is->isUser.dyn_create = (BOOL)buffer[index++];
	is->isUser.dyn_join = (BOOL)buffer[index++];
	GetLong(buffer,index,&is->isUser.maxchannels); is->isUser.maxchannels = ntohl(is->isUser.maxchannels); index += SIZE_LONG;

	/* Autojoin channels? */
	GetShort(buffer,index,&len_curr); len_curr = ntohs(len_curr); index += SIZE_SHORT;

	if (len_curr)
	{
		is->isUser.autojoinbuffer = (char*)malloc(len_curr + 1);
		memcpy(is->isUser.autojoinbuffer,&buffer[index],len_curr);
		is->isUser.autojoinbuffer[len_curr] = 0;
		index += len_curr;
	}

	GetShort(buffer,index,&is->isUser.max_perip); is->isUser.max_perip = ntohs(is->isUser.max_perip); index += SIZE_SHORT;
	GetLong(buffer,index,&is->isUser.max_recvq); is->isUser.max_recvq = ntohl(is->isUser.max_recvq); index += SIZE_LONG;
	GetLong(buffer,index,&is->isUser.max_sendq); is->isUser.max_sendq = ntohl(is->isUser.max_sendq); index += SIZE_LONG;
	GetShort(buffer,index,&is->isUser.ping_duration); is->isUser.ping_duration = ntohs(is->isUser.ping_duration); index += SIZE_SHORT;
	GetShort(buffer,index,&is->isUser.ping_response); is->isUser.ping_response = ntohs(is->isUser.ping_response); index += SIZE_SHORT;
	GetShort(buffer,index,&is->isUser.nickdelay); is->isUser.nickdelay = ntohs(is->isUser.nickdelay); index += SIZE_SHORT;
	GetLong(buffer,index,&is->isUser.msgdelay); is->isUser.msgdelay = ntohl(is->isUser.msgdelay); index += SIZE_LONG;

#ifndef APP_ROCKIRCXRA

	/* Add registered channels */
	RegChanList *RCL = &(is->isChannel.RCLHead);
	CHANNEL_STRUCT *chChannel;

	while (RCL->next)
	{
		RCL = RCL->next;

		chChannel = Channel_Find(RCL->name);

		if (!chChannel)
		/* Create channels that don't already exist, with 0 people */
			Channel_Create(RCL->name);
		else if (Channel_ReggedAndEmpty(chChannel))
		/* Channel exists, re-create it since we have new data */
		{
			/* Compensate for the stats subtraction which will occur during Channel_Cleanup() */
			scs.lusers.nChannels++;
			SettingsInfo.isStatus.localchannels++;
			SettingsInfo.isStatus.globalchannels++;

			Channel_Cleanup(chChannel,TRUE);
		}
	}

#endif /* APP_ROCKIRCXRA */

	return TRUE;
}

void IS_FreeBuffers(LPINFOSTRUCTSETTINGS is, BOOL bClearLists)
/*
** IS_FreeBuffers()
** Frees all dynamic buffers in the specified info structure
*/
{
	free(is->isAccounts.acctbuffer);
	free(is->isBans.banbuffer);
	free(is->isChannel.RCLbuffer);
	free(is->isGeneral.bindingbuf);
	free(is->isGeneral.motdbuffer);
	free(is->isServers.serverbuffer);
	free(is->isSecurity.spoofedbuffer);
	free(is->isUser.autojoinbuffer);
	free(is->isFilter.filterbuffer);

	if (bClearLists)
	{
		while (is->isAccounts.accthead.next)
			Account_Delete(&is->isAccounts.accthead,is->isAccounts.accthead.next);

		while (is->isBans.banhead.next)
			Banlist_Delete(&is->isBans.banhead,is->isBans.banhead.next);

		while (is->isChannel.RCLHead.next)
			RCL_Delete(&is->isChannel.RCLHead,is->isChannel.RCLHead.next);

		while (is->isServers.serverhead.next)
			Server_Delete(&is->isServers.serverhead,is->isServers.serverhead.next);

		while (is->isFilter.filterhead.next)
			Filter_Delete(&is->isFilter.filterhead,is->isFilter.filterhead.next->word,is->isFilter.filterhead.next->type);
	}
}

BOOL IS_SaveToFile(const char* filename, LPINFOSTRUCTSETTINGS is)
/*
** IS_SaveToFile()
** Takes the target IS buffer and saves it to the specified filename
*/
{
	char *buffer;
	int retval;
	FILE *fp;

	if (retval = IS_SaveToBuffer(&buffer,is))
	{
		if (fp = fopen(filename,"w+b"))
		{
			fwrite(buffer,retval,1,fp);
			free(buffer);
			fclose(fp);
		}
		else
		{
			free(buffer);
			return FALSE;
		}
	}
	else
		return FALSE;

	return TRUE;
}

BOOL IS_LoadFromFile(const char* filename, LPINFOSTRUCTSETTINGS is)
/*
** IS_LoadFromFile()
** Loads an IS buffer from the specified filename and fills the specified info settings structure
*/
{
	char *buffer = NULL;
	FILE *fp;

	if (fp = fopen(filename,"rb"))
	{
		int length = _filelength(_fileno(fp));

		buffer = (char*)malloc(length);
		fread(buffer,length,1,fp);

		if (ferror(fp))
		/* Read error */
		{
			free(buffer);
			fclose(fp);
			return FALSE;
		}

		IS_LoadFromBuffer(buffer,is);
		free(buffer);
		fclose(fp);
	}
	else
		return FALSE;

	return TRUE;
}