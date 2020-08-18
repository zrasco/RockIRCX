/*
** channel.h
**
** Contains header information relating to channels
*/

#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include "messages.h"
#include "hash.h"
#include "structs.h"
#include "..\Common Files\settings.h"

/* External variables */
extern HWND						hMain;
extern HWND						hStatus;
extern SERVER_CONTROL_STRUCT	scs;
extern INFOSTRUCTSETTINGS		SettingsInfo;
extern HASH_TABLE_STRUCT		HT_Channel;
extern HASH_TABLE_STRUCT		HT_User;
extern HASH_TABLE_STRUCT		HT_Server;

/* Defines */
#define				Channel_Limit(channel)						(channel->dwLimit ? channel->dwLimit : SettingsInfo.isChannel.defaultlimit)
#define				Channel_InviteOnly(channel)					(channel->dwModes & CHANNELMODE_INVITE ? TRUE : FALSE)
#define				Channel_Full(channel)						(channel->dwUsers >= Channel_Limit(channel) ? TRUE : FALSE)
#define				Channel_HasMemberkey(channel)				(channel->szPropMemberkey[0] ? TRUE : FALSE)
#define				Channel_AccessFull(channel)					(channel->dwAccessEntries >= SettingsInfo.isSecurity.max_access ? TRUE : FALSE)
#define				Channel_HasTopic(channel)					(channel->szPropTopic[0] ? TRUE : FALSE)
#define				Channel_IsLocal(channel)					(channel->szName[0] == '&' ? TRUE : FALSE)
#define				Channel_IsPublic(channel)					(!(channel->dwModes & CHANNELMODE_PRIVATE || channel->dwModes & CHANNELMODE_HIDDEN || channel->dwModes & CHANNELMODE_SECRET))
#define				Channel_IsPrivate(channel)					(channel->dwModes & CHANNELMODE_PRIVATE)
#define				Channel_IsHidden(channel)					(channel->dwModes & CHANNELMODE_HIDDEN)
#define				Channel_IsSecret(channel)					(channel->dwModes & CHANNELMODE_SECRET)
#define				Channel_IsRegistered(channel)				(channel->dwModes & CHANNELMODE_REGISTERED)
#define				Channel_NoExtern(channel)					(channel->dwModes & CHANNELMODE_NOEXTERN)
#define				Channel_Moderated(channel)					(channel->dwModes & CHANNELMODE_MODERATED)
#define				Channel_TopicOp(channel)					(channel->dwModes & CHANNELMODE_TOPICOP)
#define				Channel_AuthOnly(channel)					(channel->dwModes & CHANNELMODE_AUTHONLY)
#define				Channel_HasKnock(channel)					(channel->dwModes & CHANNELMODE_KNOCK)
#define				Channel_NoWhispers(channel)					(channel->dwModes & CHANNELMODE_NOWHISPER)
#define				Channel_Auditorium(channel)					(channel->dwModes & CHANNELMODE_AUDITORIUM)
#define				Channel_ReggedAndEmpty(channel)				((channel->dwModes & CHANNELMODE_REGISTERED) && !channel->dwUsers)

/* Function prototypes */
CHANNEL_STRUCT		*Channel_Find(char *szChannelName);
CHANNEL_STRUCT		*Channel_Create(char *szChannelName);
int					Channel_AddUser(CHANNEL_STRUCT *chChannel, CLIENT_STRUCT *csUser, DWORD dwUsermodes, char *szJoinState);
void				Channel_AddUsersFromList(CHANNEL_STRUCT *chChannel, const char *szUserlist);
void				Channel_DeleteAllUsers(CHANNEL_STRUCT *chChannel);
int					Channel_DeleteUser(CHANNEL_STRUCT *chChannel, CLIENT_STRUCT *csUser);
void				Channel_Cleanup(CHANNEL_STRUCT *chChannel, BOOL bKeepReg);
void				Channel_BroadcastToLocal(CHANNEL_STRUCT *chChannel, CLIENT_STRUCT *csUser, CLIENT_STRUCT *csExcludeUser, char *szToken, char *szMessage, char *szBuffer, ...);
void 				Channel_BroadcastToLocalOps(CHANNEL_STRUCT *chChannel, CLIENT_STRUCT *csUser, CLIENT_STRUCT *csExcludeUser, char *szMessage, ...);
void				Channel_BroadcastToLocalSelected(CHANNEL_STRUCT *chChannel, CLIENT_STRUCT *csUser, CLIENT_STRUCT *csExcludeUser, DWORD dwAccessMask, BOOL bAdmins, BOOL bSysops, char *szMessage, ...);
void				Channel_SendNames(CHANNEL_STRUCT *chChannel, CLIENT_STRUCT *csUser);
void				Channel_SendProperties(CHANNEL_STRUCT *chChannel, CLIENT_STRUCT *csUser, char *szProperties);
void				Channel_SendKnock(CHANNEL_STRUCT *chChannel, CLIENT_STRUCT *csFrom, int nNumeric);
void				Channel_SendOnjoin(CHANNEL_STRUCT *chChannel, CLIENT_STRUCT *csUser);
void				Channel_SendOnpart(CHANNEL_STRUCT *chChannel, CLIENT_STRUCT *csUser);
int					Channel_CreateUserlist(CHANNEL_STRUCT *chChannel, char *pszStringOutput, DWORD dwStringSize);
void				Channel_GetModeString(CHANNEL_STRUCT *chChannel, char *pszStringOutput);
DWORD				Channel_GetModeFlagFromChar(char cMode);
DWORD				Channel_GetModeFlagFromString(const char *szModeString);
DWORD				Channel_GetUserModeFlags(CHANNEL_STRUCT *chChannel, CLIENT_STRUCT *csUser);
void				Channel_KickAllLocal(CHANNEL_STRUCT *chChannel, char *szFrom, char *szReason);
void				Channel_Kill(CHANNEL_STRUCT *chChannel, CLIENT_STRUCT *csFrom, char *szReason);
void				Parse_ChannelList(const char *szChannelList, LINKED_LIST_STRUCT *llChannelHead);

#endif		/* __CHANNEL_H__ */