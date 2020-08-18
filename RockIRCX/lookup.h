/*
** lookup.h
**
** Contains all header information for hostname lookups
*/

#ifndef __LOOKUP_H__
#define __LOOKUP_H__

#include <process.h>
#include "structs.h"
#include "client.h"
#include "..\Common Files\settings.h"

/* External variables */
extern INFOSTRUCTSETTINGS SettingsInfo;

#define LOOKUP_COMPLETE	0
#define LOOKUP_PENDING	1

void		Lookup_Init();
int			Lookup_Perform(unsigned int ip, CLIENT_STRUCT *client);
void		Lookup_AddToCache(unsigned int ip, CLIENT_STRUCT *client);
void		Lookup_RemoveFromCache(unsigned int ip);
int			Lookup_Thread(void *param);
void		Lookup_Cycle(INFOSTRUCTSETTINGS *is);

#endif		/* __LOOKUP_H__ */