#pragma once
#pragma warning(disable : 4996)

#include "resource.h"

#define WM_TRAYICON			(WM_USER + 1)
#define WM_UPDATESTATUS		(WM_USER + 2)
#define WM_UPDATESERVERS	(WM_USER + 3)
#define UpdateStatus(hWnd)	PostMessage(hWnd,WM_UPDATESTATUS,0,0)

/* Define Z_D(Zebs Debug) to enable debug logging */
#define Z_D

/* Technical log bitmaps */
#define BITMAP_OK			0
#define BITMAP_INFO			1
#define BITMAP_WARNING		2
#define BITMAP_ERROR		3

/* External functions */
extern void		Message_Initialize();

/* External variables */
extern HASH_TABLE_STRUCT	HT_Channel;
extern HASH_TABLE_STRUCT	HT_User;
extern HASH_TABLE_STRUCT	HT_Server;

#define RI_RAPORT		6932
