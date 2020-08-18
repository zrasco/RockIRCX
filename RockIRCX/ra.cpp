/*
** ra.cpp
** Remote administration utility file for RockIRCX
*/

#include "ra.h"

/* Global variables */
RAINFOSTRUCT raInfo;

int RA_Startup()
/*
** RA_Startup()
** Starts up remote administration module
** Returns, 0 if successful, winsock error # of bind() call failure if not
*/
{
	SOCKADDR_IN sin;
	unsigned short port = raInfo.port;

	memset(&raInfo,0,sizeof(RAINFOSTRUCT));
	raInfo.port = port;

	InitStrPtr(raInfo.recvbuf);
	InitStrPtr(raInfo.sendbuf);

	raInfo.fdListening = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

	sin.sin_family = AF_INET;
	sin.sin_port = htons(raInfo.port);

	/* Temp */
	strcpy(raInfo.lipaddr,"0.0.0.0");

	sin.sin_addr.s_addr = inet_addr(raInfo.lipaddr);

    if (bind(raInfo.fdListening,(struct sockaddr*)&sin,sizeof(struct sockaddr)) == 0)
	/* Bound remote admin port */
	{
		SettingsInfo.isIOControl.listeningsockets++;
		SettingsInfo.isIOControl.totaldescriptors++;
		SetNonBlocking(raInfo.fdListening);
		listen(raInfo.fdListening,RA_CONNECTION_BACKLOG);
	}
	else
	/* Unable to bind port */
	{
		return WSAGetLastError();
	}

	return 0;
}

void RA_Cleanup()
/*
** RA_Cleanup()
** Cleans up all data associated with remote administration module
*/
{
	free(raInfo.recvbuf);
	free(raInfo.sendbuf);
}

void RA_Check()
/*
** RA_Check()
** Checks remote administration connection for data, or for an incoming connection
*/
{
	SOCKET s;
	SOCKADDR_IN sin;
	int slen = sizeof(struct sockaddr);
	char buf[256];

	if ((s = accept(raInfo.fdListening,(struct sockaddr*)&sin,&slen)) != INVALID_SOCKET)
	/* New connection coming in */
	{
		if (raInfo.fdConnection)
		/* Someone else attempting to connect */
		{
			closesocket(s);
			sprintf(buf,"Remote administration connection attempt from %s",inet_ntoa(sin.sin_addr));
			TechLog_AddEntry(buf,BITMAP_WARNING);
		}
		else
		/* Accept new remote admin connection */
		{
			raInfo.bVerified = FALSE;

			raInfo.sendbuf = (char*)malloc(1); raInfo.sendbuf[0] = 0;
			raInfo.recvbuf = (char*)malloc(1); raInfo.recvbuf[0] = 0;
			raInfo.fdConnection = s;
			strcpy(raInfo.ripaddr,inet_ntoa(sin.sin_addr));
			sprintf(buf,"IP address %s has connected to remote administration.",raInfo.ripaddr);
			TechLog_AddEntry(buf,BITMAP_INFO);

			SettingsInfo.isIOControl.totalconnections++;
		}
	}
	else if (raInfo.fdConnection)
	/* Read/write phase */
	{
		char buf[520];
		int retval, bytesread = 0;
		BOOL bExitRecv = FALSE, bShutDownOnSend = FALSE;

		/* Phase 1: recieve data into buffer and check for errors on connection line */
		while (!bExitRecv)
		{
			if ((retval = recv(raInfo.fdConnection,buf,512,0)) == SOCKET_ERROR)
			/* Possible error in RA connection line */
			{
				int err = WSAGetLastError();

				if (err != WSAEWOULDBLOCK)
				/* Error in recv(), close socket */
				{
					/* REPORT: Remote admin connection failed due to error */
					closesocket(raInfo.fdConnection);
					SettingsInfo.isIOControl.totalconnections--;
					raInfo.fdConnection = 0;

					/* Reset pointers to buffers */
					free(raInfo.recvbuf);
					raInfo.recvbuf = NULL;
					InitStrPtr(raInfo.recvbuf); raInfo.recvlen = 0;
					free(raInfo.sendbuf);
					raInfo.sendbuf = NULL;
					InitStrPtr(raInfo.sendbuf); raInfo.sendlen = 0;

					sprintf(buf,"Remote administration connection to %s terminated.",raInfo.ripaddr);
					TechLog_AddEntry(buf,BITMAP_INFO);

					bExitRecv = TRUE;
				}
				else
				/* No pending data, exit loop */
				{
					bExitRecv = TRUE;
				}
			}
			else if (!retval)
			/* Connection to host closed */
			{
				raInfo.fdConnection = 0;
				SettingsInfo.isIOControl.totalconnections--;

				/* Reset pointers to buffers */
				free(raInfo.recvbuf);
				InitStrPtr(raInfo.recvbuf); raInfo.recvlen = 0;
				free(raInfo.sendbuf);
				InitStrPtr(raInfo.sendbuf); raInfo.sendlen = 0;

				sprintf(buf,"Remote administration connection to %s closed.",raInfo.ripaddr);
				TechLog_AddEntry(buf,BITMAP_INFO);

				bExitRecv = TRUE;
			}
			else
			/* Data successfully read */
			{
				bytesread += retval;

				buf[retval] = 0;

				raInfo.recvlen = (unsigned short)bytesread;

				/* ERROR IS HERE!!(Stack corruption) */
				/* Cause: raInfo.recvlen > sizeof(buf), fix tomorrow */
				raInfo.recvbuf = (char*)realloc(raInfo.recvbuf,raInfo.recvlen);
				memcpy(&raInfo.recvbuf[bytesread - retval],&buf[0],retval);
			}
		}

		/* Phase 2: Process recieve buffer */
		if (!raInfo.bVerified)
		{
			if (bytesread == 4 && raInfo.recvbuf[0] == 'V')
			/* Version verification */
			{
				unsigned char vMajor = buf[1];
				unsigned short vMinor = (buf[2] * 256) + buf[3];
	            
				/* Temp: Beta version is 0.8 */
				if (vMajor != 0 || vMinor != 8)
				/* Incompatible version, send required version */
				{
					ReallocAndAdd(raInfo.sendbuf,RA_PROTOVERSIONREQUEST);
					raInfo.sendlen = (unsigned short)strlen(raInfo.sendbuf);
					raInfo.sendbuf[2] = 0;
					(*(unsigned short*)&raInfo.sendbuf[3]) = htons(8);
					bShutDownOnSend = TRUE;
				}
				else
				/* Version is compatible, send request for password */
				{
					ReallocAndAdd(raInfo.sendbuf,RA_PROTOPASSWORDREQUEST);
					raInfo.sendlen = (unsigned short)strlen(raInfo.sendbuf);
				}
			}
			else if (raInfo.recvbuf[0] == 'R' && raInfo.recvbuf[1] == 'P')
			/* Client is sending password */
			{
				unsigned short passlen;
				char password[256];

				GetShort(raInfo.recvbuf,2,&passlen); passlen = ntohs(passlen);

				if (passlen > 0 && passlen < 256)
				/* Proection check */
				{
					memcpy(password,&raInfo.recvbuf[2 + SIZE_SHORT],passlen);
					password[passlen] = 0;

					if (strcmp(password,SettingsInfo.isSecurity.RAPassword) == 0)
					/* Password correct */
					{
						ReallocAndAdd(raInfo.sendbuf,RA_PROTOPASSWORDVALID);
						raInfo.bVerified = TRUE;
					}
					else
					/* Invalid password */
					{
						sprintf(buf,"IP address %s used invalid password: %s",raInfo.ripaddr,password);
						TechLog_AddEntry(buf,BITMAP_WARNING);
						ReallocAndAdd(raInfo.sendbuf,RA_PROTOPASSWORDINVALID);
						bShutDownOnSend = TRUE;
					}

					raInfo.sendlen = (unsigned short)strlen(raInfo.sendbuf);
				}
			}
		}
		else
		/* User already verified */
		{
			unsigned long dlen = 0;
			char *response = NULL;

			if (raInfo.recvbuf[0] == 'R' &&
				raInfo.recvbuf[1] == 'Q' &&
				raInfo.recvbuf[2] == 'D')
			/* Requesting all server data */
			{
				free(raInfo.sendbuf);
				dlen = IS_SaveToBuffer(&raInfo.sendbuf,&SettingsInfo);

				/* Shift entire buffer over 6 bytes to insert header data */
				raInfo.sendbuf = (char*)realloc(raInfo.sendbuf,dlen + 6);
				memmove(&raInfo.sendbuf[6],raInfo.sendbuf,dlen);
				raInfo.sendbuf[0] = 'R'; raInfo.sendbuf[1] = 'D';
				InsertLong(raInfo.sendbuf,2,htonl(dlen));

				/* Length of send: 6 bytes(header) + length of info buffer */
				raInfo.sendlen = (unsigned short)(dlen + 6);
			}
			else if (raInfo.recvbuf[0] == 'U' &&
				raInfo.recvbuf[1] == 'P' && 
				raInfo.recvbuf[2] == 'D')
			/* Sending update for server data */
			{
				char szFileBuf[256];

				response = "RUPD";

				IS_FreeBuffers(&SettingsInfo,TRUE);

				IS_LoadFromBuffer(&raInfo.recvbuf[SIZE_LONG + 3],&SettingsInfo);
				IS_SaveToFile(SettingsFileName(szFileBuf),&SettingsInfo);

				raInfo.sendlen = (unsigned short)strlen(response);
				ReallocAndAdd(raInfo.sendbuf,response);

				/* Manual updates */
			}
			else if (raInfo.recvbuf[0] == 'R' &&
				raInfo.recvbuf[1] == 'Q' &&
				raInfo.recvbuf[2] == 'S')
			/* RA program requesting list of linked servers */
			{
				dlen = IS_CreateBuffer(&SettingsInfo.isServers.serverhead,&raInfo.sendbuf,BUFFER_TYPE_SERVER);

				/* Insert header data(4 bytes) */
				raInfo.sendbuf = (char*)realloc(raInfo.sendbuf,dlen + 4);
				memmove(&raInfo.sendbuf[4],raInfo.sendbuf,dlen);
				raInfo.sendbuf[0] = 'R'; raInfo.sendbuf[1] = 'S';
				InsertShort(raInfo.sendbuf,2,htons((unsigned short)dlen));

				raInfo.sendlen = (unsigned short)(dlen + 4);
			}
			else if (raInfo.recvbuf[0] == 'R' &&
				raInfo.recvbuf[1] == 'Q' &&
				raInfo.recvbuf[2] == 'F')
			/* RA program requesting list of filters */
			{
				dlen = IS_CreateBuffer(&SettingsInfo.isFilter.filterhead,&raInfo.sendbuf,BUFFER_TYPE_FILTER);

				/* Insert header data(4 bytes) */
				raInfo.sendbuf = (char*)realloc(raInfo.sendbuf,dlen + 4);
				memmove(&raInfo.sendbuf[4],raInfo.sendbuf,dlen);
				raInfo.sendbuf[0] = 'R'; raInfo.sendbuf[1] = 'F';
				InsertShort(raInfo.sendbuf,2,htons((unsigned short)dlen));

				raInfo.sendlen = (unsigned short)(dlen + 4);
			}
			else if (raInfo.recvbuf[0] == 'R' &&
				raInfo.recvbuf[1] == 'Q' &&
				raInfo.recvbuf[2] == 'R' &&
				raInfo.recvbuf[3] == 'C' &&
				raInfo.recvbuf[4] != 'A')
			/* RA program requesting list of registered channels */
			{
				dlen = IS_CreateBuffer(&SettingsInfo.isChannel.RCLHead,&raInfo.sendbuf,BUFFER_TYPE_RCL);

				/* Insert header data(5 bytes) */
				raInfo.sendbuf = (char*)realloc(raInfo.sendbuf,dlen + 5);
				memmove(&raInfo.sendbuf[5],raInfo.sendbuf,dlen);
				raInfo.sendbuf[0] = 'R'; raInfo.sendbuf[1] = 'R'; raInfo.sendbuf[2] = 'C';
				InsertShort(raInfo.sendbuf,3,htons((unsigned short)dlen));

				raInfo.sendlen = (unsigned short)(dlen + 5);
			}
			else if (raInfo.recvbuf[0] == 'R' &&
				raInfo.recvbuf[1] == 'Q' &&
				raInfo.recvbuf[2] == 'B')
			/* RA program requesting list of ban/exceptions */
			{
				dlen = IS_CreateBuffer(&SettingsInfo.isBans.banhead,&raInfo.sendbuf,BUFFER_TYPE_BAN);

				/* Insert header data(4 bytes) */
				raInfo.sendbuf = (char*)realloc(raInfo.sendbuf,dlen + 4);
				memmove(&raInfo.sendbuf[4],raInfo.sendbuf,dlen);
				raInfo.sendbuf[0] = 'R'; raInfo.sendbuf[1] = 'B';
				InsertShort(raInfo.sendbuf,2,htons((unsigned short)dlen));

				raInfo.sendlen = (unsigned short)(dlen + 4);
			}
			else if (raInfo.recvbuf[0] == 'R' &&
				raInfo.recvbuf[1] == 'Q' &&
				raInfo.recvbuf[2] == 'A')
			/* RA program requesting list of accounts */
			{
				dlen = IS_CreateBuffer(&SettingsInfo.isAccounts.accthead,&raInfo.sendbuf,BUFFER_TYPE_ACCOUNT);

				/* Insert header data(4 bytes) */
				raInfo.sendbuf = (char*)realloc(raInfo.sendbuf,dlen + 4);
				memmove(&raInfo.sendbuf[4],raInfo.sendbuf,dlen);
				raInfo.sendbuf[0] = 'R'; raInfo.sendbuf[1] = 'A';
				InsertShort(raInfo.sendbuf,2,htons((unsigned short)dlen));

				raInfo.sendlen = (unsigned short)(dlen + 4);
			}
			else if (raInfo.recvbuf[0] == 'R' &&
				raInfo.recvbuf[1] == 'Q' &&
				raInfo.recvbuf[2] == 'R' &&
				raInfo.recvbuf[3] == 'C' &&
				raInfo.recvbuf[4] == 'A')
			/* RA program requesting access list for a channel */
			{
				RegChanList *curr;
				char name[128];
				unsigned short slen;

				/* Find matching RCL structure */
				strcpy(name,&raInfo.recvbuf[5 + SIZE_SHORT]);
				GetShort(raInfo.recvbuf,5,&slen); slen = ntohs(slen);
				name[slen] = 0;

				curr = RCL_Find(&SettingsInfo.isChannel.RCLHead,name);

				if (curr)
				{
					dlen = IS_CreateBuffer(curr,&raInfo.sendbuf,BUFFER_TYPE_ACCESS);

					/* Insert header data(6 bytes) */
					raInfo.sendbuf = (char*)realloc(raInfo.sendbuf,dlen + 4);
					memmove(&raInfo.sendbuf[4],raInfo.sendbuf,dlen);
					raInfo.sendbuf[0] = 'R'; raInfo.sendbuf[1] = 'R';
					raInfo.sendbuf[2] = 'C'; raInfo.sendbuf[3] = 'A';

					raInfo.sendlen = (unsigned short)(dlen + 4);
				}
				else
				/* Channel was not found */
				{
					ReallocAndAdd(raInfo.sendbuf,"RRCANC");
					raInfo.sendlen = 6;
				}
			}

		}

		/* Recieve buffer has been processed */
		raInfo.recvbuf = (char*)realloc(raInfo.recvbuf,1);
		raInfo.recvbuf[0] = 0;

		/* Phase 3: Empty write buffer to connected socket */
		if (raInfo.sendbuf[0])
		{
			int sent = 0;

#ifdef SIMULATEINTERNET
			Sleep(1500);
#endif	/* SIMULATEINTERNET */

			if ((sent = send(raInfo.fdConnection,raInfo.sendbuf,raInfo.sendlen,0)) != SOCKET_ERROR)
			/* SOME/ALL data was successfully sent */
			{
				if (sent == raInfo.sendlen)
				/* All data was sent */
				{
					raInfo.sendbuf = (char*)realloc(raInfo.sendbuf,1);
					raInfo.sendbuf[0] = 0; raInfo.sendlen = 0;
				}
				else
				/* Not all data was sent */
				{
				}

			}
			else
			/* Error in send */
			{
				int err = WSAGetLastError();
				char errbuf[256];

				GetErrorString(errbuf,err);

				/* REPORT: Remote admin connection failed due to error */
				sprintf(buf,"Connection to IP address %s terminated, Error %d(%s)",raInfo.ripaddr,err,errbuf);
				TechLog_AddEntry(buf,BITMAP_WARNING);

				closesocket(raInfo.fdConnection);
				raInfo.fdConnection = 0;

				/* Reset pointers to buffers */
				free(raInfo.recvbuf);
				InitStrPtr(raInfo.recvbuf);
				free(raInfo.sendbuf);
				InitStrPtr(raInfo.sendbuf);
			}

			if (bShutDownOnSend)
			/* All needed is a shutdown() call, as the next pass will fail on recv() */
				shutdown(raInfo.fdConnection,2);
		}
        
	}
}
BOOL RA_SetPort(unsigned short port)
{
	if (port)
		raInfo.port = port;
	else
		return FALSE;

	return TRUE;
}