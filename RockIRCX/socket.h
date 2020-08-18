/*
** socket.h
**
** This file contains all functions for the physical data links used in the RockIRCX
** server, and controls all network I/O
*/
#ifndef		__SOCKET_H__
#define		__SOCKET_H__

#define FD_SETSIZE 1024
#include <winsock2.h>
#include <process.h>
#include <stdio.h>
#include <time.h>
#include "..\Common Files\functions.h"
#include "ra.h"
#include "client.h"
#include "lookup.h"

/* External variables */
extern HWND hMain;
extern HWND hStatus;
extern SERVER_CONTROL_STRUCT scs;
extern HASH_TABLE_STRUCT HT_Channel;
extern HASH_TABLE_STRUCT HT_User;
extern HASH_TABLE_STRUCT HT_Server;

/* Constants/defines */
#define S_CONNECTION_BACKLOG	5
#define REMOTE_CLIENT			-1

/* Function declarations */
void					Socket_Startup();
void					Socket_Thread(void*);
void					Socket_Cleanup();
int						SetNonBlocking(SOCKET);
int						Socket_SetupListeners();
void					Socket_FlushConnections();
int						Socket_CheckSignals();
void					Socket_CheckIO(time_t delay);
BOOL					OBC_Update(char *szServerName);

/* Socket information structures */

#endif	/* __SOCKET_H__ */