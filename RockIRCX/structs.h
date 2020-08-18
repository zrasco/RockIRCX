/*
** structs.h
**
** Contains all structure declarations for use in program
*/

#ifndef __STRUCTS_H__
#define __STRUCTS_H__

#include "strbuf.h"
#include "ll.h"
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <commctrl.h>
#include <time.h>

/* Global #define's */
#define	USER_ALLUSERS			1
#define MAX_HOPS				20
#define	MAX_MODES				26
#define MAX_NAMES				26
#define MAX_SERVERCONNECTIONS	20
#define MAX_SERVERTOKENS		256
#define CRLF "\r\n"
#define ROCKIRCX_VERSION	"5.5.0.1"
#define ROCKIRCX_PROTOCOL_VERSION "0.1"
#define ROCKIRCX_CREATED	"July 7th 2004, 10:32pm PST"

/* Forward declarations */
typedef struct CLIENT_STRUCT CLIENT_STRUCT;

/* Lookup information */
typedef struct LOOKUP_CACHE_STRUCT
{
	char hostname[64];							/* Resolved hostname */
	unsigned int ip;							/* IP address being resolved */

	HANDLE tHandle;								/* Handle to lookup thread */
	unsigned int tID;							/* Thread ID # */

	int error;									/* If error, this will tell which one */
	time_t expiration;							/* Calendar time of cache expiration */
	BOOL completed;								/* Lookup complete? */

	LINKED_LIST_STRUCT chead;					/* Linked list of client(s) for lookup */

} LOOKUP_CACHE_STRUCT;

/* Auditorium switchover information */
typedef struct MODE_SWITCHOVER_INFO
{
	CLIENT_STRUCT		*csTarget;
	DWORD				dwModeFrom;
	DWORD				dwModeTo;
	DWORD				*pdwReturnMode;
	BOOL				bMinus;
} MODE_SWITCHOVER_INFO;

/* Lusers information */
typedef struct LUSERS_STRUCT
{
	int nOpsOnline;								/* How many operators are online? */
	int nInvisible;								/* How many invisible users on network? */
	int nLocalServers;							/* How many servers connected to this one? */
	int nGlobalServers;							/* How many servers on network? */
	int nUnknown;								/* How many unknown connections? */
	int nChannels;								/* How many channels formed? */
	int nLocalUsers;							/* How many local users? */
	int nLocalMax;								/* Peak number of users on server */
	int nGlobalUsers;							/* Number of users on network */
	int nGlobalMax;								/* Peak number of users on network */
} LUSERS_STRUCT;

/* Access entry */
typedef struct ACCESS_STRUCT
{
	char				szHostmask[64];			/* Hostmask used with entry */
	char				szReason[64];			/* Reason for access entry */
	char				szCreator[64];			/* Person who created the entry */

	DWORD				dwTimeout;				/* Timeout in minutes */

/* Access levels */
#define ACCESSLEVEL_OWNER	5
#define ACCESSLEVEL_HOST	4
#define ACCESSLEVEL_VOICE	3
#define ACCESSLEVEL_GRANT	2
#define ACCESSLEVEL_DENY	1
#define	ACCESSLEVEL_ALL		0
	DWORD				dwLevel;
	BOOL				bOwner;					/* Set by owner? (only used in channel access entries) */

} ACCESS_STRUCT;

/* Event entry */
typedef struct EVENT_STRUCT
{
	char				szHostmask[64];			/* Hostmask used with entry */

#define EVENT_TYPE_CHANNEL		1
#define EVENT_TYPE_MEMBER		2
#define	EVENT_TYPE_SERVER		3
#define EVENT_TYPE_CONNECT	4
#define EVENT_TYPE_SOCKET		5
#define EVENT_TYPE_USER			6

	DWORD				dwType;					/* Type of event */
} EVENT_STRUCT;

/* Channel information */
typedef struct CHANNEL_STRUCT
{
	char				szName[128];			/* Name of channel */

	/* Channel modes */
#define CHANNELMODE_PUBLIC			0x00000001	/* All users can query all channel information */
#define CHANNELMODE_PRIVATE			0x00000002	/* +p: Only the name and the number of members can be queried from non-members */
#define CHANNELMODE_SECRET			0x00000004	/* +s: No properties can be queried from non-members */
#define CHANNELMODE_HIDDEN			0x00000008	/* +h: Channel is public except name cannot be queried by the LIST or LISTX commands */
#define CHANNELMODE_MODERATED		0x00000010	/* +m: Only voice and above can talk */
#define CHANNELMODE_NOEXTERN		0x00000020	/* +n: No external messages */
#define CHANNELMODE_TOPICOP			0x00000040	/* +t: Only ops set topic */
#define CHANNELMODE_INVITE			0x00000080	/* +i: Invite only */
#define CHANNELMODE_KNOCK			0x00000100	/* +u: Knock mode */
#define CHANNELMODE_NOFORMAT		0x00000200	/* +f: Don't format messages */
#define CHANNELMODE_NOWHISPER		0x00000400	/* +w: No whispers allowed */
#define CHANNELMODE_AUDITORIUM		0x00000800	/* +x: Auditorium mode */
#define CHANNELMODE_REGISTERED		0x00001000	/* +r: Channel is registered */
#define CHANNELMODE_SERVICE			0x00002000	/* +z: Channel is being monitored */
#define CHANNELMODE_AUTHONLY		0x00004000	/* +a: Channel is auth-only */
#define CHANNELMODE_CLONEABLE		0x00008000	/* +d: Cloneable channel */
#define CHANNELMODE_CLONE			0x00010000	/* +e: Channel is a clone */
#define CHANNELMODE_COLLISIONVICTIM	0x00020000	/* Hidden mode used for channel collisions */
	DWORD				dwModes;				/* Channel modes */

	/* Channel properties */
	DWORD				dwPropOID;				/* Internal object identifier */
	DWORD				dwPropCreationTime;		/* Channel creation time */
	DWORD				dwPropClientGUID;		/* Client GUID */
	DWORD				dwPropServicepath;		/* Path of server side extension */
	DWORD				dwPropLag;				/* Channel lag(artificial delay) */
	char				szPropLanguage[32];		/* Official language of channel */
	char				szPropOwnerkey[32];		/* Ownerkey for channel */
	char				szPropHostkey[32];		/* Hostkey for channel  */
	char				szPropMemberkey[32];	/* Memberkey for channel */
	char				szPropPICS[256];		/* PICS rating */
	char				szPropTopic[768];		/* Channel topic */
	char				szPropSubject[32];		/* Channel subject */
	char				szPropClient[256];		/* Channel CLIENT property */
	char				szPropOnjoin[256];		/* Channel ONJOIN */
	char				szPropOnpart[256];		/* Channel ONPART */
	char				szPropAccount[32];		/* Channel account */

	/* Other channel data */
	DWORD				dwLimit;				/* Channel limit(0 = no +l mode) */
	DWORD				dwUsers;				/* How many people are in the channel? */
	DWORD				dwAccessEntries;		/* Number of access entries */

	ACCESS_STRUCT		*accList;				/* Array of access entries */
	LINKED_LIST_STRUCT	llUserHead;				/* List of users in channel */

#define CHANNEL_PRIVELEDGE_NONE		0x0000		/* Zero, nada, zip, zilch */
#define CHANNEL_PRIVELEDGE_OWNER	0x0001		/* +q: Channel owner(.User) */
#define CHANNEL_PRIVELEDGE_HOST		0x0002		/* +o: Channel host(@User) */
#define CHANNEL_PRIVELEDGE_VOICE	0x0004		/* +v: Voiced in channel(+User) */
	LINKED_LIST_STRUCT llUserModeHead;			/* Parallel list of user modes */
	LINKED_LIST_STRUCT llInviteHead;			/* List of invited users */
	CLIENT_STRUCT		*csServerOrigin;		/* Pointer to server channel originated from */
	int					nNJoinStatus;				/* NJoin status */

} CHANNEL_STRUCT;

/* User information */
typedef struct USER_STRUCT
{
	char				away[64];				/* Away message */
	char				nickname[32];			/* Nickname!Username@hostname */
	char				username[32];			/* Nickname!Username@hostname */
	char				fullname[64];			/* Nickname ~Username Hostname * :fullname */
	int					hashkey;				/* Hash key for user table */
	BOOL				bInIRCX;				/* User in IRCX mode? */
	time_t				lastnick;				/* Tick count of last nickname change */
	LINKED_LIST_STRUCT	llChannelHead;			/* List of channels user is on */
} USER_STRUCT;

/* Info used in ring networks */
typedef struct RING_NETWORK_INFO
{
	unsigned char	lastmsgid;					/* ID # of last message */			
} RING_NETWORK_INFO;

/* Server information */
typedef struct SERVER_STRUCT
{
#define SERVER_REMOTE	0
#define SERVER_INBOUND	1
#define SERVER_OUTBOUND	2
	unsigned char	connected;					/* How is this server on the network? */
	unsigned char	type;						/* Star, Ring, or Mesh? */
	unsigned int	id;							/* Network-wide ID # */
	char			name[64];					/* Name of server */
	char			description[128];			/* Description of server */
	int				hashkey;					/* Hash key for server table */
	unsigned char	lastmsgid;					/* ID # of last message (ring networks only) */
	unsigned int	timeouts;					/* Timeout counter (mesh only) */
	BOOL			meshed;						/* Connected to all servers in list? (mesh only) */
} SERVER_STRUCT;

/* Generic client information */
typedef struct CLIENT_STRUCT
{
	int fd;										/* Socket descriptor */
	int hops;									/* # of hops from this server */

#define CLIENT_FLAG_CLOSEONSEND		0x00000001	/* Close after sending queued data? */
#define	CLIENT_FLAG_NEWLINE			0x00000002	/* Recv buffer have a NL in it? */
#define	CLIENT_FLAG_PINGED			0x00000004	/* User has been pinged already */
#define CLIENT_FLAG_SERVERSYNC		0x00000008	/* SERVER messages are still being sent */
#define CLIENT_FLAG_USERSYNC		0x00000010	/* NICK messages are still being sent */
#define CLIENT_FLAG_CHANNELSYNC		0x00000020	/* NJOIN messages are still being sent */
#define CLIENT_FLAG_REGISTERED		0x00000040	/* Client registered? */
#define	CLIENT_FLAG_ADMIN			0x00000080	/* Client has administrator rights (seperate from admin mode below) */
#define	CLIENT_FLAG_SYSOP			0x00000100	/* Client has sysop rights (seperate from sysop mode below) */
#define	CLIENT_FLAG_POWERUSER		0x00000200	/* Client has power user rights */
#define	CLIENT_FLAG_NETSPLITVICTIM	0x00000400	/* Paired with CLIENT_STATE_DEAD, died from netsplit */


	int flags;									/* State flags */

#define CLIENT_STATE_NORMAL			0			/* On the server, logged in */
#define CLIENT_STATE_LOOKUP			1			/* An outstanding lookup is pending */
#define CLIENT_STATE_REGISTERING	2			/* Registration not yet completed */
#define CLIENT_STATE_WELCOMING		3			/* Final check before going from registering to normal */
#define CLIENT_STATE_DEAD			4			/* Client is to be disconnected */

	unsigned int state;							/* State of client */

#define CLIENT_MODE_INVISIBLE		0x00000001	/* +i: Invisible user */
#define CLIENT_MODE_ADMIN			0x00000002	/* +a: Administrator */
#define CLIENT_MODE_SYSOP			0x00000004	/* +o: Sysop */
#define CLIENT_MODE_GAG				0x00000008	/* +z: User is gagged and cannot privmsg */
	unsigned int modeflag;						/* Client modes */

	char hostname[64];							/* Hostname for client(NULL-terminated) */
	char hostmask[64];							/* Masked hostname */
	char quitmessage[64];						/* QUIT message */
	char killmessage[64];						/* KILL message */
	char ip_r[64];								/* Remote IP address (client) */
	char ip_l[64];								/* Local IP address (server) */
	BOOL bKilled;								/* Killed? */

	char servername[64];						/* Server that client is connected to */
	int serverkey;								/* Key for server hash table */

	CLIENT_STRUCT		*servernext;			/* Next server in route for messages */
	USER_STRUCT			*user;					/* Pointer to user info(NULL if server) */
	SERVER_STRUCT		*server;				/* Pointer to server info(NULL if user) */

	/*  For local clients only  */
	/*			****			*/
	/*			****			*/
	/*			****			*/
	/*			****			*/
	/*		  ********			*/
	/*		   ******			*/
	/*			****			*/
	/*			 **				*/

	StrBuffer recvQ;							/* Recieve buffer(yet to be parsed) */
	StrBuffer sendQ;							/* Send buffer */

	unsigned int ip;							/* Client's IP address */
	unsigned short port_r;						/* Remote port(userip:port_r->serv:port_l) */
	unsigned short port_l;						/* Local port(userip:port_r->serv:port_l) */

	time_t signon;								/* Calendar time of signon */
	time_t lastmsg;								/* Tick count of last message */
	time_t parsemsg;							/* Tick count of next parsing */
	time_t idlemsg;								/* Idle handling */

	LINKED_LIST_STRUCT llAccessHead;			/* Access entries */
	LINKED_LIST_STRUCT llEventHead;				/* Event entries */

	char *password;								/* Password client will be using */
} CLIENT_STRUCT;

/* Message control block */
typedef struct MESSAGE_CONTROL_STRUCT
{
	CLIENT_STRUCT *c_from;						/* Origin of message(be it a server, user, etc) */
	CLIENT_STRUCT *c_prefixfrom;				/* Server/user who sent message */
	int argc;									/* # of arguments */
	char *argv[100];							/* Array of arguments */

} MESSAGE_CONTROL_STRUCT;

typedef struct DESTINATION_INFO_STRUCT
{
	char szOriginalText[128];					/* Original name used */

	CLIENT_STRUCT *csUser;						/* If non-NULL, pointer to user */
	CHANNEL_STRUCT *chChannel;					/* If non-NULL, pointer to channel */
} DESTINATION_INFO_STRUCT;

typedef struct OUTBOUND_CONNECTION_STRUCT
/* Item in table of all outbound connections */
{

#define OBC_STATE_NOT_USED			0
#define OBC_STATE_WAITING			1
#define OBC_STATE_CONNECTING		2
#define OBC_STATE_CONNECTED			3
#define OBC_STATE_PENDING_LOOKUP	4
#define OBC_STATE_DEAD				5

	int state;									/* State of OBC(Outbound connection) */
	unsigned long timer;						/* Tick count of next connection attempt */
	unsigned short port;						/* Outbound port(hostname/ip in lookup.hostname) */
	CLIENT_STRUCT *client;						/* Pointer to servers client */
	SOCKET fd;									/* Socket handle */
	char name[64];								/* Outbound server name */
	char password[64];							/* Outbound password */
	LOOKUP_CACHE_STRUCT lookup;					/* Lookup information */
        
} OUTBOUND_CONNECTION_STRUCT;

typedef struct MESH_NODE
{
	BOOL			bConnected;					/* This server connected? */
	CLIENT_STRUCT	*csServer;					/* Server client pointer */
	char			servername[256];			/* Server name */
} MESH_NODE;

typedef struct MESH_CONTROL_STRUCT
{
#define MESH_NETWORKSTATUS_OFFLINE	0			/* Not connecting to other nodes */
#define MESH_NETWORKSTATUS_INIT		1			/* Connecting/recieving connections but not yet done */
#define	MESH_NETWORKSTATUS_PARTIAL	2			/* Partial mesh, all local servers connected */
#define	MESH_NETWORKSTATUS_FULL		3			/* Full mesh, all servers are connected to all other servers */
	unsigned char networkstatus;				/* Status code of mesh network */
	unsigned char priority_algorithm;			/* Dictates which server ID's get priority in conflicts */
	unsigned int fullmeshcount;					/* Number of servers on network fully meshed */
	unsigned int total;							/* Total servers on network */
	unsigned int inactivity_threshold;			/* % of servers with timeouts before server is disconnected */

	LINKED_LIST_STRUCT llNodeHead;				/* List of mesh nodes (all inbound + outbound servers) */
} MESH_CONTROL_STRUCT;

/* Server information block */
typedef struct SERVER_CONTROL_STRUCT
{
#define SIGNAL_NONE			0
#define SIGNAL_SHUTDOWN		1
#define SIGNAL_ENDTHREAD	2
#define SIGNAL_THREADDONE	3
	unsigned int signal;						/* Event signal */
	unsigned char networkstate;					/* Network state, expressed as current network type */
	unsigned int skey;							/* Server table key for local server */

	CLIENT_STRUCT *sclient;						/* Pointer to server client info */
	SOCKET fdHighest;							/* Highest socket # */
	SOCKET fdListening[10];						/* Listening sockets */
	CLIENT_STRUCT *CSLocal[32000];				/* Connected clients */
	CLIENT_STRUCT *CSServers[MAX_SERVERTOKENS];	/* Server list */

	OUTBOUND_CONNECTION_STRUCT obc[MAX_SERVERCONNECTIONS];
												/* Outbound server information */

	MESSAGE_CONTROL_STRUCT msginfo;				/* Information about current message */
	MESH_CONTROL_STRUCT meshctrl;				/* Mesh control block */

	/* These are the global lists of all server objects */
	LINKED_LIST_STRUCT llUserHead[MAX_HOPS + 1];	/* Linked lists of all users */
	LINKED_LIST_STRUCT llServerHead[MAX_HOPS + 1];	/* Linked lists of all servers */
	LINKED_LIST_STRUCT llChannelHead;			/* Linked list of all channels */
	LINKED_LIST_STRUCT llAccessNetworkHead;		/* Linked list of all access entries for the network */
	LINKED_LIST_STRUCT llEventHead;				/* Linked list of users with event notifications */

	LUSERS_STRUCT lusers;						/* Lusers information */

	/* Debug client(if debugging enabled) */
	CLIENT_STRUCT *dbgclient;

} SERVER_CONTROL_STRUCT;

/* Used for treeview in the main RockIRCX program */
typedef struct SERVER_TVITEM_STRUCT
{
	HTREEITEM htvi;								/* HTREEITEM handle */
	BOOL bOutBound;								/* Server an outbound connection? */
	BOOL bConnected;							/* Server connected? */
	char servername[64];						/* Name of server */
	char hostmask[64];							/* Hostname of server */

	unsigned short port;						/* Port number(if outbound) */
} SERVER_TVITEM_STRUCT;

typedef struct SERVER_TV_STRUCT
{
	HWND hTreeView;
	HIMAGELIST himl;
	HTREEITEM htviMeshStatus;
	HTREEITEM htviOutBound;
	HTREEITEM htviInBound;

	HBITMAP hBMPOutBound;
	HBITMAP hBMPInBound;
	HBITMAP hBMPConnected;
	HBITMAP hBMPDisconnected;

	SERVER_TVITEM_STRUCT stis[MAX_SERVERCONNECTIONS];
} SERVER_TV_STRUCT;

#endif		/* __STRUCTS_H__ */