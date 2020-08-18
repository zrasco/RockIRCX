/*
** messages.h
** Contains header information for all server messages
*/

#pragma once

#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#include "..\Common Files\settings.h"
#include "channel.h"
#include "client.h"
#include "numeric.h"
#include "hash.h"
#include "RockIRCX.h"
#include "structs.h"

/* External variables */
extern SERVER_TV_STRUCT sts;
extern SERVER_CONTROL_STRUCT scs;
extern INFOSTRUCTSETTINGS SettingsInfo;
extern HASH_TABLE_STRUCT HT_User;
extern HASH_TABLE_STRUCT HT_Server;
extern HASH_TABLE_STRUCT HT_Channel;

/* External functions */
void			Serverlist_ChangeStatus(const char *servername, BOOL bConnected);
int				TechLog_AddEntry(char* text, unsigned char icon);
extern int		match(char *mask, char *string);
extern int		isstrdigit(char *str);

/* Message defines */
#define		MESSAGEMAP_LIMIT	128

/* Commands */
#define		MSG_PRIVATE		"PRIVMSG"
#define		TOK_PRIVATE		"!"

#define		MSG_WHO			"WHO"
#define		TOK_WHO			"\""

#define		MSG_WHOIS		"WHOIS"
#define		TOK_WHOIS		"#"

#define		MSG_WHOWAS		"WHOWAS"
#define		TOK_WHOWAS		"$"

#define		MSG_USER		"USER"
#define		TOK_USER		"%"

#define		MSG_NICK		"NICK"
#define		TOK_NICK		"&"

#define		MSG_LIST		"LIST"
#define		TOK_LIST		"'"

#define		MSG_LISTX		"LISTX"
#define		TOK_LISTX		"("

#define		MSG_TOPIC		"TOPIC"
#define		TOK_TOPIC		")"

#define		MSG_INVITE		"INVITE"
#define		TOK_INVITE		"*"

#define		MSG_VERSION		"VERSION"
#define		TOK_VERSION		"+"

#define		MSG_QUIT		"QUIT"
#define		TOK_QUIT		","

#define		MSG_KILL		"KILL"
#define		TOK_KILL		"-"

#define		MSG_INFO		"INFO"
#define		TOK_INFO		"."

#define		MSG_LINKS		"LINKS"
#define		TOK_LINKS		"/"

#define		MSG_STATS		"STATS"
#define		TOK_STATS		"0"

#define		MSG_USERS		"USERS"
#define		TOK_USERS		"1"

#define		MSG_AWAY		"AWAY"
#define		TOK_AWAY		"2"

#define		MSG_PING		"PING"
#define		TOK_PING		"3"

#define		MSG_PONG		"PONG"
#define		TOK_PONG		"4"

#define		MSG_OPER		"OPER"
#define		TOK_OPER		"5"

#define		MSG_PASS		"PASS"
#define		TOK_PASS		"6"

#define		MSG_TIME		"TIME"
#define		TOK_TIME		"7"

#define		MSG_NAMES		"NAMES"
#define		TOK_NAMES		"8"

#define		MSG_ADMIN		"ADMIN"
#define		TOK_ADMIN		"9"

#define		MSG_NOTICE		"NOTICE"
#define		TOK_NOTICE		";"

#define		MSG_JOIN		"JOIN"
#define		TOK_JOIN		"<"

#define		MSG_CREATE		"CREATE"
#define		TOK_CREATE		"="

#define		MSG_ACCESS		"ACCESS"
#define		TOK_ACCESS		">"

#define		MSG_PROP		"PROP"
#define		TOK_PROP		"?"

#define		MSG_PART		"PART"
#define		TOK_PART		"@"

#define		MSG_LUSERS		"LUSERS"
#define		TOK_LUSERS		"A"

#define		MSG_WHISPER		"WHISPER"
#define		TOK_WHISPER		"B"

#define		MSG_IRCX		"IRCX"
#define		TOK_IRCX		"C"

#define		MSG_ISIRCX		"ISIRCX"
#define		TOK_ISIRCX		"D"

#define		MSG_MOTD		"MOTD"
#define		TOK_MOTD		"E"

#define		MSG_MODE		"MODE"
#define		TOK_MODE		"F"

#define		MSG_KICK		"KICK"
#define		TOK_KICK		"G"

#define		MSG_USERHOST	"USERHOST"
#define		TOK_USERHOST	"H"

#define		MSG_ISON		"ISON"
#define		TOK_ISON		"I"

#define		MSG_SILENCE		"SILENCE"
#define		TOK_SILENCE		"J"

#define		MSG_TRACE		"TRACE"
#define		TOK_TRACE		"K"

#define		MSG_EVENT		"EVENT"
#define		TOK_EVENT		"L"

#define		MSG_SERVER		"SERVER"
#define		TOK_SERVER		"M"

#define		MSG_SPASS		"SPASS"
#define		TOK_SPASS		"N"

#define		MSG_NJOIN		"NJOIN"
#define		TOK_NJOIN		"O"

#define		MSG_SERVERSYNC	"SERVERSYNC"
#define		TOK_SERVERSYNC	"P"

#define		MSG_USERDATA	"USERDATA"
#define		TOK_USERDATA	"Q"

#define		MSG_KNOCK		"KNOCK"
#define		TOK_KNOCK		"R"

#define		MSG_MESH		"MESH"
#define		TOK_MESH		"S"

#ifdef Z_D
#define		MSG_DEBUG		"DEBUG"
#define		TOK_DEBUG		"T"
#endif		/* Z_D */

/* Function prototypes */
void			Message_Initialize();
int				Message_Execute();

/* Message processing(MP) functions */
/* NOTE: All these functions access the message control struct in the variable scs! */
extern int		MP_Privmsg();
extern int		MP_Who();
extern int		MP_Whois();
extern int		MP_Whowas();
extern int		MP_User();
extern int		MP_Nick();
extern int		MP_List();
extern int		MP_Listx();
extern int		MP_Topic();
extern int		MP_Invite();
extern int		MP_Version();
extern int		MP_Quit();
extern int		MP_Kill();
extern int		MP_Info();
extern int		MP_Links();
extern int		MP_Stats();
extern int		MP_Users();
extern int		MP_Away();
extern int		MP_Ping();
extern int		MP_Pong();
extern int		MP_Oper();
extern int		MP_Pass();
extern int		MP_Time();
extern int		MP_Names();
extern int		MP_Admin();
extern int		MP_Notice();
extern int		MP_Join();
extern int		MP_Create();
extern int		MP_Access();
extern int		MP_Prop();
extern int		MP_Part();
extern int		MP_Lusers();
extern int		MP_Whisper();
extern int		MP_Ircx();
extern int		MP_IsIrcx();
extern int		MP_MOTD();
extern int		MP_Mode();
extern int		MP_Kick();
extern int		MP_Userhost();
extern int		MP_Ison();
extern int		MP_Silence();
extern int		MP_Trace();
extern int		MP_Event();
extern int		MP_Server();
extern int		MP_SPass();
extern int		MP_NJoin();
extern int		MP_Serversync();
extern int		MP_Userdata();
extern int		MP_Knock();
extern int		MP_Mesh();

#ifdef Z_D
extern int		MP_Debug();
#endif		/* Z_D */

typedef struct Message
{
	char			*command;							/* Message command */
	int				(*function)();						/* Function to process */
#define MSGPARAM_INFINITE				-1
	int				parameters;							/* # of max parameters */

#define MSGFLAG_IRCXONLY				0x0001			/* Only IRCX clients */
#define MSGFLAG_COMMANDNOTSUPPORTED		0x0002			/* Command not supported */
#define MSGFLAG_COMMANDDISABLED			0x0004			/* Command disabled */
#define MSGFLAG_DELAY					0x0008			/* This message make a delay? */
#define	MSGFLAG_REGISTERCOMMAND			0x0010			/* Flag for registration commands */
#define MSGFLAG_SERVERCOMMAND			0x0020			/* Used for server-to-server */
#define MSGFLAG_NOIDLERESET				0x0040			/* Command does not reset idle timer */
	unsigned char	flag;								/* Message control flags */
	unsigned char	token[2];							/* Message token */
} Message;

/* Message & lookup table */
extern Message *MessageMap[MESSAGEMAP_LIMIT];
extern Message MessageTable[];

#endif		/* __MESSAGES_H__ */