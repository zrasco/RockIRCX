/*
** messages.cpp
**
** This file handles all message processing for the server
*/

#include "messages.h"

/* Create message table & lookup table */
Message *MessageMap[MESSAGEMAP_LIMIT];
Message MessageTable[] = 
{
	{ MSG_PRIVATE, MP_Privmsg, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_PRIVATE },
	{ MSG_WHO, MP_Who, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_WHO },
	{ MSG_WHOIS, MP_Whois, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_WHOIS },
	{ MSG_WHOWAS, MP_Whowas, MSGPARAM_INFINITE, MSGFLAG_DELAY | MSGFLAG_COMMANDNOTSUPPORTED, TOK_WHOWAS },
	{ MSG_USER, MP_User, MSGPARAM_INFINITE, MSGFLAG_DELAY | MSGFLAG_REGISTERCOMMAND, TOK_USER },
	{ MSG_NICK, MP_Nick, MSGPARAM_INFINITE, MSGFLAG_DELAY | MSGFLAG_REGISTERCOMMAND, TOK_NICK },
	{ MSG_LIST, MP_List, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_LIST },
	{ MSG_LISTX, MP_Listx, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_LISTX },
	{ MSG_TOPIC, MP_Topic, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_TOPIC },
	{ MSG_INVITE, MP_Invite, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_INVITE },
	{ MSG_VERSION, MP_Version, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_VERSION },
	{ MSG_QUIT, MP_Quit, MSGPARAM_INFINITE, MSGFLAG_REGISTERCOMMAND, TOK_QUIT },
	{ MSG_KILL, MP_Kill, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_KILL },
	{ MSG_INFO, MP_Info, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_INFO },
	{ MSG_LINKS, MP_Links, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_LINKS },
	{ MSG_STATS, MP_Stats, MSGPARAM_INFINITE, MSGFLAG_DELAY | MSGFLAG_COMMANDNOTSUPPORTED, TOK_STATS },
	{ MSG_USERS, MP_Users, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_USERS },
	{ MSG_AWAY, MP_Away, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_AWAY },
	{ MSG_PING, MP_Ping, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_PING},
	{ MSG_PONG, MP_Pong, MSGPARAM_INFINITE, MSGFLAG_DELAY | MSGFLAG_NOIDLERESET, TOK_PONG },
	{ MSG_OPER, MP_Oper, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_OPER },
	{ MSG_PASS, MP_Pass, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_PASS },
	{ MSG_TIME, MP_Time, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_TIME },
	{ MSG_NAMES, MP_Names, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_NAMES },
	{ MSG_ADMIN, MP_Admin, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_ADMIN },
	{ MSG_NOTICE, MP_Notice, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_NOTICE },
	{ MSG_JOIN, MP_Join, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_JOIN },
	{ MSG_CREATE, MP_Create, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_CREATE },
	{ MSG_ACCESS, MP_Access, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_ACCESS },
	{ MSG_PROP, MP_Prop, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_PROP },
	{ MSG_PART, MP_Part, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_PART },
	{ MSG_LUSERS, MP_Lusers, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_LUSERS },
	{ MSG_WHISPER, MP_Whisper, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_WHISPER },
	{ MSG_IRCX, MP_Ircx, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_IRCX },
	{ MSG_ISIRCX, MP_IsIrcx, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_ISIRCX },
	{ MSG_MOTD, MP_MOTD, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_MOTD },
	{ MSG_MODE, MP_Mode, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_MODE },
	{ MSG_KICK, MP_Kick, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_KICK },
	{ MSG_USERHOST, MP_Userhost, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_USERHOST },
	{ MSG_ISON, MP_Ison, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_ISON },
	{ MSG_SILENCE, MP_Silence, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_SILENCE },
	{ MSG_TRACE, MP_Trace, MSGPARAM_INFINITE, MSGFLAG_DELAY | MSGFLAG_COMMANDNOTSUPPORTED, TOK_TRACE },
	{ MSG_EVENT, MP_Event, MSGPARAM_INFINITE, MSGFLAG_DELAY, TOK_EVENT },
	{ MSG_SERVER, MP_Server, MSGPARAM_INFINITE, MSGFLAG_SERVERCOMMAND, TOK_SERVER },
	{ MSG_SPASS, MP_SPass, MSGPARAM_INFINITE, MSGFLAG_SERVERCOMMAND | MSGFLAG_REGISTERCOMMAND, TOK_SPASS },
	{ MSG_NJOIN, MP_NJoin, MSGPARAM_INFINITE, MSGFLAG_SERVERCOMMAND, TOK_NJOIN },
	{ MSG_SERVERSYNC, MP_Serversync, 1, MSGFLAG_SERVERCOMMAND, TOK_SERVERSYNC },
	{ MSG_USERDATA, MP_Userdata, MSGPARAM_INFINITE, MSGFLAG_SERVERCOMMAND, TOK_USERDATA },
	{ MSG_KNOCK, MP_Knock, MSGPARAM_INFINITE, MSGFLAG_SERVERCOMMAND, TOK_KNOCK },
	{ MSG_MESH, MP_Mesh, MSGPARAM_INFINITE, MSGFLAG_SERVERCOMMAND, TOK_MESH },

#ifdef Z_D
	{ MSG_DEBUG, MP_Debug, 1,MSGFLAG_DELAY,TOK_DEBUG },
#endif		/* Z_D */

	{ NULL, (int (*)()) 0, 0, 0, { 0, 0} }
};


void Message_Initialize()
/*
** Message_Initialize()
** This function initializes all message processing
**
** 1) It initializes the message map in order to speed up processing of server-server
** messages.
*/
{
	int count;
	int zeroid;

	/* Find last entry and fill in message map with it */
	for (zeroid = 0; MessageTable[zeroid].command; zeroid++);

	for (count = 0; count < MESSAGEMAP_LIMIT; count++)
		MessageMap[count] = &MessageTable[zeroid];

	/* Create lookup table indexed by message table tokens */
	for (count = 0; MessageTable[count].command; count++)
		MessageMap[MessageTable[count].token[0]] = &MessageTable[count];
}

int	Message_Execute()
/*
** Message_Execute()
** We have a message waiting to be executed, so we'll do it here
*/
{
	int retval, count, mmindex = -1;
	CLIENT_STRUCT *client = scs.msginfo.c_from;

	if (Client_IsUser(client))
	/* 
	** Parse the message via string comparison, usually in the form of:
	** <command> <parameters>
	** But may also be in the form:
	** :prefix <command> <parameters>
	** NOTE: Although RFC 2518 says the only valid prefix coming from a user
	** is the users nickname, exchange appears to ignore it. Therefore, so will we.
	*/
	{
		if (scs.msginfo.argv[0][0] == ':')
		/* Prefix found in message, so lets pretend it doesnt exist */
		{
			if (scs.msginfo.argc == 1)
			/* We can ignore this message */
				return 0;

			for (count = 0; count < scs.msginfo.argc; count++)
			/* Shift all parameters to the left */
				scs.msginfo.argv[count] = scs.msginfo.argv[count + 1];

			scs.msginfo.argc--;

		}

		for (count = 0; MessageTable[count].command; count++)
		{
			if (!_stricmp(scs.msginfo.argv[0],MessageTable[count].command))
			/* Found matching command */
			{
				mmindex = MessageTable[count].token[0];
				break;
			}
		}
	}
	else if (Client_IsServer(client))
	/*
	** Parse the message via the message map.
	** Server messages are in the form:
	** :prefix <command token> <parameters>
	** NOTE: Server commands MUST be sent in tokenized form! And... we trust our servers!
	** Therefore we will assume that if we are here, argv[0] is the prefix, and argv[1] is the command
	*/
	{
		CLIENT_STRUCT *c_prefix = NULL;

		char *prefix = &scs.msginfo.argv[0][1];

		if (!scs.msginfo.argv[1][1])
		/* Sent in tokenized form */
			mmindex = scs.msginfo.argv[1][0];
		else
		/* Server sent a string */
		{
			for (count = 0; MessageTable[count].command; count++)
			{
				if (!_stricmp(scs.msginfo.argv[1],MessageTable[count].command))
				/* Found matching command */
					mmindex = MessageTable[count].token[0];
			}
		}
		
		/* Process the prefix, which is either a server token or a nickname */

		if (isstrdigit(prefix))
		/* Prefix is possibly an id */
		{
			unsigned int id = atoi(prefix);

			c_prefix = scs.CSServers[id];

			if (id >= 0 && id < MAX_SERVERTOKENS &&
				c_prefix)
			/* The token is valid */
			{
				scs.msginfo.c_prefixfrom = c_prefix;
			}
			else
			/* Invalid id, silently drop(per RFC 2813). */
				return 0;
		}
		else
		{
			if (!strcmpi(prefix,client->server->name))
			/* A prefix of the server name MAY be sent ONLY by that server! */
			{
				scs.msginfo.c_prefixfrom = client;
			}
			else
			{
				char nickname[256];

				/* Use the nickname only for the prefix lookup */
				gettok(nickname,prefix,1,33);

				c_prefix = User_Find(nickname);

				if (!c_prefix)
				/* Invalid prefix, silently drop(per RFC 2813) */
					return 0;
				else
					scs.msginfo.c_prefixfrom = c_prefix;
			}
		}

		for (count = 0; count < scs.msginfo.argc; count++)
		/* Shift all parameters to the left */
			scs.msginfo.argv[count] = scs.msginfo.argv[count + 1];

		scs.msginfo.argc--;
	}

	if (mmindex == -1)
	/* Command not found, but give em a penalty anyways! */
	{
		Client_SendNumericToOne(client,ERR_UNKNOWNCOMMAND,scs.msginfo.argv[0]);
		client->parsemsg += SettingsInfo.isUser.msgdelay;
	}
	else
	/* Command found, check to see if it can be used */
	{
		if ((!(MessageMap[mmindex]->flag & MSGFLAG_REGISTERCOMMAND)) &&
			client->state == CLIENT_STATE_REGISTERING &&
			!Client_IsServer(client))
		/* Not a registration-related command, and client is not yet registered */
			Client_SendNumericToOne(client,ERR_NOTREGISTERED,NULL);
		else if ((MessageMap[mmindex]->flag & MSGFLAG_SERVERCOMMAND) &&
			(!(MessageMap[mmindex]->flag & MSGFLAG_REGISTERCOMMAND )) &&
			!Client_IsServer(client))
			Client_SendNumericToOne(client,ERR_UNKNOWNCOMMAND,scs.msginfo.argv[0]);
		else if (MessageMap[mmindex]->flag & MSGFLAG_COMMANDNOTSUPPORTED)
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :Command not supported.",SettingsInfo.isGeneral.servername,554,Client_Nickname(client),scs.msginfo.argv[0]);
		else
		/* Call function to process message(MP_Privmsg, etc...) */
		{
			/* Track idle time here, resets after every command except PONG */
			if (!(MessageMap[mmindex]->flag & MSGFLAG_NOIDLERESET))
			{
				client->idlemsg = GetTickCount();
			}
			/* TODO: Check all access entries and adjust timeouts */

			retval = (MessageMap[mmindex]->function());

			if (MessageMap[mmindex]->flag & MSGFLAG_DELAY &&
				(!(client->flags & CLIENT_FLAG_ADMIN)) && (!(client->flags & CLIENT_FLAG_SYSOP)) &&
				(!(client->flags & CLIENT_FLAG_POWERUSER)))
			/* Add penalty for messages for regular users  */
				client->parsemsg += SettingsInfo.isUser.msgdelay;

			return retval;
		}
	}

	return 0;
}


int MP_Privmsg()
/*
** MP_Privmsg()
** This is called whenever a user messages a destination
** NOTE: We may also get here via MP_Notice(), so we gotta watch for that!
*/
{

	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CLIENT_STRUCT *client = mi->c_prefixfrom;
	char tok_touse[2], msg_touse[32];
	BOOL bNotice = FALSE;
	int retval;

	/* Quick check if its a notice or privmsg */
	if (mi->argv[0][0] == 'n' || mi->argv[0][0] == 'N' ||
		mi->argv[0][0] == TOK_NOTICE[0])
	/* Notice */
	{
		strcpy(tok_touse,TOK_NOTICE);
		strcpy(msg_touse,MSG_NOTICE);
		bNotice = TRUE;
	}
	else
	/* Privmsg */
	{
		strcpy(tok_touse,TOK_PRIVATE);
		strcpy(msg_touse,MSG_PRIVATE);
	}

	if (mi->argc == 1)
	/* No recipient given */
		Client_SendNumericToOne(client,ERR_NORECIPIENT,mi->argv[0]);
	else if (mi->argc == 2)
	/* No text to send */
		Client_SendNumericToOne(client,ERR_NOTEXTTOSEND,mi->argv[0]);
	else
	/* Begin parsing of targets */
	{
		char *target = mi->argv[1];
		CLIENT_STRUCT *ctarget;
		LINKED_LIST_STRUCT llDestinationList, *llPtr = &llDestinationList;
		DESTINATION_INFO_STRUCT *dis;

		llDestinationList.data = NULL;
		llDestinationList.next = NULL;

		Parse_DestinationList(target,&llDestinationList);

		while (llPtr->next)
		{
			llPtr = llPtr->next;

			dis = (DESTINATION_INFO_STRUCT*)llPtr->data;

			ctarget = dis->csUser;

			if (ctarget)
			/* Is the target a nickname? */
			{
				ACCESS_STRUCT *accInfo = NULL;
				retval = Client_IsOnAccessList(client,NULL,&ctarget->llAccessHead);

				if (( !Client_IsGagged(client) || client == ctarget) && !(retval != -1 && ((ACCESS_STRUCT*)retval)->dwLevel == ACCESSLEVEL_DENY))
				/* Do not send the message, they are on the target user's deny/silence list */
					Client_SendMessage(ctarget,client,tok_touse,msg_touse,"%s :%s",ctarget->user->nickname,mi->argv[2]);

				/* Send away message for privmsg's only (not notices) */
				if (Client_IsAway(ctarget) && bNotice == FALSE)
					Client_SendToOne(client,FALSE,":%s %3.3d %s %s :%s",SettingsInfo.isGeneral.servername,RPL_AWAY,Client_Nickname(client),Client_Nickname(ctarget),ctarget->user->away);
			}
			else
			/* Not a nickname, see if its a channel */
			{
				CHANNEL_STRUCT *chChannel = dis->chChannel;

				if (chChannel && !Channel_ReggedAndEmpty(chChannel))
				/* Destination is a channel */
				{
					DWORD dwPriveledge = 0;

					if ((!Client_IsOnChannel(client,chChannel,&dwPriveledge,NULL)) &&
						(Channel_NoExtern(chChannel) || Channel_Moderated(chChannel)))
					/* User is outside of channel, cant send message due to +m or +n mode */
						Client_SendNumericToOne(client,ERR_CANNOTSENDTOCHAN,dis->szOriginalText);
					else
					/* User should be able to message the channel, unless... */
					{
						if (Channel_Moderated(chChannel) &&
							(dwPriveledge == CHANNEL_PRIVELEDGE_NONE))
						/* Can't send message due to +m mode */
							Client_SendNumericToOne(client,ERR_CANNOTSENDTOCHAN,dis->szOriginalText);
						else if (!Client_IsGagged(client) || client == ctarget)
						/* Just message the thing already! */
						{
							Channel_BroadcastToLocal(chChannel,client,client,tok_touse,msg_touse,"%s :%s",chChannel->szName,mi->argv[2]);
							Server_BroadcastFromUser(client,tok_touse,msg_touse,"%s :%s",chChannel->szName,mi->argv[2]);
						}
					}
				}
				else
				/* No such nick/channel */
					Client_SendNumericToOne(client,ERR_NOSUCHNICK,dis->szOriginalText);
			}
		}

		LL_ClearAndFree(&llDestinationList);
	}

	return 0;
}
int MP_Who()
/*
** MP_Who()
** Executes the WHO command
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;	
	CLIENT_STRUCT *client = mi->c_from, *c_target = NULL;
	LINKED_LIST_STRUCT llDestinationHead, *llNode = &llDestinationHead, *llReturnMode = NULL, *llReturnNode = NULL,
		*llUserPtr = NULL, *llUserModePtr = NULL, llLocalChannelHead, *llChannelPtr = NULL;
	DESTINATION_INFO_STRUCT *disInfo = NULL;
	BOOL bOnChannel = FALSE, bOpers = FALSE;
	DWORD dwPriveledge = 0;
	char szLocal[256], *szPtr, szTemp[256];
	int nIndex = 0, nCount = 0;

	llDestinationHead.data = NULL;
	llDestinationHead.next = NULL;

	if (mi->argc < 2)
	/* Default to * as second parameter */
	{
		szLocal[0] = '*'; szLocal[1] = 0;
		mi->argv[1] = szLocal;
		mi->argc = 2;
	}
	else if (mi->argc > 2)
	/* if 'o' is specified, display opers */
	{
		if (mi->argv[2][0] == 'o')
			bOpers = TRUE;
	}

	Parse_DestinationList(mi->argv[1],&llDestinationHead);

	while (llNode->next)
	{
		llNode = llNode->next;

		disInfo = (DESTINATION_INFO_STRUCT*)(llNode->data);

		if (disInfo->chChannel)
		/* List users in the channel */
		{
			bOnChannel = Client_IsOnChannel(client,disInfo->chChannel,&dwPriveledge,&llReturnMode);

			if (bOnChannel || Channel_IsPublic(disInfo->chChannel) || Client_IsPriveledged(client))
			{
				/* Output the list of users from the channel. Determine whether to mask IP for each user */
				llUserPtr = &(disInfo->chChannel->llUserHead);
				llUserModePtr = &(disInfo->chChannel->llUserModeHead);

				while (llUserPtr->next)
				{
					llUserPtr = llUserPtr->next;
					llUserModePtr = llUserModePtr->next;

					c_target = (CLIENT_STRUCT*)(llUserPtr->data);

					if (User_DoWeMask(c_target,client))
						szPtr = c_target->hostmask;
					else
						szPtr = c_target->hostname;

					/*
					** Send /WHO reply
					**
					** Example:
					** :node10 352 test #tesT Zeb ip.addr.is.here node10 Nickname Ha@ :0 FullName Ishere
					*/

					/* Construct the "Ha@" string */
					nIndex = 0;

					szTemp[nIndex++] = Client_IsAway(c_target) ? 'G' : 'H';

					if (Client_IsAdmin(c_target))
						szTemp[nIndex++] = 'a';
					else if (Client_IsSysop(c_target))
						szTemp[nIndex++] = '*';

					if ((DWORD)llUserModePtr->data & CHANNEL_PRIVELEDGE_OWNER)
						szTemp[nIndex] = '.';
					else if ((DWORD)llUserModePtr->data & CHANNEL_PRIVELEDGE_HOST)
						szTemp[nIndex] = '@';
					else if ((DWORD)llUserModePtr->data & CHANNEL_PRIVELEDGE_VOICE)
						szTemp[nIndex] = '+';
					else
						szTemp[nIndex] = 0;

					szTemp[nIndex + 1] = 0;

					if (bOpers == FALSE || (bOpers == TRUE && Client_IsPriveledged(c_target))	)					
						Client_SendToOne(client,FALSE,":%s %d %s %s %s %s %s %s %s :%d %s",
						scs.sclient->server->name,
						RPL_WHOREPLY,
						Client_Nickname(client),
						disInfo->szOriginalText,
						c_target->user->username,
						szPtr,
						c_target->servername,
						Client_Nickname(c_target),
						szTemp,
						c_target->hops,
						c_target->user->fullname);
				}

			}

		}
		else
		/* Search for users */
		{
			if (strchr(disInfo->szOriginalText,'*') || strchr(disInfo->szOriginalText,'?'))
			/* Wildcard */
			{
				for (nCount = 0; nCount <= MAX_HOPS; nCount++)
				{
					LINKED_LIST_STRUCT *llUserPtr = &scs.llUserHead[nCount];
					BOOL bCommonChan = FALSE;

					while (llUserPtr->next)
					{
						llUserPtr = llUserPtr->next;

						c_target = (CLIENT_STRUCT*)(llUserPtr->data);

						if (match(disInfo->szOriginalText,c_target->hostmask) == 0 ||
							match(disInfo->szOriginalText,c_target->servername) == 0 ||
							match(disInfo->szOriginalText,c_target->user->username) == 0 ||
							match(disInfo->szOriginalText,c_target->user->fullname) == 0 ||
							match(disInfo->szOriginalText,Client_Nickname(c_target)) == 0)
						/* Match! */
						{
							llLocalChannelHead.data = NULL;
							llLocalChannelHead.next = NULL;

							bCommonChan = User_GetCommonChannels(client,c_target,NULL,&llLocalChannelHead);

							if (Client_IsPriveledged(client) ||
								!Client_Invisible(c_target) ||
								bCommonChan ||
								client == c_target)
							/* Display WHO info */
							{
								CHANNEL_STRUCT *chChannel = NULL;
								char szChannelString[256];

								strcpy(szChannelString,"*");

								if (Client_IsPriveledged(client) && Client_InAChannel(c_target))
								/* Display first channel */
								{
									chChannel = (CHANNEL_STRUCT*)(c_target->user->llChannelHead.next->data);
									strcpy(szChannelString,chChannel->szName);
								}

								else if (bCommonChan)
								/* Display first common channel */
								{
									chChannel = ((CHANNEL_STRUCT*)(llLocalChannelHead.next->data));
									strcpy(szChannelString,chChannel->szName);
								}
								else
								/* Display first public channel */
								{
									llChannelPtr = &(c_target->user->llChannelHead);

									while (llChannelPtr->next)
									{
										llChannelPtr = llChannelPtr->next;

										chChannel = (CHANNEL_STRUCT*)(llChannelPtr->data);

										if (Channel_IsPublic(((CHANNEL_STRUCT*)llChannelPtr->data)))
										{
											strcpy(szChannelString,((CHANNEL_STRUCT*)llChannelPtr->data)->szName);
											break;
										}
									}
								}

								/* Mask hostmask */
								if (User_DoWeMask(c_target,client))
									szPtr = c_target->hostmask;
								else
									szPtr = c_target->hostname;

								/* Construct the "Ha@" string */
								nIndex = 0;

								szTemp[nIndex++] = Client_IsAway(c_target) ? 'G' : 'H';

								if (Client_IsAdmin(c_target))
									szTemp[nIndex++] = 'a';
								else if (Client_IsSysop(c_target))
									szTemp[nIndex++] = '*';

								szTemp[nIndex] = 0;

								if (chChannel != NULL)
								{
									dwPriveledge = 0;
									Client_IsOnChannel(c_target,chChannel,&dwPriveledge,NULL);

									if (dwPriveledge & CHANNEL_PRIVELEDGE_OWNER)
										szTemp[nIndex] = '.';
									else if (dwPriveledge & CHANNEL_PRIVELEDGE_HOST)
										szTemp[nIndex] = '@';
									else if (dwPriveledge & CHANNEL_PRIVELEDGE_VOICE)
										szTemp[nIndex] = '+';
									else
										szTemp[nIndex] = 0;
								}

								szTemp[nIndex + 1] = 0;

								if (bOpers == FALSE || (bOpers == TRUE && Client_IsPriveledged(c_target)))
									Client_SendToOne(client,FALSE,":%s %d %s %s %s %s %s %s %s :%d %s",
										scs.sclient->server->name,
										RPL_WHOREPLY,
										Client_Nickname(client),
										szChannelString,
										c_target->user->username,
										szPtr,
										c_target->servername,
										Client_Nickname(c_target),
										szTemp,
										c_target->hops,
										c_target->user->fullname);
							}

							LL_Clear(&llLocalChannelHead);
						}
					}
				}
			}
		}

		Client_SendToOne(client,FALSE,":%s %d %s %s :End of /WHO list",
			scs.sclient->server->name,
			RPL_ENDOFWHO,
			Client_Nickname(client),
			disInfo->szOriginalText);
	}

	LL_ClearAndFree(&llDestinationHead);

	return 0;
}
int MP_Whois()
/*
** MP_Whois()
** Called whenever a user requests information on another user
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;	
	CLIENT_STRUCT *c_target = NULL, *client = mi->c_from;
	char *nickname = mi->argv[1];
	char linebuf[512], sendbuf[4096];

	sendbuf[0] = 0;

    if (mi->argc < 2)
	/* Not enough parameters */
		Client_SendNumericToOne(mi->c_from,ERR_NONICKNAMEGIVEN,NULL);
	else
	/* TODO: Make accept wildcards & commas */
	{
		if (c_target = User_Find(nickname))
		/* Found 'em! */
		{
			char chbuf[1024];

			/*
			** Only display /whois information if one of the following conditions are met:
			** 1) Requesting user has at least 1 common channel with target user
			** 2) Requesting user is an admin or sysop
			** 3) Target party does not have usermode +i (invisible) set
			*/

			if (c_target == client || (!Client_Invisible(c_target)) || Client_IsPriveledged(client) || User_GetCommonChannels(client,c_target,NULL,NULL))
			{
				/* Donkey is ~ewtert 63.129.150.15 * [486] Advanced */
				if (User_DoWeMask(c_target,client))
					sprintf(linebuf,":%s %d %s %s ~%s %s * :%s" CRLF,
						scs.sclient->server->name,
						RPL_WHOISUSER,
						mi->c_from->user->nickname,
						c_target->user->nickname,
						c_target->user->username,
						c_target->hostmask,
						c_target->user->fullname);
				else
					sprintf(linebuf,":%s %d %s %s ~%s %s * :%s" CRLF,
						scs.sclient->server->name,
						RPL_WHOISUSER,
						mi->c_from->user->nickname,
						c_target->user->nickname,
						c_target->user->username,
						c_target->hostname,
						c_target->user->fullname);


				strcat(sendbuf,linebuf);

				/* Donkey is on +#30s1 .#Cmp3 .#root @#TopPops */
				if (Client_GetWhoisChannels(mi->c_from,c_target,chbuf) != -1)
				{
					sprintf(linebuf,":%s %d %s %s :%s" CRLF,
						scs.sclient->server->name,
						RPL_WHOISCHANNELS,
						mi->c_from->user->nickname,
						c_target->user->nickname,
						chbuf);
					strcat(sendbuf,linebuf);
				}

				/* Donkey is on RockIRCX-1 RockIRCX Server #1 */
				sprintf(linebuf,":%s %d %s %s %s :%s" CRLF,
					scs.sclient->server->name,
					RPL_WHOISSERVER,
					mi->c_from->user->nickname,
					c_target->user->nickname,
					c_target->servername,
					Server_Description(Client_GetServer(c_target)));
				strcat(sendbuf,linebuf);

				/* Donkey is away: writing code */
				if (Client_IsAway(c_target))
				{
					sprintf(linebuf,":%s %d %s %s :%s" CRLF,
						scs.sclient->server->name,
						RPL_AWAY,
						mi->c_from->user->nickname,
						c_target->user->nickname,
						c_target->user->away);
					strcat(sendbuf,linebuf);
				}

				/* Donkey is an IRC Administrator */
				if (Client_IsAdmin(c_target) || Client_IsSysop(c_target))
				{
					sprintf(linebuf,":%s %d %s %s :is an IRC %s" CRLF,
						scs.sclient->server->name,
						RPL_WHOISOPERATOR,
						mi->c_from->user->nickname,
						c_target->user->nickname,
						Client_IsAdmin(c_target) ? "Administrator" : "Operator");
					strcat(sendbuf,linebuf);
				}

				/* Donkey signed on Sun Nov 06 11:58:10 2005 and has been idle for 5secs */
				if (Client_IsLocal(c_target))
				{
					sprintf(linebuf,":%s %d %s %s %d %u :seconds idle, signon time" CRLF,
						scs.sclient->server->name,
						RPL_WHOISIDLE,
						mi->c_from->user->nickname,
						c_target->user->nickname,
						Client_GetIdle(c_target),
						c_target->signon);
					strcat(sendbuf,linebuf);
				}

				/* Donkey from IP 127.0.0.1 */
				if (Client_IsAdmin(mi->c_from) || Client_IsSysop(mi->c_from))
				{
					sprintf(linebuf,":%s %d %s %s :from IP %s" CRLF,
						scs.sclient->server->name,
						RPL_WHOISSPECIAL,
						mi->c_from->user->nickname,
						c_target->user->nickname,
						IPFromLong(c_target->ip));
					strcat(sendbuf,linebuf);
				}
			}
		}
		else
		/* User not found */
		{
			sprintf(linebuf, ":%s %3.3d %s %s :No such nick/channel" CRLF,scs.sclient->server->name,ERR_NOSUCHNICK,mi->c_from->user->nickname,nickname);
			strcat(sendbuf,linebuf);
		}

		/* End of /Whois for Donkey */
		sprintf(linebuf,":%s %d %s %s :End of /WHOIS list" CRLF,
			scs.sclient->server->name,
			RPL_ENDOFWHOIS,
			mi->c_from->user->nickname,
			nickname);

		strcat(sendbuf,linebuf);
		Client_SendToOne(mi->c_from,FALSE,sendbuf);
	}
	return 0;
}
int MP_Whowas() { return 0; }
int MP_User()
/*
** MP_User()
** Called during initial user registration, sets the user's ident & fullname
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;

	if (mi->argc < 5)
		Client_SendNumericToOne(mi->c_from,ERR_NEEDMOREPARAMS,mi->argv[0]);
	else
	{
		CLIENT_STRUCT *client = mi->c_prefixfrom;

		if (!Client_Registered(client))
		/* Client not registered */
		{
			strcpy(client->user->username,mi->argv[1]);
			strcpy(client->user->fullname,mi->argv[4]);

			Valid_Username(client->user->username,TRUE);

			if (client->user->nickname[0])
			/* User is ready for welcome message */
			{
				client->user->hashkey = Hash_Add(&HT_User,client->user->nickname,client);
				LL_Add(&scs.llUserHead[client->hops],client);

				client->state = CLIENT_STATE_WELCOMING;

				if (!Client_CheckForBan(client))
				/* User is banned! STOP PARSING */
					User_Initiate(client);

				return STOP_PARSING;
			}
		}
		else
		/* Client already registered */
			Client_SendNumericToOne(client,ERR_ALREADYREGISTRED,NULL);
	}
	return 0;
}

int MP_Nick()
/*
** MP_Nick()
** Called whenever the "NICK" command is used by a local client
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	int nLen = strlen(mi->argv[1]);
	BOOL bFiltered = FALSE;

	if (mi->argc < 2)
		Client_SendNumericToOne(mi->c_from,ERR_NEEDMOREPARAMS,mi->argv[0]);
	else
	{
		CLIENT_STRUCT *client = mi->c_prefixfrom;
		CLIENT_STRUCT *target = NULL, *c_collision;

		if (Client_IsServer(mi->c_from))
		{
			/* Check for nickname collision */

			if (c_collision = User_Find(mi->argv[1]))
			/* If we encounter a nickname collision, the nick with the older signon time wins */
			{
				if (client->signon < c_collision->signon)
				/* Force nickname change of incoming user */
					User_ForceNickChange(client,NULL,TRUE);
				else
				/* Force nickname change of local user */
					User_ForceNickChange(c_collision,NULL,TRUE);

				return 0;
			}
		}

		FilterList *flptr;

		/* Check through list of disallowed nicks */
		if (SettingsInfo.isFilter.enabled)
		{
			flptr = &SettingsInfo.isFilter.filterhead;

			while (flptr->next)
			{
				flptr = flptr->next;

				if ((flptr->type == FILTER_TYPE_NICKNAME || flptr->type == FILTER_TYPE_ALL) && match(flptr->word,mi->argv[1]) == 0)
				{
					bFiltered = TRUE;
					break;
				}
			}
		}

		if (!Valid_Nickname(mi->argv[1]) || (SettingsInfo.isSecurity.max_nicklen && (nLen > SettingsInfo.isSecurity.max_nicklen)) )
		/* Nickname specified is invalid */
			Client_SendNumericToOne(mi->c_from,ERR_ERRONEUSNICKNAME,mi->argv[1]);
		else if ((!Client_IsPriveledged(client) && bFiltered) || ((target = User_Find(mi->argv[1])) && target != client))
		/* Nickname is already in use */
			Client_SendNumericToOne(mi->c_from,ERR_NICKNAMEINUSE,mi->argv[1]);
		else if (!Client_Registered(client))
		/* Client isn't registered, so just copy over a new nickname */
		{
			Hash_Delete(&HT_User,client->user->nickname,client);

			strcpy(client->user->nickname,mi->argv[1]);

			if (client->user->username[0])
			/* Client is ready for welcoming state */
			{
				client->user->hashkey = Hash_Add(&HT_User,client->user->nickname,client);
				LL_Add(&scs.llUserHead[client->hops],client);

				client->state = CLIENT_STATE_WELCOMING;

				if (!Client_CheckForBan(client))
				/* User is banned! STOP PARSING */
					User_Initiate(client);

				return STOP_PARSING;
			}
		}
		else if (strcmp(client->user->nickname,mi->argv[1]) != 0)
		/* Registered client requesting a nickname change, however the nickname must be different else silently ignore the request */
		{
			time_t currtime = GetTickCount();
			int delay = (currtime - client->user->lastnick) / 1000;

			if (!Client_IsPriveledged(client) && client->user->llChannelHead.next &&
				SettingsInfo.isSecurity.preventnickchginchan)
			/* Nick changes not allowed in channel? Send generic "nick in use" error */
				Client_SendNumericToOne(mi->c_from,ERR_NICKNAMEINUSE,mi->argv[1]);
			else if (!Client_IsPriveledged(client) && client->user->lastnick && (delay < SettingsInfo.isUser.nickdelay))
			/* Nickname delay */
				Client_SendToOne(client,FALSE,":%s %3.3d %s %s :Nick change too fast. Please wait %d seconds.",SettingsInfo.isGeneral.servername,ERR_NICKTOOFAST,Client_Nickname(client),mi->argv[1],SettingsInfo.isUser.nickdelay - delay);
			else
				User_ForceNickChange(client,mi->argv[1],FALSE);
		}
	}
	return 0;
}
int MP_List()
/*
** MP_List()
** LIST returns a list of all channels within the specified parameters
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;

	/* Check parameters */
	Client_SendList(mi->c_from,FALSE,mi->argv[1]);

	return 0; 
}
int MP_Listx()
/*
** MP_ListX()
** LISTX returns a list of all channels within the specified parameters (IRCX-specific command)
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	char szListXString[2048];
	int nCount;

	/* Initialize the string */
	szListXString[0] = 0;

	/* Add all parameters to list */
	for (nCount = 1; nCount < mi->argc; nCount++)
	{
		strcat(szListXString,mi->argv[nCount]);
		if (nCount != mi->argc+1)
			strcat(szListXString," ");
	}

	Client_SendList(mi->c_from,TRUE,szListXString);

	return 0;
}
int MP_Topic()
/*
** MP_Topic
** Allows the user to change the topic of a channel. Only ops can set topic if +t is set.
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CHANNEL_STRUCT *chTarget = NULL;
	CLIENT_STRUCT *client = mi->c_prefixfrom;
	DWORD dwUserModeFlags = 0;

	if (mi->argc < 2)
	/* Not enough parameters */
	{
		Client_SendNumericToOne(client,ERR_NEEDMOREPARAMS,mi->argv[0]);
		return 0;
	}

	chTarget = Channel_Find(mi->argv[1]);

	if (chTarget == NULL || Channel_ReggedAndEmpty(chTarget))
	/* Channel not found */
	{
		Client_SendNumericToOne(client,ERR_NOSUCHCHANNEL,mi->argv[1]);
		return 0;
	}

	if (mi->argc == 2)
	/* Display topic */
	{
		BOOL bDisplay = FALSE;

		if (Client_IsPriveledged(client))
			bDisplay = TRUE;
		else if (Channel_IsPublic(chTarget) || Channel_IsHidden(chTarget))
			bDisplay = TRUE;
		else if (Client_IsOnChannel(client,chTarget,NULL,NULL))
			bDisplay = TRUE;
		
		if (!Channel_HasTopic(chTarget))
			bDisplay = FALSE;

		if (bDisplay)
		/* Show topic */
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :%s",SettingsInfo.isGeneral.servername,332,Client_Nickname(client),mi->argv[1],chTarget->szPropTopic);
		else
		/* No topic is set */
			Client_SendNumericToOne(client,RPL_NOTOPIC,chTarget->szName);

		return 0;
	}

	/* Check if channel mode is +t and ops status */
	if ((chTarget->dwModes & CHANNELMODE_TOPICOP) && 
		(!(Channel_GetUserModeFlags(chTarget,client) & CHANNEL_PRIVELEDGE_OWNER) &&
		!(Channel_GetUserModeFlags(chTarget,client) & CHANNEL_PRIVELEDGE_HOST)))
	/* Cannot set topic */
		Client_SendNumericToOne(client,ERR_CHANOPRIVSNEEDED,mi->argv[1]);
	else
	/* Set channel topic */
	{
		FilterList *flptr;
		BOOL bFiltered = FALSE;

		/* Check through list of disallowed channel names */
		if (SettingsInfo.isFilter.enabled)
		{
			flptr = &SettingsInfo.isFilter.filterhead;

			while (flptr->next)
			{
				flptr = flptr->next;

				if ((flptr->type == FILTER_TYPE_TOPIC || flptr->type == FILTER_TYPE_ALL) && match(flptr->word,mi->argv[2]) == 0)
				{
					bFiltered = TRUE;
					break;
				}
			}
		}

		if ((!Client_IsPriveledged(client) && bFiltered))
			Client_SendNumericToOne(client,RPL_NOTOPIC,chTarget->szName);
		else
		{
			int nLen = strlen(mi->argv[2]);

			if (SettingsInfo.isSecurity.max_topiclen && (nLen > SettingsInfo.isSecurity.max_topiclen))
			/* Truncate topic length */
				mi->argv[2][nLen] = 0;
				
			strcpy(chTarget->szPropTopic,mi->argv[2]);

			Channel_BroadcastToLocal(chTarget,client,NULL,TOK_TOPIC,MSG_TOPIC,"%s :%s",chTarget->szName,mi->argv[2]);
			Server_BroadcastFromUser(client,TOK_TOPIC,MSG_TOPIC,"%s :%s",chTarget->szName,mi->argv[2]);
			Event_Broadcast(client,EVENT_TYPE_CHANNEL,"CHANNEL TOPIC %s %s!%s@%s %s:%u :%s",chTarget->szName,Client_Nickname(client),client->user->username,client->hostname,client->ip_r,client->port_r,chTarget->szPropTopic);
		}
	}

	return 0;
}
int MP_Invite()
/*
** MP_Invite()
** Called whenever a client invites another client to a channel
** Syntax: INVITE <Username> <Channel>
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CHANNEL_STRUCT *chChannel = NULL;
	CLIENT_STRUCT *client = mi->c_prefixfrom, *csInvited = NULL;
	BOOL bInvite = FALSE;
	BOOL bOnChannel = FALSE, bTargetOnChannel = FALSE;
	DWORD dwPriveledge = 0;

	if (mi->argc < 3)
		Client_SendNumericToOne(client,ERR_NEEDMOREPARAMS,mi->argv[0]);
	else
	{
		csInvited = User_Find(mi->argv[1]);
		chChannel = Channel_Find(mi->argv[2]);

		if (!csInvited)
		/* Target user does not exist */
			Client_SendNumericToOne(client,ERR_NOSUCHNICK,mi->argv[1]);
		else
		/*
		** Target user exists, now check if user is able to invite to the channel
		** 1) Channel does not exist
		** 2) Inviter is on channel, is a non-op, and channel is not +i
		** 3) Inviter is on channel, is an op, and channel is +i
		*/
		{
			if (!chChannel)
			/* Channel does not exist */
				bInvite = TRUE;
			else
			/* Channel exists */
			{
				bOnChannel = Client_IsOnChannel(client,chChannel,&dwPriveledge,NULL);
				bTargetOnChannel = Client_IsOnChannel(csInvited,chChannel,NULL,NULL);

				if (bTargetOnChannel == TRUE)
				/* Target user is already on the channel */
					Client_SendToOne(client,FALSE,":%s %3.3d %s %s %s:is already on channel",SettingsInfo.isGeneral.servername,ERR_USERONCHANNEL,Client_Nickname(client),mi->argv[1],mi->argv[2]);
				else if (bOnChannel == FALSE)
				/* Must be on the channel to invite a user */
					Client_SendNumericToOne(client,ERR_NOTONCHANNEL,chChannel->szName);
				else
				/* User is on channel */
				{
					if (Channel_InviteOnly(chChannel))
					{
						if (dwPriveledge & CHANNEL_PRIVELEDGE_OWNER || dwPriveledge & CHANNEL_PRIVELEDGE_OWNER)
						{
							bInvite = TRUE;

							/* Add to internal invite list */
							LL_Add(&(chChannel->llInviteHead),(void*)csInvited);
						}
						else
						/* Only ops and above can invite to this channel */
							Client_SendNumericToOne(client,ERR_CHANOPRIVSNEEDED,chChannel->szName);
					}
					else
					/* Channel is not invite-only */
						bInvite = TRUE;
				}
			}

			if (bInvite)
			{
				/* Send the invite confirmation and also the invite itself */
				Client_SendToOne(client,FALSE,":%s %3.3d %s %s %s",SettingsInfo.isGeneral.servername,RPL_INVITING,Client_Nickname(client),mi->argv[1],mi->argv[2]);
				
				ACCESS_STRUCT *accInfo = NULL;
				int retval = Client_IsOnAccessList(client,NULL,&csInvited->llAccessHead);

				if (!(retval != -1 && ((ACCESS_STRUCT*)retval)->dwLevel == ACCESSLEVEL_DENY))
				/* Do not send the invite, person is on target user's deny/silence list */
					Client_SendMessage(csInvited,client,TOK_INVITE,MSG_INVITE,"%s :%s",csInvited->user->nickname,mi->argv[2]);

				if (Client_IsAway(csInvited))
					Client_SendToOne(client,FALSE,":%s %3.3d %s %s :%s",SettingsInfo.isGeneral.servername,RPL_AWAY,Client_Nickname(client),Client_Nickname(csInvited),csInvited->user->away);

			}
		}
	}

	return 0;
}
int MP_Version()
/*
** MP_Version()
** Called whenever a client issues the "VERSION" command
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;

	Client_SendNumericToOne(mi->c_from,RPL_VERSION,"Written by Zeb Rasco");

	return 0; 
}
int MP_Quit()
/*
** MP_Quit()
** Called whenever a client issues the "QUIT" command
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CLIENT_STRUCT *client = mi->c_prefixfrom;

	/* Mark this user as dead, and record his quit message */
	client->state = CLIENT_STATE_DEAD;

	if (mi->argc > 1)
	/* Display quit message */
		strncpy(client->quitmessage,mi->argv[1],sizeof(client->quitmessage));
	else
	/* Quit message = nickname if blank */
		strncpy(client->quitmessage,Client_Nickname(client),sizeof(client->quitmessage));

	if (!Client_IsServer(client))
	/* QUIT message from local user */
	{
		/* Broadcast USER QUIT event */
		Event_Broadcast(client,EVENT_TYPE_USER,"USER QUIT %s!%s@%s %s:%u :%s",Client_Nickname(client),client->user->username,client->hostname,client->ip_r,client->port_r,client->quitmessage);

		return STOP_PARSING; 
	}
	else
	/* Server issued QUIT command */
	{
		/* Pass message along */
		//Server_Broadcast(mi->c_from,":%d " TOK_QUIT " :%s",client->server->id,mi->argv[1]);

		return STOP_PARSING;
	}
}
int MP_Kill()
/*
** MP_Kill()
** Disconnects the target user from the server, or kicks everyone out of the target channel
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CLIENT_STRUCT *client = mi->c_prefixfrom, *csTarget = NULL;
	CHANNEL_STRUCT *chTarget;

	if (mi->argc < 2)
	/* Not enough parameters */
	{
		Client_SendNumericToOne(client,ERR_NEEDMOREPARAMS,mi->argv[0]);
		return 0;
	}
	else
	/* This command can only be performed by Sysops and Admins */
	{
		if (Client_IsUser(client) && !Client_IsPriveledged(client))
			Client_SendNumericToOne(client,ERR_NOPRIVILEGES,":Permission denied - You're not an IRC operator");
		else
		/* Client has sufficient priveledges to kill */
		{
			chTarget = Channel_Find(mi->argv[1]);

			if (!chTarget || Channel_ReggedAndEmpty(chTarget))
			{
				csTarget = User_Find(mi->argv[1]);

				if (Client_IsUser(client) && !csTarget)
				/* No such nick/channel */
					Client_SendNumericToOne(client,ERR_NOSUCHNICK,mi->argv[1]);
				else
				/* User exists */
				{
					if (Client_IsSysop(client) && Client_IsAdmin(csTarget))
					/* Sysops cannot kill admins */
						Client_SendNumericToOne(client,IRCERR_SECURITY,":No permissions to perform command");
					else if (!Client_IsServer(client))
					/* Kill the specified user */
						Client_Kill(client,csTarget,mi->argv[2]);
				}
			}
			else
			/* Kill target channel */
				Channel_Kill(chTarget,client,mi->argv[2]);
		}
	}


	return 0;
}
int MP_Info()
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;

	if (mi->argc > 1)
	/* Too many parameters (Error: Server option for this command not supported */
		Client_SendToOne(mi->c_from,FALSE,":%s %3.3d %s %s :Server option for this command is not supported.",SettingsInfo.isGeneral.servername,555,Client_Nickname(mi->c_from),mi->argv[1]);
	else
	/* Show info */
	{
		Client_SendToOne(mi->c_from,FALSE,":%s %3.3d %s :RockIRCX IRC server. Copyright 2008 Zeb Rasco.",SettingsInfo.isGeneral.servername,371,Client_Nickname(mi->c_from));
		Client_SendToOne(mi->c_from,FALSE,":%s %3.3d %s :End of /INFO list",SettingsInfo.isGeneral.servername,374,Client_Nickname(mi->c_from));
	}
	return 0;
}
int MP_Links() { return 0; }
int MP_Stats() { return 0; }
int MP_Users()
/*
** MP_Users()
** Not used on RockIRCX.
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CLIENT_STRUCT *client = mi->c_from;

	Client_SendNumericToOne(client,ERR_USERSDISABLED,":USERS has been disabled");

	return 0;
}
int MP_Away()
/*
** MP_Away()
** Called whenever a user sets themselves to be away
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CLIENT_STRUCT *client = mi->c_from;

	if (mi->argc < 2)
	/* Returning from away */
	{
		client->user->away[0] = 0;
		Client_SendNumericToOne(client,RPL_UNAWAY,":You are no longer marked as being away");
	}
	else
	{
		mi->argv[1][64] = 0;
		strcpy(client->user->away,mi->argv[1]);
		Client_SendNumericToOne(client,RPL_NOWAWAY,":You have been marked as being away");
	}

	return 0;
}
int MP_Ping()
/*
** MP_Ping()
** Pings the server
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CLIENT_STRUCT *client = mi->c_from;

	if (Client_IsUser(client))
		Client_SendToOne(client,FALSE,":%s PONG %s :%s",SettingsInfo.isGeneral.servername,SettingsInfo.isGeneral.servername,Client_Nickname(client));
	else
		Client_SendToOne(client,FALSE,":%d %s %s",scs.sclient->server->id,TOK_PONG,SettingsInfo.isGeneral.servername);

	return 0;
}
int MP_Pong()
/*
** MP_Pong()
** This function is purposely designed to do nothing
*/
{
	return 0; 
}

int MP_Oper()
/*
** MP_Oper()
** Allows the user to elevate themselves to power user, sysop, or admin
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CLIENT_STRUCT *client = mi->c_prefixfrom;
	BOOL bFound = FALSE;

	char buffer[1024];

	if (mi->argc < 3)
	{
		Client_SendNumericToOne(client,ERR_NEEDMOREPARAMS,mi->argv[0]);
		return 0;
	}
	else
	{
		strcpy(buffer,Client_Nickname(client));
		strcat(buffer,"!");
		strcat(buffer,client->user->username);
		strcat(buffer,"@");
		strcat(buffer,client->hostname);

		AccountList *AccPtr = &(SettingsInfo.isAccounts.accthead);
		
		while (AccPtr->next)
		{
			AccPtr = AccPtr->next;

			if (match(AccPtr->hostmask,buffer) == 0)
			/* Possible match, check username and password */
			{
				if (lstrcmpi(mi->argv[1],AccPtr->username) == 0 &&
					lstrcmpi(mi->argv[2],AccPtr->password) == 0)
				/* Matching username and password */
				{
					bFound = TRUE;

					if (AccPtr->level == ACCOUNT_LEVEL_ADMIN)
					{
						client->flags |= CLIENT_FLAG_ADMIN;

						Client_SendNumericToOne(client,RPL_YOUREOPER,":You are now an IRC administrator");

						sprintf(buffer,"MODE %s +a\r\n",Client_Nickname(client));
						StrBuf_Add(&client->recvQ,buffer,strlen(buffer));
						
						break;
					}
					else if (AccPtr->level == ACCOUNT_LEVEL_IRCOP)
					{
						client->flags |= CLIENT_FLAG_SYSOP;

						Client_SendNumericToOne(client,RPL_YOUREOPER,":You are now an IRC operator");

						sprintf(buffer,"MODE %s +o\r\n",Client_Nickname(client));
						StrBuf_Add(&client->recvQ,buffer,strlen(buffer));
						
						break;
					}
					else if (AccPtr->level == ACCOUNT_LEVEL_POWERUSER)
					{
						client->flags |= CLIENT_FLAG_POWERUSER;

						Client_SendNumericToOne(client,RPL_YOUREOPER,":You are now an IRC power user");

						//sprintf(buffer,"MODE %s +o\r\n",Client_Nickname(client));
						//StrBuf_Add(&client->recvQ,buffer,strlen(buffer));
						
						break;
					}
				}
			}
		}

		if (!bFound)
		{
			Client_SendNumericToOne(client,ERR_NOOPERHOST,":No O-lines for your host");
			Event_Broadcast(client,EVENT_TYPE_USER,"USER BADAUTH %s!%s@%s %s:%u Username: (%s) Password: (%s)",Client_Nickname(client),client->user->username,client->hostname,client->ip_r,client->port_r,mi->argv[1],mi->argv[2]);
		}
	}
}
int MP_Pass() { return 0; }
int MP_Time()
/*
** MP_Time
** Display server time
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;

	if (mi->argc > 1)
	/* Too many parameters (Error: Server option for this command not supported */
		Client_SendToOne(mi->c_from,FALSE,":%s %3.3d %s %s :Server option for this command is not supported.",SettingsInfo.isGeneral.servername,555,Client_Nickname(mi->c_from),mi->argv[1]);
	else
	/* Show info */
	{
		time_t rawtime;
		struct tm *timeinfo;

		time(&rawtime);
		timeinfo = localtime(&rawtime);

		Client_SendToOne(mi->c_from,FALSE,":%s %3.3d %s :%s",SettingsInfo.isGeneral.servername,RPL_TIME,Client_Nickname(mi->c_from),asctime(timeinfo));
	}
	return 0;
}
int MP_Names()
/*
** MP_Names()
** Called whenever a user requests a /NAMES list for a channel
*/
{
	/* TODO: Add support for wildcards & commas */
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CLIENT_STRUCT *client = mi->c_from;
	CHANNEL_STRUCT *chTarget;

	chTarget = Channel_Find(mi->argv[1]);

	if (chTarget && !Channel_ReggedAndEmpty(chTarget))
	/* Channel found */
		Channel_SendNames(chTarget,client);
	else
		Client_SendNumericToOne(client,RPL_ENDOFNAMES,mi->argv[1]);

	return 0;
}
int MP_Admin()
/*
** MP_Admin
** Displays administrative information about the server
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CLIENT_STRUCT *client = mi->c_from;
	char buf[256];

	if (mi->argc >= 2)
	/* No current support for admin info for different servers */
	{
		sprintf(buf,"%s :Server option for this command is not supported.",mi->argv[1]);
		Client_SendNumericToOne(client,555,buf);
	}
	else
	/* Send admin info */
	{
		if (SettingsInfo.isGeneral.adminloc1[0] == 0 &&
			SettingsInfo.isGeneral.adminloc2[0] == 0 &&
			SettingsInfo.isGeneral.adminemail[0] == 0)
			Client_SendNumericToOne(client,ERR_NOADMININFO,SettingsInfo.isGeneral.servername);
		else
		{
			Client_SendNumericToOne(client,RPL_ADMINME,SettingsInfo.isGeneral.servername);
			Client_SendNumericToOne(client,RPL_ADMINLOC1,SettingsInfo.isGeneral.adminloc1);
			Client_SendNumericToOne(client,RPL_ADMINLOC2,SettingsInfo.isGeneral.adminloc2);
			Client_SendNumericToOne(client,RPL_ADMINEMAIL,SettingsInfo.isGeneral.adminemail);
		}
	}
	
	return 0;
}
int MP_Notice()
/*
** MP_Notice()
** Called whenever a user issues a NOTICE command
** We'll use a sneaky trick here, and just call MP_Privmsg with argv[0] being NOTICE ;)
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;

	if (Client_IsServer(mi->c_from) &&
		mi->argc >= 3 &&
		mi->c_from->server->connected == SERVER_OUTBOUND &&
		strcmp(mi->argv[0],"NOTICE") == 0 &&
		strcmp(mi->argv[1],"AUTH") == 0)
	/* Silently drop our introductory NOTICE AUTH messages */
		return 0;
	else
        return MP_Privmsg();
}

int MP_Join()
/*
** MP_Join()
** Called whenever a user attempts to join a channel
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CLIENT_STRUCT *client = mi->c_prefixfrom;
	CHANNEL_STRUCT *chTarget;
	LINKED_LIST_STRUCT llChannelHead, *llChannelPtr = &llChannelHead, *llPtr;
	int tokcount = 0, nIndex, nCount, nChannelsOn = 0;
	char currchannel[512], chkey[512], szModeStr[256], szJoinState[256];

	if (!Client_IsLocal(client))
	/* Joins from non-locals are a bit different, process them here instead of below */
	{
		CLIENT_STRUCT *csWinner = NULL;
		CHANNEL_STRUCT *chChannel = Channel_Find(mi->argv[2]), *chNew;
		int nJoinState = atoi(mi->argv[1]);

		if (nJoinState == 1)
		/* Join state #1 New channel */
		/* Syntax: :Zeb!Nowhere@Here JOIN 1 #test <timestamp> q */
		{
			time_t timestamp = atol(mi->argv[3]);

			if (!chChannel)
			{
				chNew = Channel_Create(mi->argv[2]);

				Channel_GetModeString(chNew,szModeStr);
				
				Event_Broadcast(client,EVENT_TYPE_CHANNEL,"CHANNEL CREATE %s %s %s!%s@%s %s:%u",chNew->szName,szModeStr,Client_Nickname(client),client->user->username,client->hostname,client->ip_r,client->port_r);

				/* Add them as owner! */
				sprintf(szJoinState,"1 %s %u q",chNew->szName,chNew->dwPropCreationTime);

				Channel_AddUser(chNew,client,CHANNEL_PRIVELEDGE_OWNER | CHANNEL_PRIVELEDGE_HOST,szJoinState);
			}
			else
			/* Channel collision! */
			{
				if (mi->argc > 4)
				/* Modes are included */
				{
					/* Check if incoming channel or existing channel was created first (or at the same time) */
					BOOL bLocalChanWins = (chChannel->dwPropCreationTime <= timestamp);

					if (bLocalChanWins)
					/* Kill local channel */
					{
						sprintf(szJoinState,"1 %s %s %s",mi->argv[2],mi->argv[3],mi->argv[4]);
						Channel_Kill(chChannel,client,"Channel collision");
						chChannel = Channel_Create(mi->argv[2]);

						Channel_AddUser(chChannel,client,CHANNEL_PRIVELEDGE_OWNER,szJoinState);
					}
					else
					/* Send KILL back to client, he lost */
					{
						Client_SendToOne(client->servernext,FALSE,":%d " TOK_KILL " %s :Channel collision",scs.sclient->server->id,chChannel->szName);
						Server_SendChannelInfo(client->servernext,chChannel);
					}
				}
				else
				/* Add as regular user, and pretend collision never happened */
				{
					sprintf(szJoinState,"1 %s %s",mi->argv[2],mi->argv[3]);
					Channel_AddUser(chChannel,client,CHANNEL_PRIVELEDGE_NONE,szJoinState);
				}
			}
		}
		else if (chChannel && nJoinState == 2)
		/* Join state #2: User has good ownerkey/hostkey */
		/* COLLISION CHECK: If this differs from actual, the key has been changed on the network mid-transit */
		/* Syntax: :Zeb!Nowhere@Here JOIN 2 #test q ownerkey */
		/* Or..... :Zeb!Nowhere@Here JOIN 2 #test o hostkey */
		{

			if (mi->argv[3][0] == 'q')
			{
				sprintf(szJoinState,"2 %s q %s",chChannel->szName,(chChannel->szPropOwnerkey[0] ? chChannel->szPropOwnerkey : "0"));
				Channel_AddUser(chChannel,client,CHANNEL_PRIVELEDGE_OWNER,szJoinState);
			}
			else if (mi->argv[3][0] == 'o')
			{
				sprintf(szJoinState,"2 %s o %s",chChannel->szName,(chChannel->szPropOwnerkey[0] ? chChannel->szPropHostkey : "0"));
				Channel_AddUser(chChannel,client,CHANNEL_PRIVELEDGE_HOST,szJoinState);
			}
		}
		else if (chChannel && nJoinState == 3)
		/* Join state #3: User is joining under an access entry (except DENY of course) */
		/* COLLISION CHECK: If user doesn't have an actual access entry, there is a collision */
		/* Syntax: :Zeb!Nowhere@Here JOIN 3 #test 1 */
		{
			DWORD dwLevel = atol(mi->argv[3]);
			sprintf(szJoinState,"3 %s %s",mi->argv[2],mi->argv[3]);

			if (dwLevel == CHANNEL_PRIVELEDGE_OWNER)
				Channel_AddUser(chChannel,client,CHANNEL_PRIVELEDGE_OWNER,szJoinState);
			else if (dwLevel == CHANNEL_PRIVELEDGE_HOST)
				Channel_AddUser(chChannel,client,CHANNEL_PRIVELEDGE_HOST,szJoinState);
			else if (dwLevel == CHANNEL_PRIVELEDGE_VOICE)
				Channel_AddUser(chChannel,client,CHANNEL_PRIVELEDGE_VOICE,szJoinState);
		}
		else if (chChannel && nJoinState == 4)
		/* Join state #4: User is joining as a priveledged user */
		/* Syntax: :Zeb!Nowhere@Here JOIN 4 #test q */
		{
			sprintf(szJoinState,"4 %s q",mi->argv[2]);
			Channel_AddUser(chChannel,client,CHANNEL_PRIVELEDGE_OWNER,szJoinState);
		}

		return 0;
	}

	if (mi->argc < 2)
	{
		Client_SendNumericToOne(client,ERR_NEEDMOREPARAMS,mi->argv[0]);
		return 0;
	}

	/* Create our list of channels */
	Parse_ChannelList(mi->argv[1],&llChannelHead);

	llPtr = &(client->user->llChannelHead);

	while (llPtr->next)
	{
		llPtr = llPtr->next;
		nChannelsOn++;
	}

	while (llChannelPtr->next)
	/* Go thru our list of channels passed */
	{
		tokcount++;
		llChannelPtr = llChannelPtr->next;

		chTarget = (CHANNEL_STRUCT*)(llChannelPtr->data);
		gettok(currchannel,mi->argv[1],tokcount,',');

		chkey[0] = 0;

		if (mi->argc > 2)
		/* Get the key supplied */
			gettok(chkey,mi->argv[2],tokcount,',');

		if (chTarget == NULL)
		/* Channel not found, create a new one */
		{
			CHANNEL_STRUCT *chNew = NULL;
			int nLen = strlen(currchannel);

			FilterList *flptr;
			BOOL bFiltered = FALSE;

			/* Check through list of disallowed channel names */
			if (SettingsInfo.isFilter.enabled)
			{
				flptr = &SettingsInfo.isFilter.filterhead;

				while (flptr->next)
				{
					flptr = flptr->next;

					if ((flptr->type == FILTER_TYPE_CHANNEL || flptr->type == FILTER_TYPE_ALL) && match(flptr->word,currchannel) == 0)
					{
						bFiltered = TRUE;
						break;
					}
				}
			}

			if ((!Client_IsPriveledged(client) && bFiltered) || (!Valid_Channelname(currchannel) || (SettingsInfo.isSecurity.max_chanlen && (nLen > SettingsInfo.isSecurity.max_chanlen))))
			/* Channelname is not valid */
				Client_SendNumericToOne(client,ERR_NOSUCHCHANNEL,currchannel);
			else
			/* Create new channel after checking if the name is valid */
			{
				CHANNEL_STRUCT *chNew = NULL;

				if (!Client_IsPriveledged(client) && SettingsInfo.isUser.dyn_create)
				/* Cannot create dynamic channels */
					Client_SendToOne(client,FALSE,":%s %3.3d %s %s :Cannot create dynamic channels due to admin restriction.",SettingsInfo.isGeneral.servername,553,Client_Nickname(client),mi->argv[1]);
				else if (SettingsInfo.isUser.maxchannels > 0 && !Client_IsPriveledged(client) &&
					nChannelsOn >= SettingsInfo.isUser.maxchannels)
					Client_SendNumericToOne(client,ERR_TOOMANYCHANNELS,currchannel);
				else
				{
					char szJoinState[256];

					chNew = Channel_Create(currchannel);
					Channel_GetModeString(chNew,szModeStr);
					
					Event_Broadcast(client,EVENT_TYPE_CHANNEL,"CHANNEL CREATE %s %s %s!%s@%s %s:%u",chNew->szName,szModeStr,Client_Nickname(client),client->user->username,client->hostname,client->ip_r,client->port_r);

					/* Add user as owner! */
					sprintf(szJoinState,"1 %s %u q",currchannel,chNew->dwPropCreationTime);

					Channel_AddUser(chNew,client,CHANNEL_PRIVELEDGE_OWNER | CHANNEL_PRIVELEDGE_HOST,szJoinState);
				}
			}
		}
		else
		/* Channel found */
		{
			char szJoinState[256];

			if (Client_IsPriveledged(client))
			/* Give q to sysops/admins */
			{
				sprintf(szJoinState,"4 %s q",chTarget->szName);
				Channel_AddUser(chTarget,client,CHANNEL_PRIVELEDGE_OWNER,szJoinState);
			}
			else if (Client_IsLocal(client) && !Channel_IsRegistered(chTarget) &&
				!Client_IsPriveledged(client) && SettingsInfo.isUser.dyn_join)
			/* Cannot join dynamic channels (error to local only) */
				Client_SendToOne(client,FALSE,":%s %3.3d %s %s :Cannot join dynamic channels due to admin restriction.",SettingsInfo.isGeneral.servername,553,Client_Nickname(client),mi->argv[1]);
			else if (Client_IsLocal(client) &&
				SettingsInfo.isUser.maxchannels > 0 &&
				!Client_IsPriveledged(client) &&
				nChannelsOn >= SettingsInfo.isUser.maxchannels)
			/* User is on too many channels (error to local only) */
				Client_SendNumericToOne(client,ERR_TOOMANYCHANNELS,currchannel);			
			else if (Client_IsLocal(client) && Channel_AuthOnly(chTarget))
			/* Regular users cannot enter channel if auth only (only stop locals) */
			{
				Client_SendNumericToOne(client,IRCERR_NOACCESS,currchannel);
				Channel_SendKnock(chTarget,client,IRCERR_NOACCESS);
			}
			else
			/* Regular users go here */
			{
				ACCESS_STRUCT *accInfo;
				BOOL bKeyUsed = FALSE;
				BOOL bContinue = TRUE;
				char szJoinState[256];

				/* Check if user is on the access list */
				nIndex = Client_IsOnAccessList(client,chTarget->accList,NULL);
				accInfo = &chTarget->accList[nIndex];

				if (chkey[0])
				/* A key was given by the user */
				{
					if (lstrcmpi(chkey,chTarget->szPropOwnerkey) == 0)
					{
						sprintf(szJoinState,"2 %s q %s",chTarget->szName,chkey);
						Channel_AddUser(chTarget,client,CHANNEL_PRIVELEDGE_OWNER,szJoinState);
						bContinue = FALSE;
					}
					else if (lstrcmpi(chkey,chTarget->szPropHostkey) == 0)
					{
						sprintf(szJoinState,"2 %s o %s",chTarget->szName,chkey);
						Channel_AddUser(chTarget,client,CHANNEL_PRIVELEDGE_HOST,szJoinState);
						bContinue = FALSE;
					}
				}
				if (bContinue == TRUE)
				{
					if (nIndex == -1)
					/* Not on access list */
					{
						BOOL bAddToChan = TRUE;

						if (Channel_Full(chTarget))
						/* Channel is full */
						{
							Client_SendNumericToOne(client,ERR_CHANNELISFULL,currchannel);
							Channel_SendKnock(chTarget,client,ERR_CHANNELISFULL);
							bAddToChan = FALSE;
						}
						else if (Channel_InviteOnly(chTarget))
						/* Channel is invite only */
						{
							LINKED_LIST_STRUCT *llUser = LL_Find(&(chTarget->llInviteHead),(void*)client);

							if (llUser)
							/* User has been invited to channel! */
								LL_Remove(&(chTarget->llInviteHead),(void*)client);
							else if (Channel_InviteOnly(chTarget))
							/* Channel is invite-only, user has not been invited */
							{
								Client_SendNumericToOne(client,ERR_INVITEONLYCHAN,currchannel);
								Channel_SendKnock(chTarget,client,ERR_INVITEONLYCHAN);
								bAddToChan = FALSE;
							}
						}
						else if (Channel_HasMemberkey(chTarget))
						/* Channel has a member key (+k) */
						{
							if (chkey[0] && lstrcmpi(chkey,chTarget->szPropMemberkey) == 0)
							/* Key matches, let them in! */
								bAddToChan = TRUE;
							else
							/* Key does not match */
							{
								Client_SendNumericToOne(client,ERR_BADCHANNELKEY,currchannel);
								Channel_SendKnock(chTarget,client,ERR_BADCHANNELKEY);
								bAddToChan = FALSE;
							}
						}

						if (bAddToChan)
							Channel_AddUser(chTarget,client,CHANNEL_PRIVELEDGE_NONE,NULL);
					}
					else if (accInfo->dwLevel == ACCESSLEVEL_OWNER)
					/* Make person an owner */
					{
						sprintf(szJoinState,"3 %s %d",chTarget->szName,CHANNEL_PRIVELEDGE_OWNER);
						Channel_AddUser(chTarget,client,CHANNEL_PRIVELEDGE_OWNER,szJoinState);
					}
					else if (accInfo->dwLevel == ACCESSLEVEL_HOST)
					/* Make person a host */
					{
						sprintf(szJoinState,"3 %s %d",chTarget->szName,CHANNEL_PRIVELEDGE_HOST);
						Channel_AddUser(chTarget,client,CHANNEL_PRIVELEDGE_HOST,szJoinState);
					}
					else if (accInfo->dwLevel == ACCESSLEVEL_VOICE)
					/* Give person voice */
					{
						sprintf(szJoinState,"3 %s %d",chTarget->szName,CHANNEL_PRIVELEDGE_VOICE);
						Channel_AddUser(chTarget,client,CHANNEL_PRIVELEDGE_VOICE,szJoinState);
					}
					else if (accInfo->dwLevel == ACCESSLEVEL_GRANT)
					/* On exception list, let them in! */
						Channel_AddUser(chTarget,client,CHANNEL_PRIVELEDGE_NONE,NULL);
					else if (chTarget->accList[nIndex].dwLevel == ACCESSLEVEL_DENY)
					/* User is banned! */
					{
						Client_SendNumericToOne(client,ERR_BANNEDFROMCHAN,chTarget->szName);
						Channel_SendKnock(chTarget,client,ERR_BANNEDFROMCHAN);
					}
				}
			}
		}
	}

	LL_Clear(&llChannelHead);

	return 0;
}

int MP_Create()
/*
** MP_Create()
** Executes the IRCX CREATE command to create a channel. Similar to JOIN
*/
{
	int nChannelsOn = 0;
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CLIENT_STRUCT *client = mi->c_prefixfrom;
	CHANNEL_STRUCT *chChannel = NULL;
	BOOL bOnChannel = FALSE;
	LINKED_LIST_STRUCT *llPtr;
	
	if (mi->argc < 2)
	{
		Client_SendNumericToOne(client,ERR_NEEDMOREPARAMS,mi->argv[0]);
		return 0;
	}

	llPtr = &(client->user->llChannelHead);

	while (llPtr->next)
	{
		llPtr = llPtr->next;
		nChannelsOn++;
	}

	FilterList *flptr;
	BOOL bFiltered = FALSE;

	/* Check through list of disallowed channel names */
	if (SettingsInfo.isFilter.enabled)
	{
		flptr = &SettingsInfo.isFilter.filterhead;

		while (flptr->next)
		{
			flptr = flptr->next;

			if ((flptr->type == FILTER_TYPE_CHANNEL || flptr->type == FILTER_TYPE_ALL) && match(flptr->word,mi->argv[1]) == 0)
			{
				bFiltered = TRUE;
				break;
			}
		}
	}

	if ((!Client_IsPriveledged(client) && bFiltered) || !Valid_Channelname(mi->argv[1]))
	/* Invalid channel name, display "No such channel" */
		Client_SendNumericToOne(client,ERR_NOSUCHCHANNEL,mi->argv[1]);
	else
	{
		/* Parse the mode string */
		char ch = 0, szDisplayStr[32], szKey[128];
		int nCount = 0, nParamIndex = 3;
		BOOL bError = FALSE;
		DWORD dwModes = 0, dwLimit = 0;

		szKey[0] = 0;
		szDisplayStr[0] = 0;

		if (mi->argc > 2)
		/* Parse the mode string */
		{
			for (nCount = 0; mi->argv[2][nCount]; nCount++)
			{
				ch = mi->argv[2][nCount];

				if (ch == 'a')
					dwModes |= CHANNELMODE_AUTHONLY;
				else if (ch == 'd')
					dwModes |= CHANNELMODE_CLONEABLE;
				else if (ch == 'f')
					dwModes |= CHANNELMODE_NOFORMAT;
				else if (ch == 'h')
				{
					dwModes |= CHANNELMODE_HIDDEN;
					dwModes &= ~CHANNELMODE_SECRET;
					dwModes &= ~CHANNELMODE_PRIVATE;
					dwModes &= ~CHANNELMODE_PUBLIC;
				}
				else if (ch == 'i')
					dwModes |= CHANNELMODE_INVITE;
				else if (ch == 'k')
				{
					if (mi->argc <= nParamIndex)
					/* Not enough parameters */
					{
						Client_SendNumericToOne(client,ERR_NEEDMOREPARAMS,mi->argv[0]);
						return 0;
					}
					else
					{
						strcpy(szKey,mi->argv[nParamIndex]);
						nParamIndex++;
					}
				}
				else if (ch == 'l')
				{
					if (mi->argc <= nParamIndex)
					/* Not enough parameters */
					{
						Client_SendNumericToOne(client,ERR_NEEDMOREPARAMS,mi->argv[0]);
						return 0;
					}
					else
					{
						dwLimit = atol(mi->argv[nParamIndex]);
						nParamIndex++;
					}
				}
				else if (ch == 'm')
					dwModes |= CHANNELMODE_MODERATED;
				else if (ch == 'n')
					dwModes |= CHANNELMODE_NOEXTERN;
				else if (ch == 'p')
				{
					dwModes |= CHANNELMODE_PRIVATE;
					dwModes &= ~CHANNELMODE_SECRET;
					dwModes &= ~CHANNELMODE_HIDDEN;
					dwModes &= ~CHANNELMODE_PUBLIC;
				}
				else if (ch == 's')
				{
					dwModes |= CHANNELMODE_SECRET;
					dwModes &= ~CHANNELMODE_PRIVATE;
					dwModes &= ~CHANNELMODE_HIDDEN;
					dwModes &= ~CHANNELMODE_PUBLIC;
				}
				else if (ch == 't')
					dwModes |= CHANNELMODE_TOPICOP;
				else if (ch == 'u')
					dwModes |= CHANNELMODE_KNOCK;
				else if (ch == 'w')
					dwModes |= CHANNELMODE_NOWHISPER;
				else if (ch == 'x')
					dwModes |= CHANNELMODE_AUDITORIUM;
				else
				{
					szDisplayStr[0] = ch; szDisplayStr[1] = 0;
					Client_SendNumericToOne(client,ERR_UNKNOWNMODE,szDisplayStr);
					return 0;
				}
			}
		}

		chChannel = Channel_Find(mi->argv[1]);

		if (chChannel)
		/* Channel exists */
		{
			bOnChannel = Client_IsOnChannel(client,chChannel,NULL,NULL);

			if (bOnChannel)
				Client_SendNumericToOne(client,IRCERR_ALREADYONCHANNEL,mi->argv[1]);
			else
			/* Have user JOIN the channel */
			{
				strcpy(mi->argv[0],"JOIN");

				return MP_Join();
			}
		}
		else
		/* Channel does not exist */
		{
			CHANNEL_STRUCT *chNew = NULL;
			char szModeStr[256];

			if (!Client_IsPriveledged(client) && SettingsInfo.isUser.dyn_create)
			/* Cannot create dynamic channels */
				Client_SendToOne(client,FALSE,":%s %3.3d %s %s :Cannot create dynamic channels due to admin restriction.",SettingsInfo.isGeneral.servername,553,Client_Nickname(client),mi->argv[1]);
			else if (SettingsInfo.isUser.maxchannels > 0 && !Client_IsPriveledged(client) &&
				nChannelsOn >= SettingsInfo.isUser.maxchannels)
				Client_SendNumericToOne(client,ERR_TOOMANYCHANNELS,mi->argv[1]);
			else
			{
				char szJoinState[256];

				chNew = Channel_Create(mi->argv[1]);
				chNew->dwModes = dwModes;
				Channel_GetModeString(chNew,szModeStr);
				
				Event_Broadcast(client,EVENT_TYPE_CHANNEL,"CHANNEL CREATE %s %s %s!%s@%s %s:%u",chNew->szName,szModeStr,Client_Nickname(client),client->user->username,client->hostname,client->ip_r,client->port_r);

				if (dwLimit)
					chNew->dwLimit = dwLimit;

				if (szKey)
					strcpy(chNew->szPropMemberkey,szKey);

				/* Add him as owner! */
				sprintf(szJoinState,"1 %s %u q",chNew->szName,chNew->dwPropCreationTime);
				Channel_AddUser(chNew,client,CHANNEL_PRIVELEDGE_OWNER,szJoinState);
			}
		}
	}

	return 0;
}
int MP_Access()
/*
** MP_Access()
** Called whenever a user issues an access command
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CLIENT_STRUCT *client = mi->c_prefixfrom, *csTarget = NULL;
	CHANNEL_STRUCT *chTarget = NULL;
	LINKED_LIST_STRUCT *llAccessNetwork = NULL, *llAccessServer = NULL;
	ACCESS_STRUCT *accInfo = NULL;
	BOOL bHasAccess = FALSE, bShowCreators = FALSE, bServerAccess = FALSE, bOwnAccess = FALSE;
	DWORD dwChannelPriv = NULL;
	char *szTemp[128];

	szTemp[0] = 0;

	if (mi->argc < 2)
	{
		Client_SendNumericToOne(client,ERR_NEEDMOREPARAMS,mi->argv[0]);
		return 0;
	}
	else if (mi->argc > 7)
	{
		Client_SendNumericToOne(client,IRCERR_TOOMANYARGUMENTS,mi->argv[0]);
		return 0;
	}

	/* Find the object being referenced */
	if (mi->argv[1][0] == '*' && mi->argv[1][1] == 0)
	/* Network access */
	{
		if (Client_IsAdmin(client))
		{
			bHasAccess = TRUE;
			llAccessNetwork = &scs.llAccessNetworkHead;
			strncpy((char*)&szTemp,mi->argv[1],sizeof(szTemp));
		}
		else
			Client_SendNumericToOne(client,IRCERR_NOACCESS,mi->argv[1]);
	}
	else if (mi->argv[1][0] == '$' && mi->argv[1][1] == 0)
	/* Server access */
	{
		if (Client_IsPriveledged(client))
		{
			bHasAccess = TRUE;
			bServerAccess = TRUE;
			llAccessServer = &scs.sclient->llAccessHead;
			strncpy((char*)&szTemp,SettingsInfo.isGeneral.servername,sizeof(szTemp));
		}
		else
			Client_SendNumericToOne(client,IRCERR_NOACCESS,mi->argv[1]);
	}
	else
	{
		/* Try to find a channel */
		chTarget = Channel_Find(mi->argv[1]);

		if (chTarget && !Channel_ReggedAndEmpty(chTarget))
		/* Channel has been found */
		{
			Client_IsOnChannel(client,chTarget,&dwChannelPriv,NULL);

			if (Client_IsPriveledged(client) || dwChannelPriv & CHANNEL_PRIVELEDGE_OWNER || dwChannelPriv & CHANNEL_PRIVELEDGE_HOST)
			{
				bHasAccess = TRUE;
				strncpy((char*)&szTemp,chTarget->szName,sizeof(szTemp));
			}
			else
				Client_SendNumericToOne(client,IRCERR_NOACCESS,mi->argv[1]);
		}
		else
		{
			csTarget = User_Find(mi->argv[1]);

			if (csTarget)
			{
				if (csTarget == client)
				{
					bOwnAccess = TRUE;
					bHasAccess = TRUE;
					strncpy((char*)&szTemp,csTarget->user->nickname,sizeof(szTemp));
				}
				else
					Client_SendNumericToOne(client,IRCERR_NOACCESS,mi->argv[1]);

			}
			else
			/* Object cannot be found! */
				Client_SendNumericToOne(client,IRCERR_NOSUCHOBJECT,mi->argv[1]);
		}
	}

	if (bHasAccess)
	{
		LINKED_LIST_STRUCT *llNode = (llAccessNetwork ? llAccessNetwork : (llAccessServer ? llAccessServer : NULL));
		ACCESS_STRUCT accEntry;
		accInfo = &accEntry;

		memset(accInfo,0,sizeof(accEntry));

		if (llNode == NULL && csTarget)
			llNode = &csTarget->llAccessHead;

		/* Check if individual is an owner (or admin, for server access entries. Sysops cannot override an admin's entry) */
		accInfo->bOwner = (chTarget ? (dwChannelPriv & CHANNEL_PRIVELEDGE_OWNER ? TRUE : FALSE) : Client_IsAdmin(client));

		if (chTarget && Client_IsPriveledged(client))
		/* Admins/sysops have unrestricted access to any channel */
			accInfo->bOwner = TRUE;

		/* Do a list by default, otherwise accept commands */
		if (mi->argc == 2)
			Access_OutputEntries(client,mi->argv[1],llAccessNetwork,llAccessServer,chTarget,csTarget);
		else
		{
			if (lstrcmpi(mi->argv[2],"LIST") == 0)
			/* Send list of access entries */
				Access_OutputEntries(client,mi->argv[1],llAccessNetwork,llAccessServer,chTarget,csTarget);
			else if (lstrcmpi(mi->argv[2],"ADD") == 0 || lstrcmpi(mi->argv[2],"DELETE") == 0 || lstrcmpi(mi->argv[2],"CLEAR") == 0)
			/* Check the level */
			{
				if (mi->argc == 3)
				{
					if ((mi->argv[2][0] == 'c') || (mi->argv[2][0] == 'C'))
					/* Clear all access entries */
					{
						int nCleared = Access_Clear((chTarget ? chTarget->accList : NULL),llNode,accInfo->bOwner,ACCESSLEVEL_ALL);

						if (nCleared < 0)
						{
							Client_SendNumericToOne(client,IRCERR_ACCESSSECURITY,":Some entries not cleared due to security");
							if (chTarget)
								chTarget->dwAccessEntries += nCleared;
						}
						else
						{
							Client_SendToOne(client,FALSE,":%s %3.3d %s %s * :Clear",SettingsInfo.isGeneral.servername,820,Client_Nickname(client),szTemp);
							if (chTarget)
								chTarget->dwAccessEntries -= nCleared;
						}
						
						if (!bOwnAccess && !bServerAccess)
							Server_BroadcastFromUser(client,TOK_ACCESS,MSG_ACCESS,"%s CLEAR",
								szTemp												/* Object */
								);
					}
					else
					/* Bad level */
						Client_SendNumericToOne(client,IRCERR_BADLEVEL,mi->argv[1]);
				}
				else
				{
					int nResult = 0;

					/* Get the access level */
					Access_GetTypeFromString(mi->argv[3],&accInfo->dwLevel,(chTarget ? TRUE : FALSE));

					/* If no hostmask is specified, assume *!*@* */
					if (mi->argc == 4)
						Validate_AccessHostmask("*!*@*$*",accInfo->szHostmask);
					else
						Validate_AccessHostmask(mi->argv[4],accInfo->szHostmask);

					if (accInfo->dwLevel == ACCESSLEVEL_ALL || (accInfo->bOwner == FALSE && accInfo->dwLevel == ACCESSLEVEL_OWNER))
						Client_SendNumericToOne(client,IRCERR_BADLEVEL,mi->argv[3]);
					else if (mi->argv[2][0] == 'A' || mi->argv[2][0] == 'a')
					/* Add access entry */
					{
						/* Fill out the ACCESS_STRUCT with the parameters */
						strcpy(accInfo->szCreator,client->user->nickname);

						if (mi->argc >= 6)
						/* Timeout is specified */
						{
							accInfo->dwTimeout = atol(mi->argv[5]);

							if (mi->argc == 7)
								strncpy(accInfo->szReason,mi->argv[6],sizeof(accInfo->szReason));
						}

						nResult = Access_Add((chTarget ? chTarget->accList : NULL),llNode,accInfo);

						if (nResult == -1)
						/* Duplicate entry */
							Client_SendNumericToOne(client,IRCERR_DUPACCESS,":Duplicate access entry");
						else if (nResult == -2)
						/* Access list is full */
							Client_SendNumericToOne(client,IRCERR_TOOMANYACCESSES,":Too many access entries");
						else
						/* Entry added */
						{
							if (chTarget)
								chTarget->dwAccessEntries++;

							Client_SendToOne(client,FALSE,":%s %3.3d %s %s %s %s %d %s :%s",SettingsInfo.isGeneral.servername,IRCRPL_ACCESSADD,Client_Nickname(client),
								szTemp,												/* Object */
								Access_GetTypeString(accInfo->dwLevel),				/* Level */
								accInfo->szHostmask,								/* Hostmask */
								accInfo->dwTimeout,									/* Timeout */
								(bShowCreators == TRUE ? accInfo->szCreator : "*"), /* Creator */
								accInfo->szReason									/* Reason */
								);

							if (!bOwnAccess && !bServerAccess)
								Server_BroadcastFromUser(client,TOK_ACCESS,MSG_ACCESS,"%s ADD %s %s %d :%s",
									szTemp,												/* Object */
									Access_GetTypeString(accInfo->dwLevel),				/* Level */
									accInfo->szHostmask,								/* Hostmask */
									accInfo->dwTimeout,									/* Timeout */
									(bShowCreators == TRUE ? accInfo->szCreator : "*"), /* Creator */
									accInfo->szReason									/* Reason */
									);
						}
					}
					else if (mi->argv[2][0] == 'D' || mi->argv[2][0] == 'd')
					/* Delete an entry */
					{
						nResult = Access_Delete((chTarget ? chTarget->accList : NULL),llNode,accInfo->szHostmask,accInfo->dwLevel,accInfo->bOwner);

						if (nResult == 0)
						/* Deleted successfully */
						{
							if (chTarget)
								chTarget->dwAccessEntries--;

							Client_SendToOne(client,FALSE,":%s %3.3d %s %s %s %s",SettingsInfo.isGeneral.servername,IRCRPL_ACCESSDELETE,Client_Nickname(client),
								szTemp,												/* Object */
								Access_GetTypeString(accInfo->dwLevel),				/* Level */
								accInfo->szHostmask									/* Hostmask */
								);

							if (!bOwnAccess && !bServerAccess)
								Server_BroadcastFromUser(client,TOK_ACCESS,MSG_ACCESS,"%s DELETE %s %s",
									szTemp,												/* Object */
									Access_GetTypeString(accInfo->dwLevel),				/* Level */
									accInfo->szHostmask									/* Hostmask */
									);
						}
						else if (nResult == -1)
						/* Cannot delete entry due to security */
							Client_SendNumericToOne(client,IRCERR_NOACCESS,mi->argv[1]);
						else if (nResult == -2)
						/* Entry does not exist */
							Client_SendNumericToOne(client,IRCERR_MISACCESS,":Unknown access entry");
					}
					/* Clear the list (although this time we have a valid level) */
					else if (mi->argv[2][0] == 'C' || mi->argv[2][0] == 'c')
					{
						if (accInfo->bOwner == FALSE && accInfo->dwLevel == ACCESSLEVEL_OWNER)
						/* Bad level, hosts cannot touch owner entries */
							Client_SendNumericToOne(client,IRCERR_BADLEVEL,mi->argv[1]);
						else
						/* Delete entries */
						{
							int nCleared = Access_Clear((chTarget ? chTarget->accList : NULL),llNode,accInfo->bOwner,accInfo->dwLevel);
							
							if (nCleared <= 0)
							{
								Client_SendNumericToOne(client,IRCERR_ACCESSSECURITY,":Some entries not cleared due to security");
								if (chTarget)
									chTarget->dwAccessEntries += nCleared;
							}
							else
							{
								Client_SendToOne(client,FALSE,":%s %3.3d %s %s %s :Clear",SettingsInfo.isGeneral.servername,820,Client_Nickname(client),szTemp,Access_GetTypeString(accInfo->dwLevel));
								if (chTarget)
									chTarget->dwAccessEntries -= nCleared;
							}

							if (!bOwnAccess && !bServerAccess)
								Server_BroadcastFromUser(client,TOK_ACCESS,MSG_ACCESS,"%s CLEAR %s",szTemp,Access_GetTypeString(accInfo->dwLevel));
						}
					}
				}
			}
		}
	}

	return 0;
}
int MP_Prop()
/*
** MP_Prop()
** Sets properties on the specified object
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CLIENT_STRUCT *client = mi->c_prefixfrom, *csTarget;
	CHANNEL_STRUCT *chChannel = NULL;
	int nCount = 0, nTokens = 0;
	char buf[4096];

	if (mi->argc < 3)
	/* Not enough parameters */
	{
		Client_SendNumericToOne(client,ERR_NEEDMOREPARAMS,mi->argv[0]);
		return 0;
	}

	chChannel = Channel_Find(mi->argv[1]);
	csTarget = User_Find(mi->argv[1]);

	if (chChannel && !Channel_ReggedAndEmpty(chChannel))
	/* Viewing/setting properties for a channel */
	{
		if (mi->argc == 3)
			Channel_SendProperties(chChannel,client,mi->argv[2]);
		else
		/* User is attempting to change property */
		{
			DWORD dwPriveledge = 0;
			int nCount = 0;
			BOOL bIsOnChannel = Client_IsOnChannel(client,chChannel,&dwPriveledge,NULL);

			if (numtok(mi->argv[2],',') > 1)
				Client_SendNumericToOne(client,IRCERR_BADCOMMAND,mi->argv[1]);
			else
			{
				/* Assess user priveledges */
				if (lstrcmpi(mi->argv[2],"OID") == 0 ||
					lstrcmpi(mi->argv[2],"NAME") == 0 ||
					lstrcmpi(mi->argv[2],"CREATION") == 0)
					Client_SendNumericToOne(client,IRCERR_SECURITY,":No permissions to perform command");
				else if (lstrcmpi(mi->argv[2],"LANGUAGE") == 0)
				/* Can be set by admins, owners, and hosts */
				{
					if ((Client_IsAdmin(client) && bIsOnChannel) || dwPriveledge & CHANNEL_PRIVELEDGE_OWNER || dwPriveledge & CHANNEL_PRIVELEDGE_HOST)
					/* Set language (limited to 31 characters) */
					{
						strncpy(chChannel->szPropLanguage,mi->argv[3],31);
						Channel_BroadcastToLocal(chChannel,client,NULL,TOK_PROP,MSG_PROP,"%s LANGUAGE :%s",chChannel->szName,chChannel->szPropLanguage);
						Server_BroadcastFromUser(client,TOK_PROP,MSG_PROP,"%s LANGUAGE :%s",chChannel->szName,chChannel->szPropLanguage);
					}
					else
					/* Denied! */
						Client_SendNumericToOne(client,IRCERR_SECURITY,":No permissions to perform command");
				}
				else if (lstrcmpi(mi->argv[2],"OWNERKEY") == 0)
				/* Can be set by admins and owners */
				{
					if ((Client_IsAdmin(client) && bIsOnChannel) || dwPriveledge & CHANNEL_PRIVELEDGE_OWNER)
					/* Set ownerkey (limited to 31 characters) */
					{
						strncpy(chChannel->szPropOwnerkey,mi->argv[3],31);
						Channel_BroadcastToLocalSelected(chChannel,client,NULL,ACCESSLEVEL_OWNER,TRUE,FALSE,MSG_PROP " %s OWNERKEY :%s",chChannel->szName,chChannel->szPropOwnerkey);
						Server_BroadcastFromUser(client,TOK_PROP,MSG_PROP,"%s OWNERKEY :%s",chChannel->szName,chChannel->szPropOwnerkey);						
					}
					else
					/* Denied! */
						Client_SendNumericToOne(client,IRCERR_SECURITY,":No permissions to perform command");
				}
				else if (lstrcmpi(mi->argv[2],"HOSTKEY") == 0)
				/* Can be set by admins and owners */
				{
					if ((Client_IsAdmin(client) && bIsOnChannel) || dwPriveledge & CHANNEL_PRIVELEDGE_OWNER)
					/* Set hostkey (limited to 31 characters) */
					{
						strncpy(chChannel->szPropHostkey,mi->argv[3],31);
						Channel_BroadcastToLocalSelected(chChannel,client,NULL,ACCESSLEVEL_OWNER,TRUE,FALSE,MSG_PROP " %s HOSTKEY :%s",chChannel->szName,chChannel->szPropHostkey);
						Server_BroadcastFromUser(client,TOK_PROP,MSG_PROP,"%s HOSTKEY :%s",chChannel->szName,chChannel->szPropHostkey);
					}
					else
					/* Denied! */
						Client_SendNumericToOne(client,IRCERR_SECURITY,":No permissions to perform command");
				}
				else if (lstrcmpi(mi->argv[2],"MEMBERKEY") == 0)
				/* Can be set by admins and owners */
				{
					if ((Client_IsAdmin(client) && bIsOnChannel) || dwPriveledge & CHANNEL_PRIVELEDGE_OWNER)
					/* Set memberkey (limited to 31 characters) */
					{
						strncpy(chChannel->szPropMemberkey,mi->argv[3],31);
						Channel_BroadcastToLocalSelected(chChannel,client,NULL,ACCESSLEVEL_OWNER,TRUE,FALSE,MSG_PROP " %s MEMBERKEY :%s",chChannel->szName,chChannel->szPropMemberkey);
						Server_BroadcastFromUser(client,TOK_PROP,MSG_PROP,"%s MEMBERKEY :%s",chChannel->szName,chChannel->szPropMemberkey);
						Event_Broadcast(client,EVENT_TYPE_CHANNEL,"CHANNEL KEYWORD %s %s!%s@%s %s:%u :%s",chChannel->szName,Client_Nickname(client),client->user->username,client->hostname,client->ip_r,client->port_r,chChannel->szPropMemberkey);
					}
					else
					/* Denied! */
						Client_SendNumericToOne(client,IRCERR_SECURITY,":No permissions to perform command");
				}
				else if (lstrcmpi(mi->argv[2],"PICS") == 0)
				/* Can be set by admins only */
				{
					if ((Client_IsAdmin(client) && bIsOnChannel))
					/* Set PICS (limited to 255 characters) */
					{
						strncpy(chChannel->szPropPICS,mi->argv[3],255);

						Channel_BroadcastToLocal(chChannel,client,NULL,TOK_PROP,MSG_PROP,"%s PICS :%s",chChannel->szName,chChannel->szPropPICS);
						Server_BroadcastFromUser(client,TOK_PROP,MSG_PROP,"%s PICS :%s",chChannel->szName,chChannel->szPropPICS);
					}
					else
					/* Denied! */
						Client_SendNumericToOne(client,IRCERR_SECURITY,":No permissions to perform command");
				}
				else if (lstrcmpi(mi->argv[2],"TOPIC") == 0)
				/* Can be set by admins, owners, and hosts */
				{
					if ((Client_IsAdmin(client) && bIsOnChannel) || dwPriveledge & CHANNEL_PRIVELEDGE_OWNER || dwPriveledge & CHANNEL_PRIVELEDGE_HOST)
					/* Almost ready to set topic...just check filter first */
					{
						FilterList *flptr;
						BOOL bFiltered = FALSE;

						/* Check through list of disallowed channel names */
						if (SettingsInfo.isFilter.enabled)
						{
							flptr = &SettingsInfo.isFilter.filterhead;

							while (flptr->next)
							{
								flptr = flptr->next;

								if ((flptr->type == FILTER_TYPE_TOPIC || flptr->type == FILTER_TYPE_ALL) && match(flptr->word,mi->argv[3]) == 0)
								{
									bFiltered = TRUE;
									break;
								}
							}
						}

						if ((!Client_IsPriveledged(client) && bFiltered))
							Client_SendNumericToOne(client,RPL_NOTOPIC,chChannel->szName);
						else
						{
							strncpy(chChannel->szPropTopic,mi->argv[3],(SettingsInfo.isSecurity.max_topiclen ? SettingsInfo.isSecurity.max_topiclen : 768));
							Channel_BroadcastToLocal(chChannel,client,NULL,TOK_PROP,MSG_PROP,"%s TOPIC :%s",chChannel->szName,chChannel->szPropTopic);
							Server_BroadcastFromUser(client,TOK_PROP,MSG_PROP,"%s TOPIC :%s",chChannel->szName,chChannel->szPropTopic);
							Event_Broadcast(client,EVENT_TYPE_CHANNEL,"CHANNEL TOPIC %s %s!%s@%s %s:%u :%s",chChannel->szName,Client_Nickname(client),client->user->username,client->hostname,client->ip_r,client->port_r,chChannel->szPropTopic);
						}
					}
					else
					/* Denied! */
						Client_SendNumericToOne(client,IRCERR_SECURITY,":No permissions to perform command");
				}
				else if (lstrcmpi(mi->argv[2],"SUBJECT") == 0)
				/* Can be set by admins, owners, and hosts */
				{
					if ((Client_IsAdmin(client) && bIsOnChannel) || dwPriveledge & CHANNEL_PRIVELEDGE_OWNER || dwPriveledge & CHANNEL_PRIVELEDGE_HOST)
					/* Set subject (limited to 31 characters) */
					{
						strncpy(chChannel->szPropSubject,mi->argv[3],31);
							Channel_BroadcastToLocal(chChannel,client,NULL,TOK_PROP,MSG_PROP,"%s SUBJECT :%s",chChannel->szName,chChannel->szPropSubject);
							Server_BroadcastFromUser(client,TOK_PROP,MSG_PROP,"%s SUBJECT :%s",chChannel->szName,chChannel->szPropSubject);
					}
					else
					/* Denied! */
						Client_SendNumericToOne(client,IRCERR_SECURITY,":No permissions to perform command");
				}
				else if (lstrcmpi(mi->argv[2],"CLIENT") == 0)
				/* Can be set by admins, owners, and hosts */
				{
					if ((Client_IsAdmin(client) && bIsOnChannel) || dwPriveledge & CHANNEL_PRIVELEDGE_OWNER || dwPriveledge & CHANNEL_PRIVELEDGE_HOST)
					/* Set client (limited to 255 characters) */
					{
						strncpy(chChannel->szPropClient,mi->argv[3],255);
							Channel_BroadcastToLocal(chChannel,client,NULL,TOK_PROP,MSG_PROP,"%s CLIENT :%s",chChannel->szName,chChannel->szPropClient);
							Server_BroadcastFromUser(client,TOK_PROP,MSG_PROP,"%s CLIENT :%s",chChannel->szName,chChannel->szPropClient);
					}
					else
					/* Denied! */
						Client_SendNumericToOne(client,IRCERR_SECURITY,":No permissions to perform command");
				}
				else if (lstrcmpi(mi->argv[2],"ONJOIN") == 0)
				/* Can be set by admins, owners, and hosts */
				{
					if ((Client_IsAdmin(client) && bIsOnChannel) || dwPriveledge & CHANNEL_PRIVELEDGE_OWNER || dwPriveledge & CHANNEL_PRIVELEDGE_HOST)
					/* Set onjoin (limited to 255 characters) */
					{
						strncpy(chChannel->szPropOnjoin,mi->argv[3],255);
							Channel_BroadcastToLocal(chChannel,client,NULL,TOK_PROP,MSG_PROP,"%s ONJOIN :%s",chChannel->szName,chChannel->szPropOnjoin);
							Server_BroadcastFromUser(client,TOK_PROP,MSG_PROP,"%s ONJOIN :%s",chChannel->szName,chChannel->szPropOnjoin);
					}
					else
					/* Denied! */
						Client_SendNumericToOne(client,IRCERR_SECURITY,":No permissions to perform command");
				}
				else if (lstrcmpi(mi->argv[2],"ONPART") == 0)
				/* Can be set by admins, owners, and hosts */
				{
					if ((Client_IsAdmin(client) && bIsOnChannel) || dwPriveledge & CHANNEL_PRIVELEDGE_OWNER || dwPriveledge & CHANNEL_PRIVELEDGE_HOST)
					/* Set onpart (limited to 255 characters) */
					{
						strncpy(chChannel->szPropOnpart,mi->argv[3],255);
							Channel_BroadcastToLocal(chChannel,client,NULL,TOK_PROP,MSG_PROP,"%s ONPART :%s",chChannel->szName,chChannel->szPropOnpart);
							Server_BroadcastFromUser(client,TOK_PROP,MSG_PROP,"%s ONPART :%s",chChannel->szName,chChannel->szPropOnpart);
					}
					else
					/* Denied! */
						Client_SendNumericToOne(client,IRCERR_SECURITY,":No permissions to perform command");
				}
				else if (lstrcmpi(mi->argv[2],"LAG") == 0)
				/* Can be set by admins and owners */
				{
					if ((Client_IsAdmin(client) && bIsOnChannel) || dwPriveledge & CHANNEL_PRIVELEDGE_OWNER)
					/* Set lag */
					{
						chChannel->dwPropLag = atol(mi->argv[3]);
						Channel_BroadcastToLocal(chChannel,client,NULL,TOK_PROP,MSG_PROP,"%s LAG :%d",chChannel->szName,chChannel->dwPropLag);
						Server_BroadcastFromUser(client,TOK_PROP,MSG_PROP,"%s LAG :%d",chChannel->szName,chChannel->dwPropLag);
					}
					else
					/* Denied! */
						Client_SendNumericToOne(client,IRCERR_SECURITY,":No permissions to perform command");
				}
				else if (lstrcmpi(mi->argv[2],"ACCOUNT") == 0)
				/* Can be set by admins and owners */
				{
					if ((Client_IsAdmin(client) && bIsOnChannel))
					/* Set account (limited to 31 characters) */
					{
						strncpy(chChannel->szPropAccount,mi->argv[3],31);
							Channel_BroadcastToLocal(chChannel,client,NULL,TOK_PROP,MSG_PROP,"%s ACCOUNT :%s",chChannel->szName,chChannel->szPropAccount);
							Server_BroadcastFromUser(client,TOK_PROP,MSG_PROP,"%s ACCOUNT :%s",chChannel->szName,chChannel->szPropAccount);
					}
					else
					/* Denied! */
						Client_SendNumericToOne(client,IRCERR_SECURITY,":No permissions to perform command");
				}
				else if (lstrcmpi(mi->argv[2],"CLIENTGUID") == 0)
				/* Can be set by admins and owners */
				{
					if ((Client_IsAdmin(client) && bIsOnChannel) || dwPriveledge & CHANNEL_PRIVELEDGE_OWNER)
					/* Set ClientGUID */
					{
						chChannel->dwPropClientGUID = atol(mi->argv[3]);
						Channel_BroadcastToLocal(chChannel,client,NULL,TOK_PROP,MSG_PROP,"%s CLIENTGUID :%d",chChannel->szName,chChannel->dwPropClientGUID);
						Server_BroadcastFromUser(client,TOK_PROP,MSG_PROP,"%s CLIENTGUID :%d",chChannel->szName,chChannel->dwPropClientGUID);
					}
					else
					/* Denied! */
						Client_SendNumericToOne(client,IRCERR_SECURITY,":No permissions to perform command");
				}
				else if (lstrcmpi(mi->argv[2],"SERVICEPATH") == 0)
				/* Can be set by admins and owners */
				{
					if ((Client_IsAdmin(client) && bIsOnChannel) || dwPriveledge & CHANNEL_PRIVELEDGE_OWNER)
					/* Set ClientGUID */
					{
						chChannel->dwPropServicepath = atol(mi->argv[3]);
						Channel_BroadcastToLocal(chChannel,client,NULL,TOK_PROP,MSG_PROP,"%s SERVICEPATH :%d",chChannel->szName,chChannel->dwPropServicepath);
						Server_BroadcastFromUser(client,TOK_PROP,MSG_PROP,"%s SERVICEPATH :%d",chChannel->szName,chChannel->dwPropServicepath);
					}
					else
					/* Denied! */
						Client_SendNumericToOne(client,IRCERR_SECURITY,":No permissions to perform command");
				}
				else
				/* Bad property specified */
					Client_SendNumericToOne(client,IRCERR_BADPROPERTY,mi->argv[1]);

			}

		}
	}
	else if (csTarget)
	/* Properties are directed towards a user */
	{
		if (mi->argv[2][0] == '*' && mi->argv[2][1] == 0)
		/* Command not supported by object */
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :Command not supported by object",SettingsInfo.isGeneral.servername,IRCERR_NOTSUPPORTED,Client_Nickname(client),mi->argv[1]);
		else
		/* Bad property specified */
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :Bad property specified",SettingsInfo.isGeneral.servername,IRCERR_BADPROPERTY,Client_Nickname(client),mi->argv[1]);
	}	
	else
		Client_SendToOne(client,FALSE,":%s %3.3d %s %s :No such object found",SettingsInfo.isGeneral.servername,IRCERR_NOSUCHOBJECT,Client_Nickname(client),mi->argv[1]);

	return 0;
}
int MP_Part()
/*
** MP_Part()
** This function is called whenever a user parts a channel
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CLIENT_STRUCT *client = mi->c_prefixfrom;
	CHANNEL_STRUCT *chTarget = NULL;
	LINKED_LIST_STRUCT llChannelHead, *llChannelPtr = &llChannelHead;
	int tokcount = 0;
	char currchannel[512];

	if (mi->argc < 2)
	{
		Client_SendNumericToOne(client,ERR_NEEDMOREPARAMS,mi->argv[0]);
		return 0;
	}

	/* Create our list of channels */
	Parse_ChannelList(mi->argv[1],&llChannelHead);

	while (llChannelPtr->next)
	/* Go thru our list of channels passed */
	{
		tokcount++;
		llChannelPtr = llChannelPtr->next;

		chTarget = (CHANNEL_STRUCT*)(llChannelPtr->data);
		gettok(currchannel,mi->argv[1],tokcount,',');

		if (chTarget == NULL || Channel_ReggedAndEmpty(chTarget))
		/* Channel doesn't exist */
			Client_SendNumericToOne(client,ERR_NOSUCHCHANNEL,currchannel);
		else
		/* Channel found, let's see if they're on it */
		{
			if (Client_IsOnChannel(client,chTarget,NULL,NULL))
			/* Let's get them outta there! */
			{
				Channel_BroadcastToLocal(chTarget,client,NULL,TOK_PART,MSG_PART,chTarget->szName);
				Server_BroadcastFromUser(client,TOK_PART,MSG_PART,chTarget->szName);

				Channel_DeleteUser(chTarget,client);

				Event_Broadcast(client,EVENT_TYPE_MEMBER,"MEMBER PART %s %s!%s@%s %s:%u",chTarget->szName,Client_Nickname(client),client->user->username,client->hostname,client->ip_r,client->port_r);


				if (chTarget->dwUsers < 1)
				{
					Event_Broadcast(client,EVENT_TYPE_CHANNEL,"CHANNEL DESTROY %s",chTarget->szName);
					Channel_Cleanup(chTarget,TRUE);
				}
			}
			else
			/* They aren't on that channel */
				Client_SendNumericToOne(client,ERR_NOTONCHANNEL,currchannel);
		}
	}

	LL_Clear(&llChannelHead);

	return 0;
}

int MP_Lusers()
/*
** MP_Lusers()
** Called whenever a lusers request is issued by a client
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;

	Client_SendLusers(mi->c_from);

	return 0; 
}
int MP_Whisper()
/*
** MP_Whisper
** "Whispers" another client, which is basically a PRIVMSG coupled with a channel name, therefore providing context
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CLIENT_STRUCT *client = mi->c_prefixfrom;
	CHANNEL_STRUCT *chTarget = NULL;
	DESTINATION_INFO_STRUCT *disInfo;
	DWORD dwPriveledge, dwTargetPriv;
	LINKED_LIST_STRUCT llDestinationHead, *llNode = &llDestinationHead;

	llDestinationHead.data = NULL;
	llDestinationHead.next = NULL;

	if (mi->argc < 4)
		Client_SendNumericToOne(client,ERR_NEEDMOREPARAMS,mi->argv[0]);
	else
	{
		Parse_DestinationList(mi->argv[2],&llDestinationHead);

		chTarget = Channel_Find(mi->argv[1]);

		if (!chTarget || Channel_ReggedAndEmpty(chTarget))
		/* Channel was not found */
			Client_SendNumericToOne(client,ERR_NOSUCHCHANNEL,mi->argv[1]);
		else
		/* Channel exists, find out if they're on it */
		{
			if (!(Client_IsOnChannel(client,chTarget,&dwPriveledge,NULL)))
			/* Requesting user is not on the channel */
				Client_SendNumericToOne(client,ERR_NOTONCHANNEL,mi->argv[1]);
			else
			/* Requesting user is on the channel */
			{
				if (Channel_Moderated(chTarget) && dwPriveledge == CHANNEL_PRIVELEDGE_NONE)
				/* No whispers in +m channel */
					Client_SendNumericToOne(client,ERR_CANNOTSENDTOCHAN,mi->argv[1]);
				else
				{
					while (llNode->next)
					{
						llNode = llNode->next;

						disInfo = (DESTINATION_INFO_STRUCT*)llNode->data;
						
						if (!disInfo->csUser)
						/* Target user does not exist */
							Client_SendNumericToOne(client,ERR_NOSUCHNICK,disInfo->szOriginalText);
						else
						/* Target user exists, see if they're on the channel */
						{
							if (Client_IsOnChannel(disInfo->csUser,chTarget,&dwTargetPriv,NULL))
							/* User is on channel, check for no whispers mode */
							{
								if (Channel_NoWhispers(chTarget) &&
									(((dwPriveledge & CHANNEL_PRIVELEDGE_HOST) == 0 && (dwPriveledge & CHANNEL_PRIVELEDGE_OWNER) == 0) &&
									((dwTargetPriv & CHANNEL_PRIVELEDGE_HOST) == 0 && (dwTargetPriv & CHANNEL_PRIVELEDGE_OWNER) == 0)))
								/* No whispers */
									Client_SendNumericToOne(client,IRCERR_NOWHISPER,chTarget->szName);
								else
								/* Send whisper */
									Client_SendMessage(disInfo->csUser,client,TOK_WHISPER,MSG_WHISPER,"%s %s :%s",chTarget->szName,disInfo->szOriginalText,mi->argv[3]);
							}
							else
							/* User is not on that channel! */
								Client_SendToOne(client,FALSE,":%s %3.3d %s %s %s :They aren't on that channel",SettingsInfo.isGeneral.servername,ERR_USERNOTINCHANNEL,Client_Nickname(client),Client_Nickname(disInfo->csUser),chTarget->szName);
						}
					}
				}
			}
		}

		LL_ClearAndFree(&llDestinationHead);
	}
	return 0;
}
int MP_Ircx()
/*
** MP_Ircx()
** Used by clients to enter IRCX mode
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CLIENT_STRUCT *client = mi->c_from;
	char buf[256];

	client->user->bInIRCX = TRUE;

	User_GetModeString(client,buf,FALSE);
	Event_Broadcast(client,EVENT_TYPE_USER,"USER MODE %s!%s@%s %s:%u %s",Client_Nickname(client),client->user->username,client->hostname,client->ip_r,client->port_r,buf);

	MP_IsIrcx();

	return 0;
}
int MP_IsIrcx()
/*
** MP_IsIrcx()
** Used by clients to determine if the server supports IRCX
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;

	Client_SendNumericToOne(mi->c_from,IRCRPL_IRCX,NULL);

	return 0;
}
int MP_MOTD()
/*
** MP_MOTD()
** Sends the MOTD (Message of the Day) to the target user
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;

	if (mi->argc > 1)
	/* Too many parameters (Error: Server option for this command not supported */
		Client_SendToOne(mi->c_from,FALSE,":%s %3.3d %s %s :Server option for this command is not supported.",SettingsInfo.isGeneral.servername,555,Client_Nickname(mi->c_from),mi->argv[1]);
	else
	/* Show MOTD */
		Client_SendMOTD(mi->c_from);
	return 0;
}
int MP_Mode()
/*
** MP_Mode()
** Called whenever a user issues the MODE command
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CLIENT_STRUCT *client = mi->c_prefixfrom, *csTarget = NULL, *csModeTargets[MAX_MODES * 2];
	CHANNEL_STRUCT *chTarget;
	BOOL bMinus = FALSE, bForward = FALSE;
	BOOL bCheck = Client_IsLocal(client);
	int nIndex = 0, nLoop, nCount = 0;
	long dwNewLimit = 0;
	char ch, cCurrent = '\0';
	char szDisplayStr[256];

	char szModeStrings[MAX_MODES * 2][32];

#define EVENT_MASK_REGULAR	0x01
#define	EVENT_MASK_LIMIT	0x02
#define	EVENT_MASK_KEYWORD	0x04
	DWORD dwEventMask = NULL;

	memset(&szModeStrings,0,sizeof(szModeStrings));

	/* 0 = leave mode as-is */
	/* 1 = grant mode */
	/* 2 = take mode away */

	int nTransform[256];

	/* Clear transformation matrix */
	memset(&nTransform,0,sizeof(nTransform));

	if (mi->argc < 2)
	/* Not enough parameters */
	{
		Client_SendNumericToOne(client,ERR_NEEDMOREPARAMS,mi->argv[0]);
		return 0;
	}
	if (csTarget = User_Find(mi->argv[1]))
	/* User exists */
	{
		if ((Client_Invisible(csTarget) && !Client_IsPriveledged(client)) && csTarget != client)
		/* If requesting client is a non-oper, and the target user is invisible, pretend user doesn't exist */
			Client_SendNumericToOne(client,ERR_NOSUCHCHANNEL,mi->argv[1]);
		else if ((csTarget != client) && (!Client_IsPriveledged(client)))
		/* User exists and is another user, and requesting client is not priviledged */
			Client_SendNumericToOne(client,ERR_USERSDONTMATCH,NULL);
		else
		/* User is changing/displaying modes for themselves or someone else */
		{
			BOOL bWasPriveledged = Client_IsPriveledged(client);
			BOOL bWasInvisible = Client_Invisible(client);

			szDisplayStr[0] = 0;

			if (bWasPriveledged == FALSE && client != csTarget)
			/* Normal users cannot change/view modes of other users */
			{
				Client_SendNumericToOne(client,ERR_USERSDONTMATCH,NULL);
				return 0;
			}

			if (mi->argc == 2)
			/* User is displaying modes */
			{
				User_GetModeString(csTarget,szDisplayStr,(!Client_IsPriveledged(client)));
				Client_SendToOne(client,FALSE,":%s %3.3d %s +%s",SettingsInfo.isGeneral.servername,RPL_UMODEIS,Client_Nickname(csTarget),szDisplayStr);
				return 0;
			}

			while (mi->argv[2][nIndex])
			/* Go through each mode */
			{
				ch = mi->argv[2][nIndex];

				if (ch == '-')
					bMinus = TRUE;
				else if (ch == '+')
					bMinus = FALSE;
				else if (ch == 'i' || ch == 'o' || ch == 'a' || ch == 'z')
				/* Plus modes will override minus modes */
				{
					if (ch != 'z' && client != csTarget)
						Client_SendNumericToOne(client,ERR_USERSDONTMATCH,NULL);
					else if (Client_IsLocal(client) && ch == 'a' && !(client->flags & CLIENT_FLAG_ADMIN))
					/* Non admins cannot use +a/-a mode */
						Client_SendNumericToOne(client,IRCERR_SECURITY,":No permissions to perform command");
					else if (Client_IsLocal(client) && ch == 'o' && (!(client->flags & CLIENT_FLAG_ADMIN)) && (!(client->flags & CLIENT_FLAG_SYSOP)))
					/* Non admins and non-sysops cannot use +o/-o mode */
						Client_SendNumericToOne(client,IRCERR_SECURITY,":No permissions to perform command");
					else
					{
						if (ch == 'z' && ((Client_IsPriveledged(csTarget)) || client == csTarget ))
						/* Cannot set +z for opers/admins, or for yourself */
							Client_SendNumericToOne(client,ERR_USERSDONTMATCH,NULL);
						else
						/* Set mode */
							if (bMinus == FALSE)
								nTransform[ch] = 1;
							else if (nTransform[ch] != 1)
								nTransform[ch] = 2;
					}
				}
				else
					Client_SendNumericToOne(client,ERR_UMODEUNKNOWNFLAG,NULL);

				nIndex++;
			}

			/* Go through transformation matrix and apply/display changes. Plus modes displayed first and then minus modes */

			/* Add + if any modes are to be added */
			if (nTransform['a'] == 1 || nTransform['i'] == 1 || nTransform['o'] == 1 || nTransform['z'] == 1)
			{
				nIndex = 1;

				szDisplayStr[0] = '+';

				if (nTransform['a'] == 1)
				{
					csTarget->modeflag |= CLIENT_MODE_ADMIN;
					szDisplayStr[nIndex++] = 'a';
				}
				if (nTransform['i'] == 1)
				{
					csTarget->modeflag |= CLIENT_MODE_INVISIBLE;
					szDisplayStr[nIndex++] = 'i';
				}
				if (nTransform['o'] == 1)
				{
					csTarget->modeflag |= CLIENT_MODE_SYSOP;
					szDisplayStr[nIndex++] = 'o';
				}
				if (nTransform['z'] == 1)
				{
					csTarget->modeflag |= CLIENT_MODE_GAG;
					szDisplayStr[nIndex++] = 'z';
				}

				/* Add to operator count in lusers (if applicible) */
				if ((nTransform['a'] == 1 || nTransform['o'] == 1) && Client_IsPriveledged(client) && bWasPriveledged == FALSE)
					scs.lusers.nOpsOnline++;

				/* Add to invisible users count in lusers (if applicible) */
				if (nTransform['i'] == 1 && bWasInvisible == FALSE)
					scs.lusers.nInvisible++;

				szDisplayStr[nIndex] = 0;
			}
			/* Add - if any modes are to be taken away */
			if (nTransform['a'] == 2 || nTransform['i'] == 2 || nTransform['o'] == 2 || nTransform['z'] == 2)
			{
				int len = (int)strlen(szDisplayStr);

				nIndex = len;

				szDisplayStr[nIndex++] = '-';
				szDisplayStr[nIndex] = 0;

				if (nTransform['a'] == 2)
				{
					csTarget->modeflag &= ~CLIENT_MODE_ADMIN;
					szDisplayStr[nIndex++] = 'a';
				}
				if (nTransform['i'] == 2)
				{
					csTarget->modeflag &= ~CLIENT_MODE_INVISIBLE;
					szDisplayStr[nIndex++] = 'i';
				}
				if (nTransform['o'] == 2)
				{
					csTarget->modeflag &= ~CLIENT_MODE_SYSOP;
					szDisplayStr[nIndex++] = 'o';
				}
				if (nTransform['z'] == 2)
				{
					csTarget->modeflag &= ~CLIENT_MODE_GAG;
					szDisplayStr[nIndex++] = 'z';
				}

				/* Subtract from operator count in lusers (if applicible) */
				if ((nTransform['a'] == 2 || nTransform['o'] == 2) && (!Client_IsPriveledged(client)) && bWasPriveledged == TRUE)
					scs.lusers.nOpsOnline--;

				/* Subtract from invisible count in lusers (if applicible) */
				if (nTransform['i'] == 2 && bWasInvisible == TRUE)
					scs.lusers.nInvisible--;

				szDisplayStr[nIndex] = 0;
			}

			/* Display the mode changes */
			if (strlen(szDisplayStr) > 0)
			{
				char buf[256];

				if (Client_IsLocal(client))
					Client_SendToOne(client,FALSE,":%s MODE %s :%s",Client_Nickname(csTarget),Client_Nickname(csTarget),szDisplayStr);
				
				User_GetModeString(csTarget,buf,FALSE);
				Event_Broadcast(client,EVENT_TYPE_USER,"USER MODE %s!%s@%s %s:%u %s",Client_Nickname(csTarget),csTarget->user->username,csTarget->hostname,csTarget->ip_r,csTarget->port_r,buf);

				bForward = TRUE;
			}
		}
	}
	else
	/* No user, try to find a channel */
	{
		chTarget = Channel_Find(mi->argv[1]);

		if (chTarget && !Channel_ReggedAndEmpty(chTarget))
		/* Channel exists */
		{
			if (mi->argc < 3)
			/* Person is trying to display modes for channel */
			{
				char modebuf[256];
				char sendbuf[512];

				Channel_GetModeString(chTarget,modebuf);
				
				strcpy(sendbuf,chTarget->szName);
				strcat(sendbuf," ");
				strcat(sendbuf,modebuf);

				/* Display channel modes */
				Client_SendNumericToOne(client,RPL_CHANNELMODEIS,sendbuf);
			}
			else
			/* Person is either trying to change modes or get more detailed information */
			{
				DWORD dwPriveledge = 0;
				LINKED_LIST_STRUCT *llClientPriv;

				/* Get current visibility mode of channel */
				cCurrent = Channel_IsHidden(chTarget) ? 'h' : (Channel_IsSecret(chTarget) ? 's' :	(Channel_IsPrivate(chTarget) ? 'p' : '\0'));

				if (bCheck && !(Client_IsOnChannel(client,chTarget,&dwPriveledge,&llClientPriv)))
				/* User is not on channel */
					Client_SendNumericToOne(client,ERR_NOTONCHANNEL,chTarget->szName);
				else if (bCheck && (dwPriveledge & CHANNEL_PRIVELEDGE_OWNER) == 0 && (dwPriveledge & CHANNEL_PRIVELEDGE_HOST) == 0)
				/* User is on channel but not opped */
					Client_SendNumericToOne(client,ERR_CHANOPRIVSNEEDED,chTarget->szName);
				else
				/* User is on channel and is opped */
				{
					int nParamIndex = 3;
					char *szParam, cFlag = 0;
					CLIENT_STRUCT *csTarget = NULL;
					DWORD dwPriveledgeTarget = NULL, dwTempMode = NULL;
					LINKED_LIST_STRUCT llChangedHead, *llPtr = NULL, *llReturnMode = NULL;
					BOOL bInList = FALSE;

					llChangedHead.data = NULL;
					llChangedHead.next = NULL;

					while (mi->argv[2][nIndex])
					{
						ch = mi->argv[2][nIndex];

						if (ch == '-')
							bMinus = TRUE;
						else if (ch == '+')
							bMinus = FALSE;
						else if (ch == 'a' || ch == 'b' || ch == 'h' || ch == 'i' || ch == 'k' || ch == 'l' || ch == 'm' || 
							ch == 'n' || ch == 'o' || ch == 'p' || ch == 'q' || ch == 's' || ch == 't' || 
							ch == 'u' || ch == 'w' || ch == 'v')
						/* Plus modes will override minus modes */
						{
							if (ch == 'a' && !Client_IsPriveledged(client))
								Client_SendNumericToOne(client,IRCERR_SECURITY,":No permissions to perform command");
							else if (ch == 'k' || (ch == 'l' && bMinus == FALSE) || ch == 'o' || ch == 'q' || ch == 'v')
							/* These modes require parameters (except -l) */
							{
								if (mi->argc < nParamIndex + 1)
								/* Not enough parameters */
								{
									szDisplayStr[0] = 'M'; szDisplayStr[1] = 'O'; szDisplayStr[2] = 'D'; szDisplayStr[3] = 'E';
									szDisplayStr[4] = ' ';
									szDisplayStr[5] = bMinus == FALSE ? '+' : '-';
									szDisplayStr[6] = ch;
									szDisplayStr[7] = 0;
									Client_SendNumericToOne(client,ERR_NEEDMOREPARAMS,szDisplayStr);
								}
								else
								/* Process the mode change */
								{
									szParam = mi->argv[nParamIndex];

									if (ch == 'q' || ch == 'o' || ch == 'v')
									/* Mode change for a user in the channel */
									{
										csTarget = User_Find(szParam);

										if (bCheck && !csTarget)
										/* User does not exist */
											Client_SendNumericToOne(client,ERR_NOSUCHNICK,szParam);
										else if (!(Client_IsOnChannel(csTarget,chTarget,&dwPriveledgeTarget,&llReturnMode)) && bCheck)
										/* User is not on that channel */
											Client_SendToOne(client,FALSE,":%s %3.3d %s %s %s :They aren't on that channel",SettingsInfo.isGeneral.servername,ERR_USERNOTINCHANNEL,Client_Nickname(client),Client_Nickname(csTarget),chTarget->szName);
										else if (bCheck && !Client_IsPriveledged(client) && Client_IsPriveledged(csTarget))
										/* Must be an oper to change modes for an oper in the channel */
											Client_SendNumericToOne(client,ERR_NOPRIVILEGES,":Permission denied - You're not an IRC operator");
										else if (bCheck && Client_IsSysop(client) && !Client_IsAdmin(client) && Client_IsAdmin(csTarget))
										/* No permissions to perform command */
											Client_SendNumericToOne(client,IRCERR_SECURITY,":No permissions to perform command");
										else if (bCheck && ch == 'q' && ((DWORD)llClientPriv->data & CHANNEL_PRIVELEDGE_OWNER) == 0)
										/* Trying to do a +q/-q command while not owner */
											Client_SendNumericToOne(client,ERR_UNIQOPPRIVSNEEDED,chTarget->szName);
										else if (bCheck && ((DWORD)llClientPriv->data & CHANNEL_PRIVELEDGE_HOST) == 0 && ((DWORD)llClientPriv->data & CHANNEL_PRIVELEDGE_OWNER) == 0)
										/* Mode has changed and now they need ops all over again */
											Client_SendNumericToOne(client,ERR_CHANOPRIVSNEEDED,chTarget->szName);
										else
										/* Perform the mode change */
										{
											DWORD dwModeChange = 0;

											/* Must do this to avoid compiler error C2296 */
											dwTempMode = (DWORD)llReturnMode->data;

											cFlag = 0;

											if (bMinus)
											{
												if (ch == 'q')
												/* De-op if doing -q */
												{
													dwTempMode &= ~CHANNEL_PRIVELEDGE_OWNER;
													dwModeChange |= CHANNEL_PRIVELEDGE_OWNER;

													if (dwTempMode & CHANNEL_PRIVELEDGE_HOST)
													/* Remove host also */
													{
														dwTempMode &= ~CHANNEL_PRIVELEDGE_HOST;
														dwModeChange |= CHANNEL_PRIVELEDGE_HOST;
														cFlag = 'o';
													}
												}
												else if (ch == 'o')
												/* De-owner if doing -o */
												{
													dwModeChange |= CHANNEL_PRIVELEDGE_HOST;
													dwTempMode &= ~CHANNEL_PRIVELEDGE_HOST;

													if (dwTempMode & CHANNEL_PRIVELEDGE_OWNER)
													{
														dwTempMode &= ~CHANNEL_PRIVELEDGE_OWNER;
														dwModeChange |= CHANNEL_PRIVELEDGE_OWNER;
														cFlag = 'q';
													}

												}
												else if (ch == 'v')
												{
													dwTempMode &= ~CHANNEL_PRIVELEDGE_VOICE;
													dwModeChange |= CHANNEL_PRIVELEDGE_VOICE;
												}
											}
											else
											{
												if (ch == 'q')
												/* De-op if giving +q */
												{
													dwTempMode |= CHANNEL_PRIVELEDGE_OWNER;
													dwModeChange |= CHANNEL_PRIVELEDGE_OWNER;

													if (dwTempMode & CHANNEL_PRIVELEDGE_HOST)
													{
														dwTempMode &= ~CHANNEL_PRIVELEDGE_HOST;
														cFlag = 'o';
													}

												}
												else if (ch == 'o')
												/* De-owner if giving +o */
												{
													dwTempMode |= CHANNEL_PRIVELEDGE_HOST;
													dwModeChange |= CHANNEL_PRIVELEDGE_HOST;

													if (dwTempMode & CHANNEL_PRIVELEDGE_OWNER)
													{
														dwTempMode &= ~CHANNEL_PRIVELEDGE_OWNER;
														cFlag = 'q';
													}
												}
												else if (ch == 'v')
												{
													dwTempMode |= CHANNEL_PRIVELEDGE_VOICE;
													dwModeChange |= CHANNEL_PRIVELEDGE_VOICE;													
												}
											}

											szDisplayStr[0] = bMinus == FALSE ? '+' : '-';
											szDisplayStr[1] = ch;
											szDisplayStr[2] = 0;

											/* Re-assign mode mask */
											llReturnMode->data = (void*)dwTempMode;

											/* Search through lists of users already affected by usermode changes */
											bInList = FALSE;
											llPtr = &llChangedHead;

											/* Search existing list for user */
											while (llPtr->next)
											{
												llPtr = llPtr->next;

												if (csTarget == ((MODE_SWITCHOVER_INFO*)(llPtr->data))->csTarget)
												/* In the list */
												{
													bInList = TRUE;
													((MODE_SWITCHOVER_INFO*)(llPtr->data))->dwModeTo = (bMinus == FALSE ? (dwTempMode) : (dwPriveledgeTarget &= ~dwModeChange ));
													((MODE_SWITCHOVER_INFO*)(llPtr->data))->bMinus = bMinus;
													break;
												}
											}

											if (!bInList)
											/* Create new mode change effect list */
											{
												MODE_SWITCHOVER_INFO *msi;

												msi = (MODE_SWITCHOVER_INFO*)(calloc(1,sizeof(MODE_SWITCHOVER_INFO)));

												msi->csTarget = csTarget;
												msi->dwModeFrom = dwPriveledgeTarget;
												msi->dwModeTo = (bMinus == FALSE ? (dwTempMode) : (dwPriveledgeTarget &= ~dwModeChange ));
												msi->pdwReturnMode = (DWORD*)&(llReturnMode->data);
												msi->bMinus = bMinus;

												LL_Add(&llChangedHead,(void*)msi);
											}

											/*
											** Auditorium scenarios:
											**
											** User X is de-oped or de-ownered and is currently op/owner
											**		- All regular members will see mode change followed by a part
											**      - User X will see all non-op users part the channel
											** User X is de-oped or de-ownered and is a regular member
											**		- Only owners, hosts, and target user will see mode change
											** User X is oped or ownered and is currently op/owner
											**		- All users will see mode change
											** User X is oped or ownered and is a regular member
											**		- All regular members will see a JOIN followed by the MODE
											*/

											/* Send mode change to local clients, a set cFlag means display two mode changes */

											for (nCount = 0; szModeStrings[nCount][0] != 0; nCount++);

											if (!cFlag)
											{
												sprintf(szModeStrings[nCount],"%s %s",szDisplayStr,Client_Nickname(csTarget));
												csModeTargets[nCount] = csTarget;
											}
											else
											{
												szDisplayStr[3] = (cFlag == 'q' && bMinus) ? '-' : ((cFlag == 'o' && bMinus == TRUE) ? '-' : (bMinus ? '+' : '-'));
												szDisplayStr[4] = cFlag;
												szDisplayStr[5] = 0;

												if (cFlag == 'o' && bMinus == TRUE)
												{
													sprintf(szModeStrings[nCount + 1],"%s %s",&szDisplayStr[3],Client_Nickname(csTarget));
													sprintf(szModeStrings[nCount],"%s %s",szDisplayStr,Client_Nickname(csTarget));
												}
												else
												{
													sprintf(szModeStrings[nCount],"%s %s",&szDisplayStr[3],Client_Nickname(csTarget));
													sprintf(szModeStrings[nCount + 1],"%s %s",szDisplayStr,Client_Nickname(csTarget));
												}
												csModeTargets[nCount] = csTarget;
											}

											char szStr[16];
											int nCntTemp = 0;

											if (dwTempMode & CHANNEL_PRIVELEDGE_OWNER)
												szStr[nCntTemp++] = 'q';
											if (dwTempMode & CHANNEL_PRIVELEDGE_HOST)
												szStr[nCntTemp++] = 'o';
											if (dwTempMode & CHANNEL_PRIVELEDGE_VOICE)
												szStr[nCntTemp++] = 'v';

											szStr[nCntTemp] = 0;

											bForward = TRUE;
											Event_Broadcast(client,EVENT_TYPE_MEMBER,"MEMBER MODE %s %s!%s@%s %s:%u %s %s!%s@%s %s:%u",chTarget->szName,Client_Nickname(csTarget),csTarget->user->username,csTarget->hostname,csTarget->ip_r,csTarget->port_r,szStr,Client_Nickname(client),client->user->username,client->hostname,client->ip_r,client->port_r);

										}
									}
									else if (ch == 'k')
									{
										szDisplayStr[0] = bMinus == FALSE ? '+' : '-';
										szDisplayStr[1] = ch;
										szDisplayStr[2] = 0;

										if (bMinus)
										/* Attempting to take away key */
										{
											if (lstrcmpi(szParam,chTarget->szPropMemberkey) == 0)
											{
												for (nCount = 0; szModeStrings[nCount][0] != 0; nCount++);
												sprintf(szModeStrings[nCount],"%s %s",szDisplayStr,szParam);
												chTarget->szPropMemberkey[0] = 0;

												dwEventMask |= EVENT_MASK_KEYWORD;
											}
										}
										else
										/* User is setting the key */
										{
											if (Channel_HasMemberkey(chTarget))
											{
												strcpy(szDisplayStr,chTarget->szName);
												strcat(szDisplayStr," :Channel key already set");
												Client_SendNumericToOne(client,ERR_KEYSET,szDisplayStr);
											}
											else
											/* Key has been set */
											{
												strcpy(chTarget->szPropMemberkey,szParam);

												for (nCount = 0; szModeStrings[nCount][0] != 0; nCount++);
												sprintf(szModeStrings[nCount],"%s %s",szDisplayStr,szParam);

												dwEventMask |= EVENT_MASK_KEYWORD;
											}
										}
									}
									else if (ch == 'l')
									{
										/* Check validity of parameter */
										dwNewLimit = atol(szParam);

										if (!dwNewLimit)
										/* Not enough parameters */
										{
											szDisplayStr[0] = 'M'; szDisplayStr[1] = 'O'; szDisplayStr[2] = 'D'; szDisplayStr[3] = 'E';
											szDisplayStr[4] = ' ';
											szDisplayStr[5] = bMinus == FALSE ? '+' : '-';
											szDisplayStr[6] = ch;
											szDisplayStr[7] = 0;
											Client_SendNumericToOne(client,ERR_NEEDMOREPARAMS,szDisplayStr);
										}
										else
										/* Display mode change and set new limit */
										{
											chTarget->dwLimit = (DWORD)((unsigned long)dwNewLimit);

											sprintf(szDisplayStr,"%c%c %u",bMinus == FALSE ? '+' : '-',ch,chTarget->dwLimit);
											
											for (nCount = 0; szModeStrings[nCount][0] != 0; nCount++);
											sprintf(szModeStrings[nCount],"%s",szDisplayStr,szParam);

											dwEventMask |= EVENT_MASK_LIMIT;
										}
									}
									nParamIndex++;
								}
							}
							else if (ch == 'b')
								if (mi->argc < nParamIndex + 1)
								/* Display ban list */
								{
									for (nLoop = 0; nLoop < SettingsInfo.isSecurity.max_access; nLoop++)
									{
										if (chTarget->accList[nLoop].szHostmask[0] && chTarget->accList[nLoop].dwLevel == ACCESSLEVEL_DENY)
										/* Display ban */
											Client_SendToOne(client,FALSE,":%s %3.3d %s %s %s",SettingsInfo.isGeneral.servername,RPL_BANLIST,Client_Nickname(client),chTarget->szName,chTarget->accList[nLoop].szHostmask);
									}

									/* End of ban list */
									Client_SendToOne(client,FALSE,":%s %3.3d %s %s :End of Channel Ban List",SettingsInfo.isGeneral.servername,RPL_ENDOFBANLIST,Client_Nickname(client),chTarget->szName);									
								}
								else
								/* Ban or unban the target hostmask */
								{
									ACCESS_STRUCT accInfo;

									accInfo.dwLevel = ACCESSLEVEL_DENY;
									accInfo.bOwner = !(((DWORD)llClientPriv->data & CHANNEL_PRIVELEDGE_OWNER) == 0);
									accInfo.dwTimeout = 0;
									accInfo.szReason[0] = 0;

									szDisplayStr[0] = bMinus ? '-' : '+';
									szDisplayStr[1] = ch;
									szDisplayStr[2] = ' ';
									szDisplayStr[3] = 0;

									Validate_AccessHostmask(mi->argv[nParamIndex],accInfo.szHostmask);

									strcat(szDisplayStr,accInfo.szHostmask);

									/* Cut off the $ and the server */
									strchr(szDisplayStr,'$')[0] = 0;

									for (nCount = 0; szModeStrings[nCount][0] != 0; nCount++);

									if (bMinus)
									{
										Access_Delete(chTarget->accList,NULL,accInfo.szHostmask,ACCESSLEVEL_DENY,accInfo.bOwner);
										
										sprintf(szModeStrings[nCount],"%s",szDisplayStr,szParam);

									}
									else
									{
										sprintf(szModeStrings[nCount],"%s %s",szDisplayStr,szParam);
										Access_Add(chTarget->accList,NULL,&accInfo);
									}
								}
							else
							{
								/* Show the CHANNEL MODE or LIMIT event for all other modes */
								if (!(ch == 'l' && bMinus == TRUE))
									dwEventMask |= EVENT_MASK_REGULAR;
								else
									dwEventMask |= EVENT_MASK_LIMIT;

								if (bMinus == FALSE)
								{
									if (ch != cCurrent)
									{
										if (ch == 'h' || ch == 's' || ch == 'p')
										/* Set current visibility mode */
											nTransform['h'] = nTransform['s'] = nTransform['p'] = 0;

										nTransform[ch] = 1;
									}
								}
								else if (nTransform[ch] != 1)
									nTransform[ch] = 2;
							}
						}
						else
						{
							szDisplayStr[0] = ch; szDisplayStr[1] = 0;
							Client_SendNumericToOne(client,ERR_UNKNOWNMODE,szDisplayStr);
						}

						nIndex++;
					}
					
					/* Go through transformation matrix and apply/display changes. Plus modes displayed first and then minus modes */

					memset(&szDisplayStr,0,sizeof(szDisplayStr));

					char cVisibility = '\0';

					/* Add + if any modes are to be added */
					if (nTransform['a'] == 1 || nTransform['h'] == 1 || nTransform['i'] == 1 || nTransform['m'] == 1 ||
						nTransform['n'] == 1 || nTransform['p'] == 1 || nTransform['s'] == 1 || nTransform['t'] == 1 || 
						nTransform['u'] == 1 || nTransform['w'] == 1)
					{
						nIndex = 1;

						szDisplayStr[0] = '+';

						/* Test for private/secret/hidden */
						if (nTransform['h'] == 1 || nTransform['s'] == 1 || nTransform['p'] == 1)
						{
							cVisibility = nTransform['h'] == 1 ? 'h' : (nTransform['s'] == 1 ? 's' : 'p');

							chTarget->dwModes |= (nTransform['h'] == 1 ? CHANNELMODE_HIDDEN : (nTransform['s'] == 1 ? CHANNELMODE_SECRET : CHANNELMODE_PRIVATE));

							/* Subtract the existing mode */

							if (cCurrent != '\0')
								nTransform[cCurrent] = 2;

							szDisplayStr[nIndex++] = cVisibility;
						}

						if (nTransform['m'] == 1)
						{
							chTarget->dwModes |= CHANNELMODE_MODERATED;
							szDisplayStr[nIndex++] = 'm';
						}
						if (nTransform['n'] == 1)
						{
							chTarget->dwModes |= CHANNELMODE_NOEXTERN;
							szDisplayStr[nIndex++] = 'n';
						}
						if (nTransform['t'] == 1)
						{
							chTarget->dwModes |= CHANNELMODE_TOPICOP;
							szDisplayStr[nIndex++] = 't';
						}
						if (nTransform['i'] == 1)
						{
							chTarget->dwModes |= CHANNELMODE_INVITE;
							szDisplayStr[nIndex++] = 'i';
						}
						if (nTransform['a'] == 1)
						{
							chTarget->dwModes |= CHANNELMODE_AUTHONLY;
							szDisplayStr[nIndex++] = 'a';
						}
						if (nTransform['u'] == 1)
						{
							chTarget->dwModes |= CHANNELMODE_KNOCK;
							szDisplayStr[nIndex++] = 'u';
						}
						if (nTransform['w'] == 1)
						{
							chTarget->dwModes |= CHANNELMODE_NOWHISPER;
							szDisplayStr[nIndex++] = 'w';
						}

						szDisplayStr[nIndex] = 0;
					}

					/* Add - if any modes are to be taken away */
					if (nTransform['a'] == 2 || nTransform['h'] == 2 || nTransform['i'] == 2 || nTransform['m'] == 2 ||
						nTransform['n'] == 2 || nTransform['p'] == 2 || nTransform['s'] == 2 || nTransform['t'] == 2 || 
						nTransform['u'] == 2 || nTransform['w'] == 2 || nTransform['l'] == 2)
					{
						int len = (int)strlen(szDisplayStr);

						nIndex = len;

						szDisplayStr[nIndex++] = '-';
						szDisplayStr[nIndex] = 0;

						if (nTransform['h'] == 2 || nTransform['s'] == 2 || nTransform['p'] == 2)
						{
							if (cCurrent != '\0' && nTransform[cCurrent] == 2)
							{
								//chTarget->dwModes &= ~(nTransform['h'] == 2 ? CHANNELMODE_HIDDEN : (nTransform['s'] == 2 ? CHANNELMODE_SECRET : CHANNELMODE_PRIVATE));
								chTarget->dwModes &= ~(cCurrent == 'h' ? CHANNELMODE_HIDDEN : (cCurrent == 's' ? CHANNELMODE_SECRET : CHANNELMODE_PRIVATE));

								szDisplayStr[nIndex++] = cCurrent;
							}
						}

						if (nTransform['m'] == 2)
						{
							chTarget->dwModes &= ~CHANNELMODE_MODERATED;
							szDisplayStr[nIndex++] = 'm';
						}
						if (nTransform['n'] == 2)
						{
							chTarget->dwModes &= ~CHANNELMODE_NOEXTERN;
							szDisplayStr[nIndex++] = 'n';
						}
						if (nTransform['t'] == 2)
						{
							chTarget->dwModes &= ~CHANNELMODE_TOPICOP;
							szDisplayStr[nIndex++] = 't';
						}
						if (nTransform['i'] == 2)
						{
							LL_Clear(&chTarget->llInviteHead);
							chTarget->dwModes &= ~CHANNELMODE_INVITE;
							szDisplayStr[nIndex++] = 'i';
						}
						if (nTransform['a'] == 2)
						{
							chTarget->dwModes &= ~CHANNELMODE_AUTHONLY;
							szDisplayStr[nIndex++] = 'a';
						}
						if (nTransform['u'] == 2)
						{
							chTarget->dwModes &= ~CHANNELMODE_KNOCK;
							szDisplayStr[nIndex++] = 'u';
						}
						if (nTransform['w'] == 2)
						{
							chTarget->dwModes &= ~CHANNELMODE_NOWHISPER;
							szDisplayStr[nIndex++] = 'w';
						}
						if (nTransform['l'] == 2)
						{
							chTarget->dwLimit = 0;
							szDisplayStr[nIndex++] = 'l';
						}

						szDisplayStr[nIndex] = 0;

						if (szDisplayStr[nIndex - 1] == '-')
							szDisplayStr[nIndex - 1] = 0;
					}

					/* Display the mode changes */
					if (strlen(szDisplayStr) > 0)
					{
						for (nCount = 0; szModeStrings[nCount][0] != 0; nCount++);
						sprintf(szModeStrings[nCount],"%s",szDisplayStr);
					}

					/* Mode changes and consequences */
					LINKED_LIST_STRUCT *llUserPtr = &chTarget->llUserHead, *llUserModePtr = &chTarget->llUserModeHead;
					CLIENT_STRUCT *csClient = NULL;
					MODE_SWITCHOVER_INFO *msi = NULL;
					DWORD dwUserMode = 0;


					/* Assign all remaining usermode changes  */
					llPtr = &llChangedHead;

					while (llPtr->next)
					{
						llPtr = llPtr->next;

						msi = (MODE_SWITCHOVER_INFO*)llPtr->data;

						*msi->pdwReturnMode = msi->dwModeTo;
					}

					/* Show mode change event? */
					if (dwEventMask)
					{
						if (dwEventMask & EVENT_MASK_REGULAR)
						{
							char szModeStr[256];

							Channel_GetModeString(chTarget,szModeStr);
							
							Event_Broadcast(client,EVENT_TYPE_CHANNEL,"CHANNEL MODE %s %s %s!%s@%s %s:%u",chTarget->szName,szModeStr,Client_Nickname(client),client->user->username,client->hostname,client->ip_r,client->port_r);
						}
						if (dwEventMask & EVENT_MASK_LIMIT)
							Event_Broadcast(client,EVENT_TYPE_CHANNEL,"CHANNEL LIMIT %s %s!%s@%s %s:%u :%u",chTarget->szName,Client_Nickname(client),client->user->username,client->hostname,client->ip_r,client->port_r,chTarget->dwLimit);
						if (dwEventMask & EVENT_MASK_KEYWORD)
							Event_Broadcast(client,EVENT_TYPE_CHANNEL,"CHANNEL KEYWORD %s %s!%s@%s %s:%u :%s",chTarget->szName,Client_Nickname(client),client->user->username,client->hostname,client->ip_r,client->port_r,chTarget->szPropMemberkey);

						/* Forward the MODE command to other servers */
						bForward = TRUE;

					}

					if (!Channel_Auditorium(chTarget))
					{
						/* Show usermode changes to everybody! */
						for (nCount = 0; szModeStrings[nCount][0]; nCount++)
						{
							if (szModeStrings[nCount][1] == 'q' || szModeStrings[nCount][1] == 'o' || szModeStrings[nCount][1] == 'v')
							{
								Channel_BroadcastToLocal(chTarget,client,NULL,TOK_MODE,MSG_MODE,"%s %s",chTarget->szName,szModeStrings[nCount]);
							}
						}
					}
					else
					{
						/* Show JOINs, loop through affected users w/+ changes */
						llPtr = &llChangedHead;

						while (llPtr->next)
						{
							llPtr = llPtr->next;

							msi = (MODE_SWITCHOVER_INFO*)llPtr->data;

							if ((((msi->dwModeFrom & CHANNEL_PRIVELEDGE_HOST) == 0) && ((msi->dwModeFrom & CHANNEL_PRIVELEDGE_OWNER) == 0)) &&
								((msi->dwModeTo & CHANNEL_PRIVELEDGE_HOST) || (msi->dwModeTo & CHANNEL_PRIVELEDGE_OWNER)))
							/* Targeted user was a regular member, and has become an op/owner */
							{
								/* #1 Show JOIN to all regular members */
								/* #2 Show all regular members JOINing to targeted user */

								while (llUserPtr->next)
								/* Loop through channel */
								{
									llUserPtr = llUserPtr->next;
									llUserModePtr = llUserModePtr->next;

									csClient = (CLIENT_STRUCT*)llUserPtr->data;
									dwUserMode = (DWORD)llUserModePtr->data;

									if (dwUserMode & CHANNEL_PRIVELEDGE_HOST || dwUserMode & CHANNEL_PRIVELEDGE_OWNER)
									/* User is a host/owner */
									{
									}
									else
									/* User is a regular member */
									{
										if (csClient != msi->csTarget)
										{
											if (User_DoWeMask(msi->csTarget,csClient))
												Client_SendToOne(csClient,FALSE,":%s!~%s@%s JOIN %s",msi->csTarget->user->nickname,msi->csTarget->user->username,msi->csTarget->hostmask,chTarget->szName);
											else
												Client_SendToOne(csClient,FALSE,":%s!~%s@%s JOIN %s",msi->csTarget->user->nickname,msi->csTarget->user->username,msi->csTarget->hostname,chTarget->szName);

											if (User_DoWeMask(csClient,msi->csTarget))
												Client_SendToOne(msi->csTarget,FALSE,":%s!~%s@%s JOIN %s",csClient->user->nickname,csClient->user->username,csClient->hostmask,chTarget->szName);
											else
												Client_SendToOne(msi->csTarget,FALSE,":%s!~%s@%s JOIN %s",csClient->user->nickname,csClient->user->username,csClient->hostname,chTarget->szName);
										}
									}
								}
							}
						}

						/* Show usermode changes */
						for (nCount = 0; szModeStrings[nCount][0]; nCount++)
						{
							if (szModeStrings[nCount][1] == 'q' || szModeStrings[nCount][1] == 'o' || szModeStrings[nCount][1] == 'v')
							{
								if (szModeStrings[nCount][1] != 'v')
									Channel_BroadcastToLocal(chTarget,client,NULL,TOK_MODE,MSG_MODE,"%s %s",chTarget->szName,szModeStrings[nCount]);
								else
								{
									Channel_BroadcastToLocalOps(chTarget,client,csModeTargets[nCount],"MODE %s %s",chTarget->szName,szModeStrings[nCount]);
									Client_SendMessage(csModeTargets[nCount],client,TOK_MODE,MSG_MODE,"%s %s",chTarget->szName,szModeStrings[nCount]);
								}
							}
						}

						/* Show PARTs, loop through affected users w/- changes */
						llUserPtr = &chTarget->llUserHead;
						llUserModePtr = &chTarget->llUserModeHead;
						llPtr = &llChangedHead;

						while (llPtr->next)
						{
							llPtr = llPtr->next;

							msi = (MODE_SWITCHOVER_INFO*)llPtr->data;

							if (((msi->dwModeFrom & CHANNEL_PRIVELEDGE_HOST) || (msi->dwModeFrom & CHANNEL_PRIVELEDGE_OWNER)) &&
								(((msi->dwModeTo & CHANNEL_PRIVELEDGE_HOST) == 0) && ((msi->dwModeTo & CHANNEL_PRIVELEDGE_OWNER) == 0)))
							/* Targeted user was a host/owner, and has now become a regular member */
							{
								/* #1 Show PART to all regular members */
								/* #2 Show all regular members PARTing to targeted user */

								while (llUserPtr->next)
								/* Loop through channel */
								{
									llUserPtr = llUserPtr->next;
									llUserModePtr = llUserModePtr->next;

									csClient = (CLIENT_STRUCT*)llUserPtr->data;
									dwUserMode = (DWORD)llUserModePtr->data;

									if (dwUserMode & CHANNEL_PRIVELEDGE_HOST || dwUserMode & CHANNEL_PRIVELEDGE_OWNER)
									/* User is a host/owner */
									{
									}
									else
									/* User is a regular member */
									{
										if (csClient != msi->csTarget)
										{
											if (User_DoWeMask(msi->csTarget,csClient))
												Client_SendToOne(csClient,FALSE,":%s!~%s@%s PART %s",msi->csTarget->user->nickname,msi->csTarget->user->username,msi->csTarget->hostmask,chTarget->szName);
											else
												Client_SendToOne(csClient,FALSE,":%s!~%s@%s PART %s",msi->csTarget->user->nickname,msi->csTarget->user->username,msi->csTarget->hostname,chTarget->szName);

											if (User_DoWeMask(csClient,msi->csTarget))
												Client_SendToOne(msi->csTarget,FALSE,":%s!~%s@%s PART %s",csClient->user->nickname,csClient->user->username,csClient->hostmask,chTarget->szName);
											else
												Client_SendToOne(msi->csTarget,FALSE,":%s!~%s@%s PART %s",csClient->user->nickname,csClient->user->username,csClient->hostname,chTarget->szName);
										}
									}
								}
							}
						}

					}

					/* Show all other mode changes */
					for (nCount = 0; szModeStrings[nCount][0]; nCount++)
					{
						if (szModeStrings[nCount][1] != 'q' && szModeStrings[nCount][1] != 'o' && szModeStrings[nCount][1] != 'v')
							Channel_BroadcastToLocal(chTarget,client,NULL,TOK_MODE,MSG_MODE,"%s %s",chTarget->szName,szModeStrings[nCount]);
					}

					LL_ClearAndFree(&llChangedHead);
				}
			}
		}
		else
		/* Target does not exist */
			Client_SendNumericToOne(client,ERR_NOSUCHCHANNEL,mi->argv[1]);
	}

	if (bForward)
	{
		char szModebuf[1024];

		szModebuf[0] = 0;

		for (nCount = 1; nCount < mi->argc; nCount++)
		{
			strcat(szModebuf,mi->argv[nCount]);
			
			if (!(nCount + 1 == mi->argc))
			/* Not at end of loop? Add another space */
				strcat(szModebuf," ");
		}

		if (SettingsInfo.isServers.networktype != NETWORK_TYPE_MESH)
			Server_BroadcastFromUser(client,TOK_MODE,MSG_MODE,szModebuf);
		else
		/* On mesh networks, only forward mode changes from local clients */
			Server_BroadcastFromUser(client,TOK_MODE,MSG_MODE,szModebuf);
	}

	return 0;
}
int MP_Kick()
/*
** MP_Kick()
** Kicks the specified user out of a channel
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CLIENT_STRUCT *client = mi->c_prefixfrom, *csTarget = NULL;
	CHANNEL_STRUCT *chChannel = NULL;
	LINKED_LIST_STRUCT llDestinationHead_Ch, llDestinationHead_Users, *llNodeCh = NULL, *llNodeUser = NULL;
	DWORD dwFromPriv = NULL, dwTargetPriv = NULL;
	char *szKickText = NULL;

	memset(&llDestinationHead_Ch,0,sizeof(llDestinationHead_Ch));
	memset(&llDestinationHead_Users,0,sizeof(llDestinationHead_Users));

	if (mi->argc < 3)
	/* Not enough parameters */
		Client_SendNumericToOne(client,ERR_NEEDMOREPARAMS,mi->argv[0]);
	else
	/* Parse lists and parameters to kick user(s) */
	{
		if (mi->argc > 3)
			szKickText = mi->argv[3];
			
		Parse_DestinationList(mi->argv[1],&llDestinationHead_Ch);
		Parse_DestinationList(mi->argv[2],&llDestinationHead_Users);

		llNodeCh = &llDestinationHead_Ch;

		while (llNodeCh->next)
		{
			llNodeCh = llNodeCh->next;

			chChannel = (((DESTINATION_INFO_STRUCT*)llNodeCh->data)->chChannel);

			if (chChannel && !Channel_ReggedAndEmpty(chChannel))
			/* Channel is valid */
			{
				if (Client_IsOnChannel(client,chChannel,&dwFromPriv,NULL))
				{
					llNodeUser = &llDestinationHead_Users;

					while (llNodeUser->next)
					/* Go through user list */
					{
						llNodeUser = llNodeUser->next;

						csTarget = ((DESTINATION_INFO_STRUCT*)llNodeUser->data)->csUser;

						if (csTarget)
						/* User exists, check permissions and credentials */
						{
							if (Client_IsOnChannel(csTarget,chChannel,&dwTargetPriv,NULL))
							{
								BOOL bKick = FALSE;

								/* Check priveledges of user requesting kick */
								
								if (Client_IsAdmin(client))
								/* Admins can kick anyone, including other admins */
									bKick = TRUE;
								else if (Client_IsSysop(client) && !Client_IsAdmin(csTarget))
								/* Sysops can kick non-admins, including other sysops */
									bKick = TRUE;
								else if ((dwFromPriv & CHANNEL_PRIVELEDGE_OWNER) && !Client_IsPriveledged(csTarget))
								/* Owners can kick anyone except sysops and admins */
									bKick = TRUE;
								else if ((dwFromPriv & CHANNEL_PRIVELEDGE_HOST) && !Client_IsPriveledged(csTarget) &&
									!(dwTargetPriv & CHANNEL_PRIVELEDGE_OWNER))
								/* Hosts can kick anyone except owners, sysops, and admins */
									bKick = TRUE;
								else if (client == csTarget)
								/* Users can kick themselves */
									bKick = TRUE;

								if (bKick == TRUE)
								/* Kick 'em out!! */
								{
									Channel_BroadcastToLocal(chChannel,client,NULL,TOK_KICK,MSG_KICK,"%s %s :%s",chChannel->szName,Client_Nickname(csTarget),(szKickText ? szKickText : ""));
									Server_BroadcastFromUser(client,TOK_KICK,MSG_KICK,"%s %s :%s",chChannel->szName,Client_Nickname(csTarget),(szKickText ? szKickText : ""));
									Event_Broadcast(client,EVENT_TYPE_MEMBER,"MEMBER KICK %s %s!%s@%s %s:%u %s!%s@%s %s:%u :%s",chChannel->szName,Client_Nickname(csTarget),csTarget->user->username,csTarget->hostname,csTarget->ip_r,csTarget->port_r,Client_Nickname(client),client->user->username,client->hostname,client->ip_r,client->port_r,(szKickText ? szKickText : ""));
									
									Channel_DeleteUser(chChannel,csTarget);

									if (chChannel->dwUsers < 1)
									{
										Event_Broadcast(client,EVENT_TYPE_CHANNEL,"CHANNEL DESTROY %s",chChannel->szName);
										Channel_Cleanup(chChannel,TRUE);
									}
								}
								else
								/* Can't kick, explain why here */
								{
									if (Client_IsSysop(client))
									/* Sysops can't kick admins */
										Client_SendNumericToOne(client,IRCERR_SECURITY,":No permissions to perform command");
									else if ((dwFromPriv & CHANNEL_PRIVELEDGE_HOST) && (dwTargetPriv & CHANNEL_PRIVELEDGE_OWNER))
									/* Non-owners can't kick owners */
										Client_SendNumericToOne(client,ERR_UNIQOPPRIVSNEEDED,chChannel->szName);
									else if ((dwFromPriv & CHANNEL_PRIVELEDGE_OWNER) || (dwFromPriv & CHANNEL_PRIVELEDGE_HOST) &&
										Client_IsPriveledged(csTarget))
									/* Owners/hosts can't kick sysops/admins */
										Client_SendNumericToOne(client,ERR_NOPRIVILEGES,":Permission denied - You're not an IRC operator");
									else
									/* Need ops to do the rest! */
										Client_SendNumericToOne(client,ERR_CHANOPRIVSNEEDED,chChannel->szName);
								}
							}
							else
							/* User not on that channel */
								Client_SendToOne(client,FALSE,":%s %3.3d %s %s %s :They aren't on that channel",SettingsInfo.isGeneral.servername,ERR_USERNOTINCHANNEL,Client_Nickname(client),Client_Nickname(csTarget),chChannel->szName);
								
						}
						else
						/* No such user */
							Client_SendNumericToOne(client,ERR_NOSUCHNICK,((DESTINATION_INFO_STRUCT*)llNodeUser->data)->szOriginalText);
					}
				}
				else
				/* User is not on the channel */
					Client_SendNumericToOne(client,ERR_NOTONCHANNEL,((DESTINATION_INFO_STRUCT*)llNodeCh->data)->szOriginalText);
			}
			else
			/* Channel does not exist */
				Client_SendNumericToOne(client,ERR_NOSUCHCHANNEL,((DESTINATION_INFO_STRUCT*)llNodeCh->data)->szOriginalText);
		}

		LL_ClearAndFree(&llDestinationHead_Ch);
		LL_ClearAndFree(&llDestinationHead_Users);
		
	}
	return 0;
}
int MP_Userhost()
/*
** MP_Uesrhost()
** Called to get the hostname of the specified user
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CLIENT_STRUCT *client = mi->c_from, *c_target = NULL;
	int nCount = 0;
	char szDisplayStr[2048], szTemp[256];

	szDisplayStr[0] = 0;

	if (mi->argc < 2)
	/* Not enough parameters */
	{
		Client_SendNumericToOne(client,ERR_NEEDMOREPARAMS,mi->argv[0]);
		return 0;
	}
	else
	{
		/*
		** Only display /userhost information if one of the following conditions are met:
		** 1) Requesting user has at least 1 common channel with target user
		** 2) Requesting user is an admin or sysop
		** 3) Target party does not have usermode +i (invisible) set
		*/

		for (nCount = 2; nCount <= mi->argc; nCount++)
		{
			c_target = User_Find(mi->argv[nCount - 1]);

			if (c_target && ((c_target == client || (!Client_Invisible(c_target)) || Client_IsPriveledged(client) || User_GetCommonChannels(client,c_target,NULL,NULL))))
			/* Display user */
			{
				if (User_DoWeMask(c_target,client))
					sprintf(szTemp,"%s*=+%s@%s",Client_Nickname(c_target),c_target->user->username,c_target->hostmask);
				else
					sprintf(szTemp,"%s*=+%s@%s",Client_Nickname(c_target),c_target->user->username,c_target->hostname);

				strcat(szDisplayStr,szTemp);
				strcat(szDisplayStr," ");
			}
		}

		if (szDisplayStr[0])
		/* Cut off the trailing space */
		{
			szDisplayStr[strlen(szDisplayStr) - 1 ] = 0;
		}

		Client_SendNumericToOne(client,RPL_USERHOST,szDisplayStr);
	}

	return 0; 
}
int MP_Ison()
/*
** MP_Ison
** Checks to see if the target user is on the network/server
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CLIENT_STRUCT *client = mi->c_from, *c_target = NULL;
	int nCount = 0;
	char szDisplayStr[1024];

	szDisplayStr[0] = 0;

	if (mi->argc < 2)
	/* Not enough parameters */
	{
		Client_SendNumericToOne(client,ERR_NEEDMOREPARAMS,mi->argv[0]);
		return 0;
	}
	else
	{
		/*
		** Only display /ison information if one of the following conditions are met:
		** 1) Requesting user has at least 1 common channel with target user
		** 2) Requesting user is an admin or sysop
		** 3) Target party does not have usermode +i (invisible) set
		*/

		for (nCount = 1; nCount <= mi->argc; nCount++)
		{
			c_target = User_Find(mi->argv[nCount - 1]);

			if (c_target && ((c_target == client || (!Client_Invisible(c_target)) || Client_IsPriveledged(client) || User_GetCommonChannels(client,c_target,NULL,NULL))))
			/* Display user */
			{
				strcat(szDisplayStr,mi->argv[nCount - 1]);
				strcat(szDisplayStr," ");
			}
		}

		if (szDisplayStr[0])
		/* Cut off the trailing space */
		{
			szDisplayStr[strlen(szDisplayStr)- 1 ] = 0;
		}

		Client_SendNumericToOne(client,RPL_ISON,szDisplayStr);
	}

	return 0;
}
int MP_Silence()
/*
** MP_Silence()
** Silence is basically a server-side ignore. Takes 1 hostmask per command.
** On IRCX this simply is a frontend for client-based ACCESS DENY entries
** The following are ignored: invites, privmsgs, notices
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CLIENT_STRUCT *client = mi->c_from, *c_target = NULL;
	ACCESS_STRUCT *accInfo, accLocal;
	LINKED_LIST_STRUCT *llNode = &(client->llAccessHead);
	char *szDisplayStr[256], *szPtr = NULL, szHostMask[64];
	int retval;

	if (mi->argc < 2)
	/* Output the silence list */
	{
		while (llNode->next)
		{
			llNode = llNode->next;

			accInfo = (ACCESS_STRUCT*)llNode->data;

			if (accInfo->dwLevel == ACCESSLEVEL_DENY)
			/* Add to silence list */
			{
				sprintf((char*)szDisplayStr,"%s :%s",Client_Nickname(client),accInfo->szHostmask);
				Client_SendNumericToOne(client,RPL_SILELIST,(char*)szDisplayStr);
			}
		}

		/* End of silence list */
		Client_SendNumericToOne(client,RPL_ENDOFSILELIST,":End of Silence List");

	}
	else
	/* Add/remove from silence list */
	{
		szPtr = &mi->argv[1][((mi->argv[1][0] == '-' || mi->argv[1][0] == '+') ? 1 : 0)];
		Validate_AccessHostmask(szPtr,szHostMask);
		accInfo = Access_Find(NULL,&client->llAccessHead,szHostMask,ACCESSLEVEL_DENY);

		if (mi->argv[1][0] == '-')
		/* Remove from silence list */
		{
			if (accInfo)
			/* Remove existing */
			{
				Access_Delete(NULL,&client->llAccessHead,szHostMask,ACCESSLEVEL_DENY,TRUE);
				Client_SendToOne(client,FALSE,":%s SILENCE -%s",Client_Nickname(client),szHostMask);
			}
		}
		else
		/* Add to silence list */
		{
			if (!accInfo)
			/* Add new */
			{
				accLocal.bOwner = FALSE;
				accLocal.dwLevel = ACCESSLEVEL_DENY;
				accLocal.dwTimeout = 0;
				strcpy(accLocal.szCreator,Client_Nickname(client));
				strcpy(accLocal.szHostmask,szHostMask);
				accLocal.szReason[0] = 0;

				Access_Add(NULL,&client->llAccessHead,&accLocal);
				Client_SendToOne(client,FALSE,":%s SILENCE +%s",Client_Nickname(client),szHostMask);
			}
		}
	}

	return 0;
}
int MP_Trace() { return 0; }
int MP_Event()
/*
** MP_Event()
** Sets up client to recieve informational messages from the server
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CLIENT_STRUCT *client = mi->c_from, *c_target = NULL;

	if (mi->argc < 2)
	{
		Client_SendNumericToOne(client,ERR_NEEDMOREPARAMS,mi->argv[0]);
		return 0;
	}

	/*
	** Types of events
	** CHANNEL
	** MEMBER
	** SERVER
	** CONNECT
	** SOCKET
	** USER
	**
	** CHANNEL Sub-events:
	**
	** CREATE 
	** Usage: called when channel created.
	** Format: CHANNEL CREATE #test tn Donkey!Zeb@ip.addr.zero 192.168.1.105:2416 (Wed Jun 03 16:53:06)
	*/

	if (Client_IsPriveledged(client))
	/* Command only available to sysops and admins */
	{
		EVENT_STRUCT *esEvent = NULL;
		LINKED_LIST_STRUCT *llPtr = &(client->llEventHead);
		char szDisplayStr[256];

		if (lstrcmpi(mi->argv[1],"LIST") == 0)
		/* List all events */
		{
			Client_SendToOne(client,FALSE,":%s %3.3d %s * :Start of events",SettingsInfo.isGeneral.servername,IRCRPL_EVENTSTART,Client_Nickname(client));

			while (llPtr->next)
			{
				llPtr = llPtr->next;

				esEvent = (EVENT_STRUCT*)(llPtr->data);

				switch (esEvent->dwType)
				{
					case EVENT_TYPE_CHANNEL:
						strcpy(szDisplayStr,"Channel");
					break;
					case EVENT_TYPE_MEMBER:
						strcpy(szDisplayStr,"Member");
					break;
					case EVENT_TYPE_SERVER:
						strcpy(szDisplayStr,"Server");
					break;
					case EVENT_TYPE_CONNECT:
						strcpy(szDisplayStr,"Connect");
					break;
					case EVENT_TYPE_SOCKET:
						strcpy(szDisplayStr,"Socket");
					break;
					case EVENT_TYPE_USER:
						strcpy(szDisplayStr,"User");
					break;
				}

				strcat(szDisplayStr," ");
				strcat(szDisplayStr,esEvent->szHostmask);

				Client_SendToOne(client,FALSE,":%s %3.3d %s %s",SettingsInfo.isGeneral.servername,IRCRPL_EVENTLIST,Client_Nickname(client),szDisplayStr);
			}

			Client_SendToOne(client,FALSE,":%s %3.3d %s * :End of events",SettingsInfo.isGeneral.servername,IRCRPL_EVENTEND,Client_Nickname(client));


		}
		else if ((lstrcmpi(mi->argv[1],"ADD") == 0) || (lstrcmpi(mi->argv[1],"DELETE") == 0))
		{
			BOOL bAdd = ((mi->argv[1][0] == 'a' || mi->argv[1][0] == 'A') ? TRUE : FALSE);

			if (mi->argc < 3)
			/* Not enough parameters */
				Client_SendNumericToOne(client,ERR_NEEDMOREPARAMS,mi->argv[0]);
			else
			{
				EVENT_STRUCT esLocal;

				/* Determine type of event */
				if (lstrcmpi(mi->argv[2],"CHANNEL") == 0)
					esLocal.dwType = EVENT_TYPE_CHANNEL;
				else if (lstrcmpi(mi->argv[2],"MEMBER") == 0)
					esLocal.dwType = EVENT_TYPE_MEMBER;
				else if (lstrcmpi(mi->argv[2],"SERVER") == 0)
					esLocal.dwType = EVENT_TYPE_SERVER;
				else if (lstrcmpi(mi->argv[2],"CONNECT") == 0)
					esLocal.dwType = EVENT_TYPE_CONNECT;
				else if (lstrcmpi(mi->argv[2],"SOCKET") == 0)
					esLocal.dwType = EVENT_TYPE_SOCKET;
				else if (lstrcmpi(mi->argv[2],"USER") == 0)
					esLocal.dwType = EVENT_TYPE_USER;
				else
					esLocal.dwType = 0;

				if (esLocal.dwType == 0)
					Client_SendToOne(client,FALSE,":%s %3.3d %s %s :No such event",SettingsInfo.isGeneral.servername,IRCERR_NOSUCHEVENT,Client_Nickname(client),mi->argv[2]);
				else
				{
					LINKED_LIST_STRUCT *llPrev;

					/* If no hostmask is specified, assume *!*@* */
					if (mi->argc == 3)
						Validate_AccessHostmask("*!*@*$*",esLocal.szHostmask);
					else
						Validate_AccessHostmask(mi->argv[3],esLocal.szHostmask);

					while (llPtr->next)
					/* Go through the event list and see if we can find a match */
					{
						llPrev = llPtr;
						llPtr = llPtr->next;

						esEvent = (EVENT_STRUCT*)(llPtr->data);

						if (esEvent->dwType == esLocal.dwType && strcmpi(esEvent->szHostmask,esLocal.szHostmask) == 0)
						/* Match */
						{
							if (bAdd)
							/* Entry already exists */
								Client_SendToOne(client,FALSE,":%s %3.3d %s %s %s :Duplicate event entry",SettingsInfo.isGeneral.servername,IRCERR_EVENTDUP,Client_Nickname(client),mi->argv[2],esLocal.szHostmask);
							else
							/* Delete entry */
							{
								llPrev->next = llPtr->next;
								free(esEvent);

								/* Entry has been deleted */
								Client_SendToOne(client,FALSE,":%s %3.3d %s %s %s",SettingsInfo.isGeneral.servername,IRCRPL_EVENTDEL,Client_Nickname(client),mi->argv[2],esLocal.szHostmask);

								if (llPrev == &(client->llEventHead) && llPtr->next == NULL)
								/* Last one, remove from global list */
									LL_Remove(&(scs.llEventHead),(void*)client);

							}

							return 0;
						}

					}

					/* No match */

					if (bAdd)
					/* Add new entry */
					{
						LINKED_LIST_STRUCT *llNew = NULL;
						EVENT_STRUCT *esNew = (EVENT_STRUCT*)(calloc(1,sizeof(EVENT_STRUCT)));

						esNew->dwType = esLocal.dwType;
						strcpy(esNew->szHostmask,esLocal.szHostmask);

						llNew = LL_Add(&client->llEventHead,(void*)esNew);

						if (client->llEventHead.next == llNew)
						/* Add to global event list */
							LL_Add(&(scs.llEventHead),(void*)client);

						Client_SendToOne(client,FALSE,":%s %3.3d %s %s %s",SettingsInfo.isGeneral.servername,IRCRPL_EVENTADD,Client_Nickname(client),mi->argv[2],esLocal.szHostmask);
					}
					else
					/* Can't delete, requested entry doesn't exist */
						Client_SendToOne(client,FALSE,":%s %3.3d %s %s %s :Unknown event entry",SettingsInfo.isGeneral.servername,IRCERR_EVENTMIS,Client_Nickname(client),mi->argv[2],esLocal.szHostmask);

						
				}
			}
			
		}
		else
			Client_SendToOne(client,FALSE,":%s %3.3d %s %s :Bad function",SettingsInfo.isGeneral.servername,IRCERR_BADFUNCTION,Client_Nickname(client),mi->argv[1]);
	}
	else
		Client_SendNumericToOne(client,ERR_NOPRIVILEGES,":Permission denied - You're not an IRC operator");

	return 0;
}

int	MP_Server()
/*
** MP_Server()
** Command sent between servers only, used to register a new server on the network
** Format: SERVER <servername> <hopcount> <id#> <description>
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CLIENT_STRUCT *c_server = mi->c_prefixfrom, *c_new = NULL, *csFound;
	int hopcount = atoi(mi->argv[2]), id = atoi(mi->argv[3]);

	csFound = Server_HashFind(mi->argv[1]);

	if (!csFound)
	/* New server */
	{
		c_new = (CLIENT_STRUCT*)calloc(1,sizeof(CLIENT_STRUCT));
		c_new->server = (SERVER_STRUCT*)calloc(1,sizeof(SERVER_STRUCT));

		c_new->server->connected = SERVER_REMOTE;

		strcpy(c_new->servername,c_server->server->name);
		c_new->serverkey = Hash_MakeKey(&HT_Server,c_new->servername);

		/* Name and hash key of new server */
		strcpy(c_new->server->name,mi->argv[1]);
		c_new->server->hashkey = Hash_Add(&HT_Server,c_new->server->name,c_new);

		c_new->hops = hopcount + c_server->hops;
		
		/* Put this new server in the hash table & server linked lists */
		LL_Add(&scs.llServerHead[c_new->hops],c_new);

	}
	else
	/* Server that just connected giving information about itself */
	{
		c_new = c_server;
		scs.lusers.nLocalServers++;
	}

	/* Name and hash key of existing host server */
	c_new->server->id = id;

	strcpy(c_new->server->description,mi->argv[4]);
	c_new->servernext = mi->c_from;
	scs.lusers.nGlobalServers++;
	scs.CSServers[c_new->server->id] = c_new;

	if (hopcount <= 1)
		Serverlist_ChangeStatus(c_server->server->name,TRUE);

	/* Forward this new server to other servers, making sure to assign initial prefix to existing server */
	if (SettingsInfo.isServers.networktype != NETWORK_TYPE_MESH)
		Server_Broadcast(mi->c_from,":%d " TOK_SERVER " %s %d %d :%s",scs.sclient->server->id,c_new->server->name,c_new->hops,c_new->server->id,c_new->server->description);

	return 0;
}
int MP_SPass()
/*
** MP_SPass()
** New RockIRCX command, used for newly connecting servers.
** Format: SPASS <password> <network name> <server name> <version>
**
** Format: SPASS <password> <network type> <network name> <# of servers>
**				 <server name> <version>
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CLIENT_STRUCT *client = mi->c_from;
	ServerList *curr = &SettingsInfo.isServers.serverhead;
	BOOL bNoMatch = TRUE;
	MESH_NODE *mnPtr = NULL;

	if (mi->argc < 7)
	{
		Client_SendNumericToOne(client,ERR_NEEDMOREPARAMS,mi->argv[0]);
		return 0;
	}

	/* First check that network name & version are compatible */

	if (atoi(mi->argv[2]) != (int)SettingsInfo.isServers.networktype)
	/* Invalid network type */
	{
		Client_SendError(client,TRUE,"Closing Server Link: Network types do not match.");
		return STOP_PARSING;
	}
	else if (strcmp(mi->argv[3],SettingsInfo.isGeneral.networkname))
	/* Invalid network name */
	{
		Client_SendError(client,TRUE,"Closing Server Link: Network names do not match.");
		return STOP_PARSING;
	}
	else if (strcmp(mi->argv[6],ROCKIRCX_PROTOCOL_VERSION))
	/* Invalid version */
	{
		Client_SendError(client,TRUE,"Closing Server Link: Incorrect protocol version.");
		return STOP_PARSING;
	}

	while (curr->next)
	/* Attempt to find matching inbound server */
	{
		curr = curr->next;

		if (!curr->port && match(curr->hostmask,client->hostname) == 0)
		/* Found match for hostmask(inbound servers only) */
		{
			if (lstrcmpi(curr->name,mi->argv[5]) == 0)
			/* Found the right server */
			{
				bNoMatch = FALSE;

				if (strcmp(curr->password,mi->argv[1]) == 0)
				/* Right server & right password */
				{
					/* Client is no longer a user, but now a server */
					free(client->user);
					client->user = NULL;
					client->flags |= CLIENT_FLAG_SERVERSYNC;
					client->hops = 1;

					client->server = (SERVER_STRUCT*)calloc(1,sizeof(SERVER_STRUCT));
					client->serverkey = scs.skey;
					strcpy(client->servername,scs.sclient->server->name);

					strcpy(client->server->name,mi->argv[5]);
					client->server->hashkey = Hash_Add(&HT_Server,client->server->name,(void*)client);
					client->server->connected = SERVER_INBOUND;

					scs.lusers.nUnknown--;

					LL_Add(&scs.llServerHead[client->hops],(void*)client);

					/* Server has successfully authenticated */
					Server_SendServerInfo(client);
					client->flags &= ~CLIENT_FLAG_SERVERSYNC;
					client->flags |= CLIENT_FLAG_USERSYNC;

					return STOP_PARSING;
				}
				else
				/* Right server, wrong password */
					bNoMatch = TRUE;
			}
		}
	}

	if (bNoMatch)
	{
		char *buf;

		/* Bulletproof this buffer overflow opprotunity */
		buf = (char*)malloc(strlen(mi->argv[5]) + strlen(mi->argv[1]) + 512 );

		Client_SendError(client,TRUE,"Closing Server Link: Invalid password(Your attempt has been logged)");
		sprintf(buf,"Failed server login attempt from %s(%s), password used: %s",mi->argv[5],client->hostname,mi->argv[1]);
		TechLog_AddEntry(buf,BITMAP_WARNING);

		free(buf);

		return STOP_PARSING;
	}


	return 0;
}

int MP_NJoin()
/*
** MP_NJoin()
** This command is used in server-server communication only, and is used to update
** channel information.
** Usage: NJOIN <control mode> <name> <timestamp> <modes> <limit> :<User list>
** Control mode 1 syntax (initial): NJOIN 1 <name> <timestamp> <modes> <limit> :<User list>
** Control mode 2 syntax (access entries): NJOIN 2 <name> <# entries> :<AccessEntry1> <AccessEntry2>
** Control mode 3 syntax (properties): NJOIN 3 <name> :<Channel property list>
** Example: NJOIN #Pirates 932543 +ntxl 500 :.Blackbeard .Pegleg @Scallywag +Mutany-man PoopDeck
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CLIENT_STRUCT *c_server = mi->c_prefixfrom;
	CHANNEL_STRUCT *chChannel = Channel_Find(mi->argv[2]);
	char szBuf[1024], szAccInfo[512], szBigstr[8192];
	int nCount, nCtlMode = atoi(mi->argv[1]);

	if (nCtlMode == 1)
	/* Control mode 1: Initial listing */
	{
		BOOL bForwardNJoin = TRUE;
		unsigned long dwRemoteCreationTime = atol(mi->argv[3]),
						dwLimit = atol(mi->argv[5]);

		/* Do we already have this channel? If so, process for channel collision */
		if (chChannel)
		/*
		** Channel collision!
		** The mechanism for channel collisions is simple, a timestamp will be used.
		** If our channel is older, we need not do anything here as our subsequent NJOIN
		** message will automatically take care of things for us.
		*/
		{
			if (dwRemoteCreationTime <= chChannel->dwPropCreationTime)
			/* Their channel is older, and will take over ours. KILL our channel :( */
			{
				Channel_Kill(chChannel,c_server,"Channel collision");
			}
			else if (c_server->flags & CLIENT_FLAG_CHANNELSYNC)
			/* Kill their channel instead, but do nothing because the NJOIN we send will do the work */
				return 0;
			else
			/* Our channel is older, kill theirs! */
			{
				Client_SendToOne(c_server,FALSE,":%d " TOK_KILL " %s :Channel collision",scs.sclient->server->id,chChannel->szName);
				Server_SendChannelInfo(c_server,chChannel);

				chChannel->nNJoinStatus = 0;

				return 0;
			}
		}

		/* Make a new channel with their parameters */
		chChannel = Channel_Create(mi->argv[2]);

		chChannel->dwPropCreationTime = dwRemoteCreationTime;
		chChannel->dwLimit = dwLimit;

		/* Add our modes */
		chChannel->dwModes = Channel_GetModeFlagFromString(mi->argv[4]);

		/* Add our users(TODO: Find a way to account for users for nickname collisions!) */
		Channel_AddUsersFromList(chChannel,mi->argv[6]);

		chChannel->csServerOrigin = c_server;

		chChannel->nNJoinStatus = 1;

	}
	else if (nCtlMode == 2)
	/* Access entries */
	{
		int nTokens, nLen, nPos = 0, nEntries = atoi(mi->argv[3]);
		char *szPtr;

		/* TODO: Contemplate on either clearing all entries or merging them */

		if (nEntries > SettingsInfo.isSecurity.max_access)
			nEntries = SettingsInfo.isSecurity.max_access;

		for (nCount = 0; nCount < nEntries; nCount++)
		{
			/* Get pointer to access info and length of text */
			szPtr = strchr(&mi->argv[4][nPos],32);
			szPtr[0] = 0;
			nLen = atoi(&mi->argv[4][nPos]);
			szPtr[0] = 32;
			nPos += (szPtr - &mi->argv[4][nPos]) + nLen;

			strncpy(szAccInfo,szPtr,nLen);
			szAccInfo[nLen] = 0;

			/* Access string is seperated by 6 commas. Seperate each value */
			szPtr = gettok(szAccInfo,szBuf,44,1);
			if (szBuf[0] == 'T')
				chChannel->accList[nCount].bOwner = TRUE;
			else
				chChannel->accList[nCount].bOwner = FALSE;

			chChannel->accList[nCount].dwLevel = atoi(gettok(szBuf,szAccInfo,2,44));
			chChannel->accList[nCount].dwTimeout = atoi(gettok(szBuf,szAccInfo,3,44));
			strcpy(chChannel->accList[nCount].szCreator,gettok(szBuf,szAccInfo,4,44));
			strcpy(chChannel->accList[nCount].szHostmask,gettok(szBuf,szAccInfo,5,44));
			strcpy(chChannel->accList[nCount].szReason,gettok(szBuf,szAccInfo,6,44));
		}

		/* Forward the NJOIN message  */
		if (chChannel->nNJoinStatus == 1)
		{
			chChannel->nNJoinStatus = 2;
			sprintf(szBigstr,":%d " TOK_NJOIN " 2 %s %s :%s",scs.sclient->server->id,mi->argv[2],mi->argv[3],mi->argv[4]);
			Server_Broadcast(c_server,szBigstr);
		}
	}
	else if (nCtlMode == 3)
	{
		chChannel->dwPropOID = atol(gettok(szBuf,mi->argv[3],1,44));											/* Internal object identifier */
		chChannel->dwPropCreationTime = atol(gettok(szBuf,mi->argv[3],2,44));									/* Channel creation time */
		chChannel->dwPropClientGUID = atol(gettok(szBuf,mi->argv[3],3,44));										/* Client GUID */
		chChannel->dwPropServicepath = atol(gettok(szBuf,mi->argv[3],4,44));									/* Path of server side extension */
		chChannel->dwPropLag = atol(gettok(szBuf,mi->argv[3],5,44));											/* Channel lag(artificial delay) */
		gettok(szBuf,mi->argv[3],6,44); strcpy(chChannel->szPropLanguage,(szBuf[0] != 32 ? szBuf : ""));		/* Official language of channel */
		gettok(szBuf,mi->argv[3],7,44); strcpy(chChannel->szPropOwnerkey,(szBuf[0] != 32 ? szBuf : ""));		/* Ownerkey for channel */
		gettok(szBuf,mi->argv[3],8,44); strcpy(chChannel->szPropHostkey,(szBuf[0] != 32 ? szBuf : ""));			/* Hostkey for channel  */
		gettok(szBuf,mi->argv[3],9,44); strcpy(chChannel->szPropMemberkey,(szBuf[0] != 32 ? szBuf : ""));		/* Memberkey for channel */
		gettok(szBuf,mi->argv[3],10,44); strcpy(chChannel->szPropPICS,(szBuf[0] != 32 ? szBuf : ""));			/* PICS rating */
		gettok(szBuf,mi->argv[3],11,44); strcpy(chChannel->szPropTopic,(szBuf[0] != 32 ? szBuf : ""));			/* Channel topic */
		gettok(szBuf,mi->argv[3],12,44); strcpy(chChannel->szPropSubject,(szBuf[0] != 32 ? szBuf : ""));		/* Channel subject */
		gettok(szBuf,mi->argv[3],13,44); strcpy(chChannel->szPropClient,(szBuf[0] != 32 ? szBuf : ""));			/* Channel CLIENT property */
		gettok(szBuf,mi->argv[3],14,44); strcpy(chChannel->szPropOnjoin,(szBuf[0] != 32 ? szBuf : ""));			/* Channel ONJOIN */
		gettok(szBuf,mi->argv[3],15,44); strcpy(chChannel->szPropOnpart,(szBuf[0] != 32 ? szBuf : ""));			/* Channel ONPART */
		gettok(szBuf,mi->argv[3],16,44); strcpy(chChannel->szPropAccount,(szBuf[0] != 32 ? szBuf : ""));		/* Channel account */
		
		/* Forward the NJOIN message  */
		if (chChannel->nNJoinStatus == 2)
		{
			chChannel->nNJoinStatus = 0;
			sprintf(szBigstr,":%d " TOK_NJOIN " 3 %s :%s",scs.sclient->server->id,mi->argv[2],mi->argv[3]);
			Server_Broadcast(c_server,szBigstr);
		}
	}

	return 0;
}

int MP_Serversync()
/*
** MP_ServerSync()
** This command is used when a server is done sending information, used to sync
** id gathering of servers. It is called after all SERVER, NICK, and NJOIN messages.
** Usage: SERVERSYNC
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CLIENT_STRUCT *client = mi->c_from;
	char buf[256];

	if (client->flags & CLIENT_FLAG_SERVERSYNC)
	/* All server information sent */
	{
		client->flags &= ~CLIENT_FLAG_SERVERSYNC;
		client->flags |= CLIENT_FLAG_USERSYNC;

		/* The remote server has sent all the info, now send ours */
		Server_SendServerInfo(client);
	}
	else if (client->flags & CLIENT_FLAG_USERSYNC)
	/* All user information sent */
	{
		client->flags &= ~CLIENT_FLAG_USERSYNC;
		//client->flags ^= CLIENT_FLAG_CHANNELSYNC;
		client->flags |= CLIENT_FLAG_CHANNELSYNC;

		
		Server_SendUserInfo(client);
	}
	else if (client->flags & CLIENT_FLAG_CHANNELSYNC)
	/* All channel information sent */
	{
		client->flags &= ~CLIENT_FLAG_CHANNELSYNC;

		Server_SendChannelInfo(client,NULL);

		Serverlist_ChangeStatus(client->server->name,TRUE);
		sprintf(buf,"Synchronized link with %s established.",client->server->name);
		TechLog_AddEntry(buf,BITMAP_OK);

		/* Update mesh node status */
		MESH_NODE *mnPtr;
		mnPtr = Mesh_Nodelist_Find(client->server->name);

		if (mnPtr)
		{
			mnPtr->bConnected = TRUE;
			mnPtr->csServer = client;
		}
		else
			ODS("MP_SPass(): mnPtr is NULL!");

		scs.meshctrl.networkstatus = Mesh_NetworkStatus();

		if (SettingsInfo.isServers.networktype == NETWORK_TYPE_MESH &&
			(scs.meshctrl.networkstatus == MESH_NETWORKSTATUS_PARTIAL ||
			scs.meshctrl.networkstatus == MESH_NETWORKSTATUS_FULL))
		/* All servers connected */
			Server_Broadcast(scs.sclient,":%d " TOK_MESH " SERVERSTATUS A %d",scs.sclient->server->id,scs.sclient->server->id);

		Event_Broadcast(NULL,EVENT_TYPE_SERVER,"SERVER LINK %s:%d %s",client->ip_r,client->port_r,client->server->name);
	}

	return 0;
}

int MP_Userdata()
/*
** MP_Userdata()
** Server-to-server command used to add a nickname to the network
** Usage: USERDATA <nick> <hops> <username> <host> <server id#> <umode> <signon time> <realname>
** Example: USERDATA Donkey 1 ident phx.cox.net 1 +i :Full name goes here
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CLIENT_STRUCT *client = mi->c_prefixfrom;
	time_t signon;
	int nCount;
	char ch;

	if (mi->argc > 8)
	/* This should be sufficient for error-checking */
	{
		CLIENT_STRUCT *csnew = (CLIENT_STRUCT*)calloc(1,sizeof(CLIENT_STRUCT));

		/* Get signon time */
		signon = atol(mi->argv[7]);

		if (!csnew)
		/* Unable to calloc */
			ODS("Unable to allocate new user data! (MP_Userdata, csnew allocation)");
		else
		/* Assign new user stuff */
		{
			CLIENT_STRUCT *c_server = NULL;
			unsigned int id = atoi(mi->argv[5]);

			/* Step #1: Look up the server using the id */
			c_server = scs.CSServers[id];

			if (id >= 0 && id < MAX_SERVERTOKENS &&
				c_server)
			/* The id is valid */
			{
				/* Create the new user */
				csnew->user = (USER_STRUCT*)calloc(1,sizeof(USER_STRUCT));

				if (!csnew->user)
				/* Unable to allocate */
				{
					free(csnew);
					ODS("Unable to allocate new user(MP_Userdata, csnew->user allocation)");
				}
				else
				/* Create the new user information */
				{
					CLIENT_STRUCT *c_collision = NULL;
					BOOL bChanging = FALSE;

					/* Check for nickname collision */
					if (c_collision = User_Find(mi->argv[1]))
					/* If we encounter a nickname collision, the nick with the older signon time wins */
					{
						if (signon < c_collision->signon)
						/* Force nickname change of incoming user */
							bChanging = TRUE;
						else
						/* Force nickname change of local user */
							User_ForceNickChange(c_collision,NULL,TRUE);
					}

					strcpy(csnew->user->nickname,mi->argv[1]);
					strcpy(csnew->user->username,mi->argv[3]);
					strcpy(csnew->user->fullname,mi->argv[8]);

					/* Update our counters */
					scs.lusers.nGlobalUsers++;

					if (scs.lusers.nGlobalUsers > scs.lusers.nGlobalMax)
						scs.lusers.nGlobalMax = scs.lusers.nGlobalUsers;

					csnew->hops = atol(mi->argv[2]);
					csnew->hops++;
					
					csnew->state = CLIENT_STATE_NORMAL;
					csnew->flags |= CLIENT_FLAG_REGISTERED;
					csnew->signon = signon;

					csnew->servernext = c_server->servernext;
					strcpy(csnew->servername,c_server->server->name);
					csnew->serverkey = c_server->server->hashkey;

					strcpy(csnew->hostname,mi->argv[4]);

					/* TODO: Mask the IP address */
					strcpy(csnew->hostmask,csnew->hostname);

					/* Update usermodes */
					for (nCount = 0; mi->argv[6][nCount]; nCount++)
					{
						ch = mi->argv[6][nCount];

						if (ch == 'a')
							csnew->modeflag |= CLIENT_MODE_ADMIN;
						else if (ch == 'o')
							csnew->modeflag |= CLIENT_MODE_SYSOP;
						else if (ch == 'i')
							csnew->modeflag |= CLIENT_MODE_INVISIBLE;
						else if (ch == 'x')
							csnew->user->bInIRCX = TRUE;
						else if (ch == 'z')
							csnew->modeflag |= CLIENT_MODE_GAG;
					}

					if (Client_IsPriveledged(csnew))
						scs.lusers.nOpsOnline++;

					if (Client_Invisible(csnew))
						scs.lusers.nInvisible++;

					/* Add this user to the database */
					csnew->user->hashkey = Hash_Add(&HT_User,csnew->user->nickname,csnew);
					LL_Add(&scs.llUserHead[csnew->hops],csnew);

					/* Send the information to other servers (unless mesh) */
					if (SettingsInfo.isServers.networktype != NETWORK_TYPE_MESH)
						Server_Broadcast(csnew->servernext,":%d " TOK_USERDATA " %s %d %s %s %d %s %u :%s",
						c_server->server->id,
						csnew->user->nickname,
						csnew->hops,
						csnew->user->username,
						csnew->hostname,
						c_server->server->id /* temp */,
						mi->argv[6],
						(unsigned long)csnew->signon,
						csnew->user->fullname);

					/* Force the nickname change if there was a collision */
					if (bChanging)
						User_ForceNickChange(csnew,NULL,TRUE);
				}
			}
		}
	}

	return 0;
}

int MP_Knock()
/*
** MP_Knock()
** Server-to-server knock notification, sends knock notice to all local ops and forwards
** message if appropriate.
*/
{
	return 0;
}

int MP_Mesh()
/*
** MP_Mesh()
** Mesh control command
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CLIENT_STRUCT *client = mi->c_prefixfrom;
	CLIENT_STRUCT *cServer = NULL;
	int id, threshold_nbr;

	if (lstrcmpi(mi->argv[1],"SERVERSTATUS") == 0)
	{
		if (mi->argc > 3)
		{
			id = atoi(mi->argv[3]);

			cServer = scs.CSServers[id];

			if (cServer)
			/* cServer could be NULL if duplicate messages are sent. If so, silently ignore */
			{
				if (mi->argv[2][0] == 'T')
				/* Server timeout */
				{
					threshold_nbr = scs.meshctrl.inactivity_threshold / scs.meshctrl.total;

					if (threshold_nbr  < 1)
					/* If threshold is set too low, round up to one server required */
						threshold_nbr = 1;

					cServer->server->timeouts++;

					if (cServer->server->timeouts > threshold_nbr)
					/* Disconnect the server */
					{
						cServer->state = CLIENT_STATE_DEAD;
						strcpy(cServer->quitmessage,"Timed out");
					}
				}
				else if (mi->argv[2][0] == 'D')
				/* Server disconnect */
				{
					/* TODO: Determine which of the two servers will disconnect using algorithm. Right now both disconnect!! */
					cServer->state = CLIENT_STATE_DEAD;
					strcpy(cServer->quitmessage,"Disconnected");

					/* Set timeouts to -1 so Client_Disconnect() will not re-forward the message once again */
					cServer->server->timeouts = (unsigned int)(-1);
				}
				else if (mi->argv[2][0] == 'A')
				/* All servers have been connected to target server */
				{
					cServer->server->meshed = TRUE;

					/* Check if all servers are meshed */
					scs.meshctrl.networkstatus = Mesh_NetworkStatus();

				}
			}
		}
	}
	
	return 0;
}

#ifdef Z_D
int MP_Debug()
/*
** MP_Debug()
** Enabled only if Z_D is defined!
** This command allows a client to recieve debug information, and the only valid
** password is the RA password!
** Usage: DEBUG <command> <parameters>
*/
{
	MESSAGE_CONTROL_STRUCT *mi = &scs.msginfo;
	CLIENT_STRUCT *client = mi->c_from;

	if (mi->argc < 2)
	{
		Client_SendNumericToOne(client,ERR_NEEDMOREPARAMS,mi->argv[0]);
		return 0;
	}

	if (stricmp(mi->argv[1],"LOGIN") == 0)
	{
		if (strcmp(mi->argv[2],SettingsInfo.isSecurity.RAPassword) == 0)
		{
			scs.dbgclient = client;
			Client_SendNumericToOne(client,999,"You are now the debug client");
		}
		else
		/* We dont like people messing with the debugger! */
		{
			client->state = CLIENT_STATE_DEAD;
			return STOP_PARSING;
		}
	}
	else
		if (client != scs.dbgclient)
		/* We dont like people messing with the debugger! */
		{
			client->state = CLIENT_STATE_DEAD;
			return STOP_PARSING;
		}
		else
		{
			if (stricmp(mi->argv[1],"USERS") == 0)
			/* Print list of all connected users */
			{
				int count;
				LINKED_LIST_STRUCT *curr;
				CLIENT_STRUCT *c_client;
				char buf[256];

				Client_SendNumericToOne(client,999,"***DEBUG*** Users connected to network:");

				for (count = 0; count < MAX_HOPS; count++)
				{
					curr = &scs.llUserHead[count];

					while (curr->next)
					{
						curr = curr->next;
						c_client = (CLIENT_STRUCT*)curr->data;

						sprintf(buf,"***DEBUG*** R/L Ports: %d/%d, Mask == %s!%s@%s, hops == %d",c_client->port_r,c_client->port_l,c_client->user->nickname,c_client->user->username,c_client->hostname,c_client->hops);
						Client_SendNumericToOne(client,999,buf);
					}
				}

				Client_SendNumericToOne(client,999,"***DEBUG*** End of user list:");
			}
		}

	return 0;
}
#endif		/* Z_D */