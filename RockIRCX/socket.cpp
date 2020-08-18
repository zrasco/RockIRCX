/*
** socket.cpp
** This file contains all functions for initializing and cleaning up socket data for
** the RockIRCX server
*/

#include "socket.h"

void Socket_Startup()
/* Initialize socket subsystem */
{
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2,2),&wsaData))
	{
		MessageBox(NULL,"Please install Winsock 2 to run RockIRCX.","Fatal Error",MB_OK | MB_ICONERROR);
		_exit(0);
	}

	/* TEMP: Right-click on any outbound server to connect em! */
	//if (!OBC_Update())
	/* Unable to update outbound server list */
	//{
		/* TODO: Add stuff to handle OBC update failure */
	//}
}

void Socket_Thread(void*)
/*
** Socket_Thread()
** This is the main function for the secondary network thread, running in a loop it
** handles all I/O for the server
*/
{
	BOOL bExit = FALSE;
	DWORD dwUpdatetime = GetTickCount();

	while (!bExit)
	{
		/* Read signals */
		if (Socket_CheckSignals() == -1)
			/* Exit thread immediatley */
			return;

		/* Check remote administration */
		RA_Check();
                
		/* Perform lookup work */
		Lookup_Cycle(&SettingsInfo);

		/* Perform socket I/O operations */
		Socket_CheckIO(10);

		/* Flush all output buffers to sockets */
		Socket_FlushConnections();

		/* Update our dialog information every second */
		if (dwUpdatetime + 1000 <= GetTickCount())
		{
			dwUpdatetime = GetTickCount();
			UpdateStatus(hStatus);
		}
	}
}

void Socket_Cleanup()
/*
** Socket_Cleanup()
** Performs all deallocation functions
*/
{
	int count;
	CLIENT_STRUCT *client = NULL;

	/* De-allocate hash tables */
	Hash_Destroy(&HT_Channel);
	Hash_Destroy(&HT_User);
	Hash_Destroy(&HT_Server);

	/* De-allocate clients */
	for (count = (int)scs.fdHighest; count >= 0; count--)
	{
		client = scs.CSLocal[count];

		if (!client)
			continue;

		Client_Destroy(client);
	}

	if (scs.sclient)
	/* Free the local server info */
	{
		free(scs.sclient->server);
		free(scs.sclient);
	}

	WSACleanup();
}

int	Socket_SetupListeners()
/*
** Socket_SetupListeners()
** Sets up all listening sockets for regular server activity(not RA)
*/
{
	BOOL bServerOn = FALSE;
	int count;
	char ip[32], buf[256];
	unsigned short port;
	struct sockaddr_in sin;

	memset(&sin,0,sizeof(sin));

	sin.sin_family = AF_INET;

	for (count = 0; count < SettingsInfo.isGeneral.bindingnum; count++)
	/* Add all bindings */
	{
		BindingBuf_GetInfo(SettingsInfo.isGeneral.bindingbuf,(unsigned short)count,ip,&port);

		sin.sin_addr.s_addr = inet_addr(ip);
		sin.sin_port = htons(port);

		scs.fdListening[count] = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

		if (bind(scs.fdListening[count],(struct sockaddr*)&sin,sizeof(struct sockaddr)) == 0)
		/* Bound listening port */
		{
			sprintf(buf,"IRCX listening on IP %s, port %d",ip,port);
			TechLog_AddEntry(buf,BITMAP_OK);
			SetNonBlocking(scs.fdListening[count]);
			listen(scs.fdListening[count],S_CONNECTION_BACKLOG);
			bServerOn = TRUE;
			SettingsInfo.isIOControl.listeningsockets++;
			SettingsInfo.isIOControl.totaldescriptors++;
		}
		else
		/* Unable to bind port */
		{
			sprintf(buf,"Error binding IRCX to IP %s, port %d: %d",ip,port,WSAGetLastError());
			TechLog_AddEntry(buf,BITMAP_WARNING);

			closesocket(scs.fdListening[count]);
			scs.fdListening[count] = INVALID_SOCKET;
		}
	}

	if (bServerOn)
	/* Server to recieve client information */
	{
		CLIENT_STRUCT *slocal;

		slocal = scs.sclient = (CLIENT_STRUCT*)calloc(sizeof(CLIENT_STRUCT),1);

		slocal->server = (SERVER_STRUCT*)calloc(sizeof(SERVER_STRUCT),1);

		strcpy(slocal->server->name,SettingsInfo.isGeneral.servername);
		slocal->server->id = SettingsInfo.isGeneral.serverid;
		scs.CSServers[slocal->server->id] = slocal;
		strcpy(slocal->server->description,SettingsInfo.isGeneral.serverdescription);
		slocal->signon = time(NULL);
	}

	if (scs.sclient)
		scs.lusers.nGlobalServers++;

	scs.fdListening[count] = INVALID_SOCKET;

	return 0;
}

void Socket_FlushConnections()
/*
** Socket_FlushConnections()
** Flushes all open connection buffers to their sockets
*/
{
	int count;

	for (count = (int)scs.fdHighest; count >= 0; count--)
		if (scs.CSLocal[count])
			Client_FlushBuffer(scs.CSLocal[count]);
}

int Socket_CheckSignals()
/*
** Socket_CheckSignals()
** Will check signals from other threads, and process them here.
** Returns: -1 to terminate the I/O thread
*/
{
	switch (scs.signal)
	{
		case SIGNAL_NONE:
		break;
		case SIGNAL_SHUTDOWN:
		/* Time to shut the server down */
		{
			Client_SendErrorNumberToAllLocal(ERROR_005,TRUE);
			//Client_SendToAllLocal(TRUE,":%s NOTICE AUTH :*** Server shutdown initiated from local program...",SettingsInfo.isGeneral.servername);

			/* Clear all channels */
			while (scs.llChannelHead.next)
			{
				Channel_DeleteAllUsers((CHANNEL_STRUCT*)scs.llChannelHead.next->data);
				Channel_Cleanup((CHANNEL_STRUCT*)scs.llChannelHead.next->data,FALSE);
			}

			/* Clear global lists */
			LL_Clear(&scs.llEventHead);
			LL_Clear(&scs.llAccessNetworkHead);
			LL_ClearAndFree(&scs.meshctrl.llNodeHead);
			
			/* On the next pass, terminate the thread */
			scs.signal = SIGNAL_ENDTHREAD;
		}
		break;
		case SIGNAL_ENDTHREAD:
		/* Signal the primary thread to close */
		{
			scs.signal = SIGNAL_THREADDONE;
			return -1;
		}
		break;
	}

	return 0;
}

void Socket_CheckIO(time_t delay)
/*
** Socket_CheckIO()
** This function controls all input and output for user/server/proxy detector connections
*/
{
	FD_SET read_set, write_set, except_set;
	CLIENT_STRUCT *client;
	LINKED_LIST_STRUCT *curr = NULL;
	struct timeval wait;
	int count, nfds;
	char buf[256], errbuf[256];
	unsigned long tickcount = GetTickCount(), placeholder;
	struct sockaddr_in sin;
	BOOL bOBC = FALSE;

	FD_ZERO(&read_set);
	FD_ZERO(&write_set);
	FD_ZERO(&except_set);

	for (count = (int)scs.fdHighest; count >= 0; count--)
	/* Add our read/write descriptors */	
	{
		if (!(client = scs.CSLocal[count]))
			continue;

		FD_SET(count,&read_set);

		if (!StrBuf_IsEmpty(&client->sendQ))
		{
			FD_SET(count,&write_set);
		}
	}

	for (count = 0; count < MAX_SERVERCONNECTIONS; count++)
	/* Check our outbound connecting sockets */
	{
		bOBC = FALSE;

		if (scs.obc[count].state == OBC_STATE_WAITING &&
			tickcount > scs.obc[count].timer)
		/* Time to attempt this connection */
		{
			scs.obc[count].state = OBC_STATE_PENDING_LOOKUP;

			if ((sin.sin_addr.s_addr = inet_addr(scs.obc[count].lookup.hostname)) == INADDR_NONE)
			/* Initiate our async lookup */
			{
				scs.obc[count].lookup.tHandle = (HANDLE)_beginthreadex(NULL,0,(unsigned int(__stdcall*)(void*))Lookup_Thread,&(scs.obc[count].lookup),0,&scs.obc[count].lookup.tID);
				CloseHandle(scs.obc[count].lookup.tHandle);
			}
			else
			/* Outbound hostname expressed as an IP address */
			{
				scs.obc[count].lookup.ip = sin.sin_addr.s_addr;
				bOBC = TRUE;
			}
		}

		if (scs.obc[count].state == OBC_STATE_PENDING_LOOKUP)
		/* Waiting on a lookup, let's check it out */
		{
			if (scs.obc[count].lookup.completed)
			{
				if (!scs.obc[count].lookup.error)
				/* Lookup complete! */
				{
					sin.sin_addr.s_addr = scs.obc[count].lookup.ip;
					bOBC = TRUE;
				}
				else
				/* Error in hostname lookup */
				{

					/* TEMP: We will attempt connections in 10 second intervals */
					scs.obc[count].state = OBC_STATE_WAITING;
					scs.obc[count].timer = tickcount + 10000;
					
					sprintf(buf,"Unable to resolve hostname for %s(%s)",scs.obc[count].name,scs.obc[count].lookup.hostname);

					TechLog_AddEntry(buf,BITMAP_WARNING);
				}
			}
		}

		if (bOBC)
		/* All ready to attempt the connection(NOTE: It is assumed sin.sin_addr.s_addr is valid) */
		{

			/* Set state & create a new socket */
			scs.obc[count].state = OBC_STATE_CONNECTING;
			scs.obc[count].fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
			SetNonBlocking(scs.obc[count].fd);

			/* Set up connection address */
			sin.sin_family = AF_INET;
			sin.sin_port = htons(scs.obc[count].port);
			memset(sin.sin_zero,0,sizeof(sin.sin_zero));

			/* ...and connect! (or try anyways!) */
			connect(scs.obc[count].fd,(struct sockaddr*)&sin,sizeof(struct sockaddr));
		}

		if (scs.obc[count].state == OBC_STATE_CONNECTING)
		/* Add this to our except & write sets */
		{
			FD_SET(scs.obc[count].fd,&except_set);
			FD_SET(scs.obc[count].fd,&write_set);
		}
	}

	for (count = 0; count < SettingsInfo.isGeneral.bindingnum; count++)
	/* Add our listening sockets */
	{
		if (scs.fdListening[count] != INVALID_SOCKET)
			FD_SET(scs.fdListening[count],&read_set);
	}

	if (count == 0)
		return;

	/* select() timeout value */
	wait.tv_sec = 0;
	wait.tv_usec = (long)delay;

	nfds = select(FD_SETSIZE,&read_set,&write_set,&except_set,&wait);

	if (nfds == -1)
	/* select() error () */
	{
		int err = WSAGetLastError();
		
		GetErrorString(errbuf,err);
		sprintf(buf,"select() failed in Socket_CheckIO(), Error #%d(%s)",err,errbuf);
		TechLog_AddEntry(buf,BITMAP_ERROR);

		Sleep(2500);
	}
	else
	{
		int length;
		int nState;
		int err = 0, errlen = sizeof(err);

		/*
		** Check exception set
		**
		** NOTE: Windows reports all connection errors on non-blocking
		** sockets in the except set, and it sets the SO_ERROR value to
		** the connection error, so we will check them here.
		*/
		
		for (count = 0; count < MAX_SERVERCONNECTIONS; count++)
		/* Check outbound server connections */
		{
			if (scs.obc[count].state == OBC_STATE_CONNECTING)
			/* Check this outbound connection(NOTE: the fd is assumed to be valid!) */
			{
				if (FD_ISSET(scs.obc[count].fd,&except_set))
				/* Possible error in outbound server connection */
				{
					getsockopt(scs.obc[count].fd,SOL_SOCKET,SO_ERROR,(char*)&err,&errlen);

					if (err)
					/* Close this socket & report the error */
					{
						closesocket(scs.obc[count].fd);
						scs.obc[count].fd = INVALID_SOCKET;

						/* TEMP: Try connection again in 2 seconds */
						scs.obc[count].state = OBC_STATE_WAITING;
						scs.obc[count].timer = tickcount + 2000;

						GetErrorString(errbuf,err);
						sprintf(buf,"Unable to connect to %s(%s) (%s)",scs.obc[count].name,scs.obc[count].lookup.hostname,errbuf);

						TechLog_AddEntry(buf,BITMAP_WARNING);
					}
				}
			}
		}

		/* Check read set */

		for (count = 0; scs.fdListening[count]; count++)
		/* Accept connections for listening sockets */
		{
			if (scs.fdListening[count] != INVALID_SOCKET &&
				FD_ISSET(scs.fdListening[count],&read_set))
			{
				Client_AcceptNew(scs.fdListening[count]);
				FD_CLR(scs.fdListening[count],&read_set);
			}
		}

		/*
		** Check write set
		**
		** When an outbound connection has successfully connected, the descriptor
		** will be placed in write_set by select(). This is where we will check for it
		*/

		for (count = 0; count < MAX_SERVERCONNECTIONS; count++)
		/* Check for connection completions */
		{
			if (scs.obc[count].state == OBC_STATE_CONNECTING &&
				FD_ISSET(scs.obc[count].fd,&write_set))
			/* Connection complete */
			{
				SOCKET s = scs.obc[count].fd;
				CLIENT_STRUCT *csnew;
				MESH_NODE *mnPtr;
				struct sockaddr_in sin, lsin;
				int localindex, slen = sizeof(sin), lslen = sizeof(lsin);

				memset(&sin,0,sizeof(sin));
				memset(&lsin,0,sizeof(lsin));

				/* Get our socket info */
				getsockname(s,(struct sockaddr*)&sin,&slen);
				getpeername(s,(struct sockaddr*)&lsin,&lslen);

				scs.obc[count].state = OBC_STATE_CONNECTED;

				localindex = (int)s;

				if (s > scs.fdHighest)
					scs.fdHighest = s;

				/* Create new client for newly connected server */
				csnew = scs.CSLocal[localindex] = (CLIENT_STRUCT*)calloc(1,sizeof(CLIENT_STRUCT));

				/* IP/socket info */
				csnew->fd = localindex;
				csnew->port_r = ntohs(sin.sin_port);
				csnew->port_l = ntohs(lsin.sin_port);
				csnew->ip = sin.sin_addr.s_addr;
				strcpy(csnew->ip_r,inet_ntoa(sin.sin_addr));
				strcpy(csnew->ip_l,inet_ntoa(lsin.sin_addr));

				/* Server info */
				strcpy(csnew->servername,scs.sclient->server->name);
				csnew->serverkey = scs.skey;

				csnew->flags |= CLIENT_FLAG_SERVERSYNC;
				csnew->state = CLIENT_STATE_REGISTERING;
				csnew->hops = 1;

				/* Create server information */
				csnew->server = (SERVER_STRUCT*)calloc(1,sizeof(SERVER_STRUCT));
				csnew->server->connected = SERVER_OUTBOUND;
				strcpy(csnew->server->name,scs.obc[count].name);
				csnew->server->hashkey = Hash_Add(&HT_Server,csnew->server->name,(void*)csnew);

				csnew->password = scs.obc[count].password;

				LL_Add(&scs.llServerHead[csnew->hops],csnew);

				Client_SendWelcome(csnew);
			}
		}

		/* Use nState to read server messages first */
		nState = 0;

		while (nState <= 2)
		{
			nState++;

			for (count = (int)scs.fdHighest; count >= 0; count--)
			/* Check local connections for data */
			{
				if (!(client = scs.CSLocal[count]))
					continue;

				if ((nState == 1 && Client_IsServer(client)) || (nState == 2 && Client_IsUser(client)))
				{
					/* Check ping times of each client */
					if (client->lastmsg)
					{
						placeholder = ((tickcount - client->lastmsg) / 1000);

						if (placeholder > SettingsInfo.isUser.ping_duration && (!(Client_WasPinged(client))))
						{
							if (Client_IsServer(client))
							/* Send w/prefix */
								Client_SendToOne(client,FALSE,":%d %s :%s",scs.sclient->server->id,TOK_PING,SettingsInfo.isGeneral.servername);
							else
							/* No prefix */
								Client_SendToOne(client,FALSE,MSG_PING " :%s",SettingsInfo.isGeneral.servername);
							
							client->flags |= CLIENT_FLAG_PINGED;
						}
						else if (placeholder > SettingsInfo.isUser.ping_duration + SettingsInfo.isUser.ping_response)
						{
							strcpy(client->quitmessage,"Ping timeout");
							Client_SendErrorNumber(client,TRUE,ERROR_011);

							Event_Broadcast(client,EVENT_TYPE_USER,"USER TIMEOUT %s!%s@%s %s:%u",Client_Nickname(client),client->user->username,client->hostname,client->ip_r,client->port_r);
						}
					}

					/* It's likely we will have to flush the client's output buffer */
					length = 1;

					if (FD_ISSET(client->fd,&read_set) || Client_NewLine(client))
					/* Incoming data from local client, or, data is pending */
						length = Client_ReadPacket(client,&read_set);
					if (length > 0)
						Client_FlushBuffer(client);

					if (length == SOCKET_ERROR || length == 0 || Client_Dead(client))
					/* Connection has been closed either gracefully or by an error */
					{
						if (Client_IsUser(client))
							LL_Remove(&scs.llUserHead[client->hops],client);
						else if (Client_IsServer(client))
							LL_Remove(&scs.llServerHead[client->hops],client);

						Client_Disconnect(client);
					}
				}
			}
		}

		/* Check for non-local dead clients */
		for (count = 2; count < MAX_HOPS; count++)
		{
			/* Check for dead users */
			curr = &scs.llUserHead[count];

			while (curr->next)
			/* Check all the clients in the list */
			{
				curr = curr->next;
				client = (CLIENT_STRUCT*)(curr->data);

				if (Client_Dead(client))
				/* Disconnect the client */
				{
					LL_Remove(&scs.llUserHead[client->hops],client);

                    Client_Disconnect(client);

					/* Start at beginning of list again */
					curr = &scs.llUserHead[count];
				}
			}
		}
		
		/* Check for non-local dead servers */
		for (count = 2; count < MAX_HOPS; count++)
		{
			curr = &scs.llServerHead[count];

			while (curr->next)
			/* Check all the clients in the list */
			{
				curr = curr->next;
				client = (CLIENT_STRUCT*)(curr->data);

				if (Client_Dead(client))
				/* Disconnect the server */
				{
					LL_Remove(&scs.llServerHead[client->hops],client);

                    Client_Disconnect(client);

					/* Start at beginning of list again */
					curr = &scs.llServerHead[count];
				}
			}
		}
	}
}

BOOL OBC_Update(char *szServerName)
/*
** OBC_Update()
** This function populates the OBC(Outbound connection) list, used by the server
** to keep track of all outbound connections.
*/
{
	ServerList *curr = &SettingsInfo.isServers.serverhead;
	int index = 0;

	memset(&scs.obc,0,sizeof(scs.obc));

	while (curr->next)
	/* Go through OBC list and populate it */
	{
		curr = curr->next;

		if (curr->port && (szServerName == NULL || lstrcmpi(szServerName,curr->name) == 0))
		/* We have an outbound server */
		{
			strcpy(scs.obc[index].lookup.hostname,curr->hostmask);
			strcpy(scs.obc[index].name,curr->name);
			strcpy(scs.obc[index].password,curr->password);
			scs.obc[index].port = curr->port;
			/* TEMP: Have all outbound servers attempt connection on startup */
			scs.obc[index].timer = GetTickCount();
			scs.obc[index].state = OBC_STATE_WAITING;

			index++;
		}
	}
	return TRUE;
}