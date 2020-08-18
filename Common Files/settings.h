/*
** settings.h
** This file contains all functions/structures for loading/saving the settings file
*/

#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>

/* Forward declarations */
typedef struct INFOSTRUCTSETTINGS INFOSTRUCTSETTINGS;
typedef struct INFOSTRUCTSETTINGS* LPINFOSTRUCTSETTINGS;

#ifdef APP_ROCKIRCXRA
#include "..\RockIRCXRA\resource.h"
#else
#include "..\RockIRCX\channel.h"
#include "..\RockIRCX\ll.h"
#endif /* APP_ROCKIRCXRA */

#include "..\Common Files\datalists.h"
#include "..\Common Files\functions.h"

/* Constants */
char *SettingsFileName(char *szBuf);

/* External functions */
extern HWND GetSubItemHandle(int);
extern void ConvertTime(char*, char*);

/* Settings function prototypes */

/* Binding functions */
#ifdef APP_ROCKIRCXRA
int				Binding_Add(char* address, unsigned short port);
int				Binding_Delete(int index);
#endif		/* APP_ROCKIRCX_RA */

int					BindingBuf_Add(char **bindingbuf, unsigned short *numbindings , char* address, unsigned short port);
int					BindingBuf_Delete(char*, unsigned short*, int);
BOOL				BindingBuf_GetInfo(const char *bindingbuf, unsigned short num, char* ipaddr, unsigned short* port);

/* Server account functions */
#define				ACCOUNT_LEVEL_POWERUSER	0
#define				ACCOUNT_LEVEL_IRCOP		1
#define				ACCOUNT_LEVEL_ADMIN		2

int					Account_Add(AccountList* accthead, AccountList* newacct);
void				Account_Delete(AccountList* accthead, AccountList* target);
AccountList*		Account_Find(AccountList* accthead, char* username);
int					Account_Modify(AccountList* accthead, AccountList* dest, AccountList* srcinfo);
int					Account_GetTotal(AccountList* accthead, unsigned char level);

/* Filter list functions */
int					Filter_Add(FilterList* filterhead, char* word, unsigned int type);
int					Filter_Modify(FilterList* filterhead, FilterList* target, char* newword, unsigned int newtype);
void				Filter_Delete(FilterList* filterhead, char* word, unsigned int type);
FilterList*			Filter_Find(FilterList* filterhead, char* word);

/* Ban list functions */
int					Banlist_Add(BanList* banhead, char* hostmask, char* reason, BOOL bNetworkBan, BOOL bException, unsigned int expiration, unsigned char exptype);
int					Banlist_Modify(BanList* banhead, BanList* target, char* hostmask, char* reason, BOOL bNetworkBan, unsigned int expiration, unsigned char exptype);
void				Banlist_Delete(BanList* banhead, BanList* target);
BanList*			Banlist_Find(BanList* banhead, char* hostmask, BOOL bException);

/* Registered channel list(RCL) functions */
int					RCL_Add(RegChanList* RCLHead, RegChanList* RCLData);
RegChanList*		RCL_Find(RegChanList* RCLHead, char* name);
int					RCL_Modify(RegChanList* RCLHead, RegChanList* target, RegChanList* RCLData);
void				RCL_Delete(RegChanList* RCLHead, RegChanList *target);
int					RCL_AddAccess(RegChanList* RCLTarget, AccessList* aData);
AccessList*			RCL_FindAccess(RegChanList* RCLTarget, char* hostmask);
int					RCL_ModifyAccess(RegChanList* RCLTarget, AccessList* aTarget, AccessList* aData);
void				RCL_DeleteAccess(RegChanList* RCLTarget, AccessList* aTarget);

/* Linked server functions */
int					Server_Add(ServerList* serverhead, ServerList* sData);
ServerList*			Server_Find(ServerList* serverhead, char* name);
int					Server_Modify(ServerList* serverhead, ServerList* target, ServerList* sData);
void				Server_Delete(ServerList* serverhead, ServerList* target);

/* Spoofed host functions */
SpoofedList*		Spoofed_Add(SpoofedList* spoofedhead, SpoofedList* sData);
SpoofedList*		Spoofed_Find(SpoofedList* spoofedhead, char* hostname);
void				Spoofed_Delete(SpoofedList* spoofedhead, SpoofedList* target);
#define				Spoofed_Clear(hListView) ListView_DeleteAllItems(hListView);

/* Spoofed host login log functions */
void				SpoofedLogin_Add(SpoofedList* sTarget, char *hostmask, char *server, unsigned short port, unsigned long logintime);
void				SpoofedLogin_Report(SpoofedList *sTarget, SpoofedLoginLog *curr, char *targetbuf);
SpoofedLoginLog*	SpoofedLogin_Find(SpoofedList* sTarget, unsigned short index);
void				SpoofedLogin_DeleteList(SpoofedList* sTarget);
unsigned int		SpoofedLogin_Logins(SpoofedList *sTarget);
unsigned int		SpoofedLogin_Last(SpoofedList* sTarget);

/* General functions*/
unsigned int		IS_CreateBuffer(const void *listhead, char **targetbuf, int type);
BOOL				IS_CreateList(void* listhead, const char* buffer, int type);
void				IS_CreateBlank(LPINFOSTRUCTSETTINGS is);
unsigned int		IS_SaveToBuffer(char **buffer, LPINFOSTRUCTSETTINGS is);
BOOL				IS_LoadFromBuffer(char *buffer, LPINFOSTRUCTSETTINGS is);
void				IS_FreeBuffers(LPINFOSTRUCTSETTINGS is, BOOL bClearLists);
BOOL				IS_SaveToFile(const char* filename, LPINFOSTRUCTSETTINGS is);
BOOL				IS_LoadFromFile(const char* filename, LPINFOSTRUCTSETTINGS is);

/* General functions for both programs */
#define				BUFFER_TYPE_ACCOUNT		0
#define				BUFFER_TYPE_BAN			1
#define				BUFFER_TYPE_RCL			2
#define				BUFFER_TYPE_SERVER		3
#define				BUFFER_TYPE_SPOOFED		4
#define				BUFFER_TYPE_FILTER		5
#define				BUFFER_TYPE_ACCESS		6

typedef struct INFOSTRUCTSTATUS
/* Status information */
{
	unsigned int localusers;
	unsigned int localmax;
	unsigned int globalusers;
	unsigned int globalmax;

	unsigned short admins;
	unsigned short ircops;
	unsigned short powerusers;

#define SETTINGS_STATUS_RUNNING		0
#define SETTINGS_STATUS_HALTED		1
#define SETTINGS_STATUS_SHUTDOWN	2

	unsigned char status;
	unsigned long uptime;

	unsigned int localchannels;
	unsigned int globalchannels;
	unsigned int localregchannels;
	unsigned int globalregchannels;
} INFOSTRUCTSTATUS;

typedef struct INFOSTRUCTIOCONTROL
/* Information for I/O control */
{
	unsigned int totaldescriptors;
	unsigned short listeningsockets;
	unsigned int totalconnections;
	unsigned int datasent;
	unsigned int datarecvd;
} INFOSTRUCTIOCONTROL;

typedef struct INFOSTRUCTGENERAL
/* Information for general dialog */
{
	char networkname[64];
	char servername[64];
	unsigned int serverid;
	char serverdescription[128];
	unsigned short bindingnum;
	char *bindingbuf;
	char adminloc1[64];
	char adminloc2[64];
	char adminemail[128];
	BOOL dns_lookup;
	BOOL dns_cache;
	unsigned int dns_cachetime;
	unsigned char dns_cachetimeduration;

#define SETTINGS_GENERAL_DONTMASK	0
#define SETTINGS_GENERAL_XOUT		1
#define SETTINGS_GENERAL_HIDEHOST	2
	unsigned char dns_masktype;

	char *motdbuffer;
} INFOSTRUCTGENERAL;

typedef struct INFOSTRUCTACCOUNTS
/* Accounts linked list head & transmit buffer */
{
	AccountList accthead;
	char *acctbuffer;

} INFOSTRUCTACCOUNTS;

typedef struct INFOSTRUCTBANS
/* Ban/exceptions linked list & transmit buffer */
{
	BanList banhead;
	char *banbuffer;
} INFOSTRUCTBANS;

typedef struct INFOSTRUCTCHANNEL
/* Channel information, dynamic & registered */
{
	unsigned char modeflag;
	unsigned int defaultlimit;
	RegChanList RCLHead;
	char *RCLbuffer;
} INFOSTRUCTCHANNEL;

typedef struct INFOSTRUCTUSER
/* User information */
{
	BOOL dyn_create;
	BOOL dyn_join;
	char *autojoinbuffer;
	unsigned long maxchannels;
	unsigned short max_perip;
	unsigned long max_recvq;
	unsigned long max_sendq;
	unsigned short ping_duration;
	unsigned short ping_response;
	unsigned short nickdelay;
	unsigned long msgdelay;
} INFOSTRUCTUSER;

typedef struct INFOSTRUCTSERVERS
/* Network settings and server list */
{

#define NETWORK_TYPE_STAR	1
#define NETWORK_TYPE_RING	2
#define NETWORK_TYPE_MESH	3
	unsigned char networktype;
	ServerList serverhead;
	char *serverbuffer;
} INFOSTRUCTSERVERS;

typedef struct INFOSTRUCTSECURITY
/* Security/spoofed host settings */
{
	BOOL preventnickchginchan;
	BOOL showquitsasparts;
    unsigned short max_access;
	unsigned short max_nicklen;
	unsigned short max_chanlen;
	unsigned short max_topiclen;
	unsigned short max_msglen;

#define SPOOF_FLAG23	0x01
#define SPOOF_FLAG80	0x02
#define SPOOF_FLAG1080	0x04
#define SPOOF_FLAG3182	0x08
#define SPOOF_FLAG8080	0x10
	unsigned char spoofflag;
	unsigned char spoofconsequence;
	unsigned short RAPort;
	char RAPassword[64];
    
	SpoofedList spoofedhead;
	char *spoofedbuffer;
} INFOSTRUCTSECURITY;

typedef struct INFOSTRUCTFILTER
/* Filter settings */
{
	char enabled;
	FilterList filterhead;
	char *filterbuffer;
} INFOSTRUCTFILTER;

typedef struct INFOSTRUCTSETTINGS
/* The big one */
{
	INFOSTRUCTSTATUS			isStatus;
	INFOSTRUCTIOCONTROL			isIOControl;
	INFOSTRUCTGENERAL			isGeneral;
	INFOSTRUCTACCOUNTS			isAccounts;
	INFOSTRUCTBANS				isBans;
	INFOSTRUCTCHANNEL			isChannel;
	INFOSTRUCTUSER				isUser;
	INFOSTRUCTSERVERS			isServers;
	INFOSTRUCTSECURITY			isSecurity;
	INFOSTRUCTFILTER			isFilter;

} SETTINGSINFO;

#endif	/* __SETTINGS_H__ */
