/*
** client.cpp
**
** This file contains all the code for working with clients(connections to server).
** It contains nothing specific for users or servers, just the basic connection info.
*/

#include "client.h"

/* Global variables */
char recvbuf[8192];
#define MAX_ACCEPTS 100
HASH_TABLE_STRUCT	HT_User;
HASH_TABLE_STRUCT	HT_Server;
HASH_TABLE_STRUCT	HT_Channel;

void Client_AcceptNew(SOCKET fdListener)
/*
** Client_AcceptNew()
** Checks all bound server sockets for new connections, and handles them accordingly
*/
{
	int localindex, slen = sizeof(struct sockaddr);
	SOCKET s;
	LINKED_LIST_STRUCT llNewConnections, *curr = &llNewConnections;
	CLIENT_STRUCT *csnew;
	int l_len = sizeof(struct sockaddr);
	struct sockaddr_in sin, l_sin;

	memset(&llNewConnections,0,sizeof(llNewConnections));

	while ((s = accept(fdListener,(struct sockaddr*)&sin,&slen)) != INVALID_SOCKET)
	/* Get all connections queued in the backlog */
	{
		scs.lusers.nUnknown++;

		/* For performance reasons, we will manually add connections to the list */
		curr->next = LLNode_Alloc();
		curr = curr->next;

		curr->data = (void*)s;
	}

	/* Start at beginning of list again */
	curr = &llNewConnections;

	while (curr->next)
	/* Process all accepted connections */
	{
		curr = curr->next;

		SettingsInfo.isIOControl.totaldescriptors++;
		SettingsInfo.isIOControl.totalconnections++;

		l_len = sizeof(struct sockaddr);
		s = (SOCKET)curr->data;
		localindex = (int)s;

		if (s > scs.fdHighest)
			scs.fdHighest = s;

		csnew = scs.CSLocal[localindex] = (CLIENT_STRUCT*)calloc(1,sizeof(CLIENT_STRUCT));

		getpeername(s,(struct sockaddr*)&sin,&slen);

		/* IP/socket info */
		csnew->fd = (int)s;
		csnew->port_r = ntohs(sin.sin_port);
		csnew->ip = sin.sin_addr.s_addr;
		csnew->hops = 1;

		/* Server info */
		csnew->servernext = scs.sclient;
		strcpy(csnew->servername,scs.sclient->server->name);
        csnew->serverkey = scs.skey;

		/* Get info for connecting socket, in this case the local port */
		getsockname(s,(struct sockaddr*)&l_sin,&l_len);
		csnew->port_l = ntohs(l_sin.sin_port);

		if (SettingsInfo.isGeneral.dns_lookup)
		/* Initiate lookup */
		{
			Client_SendToOne(csnew,FALSE,":%s NOTICE AUTH :*** Looking up hostname...",SettingsInfo.isGeneral.servername);
			csnew->state = CLIENT_STATE_LOOKUP;
			Lookup_Perform(sin.sin_addr.s_addr,csnew);
		}
		else
		/* Use IP address as hostname */
		{
			strcpy(csnew->hostname,inet_ntoa(sin.sin_addr));
			csnew->state = CLIENT_STATE_REGISTERING;
		}

		/* Each client is a user until proven otherwise */
		csnew->user = (USER_STRUCT*)calloc(1,sizeof(USER_STRUCT));

		/* Broadcast ACCEPT event */
		strcpy(csnew->ip_r,inet_ntoa(sin.sin_addr));
		strcpy(csnew->ip_l,inet_ntoa(l_sin.sin_addr));
		Event_Broadcast(NULL,EVENT_TYPE_SOCKET,"SOCKET ACCEPT %s:%d %s:%d",csnew->ip_r,csnew->port_r,csnew->ip_l,csnew->port_l);
	}

	/* Clear new connections list */
	while (llNewConnections.next)
		LL_Remove(&llNewConnections,llNewConnections.next->data);
}

void Client_CreateDCList(CLIENT_STRUCT *csGoingDown, LINKED_LIST_STRUCT *llServerHead, LINKED_LIST_STRUCT *llUserHead )
/*
** Client_CreateDCList()
** Creates lists of servers and users that will need to be disconnected along with the target server
*/
{
	int nCount, nCount2;
	BOOL bExit;
	CLIENT_STRUCT *csNext, *csSearch;
	LINKED_LIST_STRUCT *llPtr;

	if (SettingsInfo.isServers.networktype == NETWORK_TYPE_STAR)
	{
		/* Create list of servers first */
		for (nCount = MAX_HOPS; nCount >= csGoingDown->hops + 1; nCount--)
		{
			llPtr = &scs.llServerHead[nCount];

			while (llPtr->next)
			/* Loop through servers (if any) and find out if they lead back to the one thats going down */
			{
				llPtr = llPtr->next;
				csNext = (CLIENT_STRUCT*)llPtr->data;

				bExit = FALSE;

				csSearch = csNext;

				while (csSearch && csSearch != csGoingDown && csSearch != scs.sclient)
				/* Each iteration gets one step closer towards us */
				{
					csSearch = Client_GetServer(csSearch);
					
					if (csSearch == csGoingDown)
					/* Take this server down! */
					{
						LL_Add(llServerHead,(void*)csNext);
						break;
					}
				}
			}
		}

		/* Now, create list of users. This should be cake since we have a fresh list of servers :) */
		for (nCount = MAX_HOPS; nCount >= csGoingDown->hops + 1; nCount--)
		{
			llPtr = &scs.llUserHead[nCount];

			while (llPtr->next)
			{
				llPtr = llPtr->next;

				csNext = (CLIENT_STRUCT*)llPtr->data;
				csSearch = Client_GetServer(csNext);

				if (csSearch == csGoingDown || LL_Find(llServerHead,(void*)csSearch))
					LL_Add(llUserHead,(void*)csNext);
			}
		}
	}
	else if (SettingsInfo.isServers.networktype == NETWORK_TYPE_MESH)
	/* With mesh networks this is easy. Just make list of all users on target server */
	{
		llPtr = &scs.llUserHead[1];

		while (llPtr->next)
		{
			llPtr = llPtr->next;

			csNext = (CLIENT_STRUCT*)llPtr->data;
			csSearch = Client_GetServer(csNext);

			if (csSearch == csGoingDown || LL_Find(llServerHead,(void*)csSearch))
				LL_Add(llUserHead,(void*)csNext);
		}
	}
}

void Client_Disconnect(CLIENT_STRUCT *client)
/*
** Client_Disconnect()
** Abruptly disconnects the target client from the network
** NOTE: This function can be called for any clients on the network
*/
{
	if (Client_IsLocal(client))
	{
		shutdown(client->fd,2);
		closesocket(client->fd);
		SettingsInfo.isIOControl.totaldescriptors--;
		SettingsInfo.isIOControl.totalconnections--;

		scs.CSLocal[client->fd] = NULL;

		/* Send the SOCKET CLOSE event */
		Event_Broadcast(NULL,EVENT_TYPE_SOCKET,"SOCKET CLOSE %s:%d %s:%d 0",client->ip_r,client->port_r,client->ip_l,client->port_l);

	}

	if (Client_Registered(client))
	{
		CHANNEL_STRUCT *chTarget;

		if (!SettingsInfo.isSecurity.showquitsasparts && !client->bKilled)
			User_BroadcastToAllLocalsInAllChannels(client,client,"QUIT :%s",client->quitmessage);

		/* Remove user from all channels they are in */
		while (client->user->llChannelHead.next)
		{
			chTarget = (CHANNEL_STRUCT*)client->user->llChannelHead.next->data;
			Channel_DeleteUser(chTarget,client);

			if (chTarget->dwUsers < 1)
			{
				Event_Broadcast(client,EVENT_TYPE_CHANNEL,"CHANNEL DESTROY %s",chTarget->szName);
				Channel_Cleanup(chTarget,TRUE);
			}
			else if (!client->bKilled && SettingsInfo.isSecurity.showquitsasparts)
				Channel_BroadcastToLocal(chTarget,client,NULL,TOK_PART,MSG_PART,chTarget->szName);
		}

		/* Send QUIT message to other servers */
		if (!client->bKilled)
		{
			Event_Broadcast(client,EVENT_TYPE_USER,"USER LOGOFF %s!%s@%s %s:%u %s:%u",Client_Nickname(client),client->user->username,client->hostname,client->ip_r,client->port_r,client->ip_l,client->port_l);
			Server_BroadcastFromUser(client,TOK_QUIT,MSG_QUIT,":%s",client->quitmessage);
		}

		if (Client_IsLocal(client))
			scs.lusers.nLocalUsers--;

		scs.lusers.nGlobalUsers--;
		
		LL_Remove(&scs.llEventHead,(void*)client);
	}
	else if (!Client_IsServer(client))
		scs.lusers.nUnknown--;

	/* Update luser counts */
	if (Client_IsPriveledged(client))
		scs.lusers.nOpsOnline--;
	if (Client_Invisible(client))
		scs.lusers.nInvisible--;

	if (Client_IsServer(client))
	{
		int nCount;
		LINKED_LIST_STRUCT llServerHead, llUserHead, *llPtr;
		CLIENT_STRUCT *csNext;
		CHANNEL_STRUCT *chChannel;

		llServerHead.data = llUserHead.data = NULL;
		llServerHead.next = llUserHead.next = NULL;

		/* Get list of everyone who is to be disconnected */
		Client_CreateDCList(client,&llServerHead,&llUserHead);

		Serverlist_ChangeStatus(client->server->name,FALSE);
		Hash_Delete(&HT_Server,client->server->name,client);

		scs.lusers.nGlobalServers--;

		if (Client_IsLocal(client))
			scs.lusers.nLocalServers--;

		if (Server_Synched(client))
		{
			Event_Broadcast(NULL,EVENT_TYPE_SERVER,"SERVER UNLINK %s:%d %s",client->ip_r,client->port_r,client->server->name);

			/* Star networks: remove any other servers connected to this one */

			if (SettingsInfo.isServers.networktype == NETWORK_TYPE_MESH ||
				!Client_WasNetsplitVictim(client))
			/* Only do this for the first server that fails */
			{
				llPtr = &llServerHead;

				while (llPtr->next)
				/* Mark servers as dead. This loop wont even happen on a mesh network */
				{
					llPtr = llPtr->next;

					csNext = (CLIENT_STRUCT*)llPtr->data;

					sprintf(csNext->quitmessage,"Netsplit (%s)",client->server->name);
					csNext->state = CLIENT_STATE_DEAD;
					csNext->flags |= CLIENT_FLAG_NETSPLITVICTIM;
				}

				llPtr = &llUserHead;

				while (llPtr->next)
				/* Mark clients as dead */
				{
					llPtr = llPtr->next;

					csNext = (CLIENT_STRUCT*)llPtr->data;

					if (SettingsInfo.isServers.networktype == NETWORK_TYPE_STAR)
						sprintf(csNext->quitmessage,"Netsplit (%s) was on (%s)",client->server->name,csNext->servername);
					else
						sprintf(csNext->quitmessage,"Server disconnected (%s)",client->server->name);

					csNext->state = CLIENT_STATE_DEAD;
					csNext->flags |= CLIENT_FLAG_NETSPLITVICTIM;
				}

				llPtr = &scs.llChannelHead;

				while (llPtr->next)
				/* Find channels with this origin server and set them to NULL */
				{
					llPtr = llPtr->next;

					chChannel = (CHANNEL_STRUCT*)llPtr->data;
					
					if (chChannel->csServerOrigin == client)
						chChannel->csServerOrigin = NULL;

				}

				if (SettingsInfo.isServers.networktype != NETWORK_TYPE_MESH)
				/* Send QUIT message on behalf of this server */
					Server_Broadcast(client->servernext,":%d " TOK_QUIT " :%s",client->server->id,client->quitmessage);
			}

			scs.CSServers[client->server->id] = NULL;

			MESH_NODE *mnPtr;

			mnPtr = Mesh_Nodelist_Find(client->server->name);
			mnPtr->bConnected = FALSE;
			mnPtr->csServer = NULL;

			if (SettingsInfo.isServers.networktype == NETWORK_TYPE_MESH)
			{
				scs.meshctrl.networkstatus = Mesh_NetworkStatus();

				if (client->server->timeouts != (unsigned int)(-1))
				/* Broadcast disconnect to network */
					Server_Broadcast(client,":%d " TOK_QUIT " :%s",client->server->id,client->quitmessage);
			}

			LL_Clear(&llServerHead);
			LL_Clear(&llUserHead);
		}

	}
	else
		Hash_Delete(&HT_User,client->user->nickname,client);

	Client_Destroy(client);
}


void Client_Destroy(CLIENT_STRUCT *client)
/*
** Client_Destroy()
** This function clears and deallocates a client structure
** NOTE: This function is to be used for cleaning-up purposes only...
*/
{
	if (Client_IsServer(client))
		free(client->server);
	else
		free(client->user);

	StrBuf_Empty(&client->recvQ);
	StrBuf_Empty(&client->sendQ);

	LL_ClearAndFree(&client->llAccessHead);
	LL_ClearAndFree(&client->llEventHead);

	free(client);
	client = NULL;
}

void Client_Kill(CLIENT_STRUCT *csFrom, CLIENT_STRUCT *csTarget, char *szKillMessage)
/*
** Client_Kill()
** Displays KILL message then disconnects the target user via Client_SendErrorNumber()
*/
{
	va_list vArgs;
	char buf[2048];

	LINKED_LIST_STRUCT *llUser = &(scs.llUserHead[1]);
	CLIENT_STRUCT *csUserLoop = NULL;

	if (szKillMessage)
		strcpy(csTarget->killmessage,szKillMessage);

	while (llUser->next)
	/* Loop through all users and if a common channel is shared, broadcast the kill message */
	{
		llUser = llUser->next;

		csUserLoop = (CLIENT_STRUCT*)(llUser->data);

		if (Client_IsLocal(csUserLoop) && csUserLoop == csFrom || csUserLoop == csTarget || User_GetCommonChannels(csTarget,csUserLoop,NULL,NULL) > 0)
		/* Send the message to local clients */
			Client_SendToOne(csUserLoop,FALSE,":System KILL %s :%s",Client_Nickname(csTarget),(csTarget->killmessage[0] ? csTarget->killmessage : ""));
	}

	csTarget->bKilled = TRUE;
	csTarget->state = CLIENT_STATE_DEAD;
	Client_SendToOne(csFrom,FALSE,":%s KILLED",Client_Nickname(csTarget));
	Client_SendErrorNumber(csTarget,TRUE,ERROR_007);

	Server_BroadcastFromUser(csFrom,TOK_KILL,MSG_KILL,"%s :%s",Client_Nickname(csTarget),(csTarget->killmessage[0] ? csTarget->killmessage : ""));
	Event_Broadcast(csTarget,EVENT_TYPE_USER,"USER KILL %s!%s@%s %s:%u :%s",Client_Nickname(csTarget),csTarget->user->username,csTarget->hostname,csTarget->ip_r,csTarget->port_r,(csTarget->killmessage[0] ? csTarget->killmessage : ""));
}

void Client_SendError(CLIENT_STRUCT *client, BOOL bCloseOnSend, const char *string, ...)
/*
** Client_SendError()
** This function sends an ERROR message to the specified client
*/
{
	va_list vArgs;
	char errbuf[512];
	char buf[256];

	va_start(vArgs,string);
	vsprintf(buf,string,vArgs);
	va_end(vArgs);

	sprintf(errbuf,"%s ERROR: %s",Client_Nickname(client),buf);

	strcat(errbuf,"\r\n");

	StrBuf_Add(&client->sendQ,errbuf,(int)strlen(errbuf));

	if (bCloseOnSend)
		client->flags |= CLIENT_FLAG_CLOSEONSEND;
}

void Client_SendErrorNumber(CLIENT_STRUCT *client, BOOL bCloseOnSend, const char *errstr)
/*
** Client_SendErrorNumber()
**
** This function sends a "ERROR: Closing Link:" message to the target client
** NOTE: errstr MUST be one of the ERROR_XXX constants defined in numeric.h
*/
{
	char buf[1024];
	struct in_addr in;

	if (!Client_IsLocal(client))
	/* Just kill non-local clients */
	{
		client->state = CLIENT_STATE_DEAD;
		return;
	}

	in.s_addr = client->ip;

	sprintf(buf,errstr,Client_Nickname(client),inet_ntoa(in));
	strcat(buf,"\r\n");

	StrBuf_Add(&client->sendQ,buf,(int)strlen(buf));

	if (bCloseOnSend)
		client->flags |= CLIENT_FLAG_CLOSEONSEND;
}

void Client_SendErrorNumberToAllLocal(const char *errstr, BOOL bCloseOnSend)
/*
** Client_SendErrorNumberToAllLocal()
**
** This function sends a "ERROR: Closing Link:" message to all local clients
** NOTE: errstr MUST be one of the ERROR_XXX constants defined in numeric.h
*/
{
	char buf[1024];
	struct in_addr in;
	int count;
	CLIENT_STRUCT *client = NULL;

	for (count = (int)scs.fdHighest; count >= 0; count--)
	{
		if ((client = scs.CSLocal[count]) && Client_IsUser(client))
		/* Loop through all valid connections */
		{
			in.s_addr = client->ip;

			sprintf(buf,errstr,Client_Nickname(client),inet_ntoa(in));
			strcat(buf,"\r\n");

			StrBuf_Add(&client->sendQ,buf,(int)strlen(buf));

			if (bCloseOnSend)
				client->flags |= CLIENT_FLAG_CLOSEONSEND;
		}

	}
}

void Client_SendMessage(CLIENT_STRUCT *csTo, CLIENT_STRUCT *csFrom, const char *szToken, const char *szMessage, const char *szBuffer, ...)
/*
** Client_SendMessage()
** This function will send a message to the client if local, or the next server
*/
{
	va_list vArgs;
	char buf_args[1024], buf_end[2048];
	CLIENT_STRUCT *client;

	va_start(vArgs,szBuffer);
	vsprintf(buf_args,szBuffer,vArgs);
	va_end(vArgs);

	if (csFrom != NULL)
	{
		if (!Client_IsLocal(csTo))
		/* Client is on another server */
		{
			client = csTo->servernext;

			if (szToken)
				sprintf(buf_end,":%s!%s@%s %s %s",csFrom->user->nickname,csFrom->user->username,csFrom->hostname,szToken,buf_args);
			else
				sprintf(buf_end,":%s!%s@%s %s %s",csFrom->user->nickname,csFrom->user->username,csFrom->hostname,szMessage,buf_args);

		}
		else
		/* Client is local */
		{
			BOOL bMask = User_DoWeMask(csFrom,csTo);

			client = csTo;

			if (bMask)
				sprintf(buf_end,":%s!%s@%s %s %s",csFrom->user->nickname,csFrom->user->username,csFrom->hostmask,szMessage,buf_args);
			else
				sprintf(buf_end,":%s!%s@%s %s %s",csFrom->user->nickname,csFrom->user->username,csFrom->hostname,szMessage,buf_args);
		}
		
	}


	strcat(buf_end,"\r\n");

	
	if (szToken)
		StrBuf_Add(&client->sendQ,buf_end,(int)strlen(buf_end));
}

void Client_SendToOne(CLIENT_STRUCT *client,BOOL bCloseOnSend, const char *string, ...)
/*
** Client_SendToOne()
** This function will send a CRLF-terminated message to the target client (eventually!)
*/
{
	va_list vArgs;
	char buf[4096];

	va_start(vArgs,string);
	vsprintf(buf,string,vArgs);
	va_end(vArgs);

	strcat(buf,"\r\n");

	if (!Client_IsLocal(client))
	{
		ODS("Client_SendToOne() called on non-local client! Contents: %s",buf);
		return;
	}

	StrBuf_Add(&client->sendQ,buf,(int)strlen(buf));

	if (bCloseOnSend)
		client->flags |= CLIENT_FLAG_CLOSEONSEND;
}

void Client_SendLusers(CLIENT_STRUCT *client)
/*
** Client_SendLusers()
** Sends all lusers numerics to the specified client
*/
{
	
	Client_SendToOne(client,FALSE,":%s %3.3d %s :There are %d users and %d invisible on %d servers",
		SettingsInfo.isGeneral.servername,RPL_LUSERCLIENT,Client_Nickname(client),scs.lusers.nGlobalUsers,scs.lusers.nInvisible,scs.lusers.nGlobalServers);

	if (scs.lusers.nOpsOnline)
		Client_SendToOne(client,FALSE,":%s %3.3d %s %d :operator(s) online",
		SettingsInfo.isGeneral.servername,RPL_LUSEROP,Client_Nickname(client),scs.lusers.nOpsOnline);

	if (scs.lusers.nUnknown)
		Client_SendToOne(client,FALSE,":%s %3.3d %s %d :unknown connection(s)",
		SettingsInfo.isGeneral.servername,RPL_LUSERUNKNOWN,Client_Nickname(client),scs.lusers.nUnknown);

	if (scs.lusers.nChannels)
		Client_SendToOne(client,FALSE,":%s %3.3d %s %d :channels formed",
		SettingsInfo.isGeneral.servername,RPL_LUSERCHANNELS,Client_Nickname(client),scs.lusers.nChannels);

	Client_SendToOne(client,FALSE,":%s %3.3d %s :I have %d clients and %d servers",
		SettingsInfo.isGeneral.servername,RPL_LUSERME,Client_Nickname(client),scs.lusers.nLocalUsers,scs.lusers.nLocalServers);
	Client_SendToOne(client,FALSE,":%s %3.3d %s :Current local users: %d Max: %d",
		SettingsInfo.isGeneral.servername,RPL_LOCALUSERS,Client_Nickname(client),scs.lusers.nLocalUsers,scs.lusers.nLocalMax);
	Client_SendToOne(client,FALSE,":%s %3.3d %s :Current global users: %d Max: %d",
		SettingsInfo.isGeneral.servername,RPL_GLOBALUSERS,Client_Nickname(client),scs.lusers.nGlobalUsers,scs.lusers.nGlobalMax);
}

int Client_CheckForBan(CLIENT_STRUCT *client)
/*
** Client_CheckForBan()
** Checks to see if the specified client is banned from the server or the network
** Returns: 1 if server ban, 2 if network ban, 0 if not banned
*/
{
	int nRetval;
	struct in_addr in;
	ACCESS_STRUCT *accInfo = NULL;
	BanList *banptr = &(SettingsInfo.isBans.banhead);
	BOOL bNetworkBan = FALSE , bBan = FALSE;
	char szClientMask[1024];

	/* First, we'll check the ACCESS entries. Then we'll check the permanent bans */

	/* Step 1: Look through access list */

	/* Determine if the user is banned from the server/network */
	nRetval = Client_IsOnAccessList(client,NULL,&scs.llAccessNetworkHead);

	if (nRetval != -1)
	/* Check if they have a deny entry */
	{
		accInfo = (ACCESS_STRUCT*)nRetval;
		bNetworkBan = TRUE;
	}
	else
	{
		nRetval = Client_IsOnAccessList(client,NULL,&scs.sclient->llAccessHead);

		if (nRetval != -1)
			accInfo = (ACCESS_STRUCT*)nRetval;
	}

	/* Step #2: Look through permanent bans */
	strcpy(szClientMask,Client_Nickname(client));
	strcat(szClientMask,"!");
	strcat(szClientMask,client->user->username);
	strcat(szClientMask,"@");
	strcat(szClientMask,client->hostname);

	/* Go through exceptions first */
	while (banptr->next)
	{
		banptr = banptr->next;

		if (banptr->bException && match(banptr->hostmask,szClientMask) == 0)
		/* User is on exception list, let them go */
			return 0;
	}

	/* Now, go through bans */
	banptr = &(SettingsInfo.isBans.banhead);

	while (banptr->next)
	{
		banptr = banptr->next;

		if (!banptr->bException && match(banptr->hostmask,szClientMask) == 0)
		/* User is banned! */
		{
			in.s_addr = client->ip;

			Client_SendToOne(client,TRUE,ERROR_014,Client_Nickname(client),inet_ntoa(in),(banptr->reason ? banptr->reason : "(Reason not specified)"));

			/* Send event */
			Event_Broadcast(client,EVENT_TYPE_CONNECT,"CONNECT BANNED %s!%s@%s %s:%u %s",Client_Nickname(client),client->user->username,client->hostname,client->ip_r,client->port_r,(banptr->reason ? banptr->reason : "(Reason not specified)"));
			return (banptr->bNetworkBan ? 2 : 1);
		}
	}


	if (accInfo)
	/* Check if they have a deny entry */
	{
		if (accInfo->dwLevel == ACCESSLEVEL_DENY)
		/* Ban user */
		{
			in.s_addr = client->ip;

			Client_SendToOne(client,TRUE,ERROR_014,Client_Nickname(client),inet_ntoa(in),(accInfo->szReason[0] ? accInfo->szReason : "(Reason not specified)"));

			/* Send event */
			Event_Broadcast(client,EVENT_TYPE_CONNECT,"CONNECT BANNED %s!%s@%s %s:%u %s",Client_Nickname(client),client->user->username,client->hostname,client->ip_r,client->port_r,(accInfo->szReason[0] ? accInfo->szReason : "(Reason not specified)"));
			return (bNetworkBan ? 2 : 1);
		}
	}

	return 0;
}

void Client_SendWelcome(CLIENT_STRUCT *client)
/*
** Client_SendWelcome()
** Sends numerics 001-004 to the target client, which are sent to all connecting
*/
{
	char buffer[4096];

	buffer[0] = 0;

	if (Client_IsUser(client))
	/* Send user greeting */
	{
		char *usermodes = "aioxz";
		char *channelmodes = "abhiklmnoprstuwx";

		/* Since we only do this once, update our lusers count */
		scs.lusers.nUnknown--; scs.lusers.nLocalUsers++; scs.lusers.nGlobalUsers++;

		if (scs.lusers.nLocalUsers > scs.lusers.nLocalMax)
			scs.lusers.nLocalMax = scs.lusers.nLocalUsers;

		if (scs.lusers.nGlobalUsers > scs.lusers.nGlobalMax)
			scs.lusers.nGlobalMax = scs.lusers.nGlobalUsers;

		sprintf(buffer,
			":%s %3.3d %s :Welcome to the %s %s" CRLF
			":%s %3.3d %s :Your host is %s, running version " ROCKIRCX_VERSION CRLF
			":%s %3.3d %s :This server was created " ROCKIRCX_CREATED CRLF
			":%s %3.3d %s %s " ROCKIRCX_VERSION " %s %s" CRLF,
			SettingsInfo.isGeneral.servername,RPL_WELCOME,client->user->nickname,SettingsInfo.isGeneral.serverdescription,client->user->nickname,
			SettingsInfo.isGeneral.servername,RPL_YOURHOST,client->user->nickname,SettingsInfo.isGeneral.servername,
			SettingsInfo.isGeneral.servername,RPL_CREATED,client->user->nickname,
			SettingsInfo.isGeneral.servername,RPL_MYINFO,client->user->nickname,SettingsInfo.isGeneral.servername,usermodes,channelmodes);

	}
	else if (Client_IsServer(client))
	/* Send server greeting */
		sprintf(buffer,MSG_SPASS " %s %d %s %d %s %s\r\n",client->password,(int)SettingsInfo.isServers.networktype,SettingsInfo.isGeneral.networkname,scs.lusers.nGlobalServers,SettingsInfo.isGeneral.servername,ROCKIRCX_PROTOCOL_VERSION);

	/* Set sign-on time */
	client->signon = time(NULL);

	/* Mask the IP address */
	// strcpy(client->hostmask,client->hostname);
	strcpy(client->hostmask,"NONEYA");

	StrBuf_Add(&client->sendQ,buffer,(int)strlen(buffer));

}

void Client_SendMOTD(CLIENT_STRUCT *client)
/*
** Client_SendMOTD()
** Sends the MOTD & numerics to the specified client
*/
{
	if (SettingsInfo.isGeneral.motdbuffer)
	/* Display MOTD */
	{
		char szTemp[1024], *szPtr, ch;
		int nCount, nNewlinePos = 0;

		/* Start of MOTD */
		sprintf(szTemp,":- %s Message of the Day -",SettingsInfo.isGeneral.servername);
		Client_SendNumericToOne(client,RPL_MOTDSTART,szTemp);

		/* MOTD loop */

		for (nCount = 0; SettingsInfo.isGeneral.motdbuffer[nCount]; nCount++)
		{
			ch = SettingsInfo.isGeneral.motdbuffer[nCount];
			
			if (ch == 13)
			/* CRLF */
			{
				strcpy(szTemp,":- ");
				strncat(szTemp,&SettingsInfo.isGeneral.motdbuffer[nNewlinePos],(nCount - nNewlinePos));

				Client_SendNumericToOne(client,RPL_MOTD,szTemp);

				nNewlinePos = nCount + 2;
			}
		}

		/* Final MOTD */
		strcpy(szTemp,":- ");
		strncat(szTemp,&SettingsInfo.isGeneral.motdbuffer[nNewlinePos],(nCount - nNewlinePos));

		Client_SendNumericToOne(client,RPL_MOTD,szTemp);

		/* End of MOTD */
		Client_SendNumericToOne(client,RPL_ENDOFMOTD,":End of /MOTD command");

	}
	else
	/* No MOTD is set */
		Client_SendNumericToOne(client,ERR_NOMOTD,NULL);
}

void Client_SendList(CLIENT_STRUCT *client, BOOL bListX, const char *szParameters)
/*
** Client_SendList()
** Sends a list of channels to the client specified
*/
{
	CHANNEL_STRUCT *chChannel = NULL;
	LINKED_LIST_STRUCT *llChannel = NULL;

	if (bListX == TRUE)
		Client_SendToOne(client,FALSE,":%s %3.3d %s :Start of ListX",SettingsInfo.isGeneral.servername,811,Client_Nickname(client));
	else
		Client_SendToOne(client,FALSE,":%s %3.3d %s Channel :Users	Name",SettingsInfo.isGeneral.servername,321,Client_Nickname(client));

	llChannel = &scs.llChannelHead;

	while (llChannel->next != NULL)
	/* Loop through channels and add them to the list */
	{
		char szBuffer[256];
		char szTopicBuffer[1024];
		BOOL bOnChannel = FALSE;

		llChannel = llChannel->next;

		chChannel = (CHANNEL_STRUCT*)(llChannel->data);
		Channel_GetModeString(chChannel,szBuffer);
		bOnChannel = Client_IsOnChannel(client,chChannel,NULL,NULL);

		if (Client_IsPriveledged(client) || Channel_IsPublic(chChannel) || Channel_IsPrivate(chChannel) || bOnChannel)
		/* Check channel permissions here */
		{
			if (Client_IsPriveledged(client) || Channel_IsPublic(chChannel) || bOnChannel)
				strcpy(szTopicBuffer,chChannel->szPropTopic);
			else
				szTopicBuffer[0] = 0;

			if (bListX == TRUE)
				Client_SendToOne(client,FALSE,":%s %3.3d %s %s %s %d 0 :%s",SettingsInfo.isGeneral.servername,812,Client_Nickname(client),chChannel->szName,szBuffer,chChannel->dwUsers,chChannel->szPropTopic);
			else
				Client_SendToOne(client,FALSE,":%s %3.3d %s %s %d :%s",SettingsInfo.isGeneral.servername,322,Client_Nickname(client),chChannel->szName,chChannel->dwUsers,chChannel->szPropTopic);
		}
	}

	if (bListX == TRUE)
		Client_SendToOne(client,FALSE,":%s %3.3d %s :End of ListX",SettingsInfo.isGeneral.servername,817,Client_Nickname(client));
	else
		Client_SendToOne(client,FALSE,":%s %3.3d %s :End of /LIST",SettingsInfo.isGeneral.servername,323,Client_Nickname(client));
}

void Client_SendNumericToOne(CLIENT_STRUCT *client, int numeric, const char *data)
/*
** Client_SendNumericToOne()
** Sends a RAW numeric to the specified client
*/
{
	switch (numeric)
	{
		case ERR_NOSUCHNICK:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :No such nick/channel",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case ERR_NORECIPIENT:
			Client_SendToOne(client,FALSE,":%s %3.3d %s :No recipient given (%s)",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case ERR_NOTEXTTOSEND:
			Client_SendToOne(client,FALSE,":%s %3.3d %s :No text to send (%s)",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case ERR_UNKNOWNCOMMAND:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :Unknown command",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case ERR_NONICKNAMEGIVEN:
			Client_SendToOne(client,FALSE,":%s %3.3d %s :No nickname given",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client));
		break;
		case ERR_ERRONEUSNICKNAME:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :Erroneous nickname",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case ERR_NICKNAMEINUSE:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :Nickname is already in use",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case ERR_NOMOTD:
			Client_SendToOne(client,FALSE,":%s %3.3d %s :MOTD File is missing",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client));
		break;
		case ERR_NOTREGISTERED:
			Client_SendToOne(client,FALSE,":%s %3.3d %s :You have not registered",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client));
		break;
		case ERR_NEEDMOREPARAMS:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :Not enough parameters",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case ERR_ALREADYREGISTRED:
			Client_SendToOne(client,FALSE,":%s %3.3d %s :You may not reregister",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client));
		break;
		case RPL_VERSION:
			Client_SendToOne(client,FALSE,":%s %3.3d %s RockIRCX " ROCKIRCX_VERSION " %s :%s",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),SettingsInfo.isGeneral.servername,data);
		break;
		case RPL_ENDOFNAMES:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :End of /NAMES list",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case IRCRPL_IRCX:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %d 0 ANON %d *",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),client->user->bInIRCX,SettingsInfo.isSecurity.max_msglen);
		break;
		case ERR_NOSUCHCHANNEL:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :No such channel",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case ERR_USERSDONTMATCH:
			Client_SendToOne(client,FALSE,":%s %3.3d %s :Cant change mode for other users",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client));
		break;
		case RPL_CHANNELMODEIS:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case ERR_CANNOTSENDTOCHAN:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :Cannot send to channel",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case ERR_NOTONCHANNEL:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :You're not on that channel",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case ERR_CHANOPRIVSNEEDED:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :You're not channel operator",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case ERR_UNIQOPPRIVSNEEDED:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :You're not channel owner",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case ERR_UMODEUNKNOWNFLAG:
			Client_SendToOne(client,FALSE,":%s %3.3d %s :Unknown MODE flag",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client));
		break;
		case RPL_NOTOPIC:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :No topic is set",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case ERR_NOADMININFO:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :No administrative info available",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case RPL_ADMINME:
			Client_SendToOne(client,FALSE,":%s %3.3d %s :Administrative info about %s",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case ERR_UNKNOWNMODE:
			Client_SendToOne(client,FALSE,":%s %3.3d %s :%s is unknown mode char to me",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case ERR_BANNEDFROMCHAN:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :Cannot join channel (+b)",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case ERR_BADCHANNELKEY:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :Cannot join channel (+k)",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case ERR_INVITEONLYCHAN:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :Cannot join channel (+i)",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case ERR_CHANNELISFULL:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :Cannot join channel (+l)",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case IRCERR_NOACCESS:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :No access",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case IRCERR_NOSUCHOBJECT:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :No such object found",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case IRCERR_TOOMANYARGUMENTS:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :Too many arguments",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case IRCRPL_ACCESSSTART:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :Start of access entries",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case IRCRPL_ACCESSEND:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :End of access entries",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case IRCERR_BADLEVEL:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :Bad level",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case IRCERR_BADCOMMAND:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :Bad command",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case IRCERR_BADPROPERTY:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :Bad property specified",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case RPL_ISON:
		case RPL_USERHOST:
			Client_SendToOne(client,FALSE,":%s %3.3d %s :%s",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case IRCERR_NOWHISPER:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :Does not permit whispers",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case IRCERR_ALREADYONCHANNEL:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :Already in the channel.",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
		case ERR_TOOMANYCHANNELS:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :You have joined too many channels",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;

		default:
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s",SettingsInfo.isGeneral.servername,numeric,Client_Nickname(client),data);
		break;
	}
}

void Client_SendToAllLocal(BOOL bCloseOnSend, const char *string, ...)
/*
** Client_SendToAllLocal()
** This function will send a CRLF-terminated message to all local clients
*/
{
	va_list vArgs;
	char buf[1024];
	int count, len;
	CLIENT_STRUCT *client;

	va_start(vArgs,string);
	vsprintf(buf,string,vArgs);
	va_end(vArgs);

	strcat(buf,"\r\n");
	len = (int)strlen(buf);

	for (count = (int)scs.fdHighest; count >= 0; count--)
	{
		if (client = scs.CSLocal[count])
		/* Loop through all valid connections */
		{
			StrBuf_Add(&client->sendQ,buf,len );

			if (bCloseOnSend)
				client->flags |= CLIENT_FLAG_CLOSEONSEND;
		}

	}
}

void Client_FlushBuffer(CLIENT_STRUCT *client)
/*
** Client_FlushBuffer()
** This function sends all queued data to the client's socket
*/
{
	int err, retval, length;
	char *szPtr, szBuf[256];

	if ((client->flags & CLIENT_FLAG_CLOSEONSEND) && StrBuf_IsEmpty(&client->sendQ))
	/* Close connection & de-allocate structure */
		client->state = CLIENT_STATE_DEAD;
	else if (StrBuf_Length(&client->sendQ))
	{
		if (!Client_IsPriveledged(client) && (StrBuf_Length(&client->sendQ) > SettingsInfo.isUser.max_sendq))
		/* Output saturation */
		{
			strcpy(client->quitmessage,"Output Saturation");
			StrBuf_Empty(&client->sendQ);

			Event_Broadcast(client,EVENT_TYPE_USER,"USER OUTPUT %s!%s@%s %s:%u",Client_Nickname(client),client->user->username,client->hostname,client->ip_r,client->port_r);
			Client_SendErrorNumber(client,TRUE,ERROR_009);
		}
		else
		/* Send entire contents of client's sendQ */
		{
			while (StrBuf_Length(&client->sendQ))
			{
				err = 0;

				szPtr = StrBuf_GetPtr(&client->sendQ,&length);
				retval = send(client->fd,szPtr,length,0);

				if (retval == SOCKET_ERROR)
				{
					err = WSAGetLastError();

					if (err != WSAEWOULDBLOCK)
					{
						GetErrorString(szBuf,err);
						sprintf(client->quitmessage,"Write Error: %s",szBuf);
						client->state = CLIENT_STATE_DEAD;
						break;
					}
				}
				else
				/* Data successfully sent */
				{
					SettingsInfo.isIOControl.datasent += length;
					StrBuf_Delete(&client->sendQ,length);

					if (StrBuf_IsEmpty(&client->sendQ) && (client->flags & CLIENT_FLAG_CLOSEONSEND))
						client->state = CLIENT_STATE_DEAD;
				}	
			}

		}
	}
}

int Client_ReadPacket(CLIENT_STRUCT* client, FD_SET *pread_set)
/*
** Client_ReadPacket()
** This function takes an incoming packet of data and handles all parsing, etc...
*/
{
	int nRetVal, length = 0, length2 = 0, length3 = 0, sbl = StrBuf_Length(&client->recvQ);
	time_t currtime = GetTickCount();

	char *szPtr = NULL, ch, cmdbuf[4096], bigbuf[16384];

	if (FD_ISSET(client->fd,pread_set) &&
		!(Client_IsUser(client) && ((unsigned long)sbl > SettingsInfo.isUser.max_recvq)))
	/* We have a user pending data which is still below the flood limit */
	{
		length = recv(client->fd,recvbuf,sizeof(recvbuf),0);

		client->lastmsg = currtime;

		if (Client_WasPinged(client))
			client->flags ^= CLIENT_FLAG_PINGED;

		if (client->lastmsg > client->parsemsg)
			client->parsemsg = client->lastmsg;

		if (length == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK)
		/* Not ready for reading yet */
			return 1;
		else if (length == SOCKET_ERROR)
		/* Error in reading of socket */
		{
			client->state = CLIENT_STATE_DEAD;
			GetErrorString(client->quitmessage,WSAGetLastError());

			return STOP_PARSING;
		}
		if (length <= 0)
			return length;

		/* Check to see if we have a newline(flag will be re-set appropriatley) */
		client->flags |= CLIENT_FLAG_NEWLINE;

		recvbuf[length] = 0;
		SettingsInfo.isIOControl.datarecvd += length;
	}

	if (length)
	/* Add data to RecvQ */
		StrBuf_Add(&client->recvQ,recvbuf,length);

	if (Client_IsServer(client))
	/* Immediatley process all server messages */
	{
        while (!StrBuf_IsEmpty(&client->recvQ) && Client_NewLine(client))
		{
			szPtr = StrBuf_GetPtrCRLF(&client->recvQ,&length2,&length3, 4096, bigbuf);

			if (szPtr)
			/* CRLF-terminated message is awaiting */
			{
				/* Put NUL at the end temporarily */
				ch = szPtr[length2];
				szPtr[length2] = 0;

				client->flags |= CLIENT_FLAG_NEWLINE;

				if (length2 > SettingsInfo.isSecurity.max_msglen)
				/* Message is too long, truncate */
				{
					length2 = SettingsInfo.isSecurity.max_msglen;
					length3 = length2;
					szPtr[length2 - 2] = '\r';
					szPtr[length2 - 1] = '\n';
					szPtr[length2] = 0;
				}

				strcpy(recvbuf,szPtr);
				nRetVal = Client_Parse(client,recvbuf,length2);

				StrBuf_Delete(&client->recvQ,length3);

				if (nRetVal == STOP_PARSING)
					return STOP_PARSING;
			}
			else
			{
				client->flags ^= CLIENT_FLAG_NEWLINE;
				break;
			}
		}
	}
	else
	/* Parse when the time is appropriate */
	{
		if (StrBuf_Length(&client->recvQ) > SettingsInfo.isUser.max_recvq)
		/* Floooooooood! */
		{
			strcpy(client->quitmessage,"Flooding");
			Client_SendErrorNumber(client,TRUE,ERROR_008);

			Event_Broadcast(client,EVENT_TYPE_USER,"USER INPUT %s!%s@%s %s:%u",Client_Nickname(client),client->user->username,client->hostname,client->ip_r,client->port_r);

			return STOP_PARSING;
		}

		while (!StrBuf_IsEmpty(&client->recvQ) && Client_NewLine(client) &&
			(client->state != CLIENT_STATE_LOOKUP && (client->parsemsg - currtime) <= ((signed)SettingsInfo.isUser.msgdelay * CLIENT_MAXMESSAGEBURST)))
		/* If a client has no parse delay, (CLIENT_MAXMESSAGEBURST) messages are allowed */
		{
			szPtr = StrBuf_GetPtrCRLF(&client->recvQ,&length2,&length3, 4096, bigbuf);

			if (szPtr)
			/* CRLF-terminated message is awaiting */
			{
				/* Put NUL at the end temporarily */
				ch = szPtr[length2];
				szPtr[length2] = 0;

				client->flags |= CLIENT_FLAG_NEWLINE;

				if (length2 > SettingsInfo.isSecurity.max_msglen)
				/* Message is too long, truncate */
				{
					length2 = SettingsInfo.isSecurity.max_msglen;
					szPtr[length2 - 2] = '\r';
					szPtr[length2 - 1] = '\n';
					szPtr[length2] = 0;
				}

				memcpy(cmdbuf,szPtr,length2);
				cmdbuf[length2] = 0;
				szPtr[length2] = ch;
				nRetVal = Client_Parse(client,cmdbuf,length2);

				StrBuf_Delete(&client->recvQ,length3);

				if (nRetVal == STOP_PARSING)
					return STOP_PARSING;
			}
			else
			{
				client->flags ^= CLIENT_FLAG_NEWLINE;
				break;
			}
		}
	}

	return 1;
}

int	Client_Parse(CLIENT_STRUCT *client, char *message, int length)
/*
** Client_Parse()
** Takes the message sent by a client and processes it
*/
{
	int count = 0, argc = 0;

	memset(&scs.msginfo,0,sizeof(scs.msginfo));

	/* Who is sending this message? */
	scs.msginfo.c_prefixfrom = scs.msginfo.c_from = client;

	while (count <= length)
	/* Break the passed message into tokens */
	{
		while (message[count] == ' ')
			count++;

		if (count >= length)
		/* We appear to have trailing spaces in message */
			break;

		scs.msginfo.argc = ++argc;

		if (message[count] != ':' || argc == 1)
		/* At beginning of next token */
		{
			scs.msginfo.argv[argc - 1] = &message[count];

			while (message[count] != ' ' && count <= length)
			/* Skip to end of message */
				count++;

			if (count == length)
				message[count] = 0;
			else
			/* We are at end of message */
				message[count++] = 0;
		}
		else
		/* The ':' has a special purpose...ex "PRIVMSG person :private message" */
		{
			scs.msginfo.argv[argc - 1] = &message[count + 1];

			/* End of message */
			message[length] = 0;
			break;
		}
	}

	/* We have successfully seperated the tokens and parameters, time to use them */
	return Message_Execute();
}

unsigned int Client_GetIdle(CLIENT_STRUCT *c_target)
/*
** Client_GetIdle()
** Returns the idle time of the client, in seconds
*/
{
	unsigned int retval = (unsigned int)c_target->idlemsg;

	retval = GetTickCount() - retval;
	retval /= 1000;

	return retval;
}

int Client_GetWhoisChannels(CLIENT_STRUCT *csAsking, CLIENT_STRUCT *csTarget, char *pszOutputBuffer)
/*
** Client_GetWhoisChannels()
** Retrieves the list of channels to be displayed in the /whois
** Depends on a list of factors, including channel privacy modes, user modes, etc...
** Returns -1 if there are no channels to be displayed(for whatever reason)
*/
{
	char chbuf[1024];
	CHANNEL_STRUCT *chChannel = NULL;
	LINKED_LIST_STRUCT *llChannel = &(csTarget->user->llChannelHead);
	DWORD dwPriveledge = 0;
	BOOL bReturnMinusOne = TRUE;

	/* Ready our string for strcat's */
	chbuf[0] = 0;

	if (!llChannel->next)
	/* Target user is not on any channels! */
		return -1;
	else
	/* User is on channels */
	{
		while (llChannel->next)
		/* Loop through list of target user's channels */
		{
			/* Get our next channel */
			llChannel = llChannel->next;
			chChannel = (CHANNEL_STRUCT*)(llChannel->data);

			if ((Client_IsOnChannel(csTarget,chChannel,&dwPriveledge,NULL) && Client_IsOnChannel(csAsking,chChannel,NULL,NULL)) ||
				Client_IsAdmin(csAsking) || Client_IsSysop(csAsking) ||
				Channel_IsPublic(chChannel) || csAsking == csTarget)
			/*
			** Display channel only under these circumstances:
			** 1) The asking user and the target user are both on this channel
			** 2) Asking user is an administrator or sysop
			** 3) This channel is public(no +s, +p, or +h mode set)
			** 4) The user is doing a /whois on themselves
			*/
			{
				bReturnMinusOne = FALSE;

				if (dwPriveledge & CHANNEL_PRIVELEDGE_OWNER)
					strcat(chbuf,".");
				else if (dwPriveledge & CHANNEL_PRIVELEDGE_HOST)
					strcat(chbuf,"@");
				else if (dwPriveledge & CHANNEL_PRIVELEDGE_VOICE)
					strcat(chbuf,"+");

				strcat(chbuf,chChannel->szName);
				strcat(chbuf," ");
			}
		}
	}

	if (bReturnMinusOne)
		return -1;
	else
	/* Fill in the specified buffer */
	{
		chbuf[strlen(chbuf) - 1] = 0;
		strcpy(pszOutputBuffer,chbuf);
		return 0;
	}
}

BOOL Client_IsOnChannel(CLIENT_STRUCT *csUser, CHANNEL_STRUCT *chChannel, DWORD *pdwPriveledge, LINKED_LIST_STRUCT **llReturnMode)
/*
** Client_IsOnChannel()
** Does a check to see if the user is on the channel specified
** If pdwPriveledge is specified, the priveledge of the user will be written to it.
*/
{
	LINKED_LIST_STRUCT *llChannel = NULL;

	if (!pdwPriveledge)
	/* Quick check mode(look thru the user structure) */
	{
		llChannel = LL_Find(&(csUser->user->llChannelHead),chChannel);

		if (llChannel)
			return TRUE;
	}
	else
	/* Check thru channel list so we can also return the priveledge */
	{
		LINKED_LIST_STRUCT *llUser = &(chChannel->llUserHead);
		LINKED_LIST_STRUCT *llUserMode = &(chChannel->llUserModeHead);

		*pdwPriveledge = NULL;

		while (llUser->next)
		/* Go through the user list of this channel */
		{
			llUser = llUser->next;
			llUserMode = llUserMode->next;

			if (
				((CLIENT_STRUCT*)(llUser->data)) == csUser
				)
			/* Found our user! */
			{
				*pdwPriveledge = (DWORD)llUserMode->data;

				if (llReturnMode)
					*llReturnMode = llUserMode;

				return TRUE;
			}
		}
	}

	return FALSE;
}

int	Client_IsOnAccessList(CLIENT_STRUCT *csUser,ACCESS_STRUCT *accList, LINKED_LIST_STRUCT *llAccessHead)
/*
** Client_IsOnAccessList()
** Checks if the target user is on the specified access list, and if so, returns the index (even 0)
** Returns -1 if not found
*/
{
	int nAccessCount, nCount, accesstbl[5] = { ACCESSLEVEL_OWNER, ACCESSLEVEL_HOST, ACCESSLEVEL_VOICE, ACCESSLEVEL_GRANT, ACCESSLEVEL_DENY };
	char szClientMask[256], szHostMask[256];
	LINKED_LIST_STRUCT *llNode = llAccessHead;
	ACCESS_STRUCT *accInfo = NULL;

	strcpy(szClientMask,Client_Nickname(csUser));
	strcat(szClientMask,"!");
	strcat(szClientMask,csUser->user->username);
	strcat(szClientMask,"@");
	strcat(szClientMask,csUser->hostname);

	if (llAccessHead)
	/* Linked lists will only have grant/deny access */
	{
		accesstbl[0] = ACCESSLEVEL_GRANT;
		accesstbl[1] = ACCESSLEVEL_DENY;
		accesstbl[2] = 0;
	}

	for (nAccessCount = 0; (nAccessCount < 5 || accesstbl[nAccessCount]); nAccessCount++)
	{
		if (!llAccessHead)
		{
			for (nCount = 0; nCount < SettingsInfo.isSecurity.max_access; nCount++)
			{
				if (accList[nCount].dwLevel == accesstbl[nAccessCount] && accList[nCount].szHostmask[0])
				{
					/* Omit the server portion */
					strcpy(szHostMask,accList[nCount].szHostmask);
					strchr(szHostMask,'$')[0] = 0;

					if (match(szHostMask,szClientMask) == 0)
					/* We have a match! */
						return nCount;
				}
			}
		}
		else
		{
			llNode = llAccessHead;

			while (llNode->next)
			{
				llNode = llNode->next;
				
				accInfo = (ACCESS_STRUCT*)llNode->data;

				if (accInfo->dwLevel == accesstbl[nAccessCount])
				/* Check for a match */
				{
					strcpy(szHostMask,accInfo->szHostmask);
					strchr(szHostMask,'$')[0] = 0;

					if (match(szHostMask,szClientMask) == 0)
					/* We have a match! */
						return (int)accInfo;
				}

			}
		}
	}

	return -1;
}

void Server_Broadcast(CLIENT_STRUCT *csOrigin, const char *message, ...)
/*
** Server_Broadcast()
** Broadcasts a message to all servers
** The c_exclude_server parameter is optional, and will broadcast to all servers except it.
*/
{
	LINKED_LIST_STRUCT *curr = NULL;
	CLIENT_STRUCT *c_server = NULL;
	va_list vArgs;
	char buf[2048];

	va_start(vArgs,message);
	vsprintf(buf,message,vArgs);
	va_end(vArgs);

	/* All our servers are one hop away */
	curr = &scs.llServerHead[1];

	while (curr->next)
	/* Go thru our servers and broadcast this! */
	{
		curr = curr->next;

		c_server = (CLIENT_STRUCT*)curr->data;

		if ((SettingsInfo.isServers.networktype == NETWORK_TYPE_STAR || 
			scs.networkstate == NETWORK_TYPE_STAR) &&
			c_server != csOrigin->servernext)
		/* Star networks: broadcast message but skip origin server */
			Client_SendToOne(c_server,FALSE,buf);
		else if (SettingsInfo.isServers.networktype == NETWORK_TYPE_MESH &&
			c_server != scs.sclient && c_server != csOrigin)
		/* Send to all connected servers */
			Client_SendToOne(c_server,FALSE,buf);
	}

}

void Server_BroadcastFromUser(CLIENT_STRUCT *csUser, const char *szToken, const char *szMessage, const char *szBuffer, ...)
/*
** Server_BroadcastFromUser()
** Broadcasts a message to all servers, but with the syntax like Channel_BroadcastToAllLocal()
** Sends message in all directions except the link leading to csUser.
*/
{
	LINKED_LIST_STRUCT *curr = NULL;
	CLIENT_STRUCT *c_server = NULL;
	va_list vArgs;
	char buf[2048];

	va_start(vArgs,szBuffer);
	vsprintf(buf,szBuffer,vArgs);
	va_end(vArgs);

	/* All our servers are one hop away */
	curr = &scs.llServerHead[1];

	while (curr->next)
	/* Go thru our servers and broadcast this! */
	{
		curr = curr->next;

		c_server = (CLIENT_STRUCT*)curr->data;

		if (((Client_IsServer(csUser) && c_server != csUser->servernext) || (Client_IsUser(csUser) && c_server != csUser->servernext)) &&
			(SettingsInfo.isServers.networktype == NETWORK_TYPE_STAR || 
			scs.networkstate == NETWORK_TYPE_STAR))
		/* Star networks: broadcast this message but skip the source server */
		{
			if (Client_IsUser(csUser))
				if (szToken)
					Client_SendToOne(c_server,FALSE,":%s!~%s@%s %s %s",csUser->user->nickname,csUser->user->username,csUser->hostname,szToken,buf);
				else
					Client_SendToOne(c_server,FALSE,":%s!~%s@%s %s %s",csUser->user->nickname,csUser->user->username,csUser->hostname,szMessage,buf);
			else
				if (szToken)
					Client_SendToOne(c_server,FALSE,":%d %s %s",csUser->server->id,szToken,buf);
				else
					Client_SendToOne(c_server,FALSE,":%d %s %s",csUser->server->id,szMessage,buf);				
		}
		else if (Client_IsLocal(csUser) &&
			SettingsInfo.isServers.networktype == NETWORK_TYPE_MESH)
		/* Mesh networks: Send to all servers, but only if client is local */
		{
			if (Client_IsUser(csUser))
				if (szToken)
					Client_SendToOne(c_server,FALSE,":%s!~%s@%s %s %s",csUser->user->nickname,csUser->user->username,csUser->hostname,szToken,buf);
				else
					Client_SendToOne(c_server,FALSE,":%s!~%s@%s %s %s",csUser->user->nickname,csUser->user->username,csUser->hostname,szMessage,buf);
			else
				if (szToken)
					Client_SendToOne(c_server,FALSE,":%d %s %s",csUser->server->id,szToken,buf);
				else
					Client_SendToOne(c_server,FALSE,":%d %s %s",csUser->server->id,szMessage,buf);	
		}
	}

}

void Server_SendUserInfo(CLIENT_STRUCT *csServer)
/*
** Server_SendUserInfo()
** Sends all user information about the network to target server
*/
{
	int count;
	char buffer[8192], sub_buf[512], szModeStr[32];
	CLIENT_STRUCT *c_target, *c_server;
	LINKED_LIST_STRUCT *curr;

	buffer[0] = 0;

	for (count = 0; count < MAX_HOPS; count++)
	/* Step #3: Send all known user information */
	{
		curr = &scs.llUserHead[count];

		while (curr->next)
		{
			/* Get next user */
			curr = curr->next;
			c_target = (CLIENT_STRUCT*)curr->data;
			c_server = Client_GetServer(c_target);

			if (c_target->servernext != csServer)
			/* Make sure we don't include users they already know about */
			{
				User_GetModeString(c_target,szModeStr,TRUE);

				sprintf(sub_buf,":%d " TOK_USERDATA " %s %d %s %s %d +%s %u :%s\r\n",
					c_server->server->id,
					c_target->user->nickname,
					c_target->hops,
					c_target->user->username,
					c_target->hostname,
					c_server->server->id,
					szModeStr,
					(unsigned long)c_target->signon,
					c_target->user->fullname);

				strcat(buffer,sub_buf);
			}
		}
	}

	/* No more users to send */
	sprintf(sub_buf,":%d " TOK_SERVERSYNC "\r\n",scs.sclient->server->id);
	strcat(buffer,sub_buf);

	StrBuf_Add(&csServer->sendQ,buffer,strlen(buffer));
}

void Server_SendServerInfo(CLIENT_STRUCT *csServer)
/*
** Server_SendServerInfo()
** Sends all server information about the network to target server
*/
{
	int count;
	char buffer[8192], sub_buf[512], szModeStr[32];
	CLIENT_STRUCT *c_info;
	LINKED_LIST_STRUCT *curr;

	buffer[0] = 0;

	/* First, include the info about this server */
	sprintf(sub_buf,":%s " TOK_SERVER " %s 1 %d :%s\r\n",scs.sclient->server->name,scs.sclient->server->name,scs.sclient->server->id,scs.sclient->server->description);
	strcat(buffer,sub_buf);


	/* Mesh networks do not send info about other servers */
	if (SettingsInfo.isServers.networktype == NETWORK_TYPE_STAR)
	{
		/* Next, send information about all other known servers */
		/* Skip servers which have servernext == csServer */
		for (count = 0; count < MAX_HOPS; count++)
		/* Read servers from linked lists(referenced by the number of hops) */
		{
			curr = &scs.llServerHead[count];

			while (curr->next)
			/* Get next server which is <count> hops away */
			{
				curr = curr->next;

				c_info = (CLIENT_STRUCT*)curr->data;

				if (c_info->servernext != csServer->servernext)
				{
					sprintf(sub_buf,":%d " TOK_SERVER " %s %d %d :%s\r\n",scs.sclient->server->id,c_info->server->name,c_info->hops,c_info->server->id,c_info->server->description);
					strcat(buffer,sub_buf);
				}
			}
		}
	}

	/* No more servers to send */
	sprintf(sub_buf,":%d " TOK_SERVERSYNC "\r\n",scs.sclient->server->id);
	strcat(buffer,sub_buf);

	StrBuf_Add(&csServer->sendQ,buffer,strlen(buffer));
}

void Server_SendChannelInfo(CLIENT_STRUCT *csServer, CHANNEL_STRUCT *chTarget)
/*
** Server_SendChannelInfo()
** Sends all channel information about the network to target server
*/
{
	int count, len;
	char buffer[8192], sub_sub_buf[1024], sub_buf[8192], szMode[32], szNameslist[8192], szAccBuf[1024];
	CLIENT_STRUCT *c_info;
	CHANNEL_STRUCT *chChannel;
	LINKED_LIST_STRUCT *llChannel = &(scs.llChannelHead);
	LINKED_LIST_STRUCT llDummyHead;

	buffer[0] = 0;

	if (chTarget)
	/* Create dummy linked list with one channel */
	{
		llDummyHead.data = NULL;
		llDummyHead.next = NULL;

		LL_Add(&llDummyHead,(void*)chTarget);
		llChannel = &llDummyHead;
	}

	while (llChannel->next)
	{
		llChannel = llChannel->next;

		chChannel = (CHANNEL_STRUCT*)(llChannel->data);

		if (chChannel->csServerOrigin != csServer)
		{
			/* Get our modes and names list for our NJOIN message */
			Channel_GetModeString(chChannel,szMode);
			Channel_CreateUserlist(chChannel,szNameslist,sizeof(szNameslist));

			sprintf(sub_buf,":%d " TOK_NJOIN " 1 %s %d %s %d :%s" CRLF,
				scs.sclient->server->id,
				chChannel->szName,
				chChannel->dwPropCreationTime,
				szMode,
				chChannel->dwLimit,
				szNameslist);

			strcat(buffer,sub_buf);

	//** Control mode 1 syntax (initial): NJOIN 1 <name> <timestamp> <modes> <limit> <User list>
	//** Control mode 2 syntax (access entries): NJOIN 2 <name> <# entries> <AE1 length> <AccessEntry1> <AE2 length> <AccessEntry2>
	//** Control mode 3 syntax (properties): NJOIN 3 <name> <Channel property list>

			/* Seperate access entries by commas, and include the length to prevent user-included commands from screwing things up */

			sprintf(sub_buf,":%d " TOK_NJOIN " 2 %s %d ",scs.sclient->server->id,chChannel->szName,chChannel->dwAccessEntries);

			for (count = 0; count < chChannel->dwAccessEntries; count++)
			{
				sprintf(szAccBuf,"%c,%d,%d,%s,%s,%s",
													(chChannel->accList[count].bOwner == TRUE ? 'T' : 'F'),
													chChannel->accList[count].dwLevel,
													chChannel->accList[count].dwTimeout,
													chChannel->accList[count].szCreator,
													chChannel->accList[count].szHostmask,
													(chChannel->accList[count].szReason[0] ? chChannel->accList[count].szReason : " "));

				/* Append to sub bufffer */
				len = strlen(szAccBuf);
				sprintf(sub_sub_buf,"%d :%s",len,szAccBuf);
				strcat(sub_buf,sub_sub_buf);
			}

			/* Include access entries in channel info buffer */
			strcat(buffer,sub_buf);
			strcat(buffer,CRLF);
			
			/* Send channel properties */
			sprintf(sub_buf,":%d " TOK_NJOIN " 3 %s :%u,%u,%u,%u,%u,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s" CRLF,
				scs.sclient->server->id,
				chChannel->szName,

				/* Channel properties */
				chChannel->dwPropOID,													/* Internal object identifier */
				chChannel->dwPropCreationTime,											/* Channel creation time */
				chChannel->dwPropClientGUID,											/* Client GUID */
				chChannel->dwPropServicepath,											/* Path of server side extension */
				chChannel->dwPropLag,													/* Channel lag(artificial delay) */
				(chChannel->szPropLanguage[0] ? chChannel->szPropLanguage : " "),		/* Official language of channel */
				(chChannel->szPropOwnerkey[0] ? chChannel->szPropOwnerkey : " "),		/* Ownerkey for channel */
				(chChannel->szPropHostkey[0] ? chChannel->szPropHostkey : " "),			/* Hostkey for channel  */
				(chChannel->szPropMemberkey[0] ? chChannel->szPropMemberkey : " "),		/* Memberkey for channel */
				(chChannel->szPropPICS[0] ? chChannel->szPropPICS : " "),				/* PICS rating */
				(chChannel->szPropTopic[0] ? chChannel->szPropTopic : " "),				/* Channel topic */
				(chChannel->szPropSubject[0] ? chChannel->szPropSubject : " "),			/* Channel subject */
				(chChannel->szPropClient[0] ? chChannel->szPropClient : " "),			/* Channel CLIENT property */
				(chChannel->szPropOnjoin[0] ? chChannel->szPropOnjoin : " "),			/* Channel ONJOIN */
				(chChannel->szPropOnpart[0] ? chChannel->szPropOnpart : " "),			/* Channel ONPART */
				(chChannel->szPropAccount[0] ? chChannel->szPropAccount : " "));		/* Channel account */

			strcat(buffer,sub_buf);
			strcat(buffer,CRLF);
		}
	}

	/* No more channels to send */
	sprintf(sub_buf,":%d " TOK_SERVERSYNC "\r\n",scs.sclient->server->id);
	strcat(buffer,sub_buf);

	StrBuf_Add(&csServer->sendQ,buffer,strlen(buffer));

	if (chTarget)
		LL_Clear(&llDummyHead);
}

CLIENT_STRUCT *Client_GetServer(CLIENT_STRUCT *csTarget)
/*
** User_GetServer()
** This function returns the server information for the user specified.
*/
{
	CLIENT_STRUCT *cret = NULL;

	cret = (CLIENT_STRUCT*)(HT_Server.HashEntry[csTarget->serverkey]->chainhead.next->data);
    
	return cret;
}

CLIENT_STRUCT *User_Find(const char *nickname)
/*
** User_Find()
** Finds the user structure with the nickname specified
** Returns: NULL if nickname not found, or pointer to client with nickname
*/
{
	CLIENT_STRUCT *cret = NULL, *client = NULL;
	HASH_FIND_STRUCT FindInfo;
	LINKED_LIST_STRUCT *curr = NULL;

	/* Find our user in the hash table */
	FindInfo.textkey = nickname;
	FindInfo.data = NULL;

	Hash_Find(&HT_User,&FindInfo,HASH_NOKEY);

	if (FindInfo.found)
	/* The nickname possibly exists */
	{
		curr = &(FindInfo.found->chainhead);

		while (curr->next)
		{
			curr = curr->next;

			client = (CLIENT_STRUCT*)curr->data;

			if (stricmp(nickname,client->user->nickname) == 0)
			/* Nickname has been found */
			{
				cret = client;
				break;
			}
		}
	}

	return cret;
}

void Parse_UserList(const char *szUserList, LINKED_LIST_STRUCT *llUserHead)
/*
** Parse_UserList()
** Takes a list of users and puts them in the specified linked list.
** Any users which do not exist will be placed in the list as the value NULL
** Any parameters specified as "*" will have entries with the value USER_ALLUSERS returned
*/
{
	CLIENT_STRUCT *csUser = NULL;

	int count, tokens = numtok(szUserList,',');
	char userbuf[512];

	llUserHead->data = NULL; llUserHead->next = NULL;

	for (count = 1; count <= tokens; count++)
	/* Loop through the channels seperated by commas */
	{
		csUser = User_Find(gettok(userbuf,szUserList,count,','));

		if (userbuf[0] == '*' && userbuf[1] == 0)
		/* Targeting all users */
			LL_AddNoCheck(llUserHead,(void*)USER_ALLUSERS);
		else if (csUser == NULL)
		/* User is invalid */
			LL_AddNoCheck(llUserHead,NULL);
		else
		/* User is valid(but we dont wan't duplicate entries) */
			LL_Add(llUserHead,(void*)csUser);
	}
}

CLIENT_STRUCT *Server_HashFind(const char *servername)
/*
** Server_HashFind()
** Finds the server client structure with the name specified
** Returns: NULL if server not found, or pointer to client containing server with name
*/
{
	CLIENT_STRUCT *cret = NULL, *client = NULL;
	HASH_FIND_STRUCT FindInfo;
	LINKED_LIST_STRUCT *curr = NULL;

	/* Find our server in the hash table */
	FindInfo.textkey = servername;
	FindInfo.data = NULL;

	Hash_Find(&HT_Server,&FindInfo,HASH_NOKEY);

	if (FindInfo.found)
	/* The server possibly exists */
	{
		curr = &(FindInfo.found->chainhead);

		while (curr->next)
		{
			curr = curr->next;

			client = (CLIENT_STRUCT*)curr->data;

			if (stricmp(servername,client->server->name) == 0)
			/* Server has been found */
			{
				cret = client;
				break;
			}
		}
	}

	return cret;
}

void User_GetModeString(CLIENT_STRUCT *client, char *pszStringOutput, BOOL bOmitGagMode)
/*
** User_GetModeString()
** Retrieves the modes for the specified user
*/
{
	int nIndex = 0;

	pszStringOutput[nIndex] = 0;

	if (Client_IsAdmin(client))
		pszStringOutput[nIndex++] = 'a';
	if (Client_IsSysop(client))
		pszStringOutput[nIndex++] = 'o';
	if (Client_Invisible(client))
		pszStringOutput[nIndex++] = 'i';
	if (client->user->bInIRCX == TRUE)
		pszStringOutput[nIndex++] = 'x';
	if (Client_IsGagged(client) && bOmitGagMode == FALSE)
		pszStringOutput[nIndex++] = 'z';
	
	pszStringOutput[nIndex] = 0;
}

int User_GetCommonChannels(CLIENT_STRUCT *client, CLIENT_STRUCT *csTarget, char *szChannelBuffer, LINKED_LIST_STRUCT *llChannelHead)
/*
** User_GetCommonChannels()
** Fills the output buffer (optional, pass NULL if not needed) and returns the # of common channels between two users
*/
{
	LINKED_LIST_STRUCT *llCH_Client = &(client->user->llChannelHead);
	LINKED_LIST_STRUCT *llCH_Target = NULL;

	int nCount = 0;

	if (szChannelBuffer)
		szChannelBuffer[0] = 0;

	/* Loop through channels of "requesting user" */
	while (llCH_Client->next)
	{
		llCH_Client = llCH_Client->next;

		llCH_Target = LL_Find(&(csTarget->user->llChannelHead),llCH_Client->data);

		if (llCH_Target)
		/* Common channel found */
		{
			nCount++;

			if (szChannelBuffer)
			/* Add to optional buffer */
			{
				strcat(szChannelBuffer,((CHANNEL_STRUCT*)(llCH_Target->data))->szName);
				strcat(szChannelBuffer," ");
			}
			if (llChannelHead)
			/* Add to optional linked list */
				LL_Add(llChannelHead,llCH_Target->data);
		}
	}

	if (nCount > 0 && szChannelBuffer)
	/* Cut off trailing space */
		szChannelBuffer[strlen(szChannelBuffer) - 1] = 0;

	return nCount;
}

void User_BroadcastToAllLocalsInAllChannels(CLIENT_STRUCT *csUser, CLIENT_STRUCT *csExcludeUser, char *szMessage, ...)
/*
** User_BroadcastToAllLocalsInAllChannels()
** The function name is long enough to say what it does!
** It simply calls Channel_BroadcastToLocal for all channels for the specified user
*/
{
	va_list vArgs;
	char buf[2048];
	BOOL bSend = TRUE;

	LINKED_LIST_STRUCT llChannelHead, *llChannel = &llChannelHead, *llUser = &(scs.llUserHead[1]);
	CLIENT_STRUCT *csUserLoop = NULL;
	CHANNEL_STRUCT *chChannel = NULL;
	DWORD dwPrivS = 0, dwPrivD = 0;

	llChannelHead.next = NULL;
	llChannelHead.data = NULL;

	va_start(vArgs,szMessage);
	vsprintf(buf,szMessage,vArgs);
	va_end(vArgs);

	while (llUser->next)
	/* Loop through all users and if a common channel is shared, broadcast the message */
	{
		llUser = llUser->next;

		csUserLoop = (CLIENT_STRUCT*)(llUser->data);

		if (csUserLoop != csExcludeUser && User_GetCommonChannels(csUser,csUserLoop,NULL,&llChannelHead) > 0)
		/* User shares common channels */
		{
			while (llChannel->next)
			{
				llChannel = llChannel->next;

				chChannel = (CHANNEL_STRUCT*)(llChannel->data);
				
				if (Channel_Auditorium(chChannel))
				{
					/* Check if either source or target are hosts */
					Client_IsOnChannel(csUser,chChannel,&dwPrivS,NULL);
					Client_IsOnChannel(csUserLoop,chChannel,&dwPrivD,NULL);

					if (!dwPrivS && !dwPrivD)
						bSend = FALSE;
				}
			}


			if (bSend)
			{
				/* Determine highest priveledge level the target user shares in a common channel with source user */
				if (User_DoWeMask(csUser,csUserLoop))
				/* Send message masked */
					Client_SendToOne(csUserLoop,FALSE,":%s!~%s@%s %s",csUser->user->nickname,csUser->user->username,csUser->hostmask,buf);
				else
				/* Send message unmasked */
					Client_SendToOne(csUserLoop,FALSE,":%s!~%s@%s %s",csUser->user->nickname,csUser->user->username,csUser->hostname,buf);
			}
		}
	}

	LL_Clear(&llChannelHead);
}

BOOL User_DoWeMask(CLIENT_STRUCT *csFrom, CLIENT_STRUCT *csTo)
/*
** User_GetMaskLevel()
** Determines wether a message sent from one user(csFrom) to another (csTo) should be masked.
** Returns TRUE if the message is to be masked, FALSE if the message is to be sent free and clear
*/
{
	DWORD dwPriveledge = 0;
	LINKED_LIST_STRUCT llChannelHead;
	LINKED_LIST_STRUCT *llChannel = &llChannelHead;

	llChannel->data = NULL;
	llChannel->next = NULL;

	if (Client_IsPriveledged(csTo) || csFrom == csTo)
		return FALSE;

	if (User_GetCommonChannels(csFrom,csTo,NULL,llChannel) > 0)
	/* Determine if the "to" user is a host/owner in any of the channels the "from" user is in */
	{
		while (llChannel->next)
		{
			llChannel = llChannel->next;

			if (Client_IsOnChannel(csTo,(CHANNEL_STRUCT*)(llChannel->data),&dwPriveledge,NULL))
			/* Check priveledges of target user on this channel */
			{
				if (dwPriveledge & CHANNEL_PRIVELEDGE_OWNER || dwPriveledge & CHANNEL_PRIVELEDGE_HOST)
				{
					LL_Clear(&llChannelHead);
					return FALSE;
				}
			}
		}

		LL_Clear(&llChannelHead);
	}

	return TRUE;
}

void User_Initiate(CLIENT_STRUCT *client)
/*
** User_Initiate()
** Brings a new user on the server!
*/
{
	char buf[1024];
	char szModeStr[256];

	int nIndex = 0, nCount = 0;

	/* Any more connections available? */
	if (scs.lusers.nLocalUsers >= SettingsInfo.isStatus.localmax ||
		scs.lusers.nGlobalMax >= SettingsInfo.isStatus.globalmax)
	{
		Client_SendErrorNumber(client,TRUE,ERROR_003);
		return;
	}

	/* Check if the user already has clones */
	for (nIndex = 0; nIndex < scs.fdHighest; nIndex++)
	{
		if (scs.CSLocal[nIndex])
		/* Valid client */
		{
			if (scs.CSLocal[nIndex]->ip == client->ip)
				nCount++;

			if (SettingsInfo.isUser.max_perip && nCount > SettingsInfo.isUser.max_perip)
			{
				Client_SendErrorNumber(client,TRUE,ERROR_012);
				Event_Broadcast(client,EVENT_TYPE_CONNECT,"CONNECT IPLIMIT %s!%s@%s %s:%u %d",Client_Nickname(client),client->user->username,client->hostname,client->ip_r,client->port_r,SettingsInfo.isUser.max_perip);
				return;
			}
		}
	}

	Client_SendWelcome(client);
	Client_SendLusers(client);
	Client_SendMOTD(client);
	client->state = CLIENT_STATE_NORMAL;
	client->flags |= CLIENT_FLAG_REGISTERED;

	/* Send USER LOGON event */
	Event_Broadcast(client,EVENT_TYPE_USER,"USER LOGON %s!%s@%s %s:%u %s:%u",Client_Nickname(client),client->user->username,client->hostname,client->ip_r,client->port_r,client->ip_l,client->port_l);

	/* Inject a JOIN command for autojoin */
	if (SettingsInfo.isUser.autojoinbuffer)
	{
		sprintf(buf,"JOIN %s\r\n",SettingsInfo.isUser.autojoinbuffer);
		StrBuf_Add(&client->recvQ,buf,strlen(buf));
	}

	User_GetModeString(client,szModeStr,FALSE);

	/* Let our other servers know of our guest */
	Server_Broadcast(client->servernext,":%d " TOK_USERDATA " %s %d %s %s %d +%s %u :%s",
		scs.sclient->server->id,
		client->user->nickname,
		client->hops,
		client->user->username,
		client->hostname,
		scs.sclient->server->id,
		szModeStr,
		(unsigned long)client->signon,
		client->user->fullname);
}

void User_ForceNickChange(CLIENT_STRUCT *client, const char *szNewNick, BOOL bRandom)
/*
** Executes the nickname change on the targeted client. Called during NICK and
** during nickname collisions
*/
{
	char szName[64];

	if (!bRandom)
	{
		if (!User_Find(szNewNick))
			strcpy(szName,szNewNick);
		else
		/* Can't force nick change with new nick, already on server */
			return;
	}
	else
	{
		/* Randomize seed */
		srand(GetTickCount());

		/* This is just to get it going... */
		strcpy(szName,Client_Nickname(client));

		while (User_Find(szName) != NULL)
			sprintf(szName,"Guest%6.6d",RandInt(1,1000000));

	}

	/* Broadcast to user first */
	Client_SendToOne(client,FALSE,":%s!~%s@%s NICK %s",client->user->nickname,client->user->username,client->hostname,szName);

	/* Broadcast to channels */
	User_BroadcastToAllLocalsInAllChannels(client,client,"NICK :%s",szName);

	/* Broadcast USER NICK event */
	Event_Broadcast(client,EVENT_TYPE_USER,"USER NICK %s!%s@%s %s:%u %s",Client_Nickname(client),client->user->username,client->hostname,client->ip_r,client->port_r,szName);
	
	/* Broadcast to servers */
	Server_BroadcastFromUser(client,TOK_NICK,MSG_NICK,szName);

	Hash_Delete(&HT_User,client->user->nickname,client);
	strcpy(client->user->nickname,szName);
	client->user->hashkey = Hash_Add(&HT_User,client->user->nickname,client);
	client->user->lastnick = GetTickCount();
}

int	Access_Add(ACCESS_STRUCT *accList, LINKED_LIST_STRUCT *llAccessHead, ACCESS_STRUCT *accInfo)
/*
** Access_Add()
** Attempts to add an access entry to the list. If llAccessHead is not NULL, it will be used instead of accList.
** Return values:
** 1+ = success & returns the current # of entries
** 0 = success,
** -1 = duplicate entry found
** -2 = access list is full
*/
{
	int nCount = 0, nAccessTotal = 0, nEntry = -1;
	BOOL bAdded = FALSE;
	ACCESS_STRUCT *accLocal = NULL;

	if (!llAccessHead)
	{
		for (nCount = 0; nCount < SettingsInfo.isSecurity.max_access; nCount++)
		/* Loop through access entries and find an available one */
		{
			if (accList[nCount].szHostmask[0])
			/* Check existing entry */
			{
				if (lstrcmpi(accList[nCount].szHostmask,accInfo->szHostmask) == 0)
				/* Duplicate entry */
					return -1;
				else
					nAccessTotal++;
			}
			else if (nEntry == -1)
			/* Add new entry into this position */
				nEntry = nCount;
		}

		if (nEntry != -1)
		/* Add entry */
		{
			if (nAccessTotal < SettingsInfo.isSecurity.max_access)
			/* Add the entry */
			{
				accList[nEntry].dwLevel = accInfo->dwLevel;
				accList[nEntry].dwTimeout = accInfo->dwTimeout;
				strcpy(accList[nEntry].szHostmask,accInfo->szHostmask);
				strcpy(accList[nEntry].szReason,accInfo->szReason);
				strcpy(accList[nEntry].szCreator,accInfo->szCreator);
				accList[nEntry].bOwner = accInfo->bOwner;

				nAccessTotal = nAccessTotal++;
				bAdded = TRUE;
			}
		}
	}
	else
	/* Add access entry to linked list instead of channel list */
	{
		while (llAccessHead->next)
		{
			llAccessHead = llAccessHead->next;
			nAccessTotal++;

			accLocal = (ACCESS_STRUCT*)llAccessHead->data;

			if (lstrcmpi(accInfo->szHostmask,accLocal->szHostmask) == 0)
				return -1;
		}

		/* Might as well create a new link since we've already traversed the list */
		llAccessHead->next = LLNode_Alloc();
		llAccessHead = llAccessHead->next;

		/* Create new ACCESS_STRUCT and assign it to the list via a pointer */
		llAccessHead->data = (void*)((ACCESS_STRUCT*)calloc(1,sizeof(ACCESS_STRUCT)));
		llAccessHead->next = NULL;

		/* Set elements of access entry */
		accLocal = (ACCESS_STRUCT*)(llAccessHead->data);
		accLocal->dwLevel = accInfo->dwLevel;
		accLocal->dwTimeout = accInfo->dwTimeout;
		strcpy(accLocal->szHostmask,accInfo->szHostmask);
		strcpy(accLocal->szReason,accInfo->szReason);
		strcpy(accLocal->szCreator,accInfo->szCreator);
		accLocal->bOwner = accInfo->bOwner;

		nAccessTotal++;
		bAdded = TRUE;
	}

	if (bAdded == FALSE)
	/* Entry not added, list is full */
		return -2;
	else
		return nAccessTotal;
}
int	Access_Delete(ACCESS_STRUCT *accList, LINKED_LIST_STRUCT *llAccessHead, const char *szHostMask, DWORD dwLevel, BOOL bIsOwner)
/*
** Access_Delete()
** Deletes the access entry with the specified hostmask
** Return values:
** 0 == successfully deleted
** -1 == Cannot delete entry created by owners if host
** -2 == Entry not found
*/
{
	int nCount = 0;
	ACCESS_STRUCT *accInfo = NULL;
	LINKED_LIST_STRUCT *llNode = llAccessHead;

	if (!llAccessHead)
	/* Channel access table */
	{
		for (nCount = 0; nCount < SettingsInfo.isSecurity.max_access; nCount++)
		/* Attempt to find the access entry with the target hostmask */
		{
			if (dwLevel == accList[nCount].dwLevel && accList[nCount].szHostmask[0] && lstrcmpi(accList[nCount].szHostmask,szHostMask) == 0)
			/* Found the entry! */
			{
				if (accList[nCount].bOwner && !bIsOwner)
				/* Not enough priveledges */
					return -1;
				else
				{
					accList[nCount].szHostmask[0] = 0;
					return 0;
				}
			}
		}
	}
	else
	/* Linked list */
	{
		while (llNode->next)
		{
			llNode = llNode->next;

			accInfo = (ACCESS_STRUCT*)llNode->data;

			if (accInfo->dwLevel == dwLevel && lstrcmpi(accInfo->szHostmask,szHostMask) == 0)
			/* Found the entry! */
			{
				if (accInfo->bOwner && !bIsOwner)
				/* Not enough priveledges */
					return -1;
				else
				/* Delete the entry */
				{
					free(llNode->data);
					LL_Remove(llAccessHead,llNode->data);
					return 0;
				}
			}
		}
	}
	
	/* Cannot find entry */
	return -2;

}
ACCESS_STRUCT *Access_Find(ACCESS_STRUCT *accList, LINKED_LIST_STRUCT *llAccessHead, const char *szHostMask, DWORD dwLevel)
/*
** Access_Find()
** Finds the access entry with the specified hostmask
** Returns pointer to access entry or NULL if not found
*/
{
	int nCount = 0;
	ACCESS_STRUCT *accInfo = NULL;
	LINKED_LIST_STRUCT *llNode = llAccessHead;

	if (!llAccessHead)
	/* Channel access table */
	{
		for (nCount = 0; nCount < SettingsInfo.isSecurity.max_access; nCount++)
		/* Attempt to find the access entry with the target hostmask */
			if (dwLevel == accList[nCount].dwLevel && accList[nCount].szHostmask[0] && lstrcmpi(accList[nCount].szHostmask,szHostMask) == 0)
			/* Found the entry! */
				return &accList[nCount];
	}
	else
	/* Linked list */
	{
		while (llNode->next)
		{
			llNode = llNode->next;

			accInfo = (ACCESS_STRUCT*)llNode->data;

			if (accInfo->dwLevel == dwLevel && lstrcmpi(accInfo->szHostmask,szHostMask) == 0)
			/* Found the entry! */
				return accInfo;
		}
	}
	
	/* Cannot find entry */
	return NULL;
}

int	Access_Clear(ACCESS_STRUCT *accList, LINKED_LIST_STRUCT *llAccessHead, BOOL bIsOwner, DWORD dwLevel)
/*
** Access_Clear()
** Clears the access list
** Return values:
** 0+ = successful, returns the # of entries cleared
** <0 = could not clear all entries due to security, returns # cleared in negative form
*/
{
	int nCount = 0, nCleared = 0;
	BOOL bClearedAll = TRUE;
	LINKED_LIST_STRUCT *llNode = llAccessHead, *llPrev = NULL;
	ACCESS_STRUCT *accInfo = NULL;

	if (!llAccessHead)
	{
		for (nCount = 0; nCount < SettingsInfo.isSecurity.max_access; nCount++)
		{
			if (dwLevel == ACCESSLEVEL_ALL || (accList[nCount].szHostmask[0] && accList[nCount].dwLevel == dwLevel))
			{
				if ((accList[nCount].bOwner && !bIsOwner) || (accList[nCount].dwLevel == ACCESSLEVEL_OWNER && !bIsOwner))
				/* Skip this entry */
					bClearedAll = FALSE;
				else
				/* Clear the entry */
				{
					if (accList[nCount].szHostmask[0])
						nCleared++;

					accList[nCount].szHostmask[0] = 0;
				}
			}
		}
	}
	else
	/* Clear linked list */
	{
		while (llNode->next)
		{
			llPrev = llNode;
			llNode = llNode->next;

			accInfo = (ACCESS_STRUCT*)(llNode->data);

			if (dwLevel == ACCESSLEVEL_ALL || accInfo->dwLevel == dwLevel)
			{
				if (accInfo->bOwner && !bIsOwner)
				/* Skip this entry */
					bClearedAll = FALSE;
				else
				/* Clear the entry */
				{
					nCleared++;
					free(llNode->data);
					llPrev->next = llNode->next;

					LLNode_Free(llNode);
					llNode = llPrev;
				}
			}
		}
	}

	if (!bClearedAll)
		return -1;
	else
		return nCleared;
}

void Access_OutputEntries(CLIENT_STRUCT *client, char *szObject, LINKED_LIST_STRUCT *llAccessNetwork, LINKED_LIST_STRUCT *llAccessServer, CHANNEL_STRUCT *chTarget, CLIENT_STRUCT *csTarget)
/*
** Outputs the access entries for any parameters in the list which are non-NULL (client however is manditory)
*/
{
	BOOL bShowCreators = FALSE;
	LINKED_LIST_STRUCT *llNode = NULL;
	ACCESS_STRUCT *accInfo = NULL;
	char *szObjectLocal[128];

	if (llAccessServer)
	/* Change server name from $ to actual name */
		strcpy((char*)szObjectLocal,SettingsInfo.isGeneral.servername);
	else
		strcpy((char*)szObjectLocal,szObject);
		
	Client_SendNumericToOne(client,IRCRPL_ACCESSSTART,(const char*)szObjectLocal);

	if (llAccessNetwork || llAccessServer)
	{
		/* Same code for both network/server entries, so combine the two using llAccessNetwork for both */
		if (!llAccessNetwork)
			llAccessNetwork = llAccessServer;

		while (llAccessNetwork->next)
		{
			llAccessNetwork = llAccessNetwork->next;

			accInfo = (ACCESS_STRUCT*)(llAccessNetwork->data);

			Client_SendToOne(client,FALSE,":%s %3.3d %s %s %s %s %d %s :%s",SettingsInfo.isGeneral.servername,IRCRPL_ACCESSLIST,
				Client_Nickname(client),
				szObjectLocal,
				Access_GetTypeString(accInfo->dwLevel),
				accInfo->szHostmask,
				accInfo->dwTimeout,
				bShowCreators == TRUE ? accInfo->szCreator : "*",
				accInfo->szReason);
		}
	}
	else if (chTarget)
	/* List access entries for channel */
	{
		int nCount = 0;

		for (nCount = 0; nCount < SettingsInfo.isSecurity.max_access; nCount++)
		{
			accInfo = &chTarget->accList[nCount];

			if (accInfo->szHostmask[0])
			/* List the entry if it has a hostmask, otherwise skip it */
			{
				Client_SendToOne(client,FALSE,":%s %3.3d %s %s %s %s %d %s :%s",SettingsInfo.isGeneral.servername,IRCRPL_ACCESSLIST,
					Client_Nickname(client),
					szObjectLocal,
					Access_GetTypeString(accInfo->dwLevel),
					accInfo->szHostmask,
					accInfo->dwTimeout,
					bShowCreators == TRUE ? accInfo->szCreator : "*",
					accInfo->szReason);
			}
		}
	}
	else if (csTarget)
	{
		llNode = &csTarget->llAccessHead;

		while (llNode->next)
		{
			llNode = llNode->next;

			accInfo = (ACCESS_STRUCT*)llNode->data;


			Client_SendToOne(client,FALSE,":%s %3.3d %s %s %s %s %d %s :%s",SettingsInfo.isGeneral.servername,IRCRPL_ACCESSLIST,
				Client_Nickname(client),
				szObjectLocal,
				Access_GetTypeString(accInfo->dwLevel),
				accInfo->szHostmask,
				accInfo->dwTimeout,
				bShowCreators == TRUE ? accInfo->szCreator : "*",
				accInfo->szReason);
		}
	}

	Client_SendNumericToOne(client,IRCRPL_ACCESSEND,(const char*)szObjectLocal);
}

void Access_GetTypeFromString(char *szAccessString, DWORD *dwAccessType, BOOL bChannel)
/*
** Access_GetTypeFromString
** Takes the string and determines the access level
*/
{
	*dwAccessType = ACCESSLEVEL_ALL;

	if (lstrcmpi(szAccessString,"OWNER") == 0 && bChannel)
		*dwAccessType = ACCESSLEVEL_OWNER;
	else if (lstrcmpi(szAccessString,"HOST") == 0 && bChannel)
		*dwAccessType = ACCESSLEVEL_HOST;
	else if (lstrcmpi(szAccessString,"VOICE") == 0 && bChannel)
		*dwAccessType = ACCESSLEVEL_VOICE;
	else if (lstrcmpi(szAccessString,"GRANT") == 0)
		*dwAccessType = ACCESSLEVEL_GRANT; 
	else if (lstrcmpi(szAccessString,"DENY") == 0)
		*dwAccessType = ACCESSLEVEL_DENY;
}

void Event_Broadcast(CLIENT_STRUCT *client, DWORD dwEventType, char *szString, ...)
/*
** Event_Broadcast()
** Sends the specified event message out for all peering eyes to see!
*/
{
	va_list vArgs;
	char buf[1024];
	LINKED_LIST_STRUCT *llPtr = &(scs.llEventHead), *llPtr2 = NULL;
	CLIENT_STRUCT *csTarget = NULL;
	EVENT_STRUCT *esEvent = NULL;

	va_start(vArgs,szString);
	vsprintf(buf,szString,vArgs);
	va_end(vArgs);

	while (llPtr->next)
	/* Loop through each person who is recieving event entries */
	{
		llPtr = llPtr->next;

		csTarget = (CLIENT_STRUCT*)(llPtr->data);

		llPtr2 = &(csTarget->llEventHead);

		while (llPtr2->next)
		/* Loop through event entries and broadcast if there is a match */
		{
			llPtr2 = llPtr2->next;

			esEvent = (EVENT_STRUCT*)(llPtr2->data);

			if (esEvent->dwType == dwEventType)
			/* Match on event type, check hostmask next */
			{
				if (client)
				/* If client specified, check against list */
				{
					char szTemp[1024];

					sprintf(szTemp,"%s!%s@%s$%s",client->user->nickname,client->user->username,client->hostname,client->servername);

					if (match(esEvent->szHostmask,szTemp) == 0)
					/* We have a match, send the event */
					{
						Client_SendToOne(csTarget,FALSE,":%s EVENT %u %s",SettingsInfo.isGeneral.servername,(unsigned long*)time(NULL),buf);
						break;
					}
				}
				else
				/* Just send, an event type match is enough! */
				{
					Client_SendToOne(csTarget,FALSE,":%s EVENT %u %s",SettingsInfo.isGeneral.servername,(unsigned long*)time(NULL),buf);
				}
			}

		}

	}
}

void Parse_DestinationList(const char *szDestinationList, LINKED_LIST_STRUCT *llDestinationList)
/*
** Parse_DestinationList()
** Takes the list of comma-seperated destinations and returns pointers to newly-created DESTINATION_INFO_STRUCT's
** If both the pointers in the destination info struct are NULL, the target was not found
** This function will also remove duplicates.
** NOTE: LIST MUST BE CLEARED AFTER USING!!!! Do not forget to call LL_FreeAndClear(llDestinationList)!!!!
*/
{
	CHANNEL_STRUCT *chChannel = NULL;
	CLIENT_STRUCT *csUser = NULL;
	DESTINATION_INFO_STRUCT *disTarget, *disGeneric;
	LINKED_LIST_STRUCT *llDestinationHead = llDestinationList, *llNode = llDestinationList;
	BOOL bOnList = FALSE;

	int count, tokens = numtok(szDestinationList,',');
	char buf[512];

	for (count = 1; count <= tokens; count++)
	/* Loop through the destinations seperated by commas */
	{
		bOnList = FALSE;

		gettok(buf,szDestinationList,count,',');
		
		/* Create new destination info structure */
		disTarget = (DESTINATION_INFO_STRUCT*)calloc(1,sizeof(DESTINATION_INFO_STRUCT));
		strcpy(disTarget->szOriginalText,buf);

		chChannel = Channel_Find(buf);

		if (chChannel == NULL)
		/* No channel found */
            csUser = User_Find(buf);

		disTarget->chChannel = chChannel;
		disTarget->csUser = csUser;

		/* Find if the specified user/channel is already on the list, if not then add another */
		llNode = llDestinationHead;

		while (llNode->next)
		{
			llNode = llNode->next;

			disGeneric = (DESTINATION_INFO_STRUCT*)llNode->data;

			if ((disTarget->csUser && disGeneric->csUser == disTarget->csUser ) || 
				(disTarget->chChannel && disGeneric->chChannel == disTarget->chChannel))
			{
				bOnList = TRUE;
				llNode->next = NULL;
			}
		}

		if (!bOnList)
			LL_Add(llDestinationList,(void*)disTarget);
	}
}

int	Mesh_NetworkStatus()
/*
** Mesh_NetworkStatus()
** Checks the state of the servers and returns one of the four mesh status codes
*/
{
	LINKED_LIST_STRUCT *llServerPtr = &scs.llServerHead[1];
	CLIENT_STRUCT *csServer;
	BOOL bFullMesh = FALSE;
	LINKED_LIST_STRUCT *llPtr = &scs.meshctrl.llNodeHead;
	MESH_NODE *mnPtr;

	/* Check if still connecting to nodes */

	while (llPtr->next)
	{
		llPtr = llPtr->next;

		mnPtr = (MESH_NODE*)(llPtr->data);

		if (mnPtr->bConnected == FALSE)
			return MESH_NETWORKSTATUS_INIT;
	}

	/* All are connected */
	while (llServerPtr->next)
	/* If any servers are not fully meshed, the network is considered only partially so */
	{
		llServerPtr = llServerPtr->next;

		csServer = (CLIENT_STRUCT*)(llServerPtr->data);

		bFullMesh = csServer->server->meshed;

		if (bFullMesh == FALSE)
			return MESH_NETWORKSTATUS_PARTIAL;

	}

	return MESH_NETWORKSTATUS_FULL;
}

void Mesh_Nodelist_Setup()
/*
** Mesh_Nodelist_Setup
** Sets up a list of all inbound and outbound servers on linked list scs.
*/
{
	MESH_NODE *mnData = NULL;
	int nCount = 0;

	/* Go through server list */
	ServerList *sList = &SettingsInfo.isServers.serverhead;

	while (sList->next)
	{
		sList = sList->next;

		/* Add ServerList* for reference later */
		mnData = (MESH_NODE*)(calloc(1,sizeof(MESH_NODE)));
		strcpy(mnData->servername,sList->name);
		LL_Add(&scs.meshctrl.llNodeHead,(void*)(mnData));
		nCount++;
	}

	scs.meshctrl.total = nCount;

	return;
}

MESH_NODE *Mesh_Nodelist_Find(const char *szServerName)
/*
** Mesh_Nodelist_Find()
** Returns mesh node pointer corresponding to target server
*/
{
	MESH_NODE *mnPtr = NULL;
	LINKED_LIST_STRUCT *llPtr = &scs.meshctrl.llNodeHead;

	while (llPtr->next)
	{
		llPtr = llPtr->next;

		mnPtr = (MESH_NODE*)llPtr->data;

		if (lstrcmpi(mnPtr->servername,szServerName) == 0)
		/* Matching node found */
			return mnPtr;
	}

	return mnPtr;
}