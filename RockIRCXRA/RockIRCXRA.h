#pragma once

#include "resource.h"
#include "..\Common Files\datalists.h"
#include <winsock2.h>
#include <commctrl.h>

#define ZRA_LENGTH 469
#define ZRA_WIDTH 286

/* Custom window messages */
#define WM_UPDATECONNECTINFO	(WM_USER + 1)
#define WM_RASOCKET				(WM_USER + 2)

/* Technical log bitmaps */
#define BITMAP_OK			0
#define BITMAP_INFO			1
#define BITMAP_WARNING		2
#define BITMAP_ERROR		3

/* Macro functions */
#define GETWINDOWINFOSTRUCTURE(hWnd) GetWindowLong(hWnd,GWL_USERDATA)
#define CheckBox(hCkBox, bCheck)			Button_SetCheck(hCkBox,bCheck == TRUE ? BST_CHECKED : BST_UNCHECKED)
#define CheckDlgBox(hDlg, id, bCheck)		CheckBox(GetDlgItem(hDlg,id),bCheck)
#define BoxChecked(hDlg, id)				(SendDlgItemMessage(hDlg,id,BM_GETSTATE,0,0) == BST_CHECKED) ? TRUE : FALSE

typedef struct TreeTable
/* Tree table structure */
{
	char* text;									/* Text for each item */
	HTREEITEM hItem;							/* Handle to item */
	char *status;								/* Text to go in status bar when item active */
	int id;										/* ID # of dialog box to show */
	HWND hwnd;									/* Handle of dialog box to show */
	unsigned short currtab;
} TreeTable;

typedef struct BtnInfoTable
/* Structure for which buttons get which icons */
{
	HANDLE button;								/* Bitmap to go in buttons */
	int btnid[10];								/* List of buttons to recieve the bitmap */
} BtnInfoTable;

typedef struct CHECKBOXHANDLES
/* Structure containing information about check box sub-items */
{
	int ckid;									/* ID # of check box */
	HWND ckhwnd;								/* Handle to check box */
	HWND parent;								/* Parent window of check box */

	int id[18];									/* List of items to be affected */
} CHECKBOXHANDLES;

typedef struct SCROLLEDITS
/* Structure containing information about numeric edit boxes w/scrollbars */
{
	int edit;									/* ID # for edit box */
	int scrollbar;								/* ID # for scrollbar */
	HWND hScrollBar;							/* Handle to scrollbar */
	HWND hEditBox;								/* Handle to edit box */
} SCROLLEDITS;

#define UpdateConnectInfo(hwnd) SendMessage(hwnd,WM_UPDATECONNECTINFO,0,0);
typedef struct ConnectInfo
{
	SOCKET socket;
	HWND hConnect;
	char hostname[256];
	char ipaddr[32];
	char status[256];
	char password[256];

	unsigned short port;

#define		REFRESH_ACCTS	1
#define		REFRESH_BANS	2
#define		REFRESH_CHANS	3
#define		REFRESH_SERVERS	4
#define		REFRESH_FILTERS	5
#define		REFRESH_ACCESS	6
	unsigned char refresh;

	BOOL bTerminate;
	BOOL bReady;
} ConnectInfo;
