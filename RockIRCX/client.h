/*
** client.h
**
** Contains header information for clients to server
*/

#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <stdarg.h>
#include "messages.h"
#include "numeric.h"
#include "lookup.h"
#include "hash.h"
#include "..\Common Files\settings.h"


#define RandInt(_min,_max) ( (_min) + (int)(rand()%((_max)-(_min)+1)))

/* Defines */
#define			CHANNEL_HASHSIZE					32003

#define			CLIENT_MAXMESSAGEBURST				5
#define			CLIENT_HASHSIZE						32003

#define			SERVER_HASHSIZE						103

/* Client macros */
#define			Client_IsUser(client)				(client->user ? TRUE : FALSE)
#define			Client_IsServer(client)				(client->user ? FALSE : TRUE)
#define			Client_Nickname(client)				(client->user->nickname[0] ? client->user->nickname : "*")
#define			Client_IsLocal(client)				(client->hops == 1)
#define			Client_Invisible(client)			(client->modeflag & CLIENT_MODE_INVISIBLE)
#define			Client_IsAdmin(client)				(client->modeflag & CLIENT_MODE_ADMIN)
#define			Client_IsSysop(client)				(client->modeflag & CLIENT_MODE_SYSOP)
#define			Client_IsGagged(client)				(client->modeflag & CLIENT_MODE_GAG)
#define			Client_IsPriveledged(client)		((client->modeflag & CLIENT_MODE_ADMIN) || (client->modeflag & CLIENT_MODE_SYSOP))
#define			Client_IsAway(client)				(client->user->away[0] ? TRUE : FALSE)
#define			Client_InAChannel(client)			(client->user->llChannelHead.next ? TRUE : FALSE)

/* Flag/state macros */
#define			Client_Dead(client)					(client->state == CLIENT_STATE_DEAD)
#define			Client_Welcoming(client)			(client->state == CLIENT_STATE_WELCOMING)
#define			Client_Registered(client)			(client->flags & CLIENT_FLAG_REGISTERED ? TRUE : FALSE)
#define			Client_NewLine(client)				(client->flags & CLIENT_FLAG_NEWLINE)
#define			Client_NoNewLine(client)			(!(client->flags & CLIENT_FLAG_NEWLINE))
#define			Client_WasPinged(client)			(client->flags & CLIENT_FLAG_PINGED)
#define			Client_WasNetsplitVictim(client)	(client->flags & CLIENT_FLAG_NETSPLITVICTIM)

#define			Client_GetCollisionWinner(csClient1, csClient2)		(csClient1->signon <= csClient2->signon ? csClient1 : csClient2 )
/* Other macros */
#define			Server_Description(s_client)		s_client->server->description
#define			Server_Synched(s_client)			((!(s_client->flags & CLIENT_FLAG_SERVERSYNC)) && (!(s_client->flags & CLIENT_FLAG_USERSYNC)) && (!(s_client->flags & CLIENT_FLAG_CHANNELSYNC)))
#define			Access_GetTypeString(dwAccessLevel)	(dwAccessLevel == ACCESSLEVEL_OWNER ? "OWNER" : (dwAccessLevel == ACCESSLEVEL_HOST ? "HOST" : (dwAccessLevel == ACCESSLEVEL_VOICE ? "VOICE" : (dwAccessLevel == ACCESSLEVEL_GRANT ? "GRANT" : (dwAccessLevel == ACCESSLEVEL_DENY ? "DENY" : "NULL")))))

/* Function prototypes */

void			Client_AcceptNew(SOCKET fdListener);
void			Client_CreateDCList(CLIENT_STRUCT *csGoingDown, LINKED_LIST_STRUCT *llServerHead, LINKED_LIST_STRUCT *llUserHead);
void			Client_Disconnect(CLIENT_STRUCT *client);
void			Client_Destroy(CLIENT_STRUCT *client);
void			Client_Kill(CLIENT_STRUCT *csFrom, CLIENT_STRUCT *csTarget, char *szKillMessage);
void			Client_SendError(CLIENT_STRUCT *client, BOOL bCloseOnSend, const char *string, ...);
void			Client_SendErrorNumber(CLIENT_STRUCT *client, BOOL bCloseOnSend, const char *errstr);
void			Client_SendErrorNumberToAllLocal(const char *errstr,BOOL bCloseOnSend);
void			Client_SendMessage(CLIENT_STRUCT *csTo, CLIENT_STRUCT *csFrom, const char *szToken, const char *szMessage, const char *szBuffer, ...);
void			Client_SendToOne(CLIENT_STRUCT *client,BOOL bCloseOnSend, const char *string, ...);
void			Client_SendLusers(CLIENT_STRUCT *client);
int				Client_CheckForBan(CLIENT_STRUCT *client);
void			Client_SendWelcome(CLIENT_STRUCT *client);
void			Client_SendMOTD(CLIENT_STRUCT *client);
void			Client_SendList(CLIENT_STRUCT *client, BOOL bListX, const char *szParameters);
void			Client_SendNumericToOne(CLIENT_STRUCT *client, int numeric, const char *data);
void			Client_SendToAllLocal(BOOL bCloseOnSend, const char *string, ...);
void			Client_FlushBuffer(CLIENT_STRUCT *client);
int				Client_ReadPacket(CLIENT_STRUCT* client, FD_SET *pread_set);
int				Client_Parse(CLIENT_STRUCT *client, char *message, int length);
unsigned int	Client_GetIdle(CLIENT_STRUCT *c_target);
int				Client_GetWhoisChannels(CLIENT_STRUCT *csAsking, CLIENT_STRUCT *csTarget, char *pszOutputBuffer);
BOOL			Client_IsOnChannel(CLIENT_STRUCT *csUser, CHANNEL_STRUCT *chChannel, DWORD *pdwPriveledge, LINKED_LIST_STRUCT **llReturnMode);
int				Client_IsOnAccessList(CLIENT_STRUCT *csUser,ACCESS_STRUCT *accList, LINKED_LIST_STRUCT *llAccessHead);
void			Server_Broadcast(CLIENT_STRUCT *csOrigin, const char *message, ...);
void			Server_BroadcastFromUser(CLIENT_STRUCT *csUser, const char *szToken, const char *szMessage, const char *szBuffer, ...);
void			Server_SendUserInfo(CLIENT_STRUCT *csServer);
void			Server_SendServerInfo(CLIENT_STRUCT *csServer);
void			Server_SendChannelInfo(CLIENT_STRUCT *csServer, CHANNEL_STRUCT *chChannel);
CLIENT_STRUCT	*Client_GetServer(CLIENT_STRUCT *csTarget);
CLIENT_STRUCT	*User_Find(const char *nickname);
void			Parse_UserList(const char *szUserList, LINKED_LIST_STRUCT *llUserHead);
CLIENT_STRUCT	*Server_HashFind(const char *servername);
void			User_GetModeString(CLIENT_STRUCT *client, char *pszStringOutput, BOOL bOmitGagMode);
int				User_GetCommonChannels(CLIENT_STRUCT *client, CLIENT_STRUCT *csTarget, char *szChannelBuffer, LINKED_LIST_STRUCT *llChannelHead);
void			User_BroadcastToAllLocalsInAllChannels(CLIENT_STRUCT *csUser, CLIENT_STRUCT *csExcludeUser, char *szMessage, ...);
BOOL			User_DoWeMask(CLIENT_STRUCT *csFrom, CLIENT_STRUCT *csTo);
void			User_Initiate(CLIENT_STRUCT *client);
void			User_ForceNickChange(CLIENT_STRUCT *client, const char *szNewNick, BOOL bRandom);
int				Access_Add(ACCESS_STRUCT *accList, LINKED_LIST_STRUCT *llAccessHead, ACCESS_STRUCT *accInfo);
int				Access_Delete(ACCESS_STRUCT *accList, LINKED_LIST_STRUCT *llAccessHead, const char *szHostMask, DWORD dwLevel, BOOL bIsOwner);
ACCESS_STRUCT	*Access_Find(ACCESS_STRUCT *accList, LINKED_LIST_STRUCT *llAccessHead, const char *szHostMask, DWORD dwLevel);
int				Access_Clear(ACCESS_STRUCT *accList, LINKED_LIST_STRUCT *llAccessHead, BOOL bIsOwner, DWORD dwLevel);
void			Access_OutputEntries(CLIENT_STRUCT *client, char *szObject, LINKED_LIST_STRUCT *llAccessNetwork, LINKED_LIST_STRUCT *llAccessServer, CHANNEL_STRUCT *chTarget, CLIENT_STRUCT *csTarget);
void			Access_GetTypeFromString(char *szAccessString, DWORD *dwAccessType, BOOL bChannel);
void			Event_Broadcast(CLIENT_STRUCT *client, DWORD dwEventType, char *szString, ...);
void			Parse_DestinationList(const char *szDestinationList, LINKED_LIST_STRUCT *llDestinationList);
int				Mesh_NetworkStatus();
void			Mesh_Nodelist_Setup();
MESH_NODE		*Mesh_Nodelist_Find(const char *szServerName);

/* External variables */
extern			SERVER_CONTROL_STRUCT scs;
extern			INFOSTRUCTSETTINGS SettingsInfo;

/* External functions */
extern int		Message_Execute();
extern void		Serverlist_ChangeStatus(const char*, BOOL);

#endif		/* __CLIENT_H__ */