/*
** channel.cpp
**
** This file contains all the code related to tasks related to working with channels.
*/

#include "channel.h"

CHANNEL_STRUCT *Channel_Find(char *szChannelName)
/*
** Channel_Find()
** This function will attempt to locate the channel specified
** Returns NULL if channel was not found
*/
{
	CHANNEL_STRUCT *cret = NULL, *channel = NULL;
	HASH_FIND_STRUCT FindInfo;
	LINKED_LIST_STRUCT *curr = NULL;

	/* Find our channel in the hash table */
	FindInfo.textkey = szChannelName;
	FindInfo.data = NULL;

	Hash_Find(&HT_Channel,&FindInfo,HASH_NOKEY);

	if (FindInfo.found)
	/* The channel possibly exists */
	{
		curr = &(FindInfo.found->chainhead);

		while (curr->next)
		{
			curr = curr->next;

			channel = (CHANNEL_STRUCT*)curr->data;

			if (stricmp(szChannelName,channel->szName) == 0)
			/* Channel has been found */
			{
				cret = channel;
				break;
			}
		}
	}

	return cret;
}

CHANNEL_STRUCT *Channel_Create(char *szChannelName)
/*
** Channel_Create()
** Creates a regular ol' channel with default modes, and adds it to the lists
** Returns NULL if channel already exists
*/
{
	int nCount;
	RegChanList *rclptr = &(SettingsInfo.isChannel.RCLHead);
	AccessList *RCLAccess;
	CHANNEL_STRUCT *cret = NULL;
	ACCESS_STRUCT accInfo;
	BOOL bWasReg = FALSE;

	cret = Channel_Find(szChannelName);

	if (!cret)
	/* Create a new channel */
	{
		cret = (CHANNEL_STRUCT*)calloc(1,sizeof(CHANNEL_STRUCT));

		/* Let's name our channel */
		strcpy(cret->szName,szChannelName);

		/* Set creation time */
		cret->dwPropCreationTime = (DWORD)(time(NULL));

		/* Create access list */
		cret->accList = (ACCESS_STRUCT*)(calloc(sizeof(ACCESS_STRUCT),SettingsInfo.isSecurity.max_access + 2));

		/* Add our channel to the global list & to the global hash table */
		LL_Add(&(scs.llChannelHead),cret);
		Hash_Add(&HT_Channel,cret->szName,cret);

		/* Go through list of registered channels and see if we find a match */
		while (rclptr->next)
		{
			rclptr = rclptr->next;

			if (lstrcmpi(cret->szName,rclptr->name) == 0)
			/* We've found a match! */
			{
				strcpy(cret->szName,rclptr->name);

				if (rclptr->modeflags & ZRA_CHANMODE_AUDITORIUM)
					cret->dwModes |= CHANNELMODE_AUDITORIUM;
				if (rclptr->modeflags & ZRA_CHANMODE_AUTHONLY)
					cret->dwModes |= CHANNELMODE_AUTHONLY;
				if (rclptr->modeflags & ZRA_CHANMODE_INVITEONLY)
					cret->dwModes |= CHANNELMODE_INVITE;
				if (rclptr->modeflags & ZRA_CHANMODE_KNOCK)
					cret->dwModes |= CHANNELMODE_KNOCK;
				if (rclptr->modeflags & ZRA_CHANMODE_MODERATED)
					cret->dwModes |= CHANNELMODE_MODERATED;
				if (rclptr->modeflags & ZRA_CHANMODE_NOEXTERN)
					cret->dwModes |= CHANNELMODE_NOEXTERN;
				if (rclptr->modeflags & ZRA_CHANMODE_NOWHISPERS)
					cret->dwModes |= CHANNELMODE_NOWHISPER;
				if (rclptr->modeflags & ZRA_CHANMODE_ONLYOPSCHANGETOPIC)
					cret->dwModes |= CHANNELMODE_TOPICOP;

				switch (rclptr->visibility)
				{
					case ZRA_VISIBILITY_PUBLIC: cret->dwModes |= CHANNELMODE_PUBLIC; break;
					case ZRA_VISIBILITY_PRIVATE: cret->dwModes |= CHANNELMODE_PRIVATE; break;
					case ZRA_VISIBILITY_HIDDEN: cret->dwModes |= CHANNELMODE_HIDDEN; break;
					case ZRA_VISIBILITY_SECRET: cret->dwModes |= CHANNELMODE_SECRET; break;
				}

				cret->dwLimit = rclptr->limit;
				cret->dwModes |= CHANNELMODE_REGISTERED;

				/* Add topic, subject, and other properties */
				strcpy(cret->szPropTopic,rclptr->topic);
				strcpy(cret->szPropSubject,rclptr->subject);
				strcpy(cret->szPropOnjoin,rclptr->onjoin);
				strcpy(cret->szPropOnpart,rclptr->onpart);

				/* Add access entries */
				RCLAccess = &(rclptr->accesshead);

				while (RCLAccess->next)
				{
					RCLAccess = RCLAccess->next;

					accInfo.bOwner = TRUE;
					
					switch (RCLAccess->type)
					{
						case ZRA_ACCESSLEVEL_OWNER: accInfo.dwLevel = ACCESSLEVEL_OWNER; break;
						case ZRA_ACCESSLEVEL_HOST: accInfo.dwLevel = ACCESSLEVEL_HOST; break;
						case ZRA_ACCESSLEVEL_VOICE: accInfo.dwLevel = ACCESSLEVEL_VOICE; break;
						case ZRA_ACCESSLEVEL_GRANT: accInfo.dwLevel = ACCESSLEVEL_GRANT; break;
						case ZRA_ACCESSLEVEL_DENY: accInfo.dwLevel = ACCESSLEVEL_DENY; break;
					}

					accInfo.dwTimeout = RCLAccess->expire;
					strcpy(accInfo.szCreator,rclptr->name);
					Validate_AccessHostmask(RCLAccess->hostmask,accInfo.szHostmask);
					strcpy(accInfo.szReason,RCLAccess->reason);

					Access_Add(cret->accList,NULL,&accInfo);
					cret->dwAccessEntries++;
				}

				/* Add keys */
				strcpy(cret->szPropMemberkey,rclptr->memberkey);
				strcpy(cret->szPropHostkey,rclptr->hostkey);
				strcpy(cret->szPropOwnerkey,rclptr->ownerkey);

				bWasReg = TRUE;
			}
		}

		if (!bWasReg)
		{
			/* Dynamic public channel with all default modes */
			cret->dwModes = CHANNELMODE_PUBLIC;

			if (SettingsInfo.isChannel.modeflag & ZRA_CHANMODE_MODERATED)
				cret->dwModes |= CHANNELMODE_MODERATED;
			if (SettingsInfo.isChannel.modeflag & ZRA_CHANMODE_NOEXTERN)
				cret->dwModes |= CHANNELMODE_NOEXTERN;
			if (SettingsInfo.isChannel.modeflag & ZRA_CHANMODE_ONLYOPSCHANGETOPIC)
				cret->dwModes |= CHANNELMODE_TOPICOP;
			if (SettingsInfo.isChannel.modeflag & ZRA_CHANMODE_KNOCK)
				cret->dwModes |= CHANNELMODE_KNOCK;
			if (SettingsInfo.isChannel.modeflag & ZRA_CHANMODE_NOWHISPERS)
				cret->dwModes |= CHANNELMODE_NOWHISPER;

			/* Add non-registered channel to list */
			scs.lusers.nChannels++;
			SettingsInfo.isStatus.localchannels++;
			SettingsInfo.isStatus.globalchannels++;
		}
	}

	return cret;
}

int Channel_AddUser(CHANNEL_STRUCT *chChannel, CLIENT_STRUCT *csUser, DWORD dwUsermodes, char *szJoinState)
/*
** Channel_AddUser()
** Adds the user to the channel and lets all the other members know about it.
** THIS WILL ADD THE USER NO MATTER WHAT! CHECKING MUST BE DONE BEFORE CALLING THIS!
** THIS INCLUDES ACCESS/OWNERKEY CHECKING! IT WILL ADD AN OWNER IF SPECIFIED IN THE FLAGS
** Returns -1 if user is already on channel, otherwise returns 0
*/
{
	LINKED_LIST_STRUCT *llUser = LL_Find(&(chChannel->llUserHead),csUser);

	if (!llUser)
	/* Add this user */
	{
		char szUserMode[32];

		szUserMode[1] = 0;

		if (Channel_IsRegistered(chChannel) && !chChannel->dwUsers)
		/* Show registered channels with 1 or more people */
		{
			char szModeStr[256];

			Channel_GetModeString(chChannel,szModeStr);
			Event_Broadcast(csUser,EVENT_TYPE_CHANNEL,"CHANNEL CREATE %s %s %s!%s@%s %s:%u",chChannel->szName,szModeStr,Client_Nickname(csUser),csUser->user->username,csUser->hostname,csUser->ip_r,csUser->port_r);

			scs.lusers.nChannels++;
			SettingsInfo.isStatus.localchannels++;
			SettingsInfo.isStatus.globalchannels++;
		}

		/* Add user to the list of members in the channel */
		chChannel->dwUsers++;
		LL_Add(&(chChannel->llUserHead),csUser);
		LL_AddNoCheck(&(chChannel->llUserModeHead),(void*)dwUsermodes);

		/* Add this channel to the list of channels user is on */
		LL_Add(&(csUser->user->llChannelHead),(void*)(chChannel));

		/* Send JOIN message(to all), topic, topic set date, names list, onjoin, etc... to user */
		if (szJoinState)
			Server_BroadcastFromUser(csUser,TOK_JOIN,MSG_JOIN,"%s",szJoinState);
		else
			Server_BroadcastFromUser(csUser,TOK_JOIN,MSG_JOIN,"1 %s %u",chChannel->szName,chChannel->dwPropCreationTime);

		Channel_BroadcastToLocal(chChannel,csUser,NULL,TOK_JOIN,MSG_JOIN,":%s",chChannel->szName);

		if (Client_IsLocal(csUser))
		{
			/* Send topic */
			if Channel_HasTopic(chChannel)
				Client_SendToOne(csUser,FALSE,":%s %3.3d %s %s :%s",SettingsInfo.isGeneral.servername,332,Client_Nickname(csUser),chChannel->szName,chChannel->szPropTopic);

			/* Send names */
			Channel_SendNames(chChannel,csUser);

			/* Send onjoin */
			Channel_SendOnjoin(chChannel,csUser);
		}

		/* Broadcast user mode change(if any) to all other members */
		if (dwUsermodes & CHANNEL_PRIVELEDGE_OWNER)
			szUserMode[0] = 'q';
		else if (dwUsermodes & CHANNEL_PRIVELEDGE_HOST)
			szUserMode[0] = 'o';
		else if (dwUsermodes & CHANNEL_PRIVELEDGE_VOICE)
			szUserMode[0] = 'v';

		if (dwUsermodes)
			Channel_BroadcastToLocal(chChannel,csUser,csUser,TOK_MODE,MSG_MODE,"%s +%s %s",chChannel->szName,szUserMode,csUser->user->nickname);

		Event_Broadcast(csUser,EVENT_TYPE_MEMBER,"MEMBER JOIN %s %s!%s@%s %s:%u %s",chChannel->szName,Client_Nickname(csUser),csUser->user->username,csUser->hostname,csUser->ip_r,csUser->port_r,szUserMode);
	}

	return -1;
}

void Channel_SendProperties(CHANNEL_STRUCT *chChannel, CLIENT_STRUCT *csUser, char *szProperties)
/*
** Channel_SendProperties()
** Sends the channel properties to the specified user
*/
{
	DWORD dwPriveledge;
	BOOL bIsOnChannel = Client_IsOnChannel(csUser,chChannel,&dwPriveledge,NULL);
	char buffer[4096];

	if (szProperties)
	{
		strcpy(buffer,szProperties);
		strupr(buffer);
	}

	/*
	** OID, NAME, CREATION, and LANGUAGE
	**
	** Can be read by:
	** - Sysops/admins
	** - Channel members
	** - Users outside the channel if not secret or hidden
	**
	** Cannot be read by:
	** - Users outside the channel if secret or hidden
	** 
	*/
	if (Channel_IsPublic(chChannel) || Channel_IsHidden(chChannel) ||
		Client_IsPriveledged(csUser) || bIsOnChannel)
	{
		if ((szProperties && (strstr(buffer,"OID") || (buffer[0] == '*' && buffer[1] == 0))))
			Client_SendToOne(csUser,FALSE,":%s %3.3d %s %s OID :%d",SettingsInfo.isGeneral.servername,IRCRPL_PROPLIST,Client_Nickname(csUser),chChannel->szName,chChannel->dwPropOID);
		if ((szProperties && (strstr(buffer,"NAME") || (buffer[0] == '*' && buffer[1] == 0))))
			Client_SendToOne(csUser,FALSE,":%s %3.3d %s %s Name :%s",SettingsInfo.isGeneral.servername,IRCRPL_PROPLIST,Client_Nickname(csUser),chChannel->szName,chChannel->szName);
		if ((szProperties && (strstr(buffer,"CREATION") || (buffer[0] == '*' && buffer[1] == 0))))
			Client_SendToOne(csUser,FALSE,":%s %3.3d %s %s Creation :%d",SettingsInfo.isGeneral.servername,IRCRPL_PROPLIST,Client_Nickname(csUser),chChannel->szName,chChannel->dwPropCreationTime);
		
		if (chChannel->szPropLanguage[0] && ((szProperties && (strstr(buffer,"LANGUAGE") || (buffer[0] == '*' && buffer[1] == 0)))))
			Client_SendToOne(csUser,FALSE,":%s %3.3d %s %s Language :%d",SettingsInfo.isGeneral.servername,IRCRPL_PROPLIST,Client_Nickname(csUser),chChannel->szName,chChannel->szPropLanguage);
	}

	/* OWNERKEY, HOSTKEY, and MEMBERKEY may be set, but never read */

	/*
	** PICS
	**
	** Can be read by:
	** - Everyone except users outside the channel if it is SECRET
	*/
	if (((szProperties && (strstr(buffer,"PICS") || (buffer[0] == '*' && buffer[1] == 0)))) &&
		chChannel->szPropPICS[0] && (Client_IsPriveledged(csUser) || Channel_IsPublic(chChannel) ||
		Channel_IsHidden(chChannel) || Channel_IsPrivate(chChannel) || bIsOnChannel))
		Client_SendToOne(csUser,FALSE,":%s %3.3d %s %s PICS :%s",SettingsInfo.isGeneral.servername,IRCRPL_PROPLIST,Client_Nickname(csUser),chChannel->szName,chChannel->szPropPICS);

	/*
	** TOPIC, SUBJECT, CLIENT
	** Can be read by:
	** - Admins/sysops
	** - Members of the channel
	** - Outside users if public or hidden
	*/
	if (Client_IsPriveledged(csUser) || Channel_IsPublic(chChannel) ||
		Channel_IsHidden(chChannel) || bIsOnChannel)
	{
		if (chChannel->szPropTopic[0] && ((szProperties && (strstr(buffer,"TOPIC") || (buffer[0] == '*' && buffer[1] == 0)))))
			Client_SendToOne(csUser,FALSE,":%s %3.3d %s %s Topic :%s",SettingsInfo.isGeneral.servername,IRCRPL_PROPLIST,Client_Nickname(csUser),chChannel->szName,chChannel->szPropTopic);
		if (chChannel->szPropSubject[0] && ((szProperties && (strstr(buffer,"SUBJECT") || (buffer[0] == '*' && buffer[1] == 0)))))
			Client_SendToOne(csUser,FALSE,":%s %3.3d %s %s Subject :%s",SettingsInfo.isGeneral.servername,IRCRPL_PROPLIST,Client_Nickname(csUser),chChannel->szName,chChannel->szPropSubject);
		if (chChannel->szPropClient[0] && ((szProperties && (strstr(buffer,"CLIENT") || (buffer[0] == '*' && buffer[1] == 0)))))
			Client_SendToOne(csUser,FALSE,":%s %3.3d %s %s Client :%s",SettingsInfo.isGeneral.servername,IRCRPL_PROPLIST,Client_Nickname(csUser),chChannel->szName,chChannel->szPropClient);
	}
	/*
	** ONJOIN and ONPART
	** Can be read by:
	** - Admins/sysops
	** - Owners/hosts of the channel
	*/
	if (Client_IsPriveledged(csUser) || dwPriveledge & CHANNEL_PRIVELEDGE_OWNER || dwPriveledge & CHANNEL_PRIVELEDGE_HOST)
	{
		if (chChannel->szPropOnjoin[0] && ((szProperties && (strstr(buffer,"ONJOIN") || (buffer[0] == '*' && buffer[1] == 0)))))
				Client_SendToOne(csUser,FALSE,":%s %3.3d %s %s Onjoin :%s",SettingsInfo.isGeneral.servername,IRCRPL_PROPLIST,Client_Nickname(csUser),chChannel->szName,chChannel->szPropOnjoin);
		if (chChannel->szPropOnpart[0] && ((szProperties && (strstr(buffer,"ONPART") || (buffer[0] == '*' && buffer[1] == 0)))))
				Client_SendToOne(csUser,FALSE,":%s %3.3d %s %s Onpart :%s",SettingsInfo.isGeneral.servername,IRCRPL_PROPLIST,Client_Nickname(csUser),chChannel->szName,chChannel->szPropOnpart);
	}

	/*
	** LAG
	** Can be read like the TOPIC property
	*/
	if (chChannel->dwPropLag > 0 && (Client_IsPriveledged(csUser) || Channel_IsPublic(chChannel) ||
		Channel_IsHidden(chChannel) || bIsOnChannel) && ((szProperties && (strstr(buffer,"LAG") || (buffer[0] == '*' && buffer[1] == 0)))))
		Client_SendToOne(csUser,FALSE,":%s %3.3d %s %s Lag :%d",SettingsInfo.isGeneral.servername,IRCRPL_PROPLIST,Client_Nickname(csUser),chChannel->szName,chChannel->dwPropLag);
	/*
	** ACCOUNT
	** Can only read by the following:
	** - Admins/sysops
	** - Owners
	*/
	if (chChannel->szPropAccount[0] && (Client_IsPriveledged(csUser) || dwPriveledge & CHANNEL_PRIVELEDGE_OWNER) &&
		((szProperties && (strstr(buffer,"ACCOUNT") || (buffer[0] == '*' && buffer[1] == 0)))))
		Client_SendToOne(csUser,FALSE,":%s %3.3d %s %s Account :%s",SettingsInfo.isGeneral.servername,IRCRPL_PROPLIST,Client_Nickname(csUser),chChannel->szName,chChannel->szPropAccount);

	/*
	** CLIENTGUID and SERVICEPATH
	** These are read like the TOPIC/LAG properties
	*/
	if (Client_IsPriveledged(csUser) || Channel_IsPublic(chChannel) ||
		Channel_IsHidden(chChannel) || bIsOnChannel)
	{
		if (chChannel->dwPropClientGUID > 0 && ((szProperties && (strstr(buffer,"CLIENTGUID") || (buffer[0] == '*' && buffer[1] == 0)))))
			Client_SendToOne(csUser,FALSE,":%s %3.3d %s %s ClientGUID :%d",SettingsInfo.isGeneral.servername,IRCRPL_PROPLIST,Client_Nickname(csUser),chChannel->szName,chChannel->dwPropClientGUID);
		if (chChannel->dwPropServicepath > 0 && ((szProperties && (strstr(buffer,"SERVICEPATH") || (buffer[0] == '*' && buffer[1] == 0)))))
			Client_SendToOne(csUser,FALSE,":%s %3.3d %s %s Servicepath :%d",SettingsInfo.isGeneral.servername,IRCRPL_PROPLIST,Client_Nickname(csUser),chChannel->szName,chChannel->dwPropServicepath);
	}

	/* End of properties */
	Client_SendToOne(csUser,FALSE,":%s %3.3d %s %s :End of properties",SettingsInfo.isGeneral.servername,IRCRPL_PROPEND,Client_Nickname(csUser),chChannel->szName);

}

void Channel_AddUsersFromList(CHANNEL_STRUCT *chChannel, const char *szUserlist)
/*
** Channel_AddUsersFromList
** Adds the users specified in the list, but does not broadcast joins etc
** Intended to be used in response to NJOIN commands
*/
{
	char szNextUser[64], cPriveledge;
	int tokens = numtok(szUserlist,32);
	int index;
	DWORD dwPriveledge;
	CLIENT_STRUCT *csUser;

	for (index = 1; index <= tokens; index++)
	/* Get our users */
	{
		/* Get our next user & their priveledge */
		gettok(szNextUser,szUserlist,index,32);
		cPriveledge = szNextUser[0];

		if (cPriveledge == '.')
			dwPriveledge = CHANNEL_PRIVELEDGE_OWNER;
		else if (cPriveledge == '@')
			dwPriveledge = CHANNEL_PRIVELEDGE_HOST;
		else if (cPriveledge == '+')
			dwPriveledge = CHANNEL_PRIVELEDGE_VOICE;
		else
			dwPriveledge = 0x00000000;

		if (dwPriveledge)
		/* Take our priveledge marker out before we search */
			strcpy(szNextUser,&szNextUser[1]);

		csUser = User_Find(szNextUser);

		if (csUser)
		/* We should be able to find EVERY user on the list */
			Channel_AddUser(chChannel,csUser,dwPriveledge,NULL);
		else
		/* If not, something has gone horribly wrong */
			ODS("Channel_AddUsersFromList() attempting to add an invalid user(%s)!",szNextUser);

	}

}

void Channel_DeleteAllUsers(CHANNEL_STRUCT *chChannel)
/*
** Channel_DeleteAllUsers()
** Deletes all users from the channel list!
*/
{
	while (chChannel->llUserHead.next)
		Channel_DeleteUser(chChannel,(CLIENT_STRUCT*)chChannel->llUserHead.next->data);
}

int Channel_DeleteUser(CHANNEL_STRUCT *chChannel, CLIENT_STRUCT *csUser)
/*
** Channel_DeleteUser()
** Deletes a user from the channel list, but needs to be kicked out somehow first
** This function will not issue a PART QUIT KICK or KILL on behalf of the user!
*/
{
	LINKED_LIST_STRUCT *llPrevUser = NULL, *llUser = &(chChannel->llUserHead);
	LINKED_LIST_STRUCT *llPrevUserMode = NULL, *llUserMode = &(chChannel->llUserModeHead);

	/* Send onpart message */
	Channel_SendOnpart(chChannel,csUser);

	/* Remove this channel from the list of channels user is on */
	LL_Remove(&(csUser->user->llChannelHead),(void*)(chChannel));

	/* Update our channel user count */
	chChannel->dwUsers--;

	while (llUser->next)
	/* Go thru channel list and find the user/user modes */
	{
		llPrevUser = llUser; llPrevUserMode = llUserMode;
		llUser = llUser->next;
		llUserMode = llUserMode->next;

		if ((CLIENT_STRUCT*)(llUser->data) == csUser)
		/* Found 'em! */
		{
			/* Remove from the list */
			llPrevUser->next = llUser->next;
			llPrevUserMode->next = llUserMode->next;

			LLNode_Free(llUser);
			LLNode_Free(llUserMode);

			return 0;
		}
	}

	return 0;
}

void Channel_Cleanup(CHANNEL_STRUCT *chChannel, BOOL bKeepReg)
/*
** Channel_Cleanup()
** Deallocates and clears the channel, ONLY CALL WHEN NOBODY IS IN THERE!
*/
{
	/* Clear access entry table */
	free(chChannel->accList);

	/* Remove from all lists */
	Hash_Delete(&HT_Channel,chChannel->szName,chChannel);
	LL_Remove(&(scs.llChannelHead),chChannel);

	/* Update our counters */
	if (!Channel_IsRegistered(chChannel) || Channel_ReggedAndEmpty(chChannel))
	{
		scs.lusers.nChannels--;
		SettingsInfo.isStatus.localchannels--;
		SettingsInfo.isStatus.globalchannels--;
	}

	if (bKeepReg && Channel_IsRegistered(chChannel))
	/* Re-create the channel with 0 people if registered */
		Channel_Create(chChannel->szName);

	free(chChannel);
}

void Channel_BroadcastToAll(CHANNEL_STRUCT *chChannel, CLIENT_STRUCT *csUser, CLIENT_STRUCT *csExcludeUser, char *szToken, char *szMessage, char *szBuffer, ...)
/*
** Channel_BroadcastToLocal()
** Sends the message for specified user to everyone on the channel
** If csExcludeUser is specified, it will not send to them!
*/
{
	LINKED_LIST_STRUCT *llUser = &(chChannel->llUserHead);
	LINKED_LIST_STRUCT *llUsermode = &(chChannel->llUserModeHead);
	LINKED_LIST_STRUCT llTargetServerHead, *llTSPtr = &llTargetServerHead;
	CLIENT_STRUCT *csNext = NULL;
	DWORD dwUsermode = 0, dwUsermodeSource = 0;
	BOOL bOnChannel = FALSE;

	va_list vArgs;
	char buf[2048];

	va_start(vArgs,szBuffer);
	vsprintf(buf,szBuffer,vArgs);
	va_end(vArgs);

	llTSPtr->data = NULL;
	llTSPtr->next = NULL;

	/* Get channel permissions of calling user */
	while (llUser->next)
	{
		llUser = llUser->next;
		llUsermode = llUsermode->next;
		dwUsermode = (DWORD)llUsermode->data;

		if ((CLIENT_STRUCT*)(llUser->data) == csUser)
		{
			dwUsermodeSource = dwUsermode;
			bOnChannel = TRUE;
			break;
		}
	}

	/* Reset user list reference pointers */
	llUser = &(chChannel->llUserHead);
	llUsermode = &(chChannel->llUserModeHead);

	/* Send the message to locals and pass it along */
	while (llUser->next)
	{
		llUser = llUser->next;
		llUsermode = llUsermode->next;
		dwUsermode = (DWORD)llUsermode->data;

		csNext = (CLIENT_STRUCT*)llUser->data;

		if ((!Channel_Auditorium(chChannel)) ||
			((Channel_Auditorium(chChannel)) && (csNext == csUser || ((dwUsermodeSource & CHANNEL_PRIVELEDGE_OWNER) || (dwUsermodeSource & CHANNEL_PRIVELEDGE_HOST)))) ||		/* Source user is a host, or... */
			((Channel_Auditorium(chChannel)) && (csNext == csUser || ((dwUsermode & CHANNEL_PRIVELEDGE_OWNER) || (dwUsermode & CHANNEL_PRIVELEDGE_HOST))))					/* Destination user is! */
			)
		{
			if (csNext != csExcludeUser)
			/* Send to all but this user */
			{
				if (Client_IsLocal(csNext))
				{
					if (csUser == NULL)
					/* When csUser is NULL, send message from "System" */
						Client_SendToOne(csNext,FALSE,":System %s %s",szMessage,buf);
					else if (csNext == csUser || SettingsInfo.isGeneral.dns_masktype == SETTINGS_GENERAL_DONTMASK ||
						( SettingsInfo.isGeneral.dns_masktype == SETTINGS_GENERAL_XOUT &&
						(dwUsermode & CHANNEL_PRIVELEDGE_OWNER ||
						dwUsermode & CHANNEL_PRIVELEDGE_HOST)))
					/* If masking on, only send whole hostmask to people who are owners/hosts */
						Client_SendToOne(csNext,FALSE,":%s!~%s@%s %s %s",csUser->user->nickname,csUser->user->username,csUser->hostname,szMessage,buf);
					else
					/* Send it masked */
						Client_SendToOne(csNext,FALSE,":%s!~%s@%s %s %s",csUser->user->nickname,csUser->user->username,csUser->hostmask,szMessage,buf);
				}
				else
				/* Add relevant server to list */
					LL_Add(&llTargetServerHead,(void*)csNext->servernext);
			}
		}
	}

	llTSPtr = &llTargetServerHead;

	/* Client_SendToOne skips non-locals, so we'll send to them here */
	while (llTSPtr->next)
	{
		llTSPtr = llTSPtr->next;

		csNext = (CLIENT_STRUCT*)llTSPtr->data;

		if (csUser == NULL)
		{
			if (szToken)
				Client_SendToOne(csNext,FALSE,":System %s %s",szToken,buf);
			else
				Client_SendToOne(csNext,FALSE,":System %s %s",szMessage,buf);
		}
		if (szToken)
			Client_SendToOne(csNext,FALSE,":%s!~%s@%s %s %s",csUser->user->nickname,csUser->user->username,csUser->hostname,szToken,buf);
		else
			Client_SendToOne(csNext,FALSE,":%s!~%s@%s %s %s",csUser->user->nickname,csUser->user->username,csUser->hostname,szMessage,buf);
	}

	LL_Clear(&llTargetServerHead);
}

void Channel_BroadcastToLocal(CHANNEL_STRUCT *chChannel, CLIENT_STRUCT *csUser, CLIENT_STRUCT *csExcludeUser, char *szToken, char *szMessage, char *szBuffer, ...)
/*
** Channel_BroadcastToLocal()
** Sends the message for specified user to everyone on the channel
** If csExcludeUser is specified, it will not send to them!
*/
{
	LINKED_LIST_STRUCT *llUser = &(chChannel->llUserHead);
	LINKED_LIST_STRUCT *llUsermode = &(chChannel->llUserModeHead);
	CLIENT_STRUCT *csNext = NULL;
	DWORD dwUsermode = 0, dwUsermodeSource = 0;
	BOOL bOnChannel = FALSE;

	va_list vArgs;
	char buf[2048];

	va_start(vArgs,szBuffer);
	vsprintf(buf,szBuffer,vArgs);
	va_end(vArgs);

	/* Get channel permissions of calling user */
	while (llUser->next)
	{
		llUser = llUser->next;
		llUsermode = llUsermode->next;
		dwUsermode = (DWORD)llUsermode->data;

		if ((CLIENT_STRUCT*)(llUser->data) == csUser)
		{
			dwUsermodeSource = dwUsermode;
			bOnChannel = TRUE;
			break;
		}
	}

	/* Reset user list reference pointers */
	llUser = &(chChannel->llUserHead);
	llUsermode = &(chChannel->llUserModeHead);

	/* Send the message to locals and pass it along */
	while (llUser->next)
	{
		llUser = llUser->next;
		llUsermode = llUsermode->next;
		dwUsermode = (DWORD)llUsermode->data;

		csNext = (CLIENT_STRUCT*)llUser->data;

		if ((!Channel_Auditorium(chChannel)) ||
			((Channel_Auditorium(chChannel)) && (csNext == csUser || ((dwUsermodeSource & CHANNEL_PRIVELEDGE_OWNER) || (dwUsermodeSource & CHANNEL_PRIVELEDGE_HOST)))) ||		/* Source user is a host, or... */
			((Channel_Auditorium(chChannel)) && (csNext == csUser || ((dwUsermode & CHANNEL_PRIVELEDGE_OWNER) || (dwUsermode & CHANNEL_PRIVELEDGE_HOST))))					/* Destination user is! */
			)
		{
			if (csNext != csExcludeUser)
			/* Send to all but this user */
			{
				if (Client_IsLocal(csNext))
				{
					if (csUser == NULL)
					/* When csUser is NULL, send message from "System" */
						Client_SendToOne(csNext,FALSE,":System %s %s",szMessage,buf);
					else if (csNext == csUser || SettingsInfo.isGeneral.dns_masktype == SETTINGS_GENERAL_DONTMASK ||
						( SettingsInfo.isGeneral.dns_masktype == SETTINGS_GENERAL_XOUT &&
						(dwUsermode & CHANNEL_PRIVELEDGE_OWNER ||
						dwUsermode & CHANNEL_PRIVELEDGE_HOST)))
					/* If masking on, only send whole hostmask to people who are owners/hosts */
						Client_SendToOne(csNext,FALSE,":%s!~%s@%s %s %s",csUser->user->nickname,csUser->user->username,csUser->hostname,szMessage,buf);
					else
					/* Send it masked */
						Client_SendToOne(csNext,FALSE,":%s!~%s@%s %s %s",csUser->user->nickname,csUser->user->username,csUser->hostmask,szMessage,buf);
				}
			}
		}
	}
}

void Channel_BroadcastToLocalOps(CHANNEL_STRUCT *chChannel, CLIENT_STRUCT *csUser, CLIENT_STRUCT *csExcludeUser, char *szMessage, ...)
/*
** Channel_BroadcastToLocalOps()
** Sends the message to all ops on the specified channel, and will unmask the sender
** Otherwise works identical to the function above
*/
{
	LINKED_LIST_STRUCT *llUser = &(chChannel->llUserHead);
	LINKED_LIST_STRUCT *llUsermode = &(chChannel->llUserModeHead);
	CLIENT_STRUCT *csNext = NULL;
	DWORD dwUsermode = 0;

	va_list vArgs;
	char buf[2048];

	va_start(vArgs,szMessage);
	vsprintf(buf,szMessage,vArgs);
	va_end(vArgs);

	/* Send the message to locals */
	while (llUser->next)
	{
		llUser = llUser->next;
		llUsermode = llUsermode->next;
		dwUsermode = (DWORD)llUsermode->data;

		csNext = (CLIENT_STRUCT*)llUser->data;

		if (csNext != csExcludeUser && ((dwUsermode & CHANNEL_PRIVELEDGE_OWNER) || (dwUsermode & CHANNEL_PRIVELEDGE_HOST)))
		/* Send to all local ops, excluding csExcludeUser of course */
		{
			if (Client_IsLocal(csNext))
				Client_SendToOne(csNext,FALSE,":%s!~%s@%s %s",csUser->user->nickname,csUser->user->username,csUser->hostname,buf);
		}
	}

}
void Channel_BroadcastToLocalSelected(CHANNEL_STRUCT *chChannel, CLIENT_STRUCT *csUser, CLIENT_STRUCT *csExcludeUser, DWORD dwAccessMask, BOOL bAdmins, BOOL bSysops, char *szMessage, ...)
/*
** Channel_BroadcastToLocalSelected()
** Sends the message to the target recipients as indicated in these parameters:
** dwAccessMask = Bitwise OR of flags...example: ACCESSLEVEL_OWNER | ACCESSLEVEL_HOST
** bSysops = Message sysops
** bAdmins = Message admins
*/
{
	LINKED_LIST_STRUCT *llUser = &(chChannel->llUserHead);
	LINKED_LIST_STRUCT *llUsermode = &(chChannel->llUserModeHead);
	CLIENT_STRUCT *csNext = NULL;
	DWORD dwUsermode = 0;

	va_list vArgs;
	char buf[2048];

	va_start(vArgs,szMessage);
	vsprintf(buf,szMessage,vArgs);
	va_end(vArgs);

	/* Send the message to locals and pass it along */
	while (llUser->next)
	{
		llUser = llUser->next;
		llUsermode = llUsermode->next;
		dwUsermode = (DWORD)llUsermode->data;

		csNext = (CLIENT_STRUCT*)llUser->data;

		if (Client_IsLocal(csNext))
		{
			if ((csNext != csExcludeUser && (dwAccessMask & dwUsermode)) || (bAdmins && Client_IsAdmin(csNext)) || (bSysops && Client_IsSysop(csNext)))
			/* Send to everyone with the parameters specified */
			Client_SendToOne(csNext,FALSE,":%s!~%s@%s %s",csUser->user->nickname,csUser->user->username,csUser->hostname,buf);
		}
	}
}
void Channel_SendNames(CHANNEL_STRUCT *chChannel, CLIENT_STRUCT *csUser)
/*
** Channel_SendNames()
** Sends a NAMES list for a channel to the specified user, and performs all checking
*/
{
	LINKED_LIST_STRUCT *llUser = &(chChannel->llUserHead);
	LINKED_LIST_STRUCT *llUsermode = &(chChannel->llUserModeHead);
	DWORD len = 0;
	DWORD dwUsermode = 0, dwPriveledge = 0;
	char buf[8192], szWork[128];
	BOOL bIsOnChannel = Client_IsOnChannel(csUser,chChannel,&dwPriveledge,NULL);
	int nNamesCount = 0;

	buf[0] = 0;

	while (llUser->next)
	/* Go thru list and start addin em! */
	{
        llUser = llUser->next;
		llUsermode = llUsermode->next;
		dwUsermode = (DWORD)llUsermode->data;

		/*
		** Display names as follows:
		**
		** 1) In +x channel mode, omit all users to non-hosts and non-owners except owners, hosts, and the calling client.
		** 2) If client is on the channel, ignore +i flag on users.
		** 3) Do not show names to non-members if channel mode is +p, +s, or +h.
		*/
		if (bIsOnChannel || Channel_IsPublic(chChannel))									/* #3 */
		{
			/* Add our mode */
			if (dwUsermode & CHANNEL_PRIVELEDGE_OWNER)
				strcpy(szWork,".");
			else if (dwUsermode & CHANNEL_PRIVELEDGE_HOST)
				strcpy(szWork,"@");
			else if (dwUsermode & CHANNEL_PRIVELEDGE_VOICE)
				strcpy(szWork,"+");
			else
				szWork[0] = 0;

			if (!Channel_Auditorium(chChannel) ||
				szWork[0] == '.' ||
				szWork[0] == '@' ||
				dwPriveledge & CHANNEL_PRIVELEDGE_OWNER ||
				dwPriveledge & CHANNEL_PRIVELEDGE_HOST ||
				(CLIENT_STRUCT*)llUser->data == csUser)										/* #1 */
			{
				if (bIsOnChannel || !Client_Invisible(((CLIENT_STRUCT*)(llUser->data))))	/* #2 */
				{
					/* Add our user */
					strcat(szWork,((CLIENT_STRUCT*)llUser->data)->user->nickname);
					strcat(szWork," ");

					len += (DWORD)strlen(szWork);
					nNamesCount++;

					/* Add to the list */
					strcat(buf,szWork);
				}
			}
		}

		if (nNamesCount >= MAX_NAMES || !llUser->next)
		/* Dump the buffer */
		{
			if (nNamesCount > 0)
				Client_SendToOne(csUser,FALSE,":%s %d %s = %s :%s",
					SettingsInfo.isGeneral.servername,
					RPL_NAMREPLY,
					csUser->user->nickname,
					chChannel->szName,
					buf);

			buf[0] = 0;
			nNamesCount = 0;
		}
	}

	/* End of /NAMES */
	Client_SendToOne(csUser,FALSE,":%s %d %s %s :End of /NAMES list",
		SettingsInfo.isGeneral.servername,
		RPL_ENDOFNAMES,
		csUser->user->nickname,
		chChannel->szName);
}

void Channel_SendKnock(CHANNEL_STRUCT *chChannel, CLIENT_STRUCT *csFrom, int nNumeric)
/*
** Channel_SendKnock()
** Sends a knock notification to the entire channel
*/
{
	if (Channel_HasKnock(chChannel))
	{
		Channel_BroadcastToLocalOps(chChannel,csFrom,NULL,"KNOCK %s %3.3d",chChannel,nNumeric);
		Server_BroadcastFromUser(csFrom,TOK_KNOCK,MSG_KNOCK,"%s %3.3d",chChannel,nNumeric);
	}
}

void Channel_SendOnjoin(CHANNEL_STRUCT *chChannel, CLIENT_STRUCT *csUser)
/*
** Channel_SendOnjoin()
** Sends the onjoin privmsgs to the target user
*/
{
	BOOL bQuit = FALSE;
	int nCount = 0;
	char buf[1024];
	char *szPtr = chChannel->szPropOnjoin, *szPtr2 = NULL;

	if (chChannel->szPropOnjoin[0])
	{
		while (bQuit == FALSE)
		{
			szPtr2 = strstr(szPtr,"\\n");

			if (szPtr2)
				szPtr2 = &szPtr2[2];

			if (szPtr2)
			/* Not the last occurance */
			{
				strncpy(buf,szPtr,szPtr2-szPtr-2);
				buf[szPtr2-szPtr-2] = 0;
			}
			else
			/* Last occurance, should hit the NULL */
			{
				strncpy(buf,szPtr,sizeof(buf));
				bQuit = TRUE;
			}

			if (!(buf[0] == 0 && bQuit == TRUE))
				Client_SendToOne(csUser,FALSE,":%s PRIVMSG %s :%s",chChannel->szName,chChannel->szName,buf);

			szPtr = szPtr2;
		}

	}
}
void Channel_SendOnpart(CHANNEL_STRUCT *chChannel, CLIENT_STRUCT *csUser)
/*
** Channel_SendOnpart()
** Sends the onjoin privmsgs to the target user
*/
{
	BOOL bQuit = FALSE;
	int nCount = 0;
	char buf[1024];
	char *szPtr = chChannel->szPropOnpart, *szPtr2 = NULL;

	if (chChannel->szPropOnpart[0])
	{
		while (bQuit == FALSE)
		{
			szPtr2 = strstr(szPtr,"\\n");

			if (szPtr2)
				szPtr2 = &szPtr2[2];

			if (szPtr2)
			/* Not the last occurance */
			{
				strncpy(buf,szPtr,szPtr2-szPtr-2);
				buf[szPtr2-szPtr-2] = 0;
			}
			else
			/* Last occurance, should hit the NULL */
			{
				strncpy(buf,szPtr,sizeof(buf));
				bQuit = TRUE;
			}

			if (!(buf[0] == 0 && bQuit == TRUE))
				Client_SendToOne(csUser,FALSE,":%s NOTICE %s :%s",chChannel->szName,Client_Nickname(csUser),buf);

			szPtr = szPtr2;
		}

	}
}
int Channel_CreateUserlist(CHANNEL_STRUCT *chChannel, char *pszStringOutput, DWORD dwStringSize)
/*
** Channel_CreateUserlist()
** Fills the specified string with a list of users for the channel
** Returns -1 if the list needed to be truncated
*/
{
	LINKED_LIST_STRUCT *llUser = &(chChannel->llUserHead);
	LINKED_LIST_STRUCT *llUsermode = &(chChannel->llUserModeHead);
	char *buf = (char*)calloc(1,dwStringSize);
	DWORD len = 0;
	DWORD dwUsermode = 0;
	char namebuf[128];

	while (llUser->next)
	/* Go thru list and start addin em! */
	{
        llUser = llUser->next;
		llUsermode = llUsermode->next;
		dwUsermode = (DWORD)llUsermode->data;

		/* Add our mode */
		if (dwUsermode & CHANNEL_PRIVELEDGE_OWNER)
			strcpy(namebuf,".");
		else if (dwUsermode & CHANNEL_PRIVELEDGE_HOST)
			strcpy(namebuf,"@");
		else if (dwUsermode & CHANNEL_PRIVELEDGE_VOICE)
			strcpy(namebuf,"+");
		else
			namebuf[0] = 0;

		/* Add our user */
		strcat(namebuf,((CLIENT_STRUCT*)llUser->data)->user->nickname);
		strcat(namebuf," ");

		len += (DWORD)strlen(namebuf);

		if (len >= dwStringSize)
		/* List supplied not big enough! */
		{
			free(buf);
			return -1;
		}
		else
		/* Add to the list */
			strcat(buf,namebuf);
	}

	strcpy(pszStringOutput,buf);

	free(buf);

	return 0;
}

void Channel_GetModeString(CHANNEL_STRUCT *chChannel, char *pszStringOutput)
/*
** Channel_GetModeString()
** Takes the modes of the channel and creates a string with it
** The string is outputted to the pointer passed in pszStringOutput
*/
{
	int nIndex = 0;
	char buf[256], tmpbuf[256];

	memset(buf,0,sizeof(buf));

	buf[nIndex++] = '+';

	if (Channel_IsPrivate(chChannel))
		buf[nIndex++] = 'p';
	if (Channel_IsSecret(chChannel))
		buf[nIndex++] = 's';
	if (Channel_IsHidden(chChannel))
		buf[nIndex++] = 'h';
	if (Channel_Moderated(chChannel))
		buf[nIndex++] = 'm';
	if (Channel_TopicOp(chChannel))
		buf[nIndex++] = 't';
	if (Channel_InviteOnly(chChannel))
		buf[nIndex++] = 'i';
	if (Channel_AuthOnly(chChannel))
		buf[nIndex++] = 'a';
	if (Channel_NoExtern(chChannel))
		buf[nIndex++] = 'n';
	if (Channel_HasKnock(chChannel))
		buf[nIndex++] = 'u';
	if (Channel_NoWhispers(chChannel))
		buf[nIndex++] = 'w';
	if (Channel_Auditorium(chChannel))
		buf[nIndex++] = 'x';
	if (Channel_IsRegistered(chChannel))
		buf[nIndex++] = 'r';

	if (chChannel->dwLimit > 0)
		buf[nIndex++] = 'l';
	if (Channel_HasMemberkey(chChannel))
		buf[nIndex++] = 'k';

	if (buf[nIndex - 1] == 'l' || buf[nIndex - 1] == 'k')
	/* Continue! */
	{
		buf[nIndex++] = ' ';

		if (chChannel->dwLimit > 0)
		{
			strcat(buf,_ltoa(chChannel->dwLimit,tmpbuf,10));
			
			if (Channel_HasMemberkey(chChannel))
				strcat(buf," ");
		}

		if (Channel_HasMemberkey(chChannel))
			strcat(buf,chChannel->szPropMemberkey);
	}

	/* Return our mode string */
	strcpy(pszStringOutput,buf);
}

DWORD Channel_GetModeFlagFromChar(char cMode)
{
	switch (cMode)
	/* What we got? */
	{
		case 't':
			return CHANNELMODE_TOPICOP;
		break;
		case 'u':
			return CHANNELMODE_KNOCK;
		break;
		case 'w':
			return CHANNELMODE_NOWHISPER;
		break;
		case 'm':
			return CHANNELMODE_MODERATED;
		break;
		case 'n':
			return CHANNELMODE_NOEXTERN;
		break;
		case 'i':
			return CHANNELMODE_INVITE;
		break;
		case 'f':
			return CHANNELMODE_NOFORMAT;
		break;
		case 'x':
			return CHANNELMODE_AUDITORIUM;
		break;
		case 'r':
			return CHANNELMODE_REGISTERED;
		break;
		case 'z':
			return CHANNELMODE_SERVICE;
		break;
		case 'a':
			return CHANNELMODE_AUTHONLY;
		break;
		case 'd':
			return CHANNELMODE_CLONEABLE;
		break;
		case 'e':
			return CHANNELMODE_CLONE;
		break;
		default:
			return 0x00000000;
		break;
	}
}

DWORD Channel_GetModeFlagFromString(const char *szModeString)
/*
** Channel_GetModeFlag()
** Converts and returns the specified mode string into a set of mode flags
*/
{
	DWORD dwRetval = 0;
	int index;

	for (index = 0; szModeString[index]; index++)
	/* Parse our mode list */
		dwRetval |= Channel_GetModeFlagFromChar(szModeString[index]);

	return dwRetval;
}

DWORD Channel_GetUserModeFlags(CHANNEL_STRUCT *chChannel, CLIENT_STRUCT *csUser)
/*
** Channel_GetUserModes()
** Returns mode flag bit field for the specified user, on the specified channel
*/
{
	LINKED_LIST_STRUCT *llUser = &(chChannel->llUserHead);
	LINKED_LIST_STRUCT *llUserMode = &(chChannel->llUserModeHead);

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
			return (DWORD)llUserMode->data;
		}
	}

	/* 0 if user is not in channel */
	return 0;
}

void Channel_KickAllLocal(CHANNEL_STRUCT *chChannel, char *szFrom, char *szReason)
/*
** Channel_KickAllLocal()
** Kicks everyone out of the channel! Only use in channel collision
** as this function WILL NOT remove them from the list! A subsequent call to
** Channel_DeleteAllUsers() should do that!
** REPEAT: THIS FUNCTION WILL NOT DELETE THEM FROM THE LIST!!!! WILL NOT!!!!
*/
{
	LINKED_LIST_STRUCT *llUser = &(chChannel->llUserHead);
	CLIENT_STRUCT *csUser = NULL;

	while (llUser->next)
	/* Go through our list */
	{
		llUser = llUser->next;
		csUser = (CLIENT_STRUCT*)llUser->data;

		if (Client_IsLocal(csUser))
		/* Now the fun begins... :) */
		{
			if (szReason)
			Client_SendToOne(csUser,FALSE,":%s " MSG_KICK " %s %s :%s",
				szFrom,
				chChannel->szName,
				Client_Nickname(csUser),
				szReason);
			else
			Client_SendToOne(csUser,FALSE,":%s " MSG_KICK " %s %s :",
				szFrom,
				chChannel->szName,
				Client_Nickname(csUser));

		}
	}
}

void Channel_Kill(CHANNEL_STRUCT *chChannel, CLIENT_STRUCT *csFrom, char *szReason)
/*
** Channel_Kill()
** Kicks everyone from the specified channel
*/
{
	LINKED_LIST_STRUCT *llUser = &(chChannel->llUserHead);
	CLIENT_STRUCT *csUser = NULL;

	while (chChannel->llUserHead.next)
	/* Go through our list */
	{
		llUser = chChannel->llUserHead.next;
		csUser = (CLIENT_STRUCT*)llUser->data;

		Event_Broadcast(csUser,EVENT_TYPE_MEMBER,"MEMBER PART %s %s!%s@%s %s:%u",chChannel->szName,Client_Nickname(csUser),csUser->user->username,csUser->hostname,csUser->ip_r,csUser->port_r);
		Channel_BroadcastToLocal(chChannel,NULL,NULL,TOK_KICK,MSG_KICK,"%s %s :%s",chChannel->szName,Client_Nickname(csUser),(szReason ? szReason : ""));
		Channel_DeleteUser(chChannel,csUser);
	}

	if (csFrom && Client_IsUser(csFrom) && Client_IsOnChannel(csFrom,chChannel,NULL,NULL))
		Client_SendToOne(csFrom,FALSE,":System " MSG_KICK " %s %s :%s",
			chChannel->szName,
			Client_Nickname(csFrom),
			(szReason ? szReason : ""));

	/* Broadcast the CHANNEL KILL to other servers */
	if (Client_IsUser(csFrom))
		Server_BroadcastFromUser(csFrom,TOK_KILL,MSG_KILL,"%s :%s",chChannel->szName,(szReason ? szReason : ""));
	else
		Server_BroadcastFromUser(csFrom,TOK_KILL,MSG_KILL,"%s :%s",chChannel->szName,(szReason ? szReason : ""));

	Channel_DeleteAllUsers(chChannel);

	Event_Broadcast(csFrom,EVENT_TYPE_CHANNEL,"CHANNEL DESTROY %s",chChannel->szName);
	Channel_Cleanup(chChannel,TRUE);
}

void Parse_ChannelList(const char *szChannelList, LINKED_LIST_STRUCT *llChannelHead)
/*
** Parse_ChannelList()
** Takes a list of channels and puts them in the specified linked list.
** Any channels which do not exist will be placed in the list as the value NULL
*/
{
	CHANNEL_STRUCT *chChannel = NULL;

	int count, tokens = numtok(szChannelList,',');
	char chbuf[512];

	llChannelHead->next = NULL; llChannelHead->data = NULL;

	for (count = 1; count <= tokens; count++)
	/* Loop through the channels seperated by commas */
	{
		chChannel = Channel_Find(gettok(chbuf,szChannelList,count,','));

		if (chChannel == NULL)
		/* Add our NULL entry */
            LL_AddNoCheck(llChannelHead,(void*)NULL);
		else
		/* Add our channel, but we don't want duplicate entries */
			LL_Add(llChannelHead,(void*)chChannel);
	}
}