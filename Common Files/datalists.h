/*
** datalists.h
** Contains all lists for accounts, bans, filters, etc
*/

#ifndef __DATALISTS_H__
#define __DATALISTS_H__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef struct MOTDColors
/* MOTD color structure */
{
	char *szDisplayStr;
	COLORREF crRGB;
} MOTDColors;

typedef struct AccountList
/* Account information structure */
{
	unsigned char level;						/* Level of account */
	char username[32];							/* Username to use for login */
	char hostmask[32];							/* Allowed hostmask */
	char password[32];							/* Login password */
	unsigned short logins;						/* Number of logins */

	AccountList *next;							/* Pointer to next account structure */
} AccountList;

#define FILTER_TYPE_NICKNAME	0x001
#define FILTER_TYPE_CHANNEL		0x002
#define FILTER_TYPE_TOPIC		0x004
#define FILTER_TYPE_ALL			FILTER_TYPE_NICKNAME | FILTER_TYPE_CHANNEL | FILTER_TYPE_TOPIC

typedef struct FilterList
/* Filter list information */
{
	char word[32];								/* Word to be filtered */
	unsigned int type;							/* Bitmask for where filter goes */

	FilterList *next;							/* Pointer to next item */
} FilterList;

#define EXPIRE_TYPE_SECONDS 0
#define EXPIRE_TYPE_MINUTES 1
#define EXPIRE_TYPE_HOURS	2
#define EXPIRE_TYPE_DAYS	3
#define EXPIRE_TYPE_WEEKS	4

typedef struct BanList
/* Ban list information */
{
	BOOL bException;
	BOOL bNetworkBan;
	char *hostmask;
	char *reason;

	unsigned int expiration;
	unsigned char exptype;

	BanList *next;

} BanList;

#define ZRA_ACCESSLEVEL_OWNER 0
#define ZRA_ACCESSLEVEL_HOST 1
#define ZRA_ACCESSLEVEL_VOICE 2
#define ZRA_ACCESSLEVEL_GRANT 3
#define ZRA_ACCESSLEVEL_DENY 4

typedef struct AccessList
{
	char hostmask[64];
	char *reason;
	unsigned char type;
	unsigned long expire;
	unsigned char exptype;

	struct AccessList *next;
} AccessList;

#define ZRA_VISIBILITY_PUBLIC 0
#define ZRA_VISIBILITY_PRIVATE 1
#define ZRA_VISIBILITY_HIDDEN 2
#define ZRA_VISIBILITY_SECRET 3

#define ZRA_CHANMODE_AUTHONLY				0x01
#define ZRA_CHANMODE_MODERATED				0x02
#define ZRA_CHANMODE_ONLYOPSCHANGETOPIC		0x04
#define ZRA_CHANMODE_NOWHISPERS				0x08
#define ZRA_CHANMODE_INVITEONLY				0x10
#define ZRA_CHANMODE_NOEXTERN				0x20
#define ZRA_CHANMODE_KNOCK					0x40
#define ZRA_CHANMODE_AUDITORIUM				0x80

typedef struct RegChanList
{
	char name[64];
	char *topic;
	char subject[64];
	char *onjoin;
	char *onpart;

	AccessList accesshead;
	char ownerkey[64];
	char hostkey[64];
	char memberkey[64];
	unsigned char visibility;
	unsigned long limit;
	unsigned int modeflags;

	struct RegChanList *next;
} RegChanList;

typedef struct ServerList
{
	char name[64];
	char hostmask[64];
	char password[64];
	BOOL bConnected;

#define NETWORK_ENCRYPTION_NONE 0
#define NETWORK_ENCRYPTION_AES	1
	unsigned char encryption;

	unsigned short port;
	unsigned short ping_frequency;
	unsigned short ping_response;

	struct ServerList* next;
} ServerList;


typedef struct SpoofedLoginLog
{
	unsigned short index;
	char *hostmask;
	char *server;
	unsigned short port;
	unsigned long time;

	SpoofedLoginLog *next;
} SpoofedLoginLog;

typedef struct SpoofedList
{
	char *hostname;
	unsigned short port[7];

	SpoofedLoginLog loginhead;

	struct SpoofedList *next;
} SpoofedList;


#endif		/* __DATALISTS_H__ */