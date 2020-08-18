/*
** ra.h
** Header file for all functions associated with remote administration module
*/

#ifndef		__RA_H__
#define		__RA_H__

#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include "..\Common Files\settings.h"
#include "..\Common Files\functions.h"

/* Technical log bitmaps */
#define BITMAP_OK			0
#define BITMAP_INFO			1
#define BITMAP_WARNING		2
#define BITMAP_ERROR		3

/* Constants/macros */
#define RA_PROTOVERSIONREQUEST				"NV000"
#define RA_PROTOPASSWORDREQUEST				"RQP"
#define RA_PROTOPASSWORDVALID				"YP"
#define RA_PROTOPASSWORDINVALID				"NP"
#define	RA_CONNECTION_BACKLOG				5

/* #define this to add a 1.5 second delay on all outbound remote administration data */
/* #define SIMULATEINTERNET */

/* External variables */
extern INFOSTRUCTSETTINGS SettingsInfo;

/* Function prototypes */
int						RA_Startup();
void					RA_Cleanup();
void					RA_Check();
BOOL					RA_SetPort(unsigned short port);

/* External functions */
int					TechLog_AddEntry(char*, unsigned char);
void				TechLog_Focus();

/* Remote admin data structures */
typedef struct RAINFOSTRUCT
/* Remote administration information structure */
{
    SOCKET				fdListening;
	SOCKET				fdConnection;
	char				ripaddr[32];
	char				lipaddr[32];
	unsigned short		port;
	char				*recvbuf;
	unsigned short		recvlen;
	char				*sendbuf;
	unsigned short		sendlen;
	BOOL				bVerified;
} RAINFOSTRUCT;

#endif	/* __RA_H__ */