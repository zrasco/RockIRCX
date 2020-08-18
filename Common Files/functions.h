/*
** Functions.h
** This header file contains all macros/define's for generic functions used
*/

#ifndef		__FUNCTIONS_H__
#define		__FUNCTIONS_H__

#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <stdio.h>
#include "stdafx.h"
#include "..\Common Files\XWSAErrorTable.h"

#define SIZE_SHORT	2
#define SIZE_LONG	4
#define MAXSTR		2048

#define			ReallocAndAdd(ptr,str)	{ ptr = (char*)realloc(ptr,strlen(ptr) + strlen(str) + 1) ; strcat(ptr,str); }
#define			InitStrPtr(ptr)			{ ptr = (char*)malloc(1); ptr[0] = 0; }
#define			InsertShort(string,index,snbr)	(*(unsigned short*)&string[index]) = snbr;
#define			InsertLong(string,index,lnbr)	(*(unsigned long*)&string[index]) = lnbr;
#define			GetShort(string, index, sptr)	*sptr = *(unsigned short*)&string[index];
#define			GetLong(string, index, lptr)	*lptr = *(unsigned long*)&string[index];
#define			CREATEPTRFROMSTRING(ptr,string)	ptr = (char*)malloc(strlen(string) + 1); strcpy(ptr,string);
#define			MODIFYPTRFROMSTRING(ptr,string) ptr = (char*)realloc(ptr,strlen(string) + 1); strcpy(ptr,string);

/* Function prototypes */
int				SetNonBlocking(SOCKET s);
void			ConvertTime(char* ctime, char* targetbuf);
void			GetErrorString(char *errbuffer, unsigned int error);
int				index(const char *buffer, char value, int indexnbr);
void			ToggleClose(HWND hwnd, BOOL bEnable);
void			ODS(const char *string, ...);
char			*IPFromLong(unsigned int ip);

/* Functions used for token processing */
char			*tokenrange(char *targetbuf, const char *str, int token, int numtoks, int seperator);
char			*gettok(char *targetbuf, const char* str, int token, int seperator);
int				numtok(const char* str,int seperator);

/* Validation checks */
BOOL			Valid_Username(char *username, BOOL bMakeValid);
BOOL			Valid_Nickname(const char *nickname);
BOOL			Valid_Channelname(const char *szChannelname);
void			Validate_AccessHostmask(const char *szInput, char *szOutput);
void			ModeCleanup_Usermode(char *szModeStr);
void			ModeCleanup_Channelmode(char *szModeStr);

#endif		/* __FUNCTIONS_H__ */
