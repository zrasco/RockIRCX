/*
** RockIRCX.cpp
**
** This is the main file for the RockIRCX IRCX server.
** It contains the GUI & sets up all networking activity.
**
** Programmer: Zeb Rasco
** Copyright 2004
*/

/* #pragma comment(linker, "/OPT:NOWIN98") */

#include "stdafx.h"


#include <commctrl.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <windowsx.h>

#include "socket.h"
#include "hash.h"
#include "..\Common Files\settings.h"
#include "RockIRCX.h"

#define MAX_LOADSTRING 100

// Global Variables:
HBITMAP hBMPListBox[4];								// List box bitmaps
SERVER_TV_STRUCT sts;								/* Treeview info for servers */
NOTIFYICONDATA niData;								// System tray information
HINSTANCE hInst;									// current instance
HMENU hSystrayMenu;									// Systray menu handle

#define SM_CONNECTED	0
#define SM_DISCONNECTED	1
HMENU hServersMenu[2];								/* Servers popup menus */

HANDLE hSocketThread;								// Handle to sockets thread
HWND hMain;											// Main dialog box
HWND hStatus;										// Status sub-window
HWND hServers;										// Servers sub-window
HWND hLog;											// Log sub-window
HWND hAbout;										// About sub-window
TCHAR szTitle[MAX_LOADSTRING] = "RockIRCX";			// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING] = "ROCKIRCX";	// the main window class name

/* Server globals */
INFOSTRUCTSETTINGS SettingsInfo;
SERVER_CONTROL_STRUCT scs;

// Forward declarations of functions included in this code module:
BOOL				InitInstance(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK	MainDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
int					CreateSubWindows(HWND hParent);
int					TechLog_AddEntry(char* text, unsigned char icon);
void				TechLog_Focus();
void				Serverlist_Initialize();
void				Serverlist_ChangeStatus(const char *servername, BOOL bConnected);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE,
                     LPTSTR,
                     int       nCmdShow)
{
	MSG msg;
	HACCEL hAccelTable;
	int err = 0;
	unsigned int retval;
	unsigned short port;
	INITCOMMONCONTROLSEX icc;
	char buf[256];

#ifdef Z_D
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

#endif		/* Z_D */

	/* Zero all miscellaneous items */
	memset(&sts,0,sizeof(sts));
	memset(&scs,0,sizeof(scs));

	/* Initalize linked lists */
	LL_Init();

	/* Initialize common controls */
	icc.dwICC = ICC_TREEVIEW_CLASSES | ICC_TAB_CLASSES;
	icc.dwSize = sizeof(icc);

	InitCommonControlsEx(&icc);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	/* Create hash tables */
	Hash_Create(&HT_Channel,CHANNEL_HASHSIZE);
	Hash_Create(&HT_User,CLIENT_HASHSIZE);
	Hash_Create(&HT_Server,SERVER_HASHSIZE);

	/* Load settings information */
	char szFileBuf[256];

	if (!IS_LoadFromFile(SettingsFileName(szFileBuf),&SettingsInfo))
	{
		IS_CreateBlank(&SettingsInfo);
		err = 1;
	}

	/* Set up list of mesh nodes */
	Mesh_Nodelist_Setup();

	/* Set up buffers */
	StrBuf_Init();

	/* Create sub-windows */
	CreateSubWindows(hMain);
	InvalidateRect(hMain,NULL,TRUE);
	UpdateWindow(hMain);

	char szFileBuf1[256];
	char szMsg[1024];

	if (err == 1)
	{
		sprintf(szMsg,"Unable to load %s",SettingsFileName(szFileBuf1));
		TechLog_AddEntry(szMsg,BITMAP_WARNING);
	}
	else
	{
		sprintf(szMsg,"Successfully loaded %s",SettingsFileName(szFileBuf1));
		TechLog_AddEntry(szMsg,BITMAP_OK);
	}

	/* Start up socket subsystem */
	Socket_Startup();
	Lookup_Init();

	/* Setup & bind RA */
	port = (unsigned short)GetDlgItemInt(hStatus,IDC_EDITPORT,NULL,FALSE);
	RA_SetPort(port);

	/* Initialize message processing system */
	Message_Initialize();

	/* Bind server listening ports & add server to hash table */
	Socket_SetupListeners();

	if (scs.sclient)
	/* If server is online, add to hash table */
	{
		if (!(scs.skey = scs.sclient->server->hashkey = Hash_Add(&HT_Server,scs.sclient->server->name,scs.sclient)))
		/* Couldn't add local server to hash table */
		{
			sprintf(buf,"Unable to add local server entry to server hash table",err);
			TechLog_AddEntry(buf,BITMAP_ERROR);
		}
	}

	/* Start sockets */
	hSocketThread = (HANDLE)_beginthreadex(NULL,0,(unsigned int(__stdcall*)(void*))Socket_Thread,NULL,0,&retval);

	if (!hSocketThread)
	/* Could not create thread */
	{
		err = GetLastError();
		sprintf(buf,"Unable to create sockets thread(Error %d)",GetLastError());
		TechLog_AddEntry(buf,BITMAP_ERROR);
	}
	if (err = RA_Startup())
	/* Remote administration failed to start */
	{
		char buf[256];

		sprintf(buf,"Unable to start remote administration, error %d",err);
		TechLog_AddEntry(buf,BITMAP_ERROR);

		RA_Cleanup();
	}
	else
	/* Remote administration started up OK */
	{
		char buf[256];

		sprintf(buf,"Remote administration OK, listening on TCP port %d",port);
		TechLog_AddEntry(buf,BITMAP_OK);
	}

	/* Server is up! */
	SettingsInfo.isStatus.uptime = time(NULL);

	UpdateStatus(hMain);

	/* Load accelerator table */
	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_ROCKIRCX);

	/* TEMP: Hardcoded variables! */
	scs.meshctrl.inactivity_threshold = 25;

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!IsDialogMessage(hMain,&msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hMain = hWnd = CreateDialog(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_MAIN),0,(DLGPROC)MainDlgProc);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);

   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//

LRESULT CALLBACK MainDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
/*
** Main dialogbox callback procedure
*/
{
	switch (Message)
	{
		case WM_INITDIALOG:
		{
			HICON icon;
			char buf[256];

			GetWindowText(hwnd,buf,256);

			if (strcmp(buf,"RockIRCX") == 0)
			/* Main dialog */
			{
				/* Set icon */
				icon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_ROCKIRCX));
				SendMessage(hwnd,WM_SETICON,TRUE,(LPARAM)icon);

				/* Load technical log bitmaps */
				hBMPListBox[BITMAP_OK] = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_OK));
				hBMPListBox[BITMAP_INFO] = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_INFO));
				hBMPListBox[BITMAP_WARNING] = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_WARNING));
				hBMPListBox[BITMAP_ERROR] = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_ERROR));

				/* Load server icons */
				sts.hBMPConnected = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_SERVEROK));
				sts.hBMPDisconnected = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_SERVERNOTOK));
				sts.hBMPOutBound = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_SERVEROUTBOUND));
				sts.hBMPInBound = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_SERVERINBOUND));

				HWND hTab = GetDlgItem(hwnd,IDC_TABMAIN);
				TC_ITEM pitem;

				pitem.mask = TCIF_TEXT;

				/* Add tab items */
				pitem.pszText = "Status";
				TabCtrl_InsertItem(hTab,0,&pitem);

				pitem.pszText = "Servers";
				TabCtrl_InsertItem(hTab,1,&pitem);

				pitem.pszText = "Log";
				TabCtrl_InsertItem(hTab,2,&pitem);

				pitem.pszText = "About";
				TabCtrl_InsertItem(hTab,3,&pitem);
			}
		}
		break;
		case WM_COMMAND:
		{
			int wNotifyCode = HIWORD(wParam);
			int wID = LOWORD(wParam);
			// HWND hFrom = (HWND) lParam;

			if (wNotifyCode == BN_CLICKED)
			{
				switch (wID)
				{
					case IDC_BTNAPPLY:
						{
							//CLIENT_STRUCT *csUser = (CLIENT_STRUCT*)scs.llUserHead[1].next->data;

							//if (csUser)
							//{
							//	User_ForceNickChange(csUser,NULL,TRUE);
							//}

							_CrtDumpMemoryLeaks();
						}
					break;
					case IDC_BTNCLOSE:
						SendMessage(hwnd,WM_CLOSE,0,0);
					break;
					case IDM_SHOW:
						PostMessage(hwnd,WM_TRAYICON,0,(LPARAM)WM_LBUTTONDBLCLK);
					break;
					case IDM_SHUTDOWN:
					case IDC_BTNSHUTDOWN:
						DestroyWindow(hwnd);
					break;
				}
			}
		}
		break;
		case WM_NOTIFY:
		{
			// int idCtrl = (int) wParam;
			LPNMHDR pnmh = (LPNMHDR) lParam;

			switch (pnmh->idFrom)
			{
				case IDC_TABMAIN:
				{
					int tab = TabCtrl_GetCurSel(pnmh->hwndFrom);
					
					if (pnmh->code == TCN_SELCHANGE)
					/* Selection is changing */
					{
						HWND hTabWnd[] = { hStatus, hServers, hLog, hAbout };
						int count;

						for (count = 0; count < 3; count++)
						{
							if (count == tab)
								ShowWindow(hTabWnd[count],SW_SHOW);
							else
								ShowWindow(hTabWnd[count],SW_HIDE);
						}
					}
				}
				break;
				case IDC_TVSERVERS:
				{
					if (pnmh->code == TVN_GETDISPINFO)
					{
						LPNMTVDISPINFO lptvdi = (LPNMTVDISPINFO)lParam;

						if (lptvdi->item.hItem == sts.htviInBound)
						{
							lptvdi->item.pszText = "Inbound";
							lptvdi->item.cchTextMax = (int)strlen(lptvdi->item.pszText);
							lptvdi->item.iImage = 0;
							lptvdi->item.iSelectedImage = 0;
						}
						else if (lptvdi->item.hItem == sts.htviOutBound)
						{
							lptvdi->item.pszText = "Outbound";
							lptvdi->item.cchTextMax = (int)strlen(lptvdi->item.pszText);
							lptvdi->item.iImage = 1;
							lptvdi->item.iSelectedImage = 1;
						}
						else if (SettingsInfo.isServers.networktype == NETWORK_TYPE_MESH &&
							lptvdi->item.hItem == sts.htviMeshStatus)
						{
							lptvdi->item.iImage = 3;
							lptvdi->item.iSelectedImage = 3;

							switch (scs.meshctrl.networkstatus)
							{
								case MESH_NETWORKSTATUS_OFFLINE:
								{
									lptvdi->item.pszText = "Mesh Status: Offline";
									break;
								}
								case MESH_NETWORKSTATUS_INIT:
								{
									lptvdi->item.pszText = "Mesh Status: Connecting";
									break;
								}
								case MESH_NETWORKSTATUS_PARTIAL:
								{
									lptvdi->item.pszText = "Mesh Status: Partial mesh";
									break;
								}
								case MESH_NETWORKSTATUS_FULL:
								{
									lptvdi->item.pszText = "Mesh Status: Full mesh";
									lptvdi->item.iImage = 2;
									lptvdi->item.iSelectedImage = 2;
									break;
								}
							}

							lptvdi->item.cchTextMax = (int)strlen(lptvdi->item.pszText);
						}
						else
						{
							int count;
							char buf[256];

							for (count = 0; sts.stis[count].htvi; count++)
							{
								if (lptvdi->item.hItem == sts.stis[count].htvi)
								/* Found it */
								{
									if (sts.stis[count].bOutBound)
									/* Outbound server */
										sprintf(buf,"%s(%s, port %d)",sts.stis[count].servername,sts.stis[count].hostmask,sts.stis[count].port);
									else
									{
										if (sts.stis[count].bConnected)
										/* Inbound server which is connected */
										{
											CLIENT_STRUCT *sclient = Server_HashFind(sts.stis[count].servername);
											struct in_addr in;

											in.s_addr = sclient->ip;

											sprintf(buf,"%s(From %s[%s])",sts.stis[count].servername,sclient->hostname,inet_ntoa(in));
										}
										else
										/* ...and not connected */
											sprintf(buf,"%s(%s)",sts.stis[count].servername,sts.stis[count].hostmask);
									}

									lptvdi->item.pszText = buf;
									lptvdi->item.cchTextMax = (int)strlen(lptvdi->item.pszText);
									lptvdi->item.iImage = (sts.stis[count].bConnected ? 2 : 3);
									lptvdi->item.iSelectedImage = lptvdi->item.iImage;
									break;
								}
							}
						}
					}
					else if (pnmh->code == NM_RCLICK)
					/* User right-clicked an item */
					{
						int count;
						POINT pt;
						TVHITTESTINFO tvht;
						RECT rect;

						/*
						** Get coordinates relative to the tree view control
						** pt = X/Y coordinates on screen
						** tvht.pt = X/Y coordinates in tree view control
						*/
						GetCursorPos(&pt);
						GetWindowRect(pnmh->hwndFrom,&rect);
						tvht.pt.x = pt.x - rect.left;
						tvht.pt.y = pt.y - rect.top;

						TreeView_HitTest(pnmh->hwndFrom,&tvht);

						if (tvht.hItem)
						{
							for (count = 0; sts.stis[count].htvi; count++)
							/* Find out which one was right-clicked */
							{
								if (tvht.hItem == sts.stis[count].htvi)
								/* Found it! */
								{
									if (!sts.stis[count].bConnected)
										OBC_Update(sts.stis[count].servername);
									else
									/* Find server with matching name and close connection */
									{
										LINKED_LIST_STRUCT *llPtr = &scs.llServerHead[1];
										CLIENT_STRUCT *csServer;

										while (llPtr->next)
										{
											llPtr = llPtr->next;

											csServer = (CLIENT_STRUCT*)llPtr->data;

											if (strcmp(csServer->server->name,sts.stis[count].servername) == 0)
											/* Found it! */
											{
												char szBuf[256];

												sprintf(szBuf,":%d " TOK_QUIT " :Right click D/C!\r\n",scs.sclient->server->id);
												StrBuf_Add(&csServer->sendQ,szBuf,strlen(szBuf));
												break;
											}
										}
									}

									break;
								}
							}
						}
					}
				}
				break;
			}
		}
		break;
		case WM_CLOSE:
		{
			ZeroMemory(&niData,sizeof(NOTIFYICONDATA));

			niData.cbSize = sizeof(NOTIFYICONDATA);

			niData.uID = IDC_TRAYICON;
			niData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
			niData.hIcon = (HICON)LoadImage(hInst,MAKEINTRESOURCE(IDI_ROCKIRCX),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),LR_DEFAULTCOLOR);
			niData.hWnd = hwnd;
			niData.uCallbackMessage = WM_TRAYICON;
			lstrcpyn(niData.szTip,"RockIRCX",sizeof(niData.szTip));
            
			Shell_NotifyIcon(NIM_ADD,&niData);
			//AnimateWindow(hwnd,200,AW_HIDE | AW_SLIDE | AW_VER_POSITIVE);
			ShowWindow(hwnd,SW_HIDE);
		}
		break;
		case WM_TRAYICON:
		{
			switch (lParam)
			{
				case WM_LBUTTONDBLCLK:
				/* Double clicked tray icon */
					Shell_NotifyIcon(NIM_DELETE,&niData);
					//AnimateWindow(hwnd,200,AW_ACTIVATE | AW_SLIDE | AW_VER_NEGATIVE);
					ShowWindow(hwnd,SW_SHOW);
				break;
				case WM_RBUTTONDOWN:
				case WM_CONTEXTMENU:
					{
						POINT pt;

						GetCursorPos(&pt);

						SetForegroundWindow(hwnd);

						TrackPopupMenu(hSystrayMenu,TPM_BOTTOMALIGN | TPM_RIGHTALIGN,pt.x,pt.y,0,hwnd,NULL);

					}
				break;
			}

		}
		break;
		case WM_VSCROLL:
		{
			int nScrollCode = (int)LOWORD(wParam);
			HWND hwndScrollBar = (HWND)lParam;

			if (hwndScrollBar == GetDlgItem(hStatus,IDC_VSPORT))
			/* Port scrollbar */
			{
				unsigned short port = (unsigned short)GetDlgItemInt(hStatus,IDC_EDITPORT,NULL,FALSE);

				/* Adjust port # in box */
				if (nScrollCode == SB_LINEUP)
					port++;
				else if (nScrollCode == SB_LINEDOWN)
					port--;

				SetDlgItemInt(hStatus,IDC_EDITPORT,port,FALSE);
				SetFocus(GetDlgItem(hStatus,IDC_EDITPORT));
			}

		}
		break;
		case WM_UPDATESTATUS:
		/* Here is where we update our info */
		{
			double dResult;
			char szBuf[256];
			unsigned long secs = time(NULL) - SettingsInfo.isStatus.uptime;

			SetDlgItemInt(hStatus,IDC_EDITLOCALUSERS,scs.lusers.nLocalUsers,FALSE);
			SetDlgItemInt(hStatus,IDC_EDITLOCALUSERSMAX,SettingsInfo.isStatus.localmax,FALSE);
			SetDlgItemInt(hStatus,IDC_EDITGLOBALUSERS,scs.lusers.nGlobalUsers,FALSE);
			SetDlgItemInt(hStatus,IDC_EDITGLOBALUSERSMAX,SettingsInfo.isStatus.globalmax,FALSE);
			SetDlgItemInt(hStatus,IDC_EDITLOCALCHANNELS,scs.lusers.nChannels,FALSE);
			SetDlgItemInt(hStatus,IDC_EDITGLOBALCHANNELS,SettingsInfo.isStatus.globalchannels,FALSE);

			dResult = (SettingsInfo.isIOControl.datarecvd  > 0) ? ((double)SettingsInfo.isIOControl.datarecvd / 1024 / secs) : 0;
			sprintf(szBuf,"%.1fK/sec",dResult);
			SetDlgItemText(hStatus,IDC_EDITDOWNLOAD,szBuf);

			dResult = (SettingsInfo.isIOControl.datasent  > 0) ? ((double)SettingsInfo.isIOControl.datasent / 1024 / secs) : 0;
			sprintf(szBuf,"%.1fK/sec",dResult);
			SetDlgItemText(hStatus,IDC_EDITUPLOAD,szBuf);

		}
		break;
		case WM_UPDATESERVERS:
		/* Here is where we update the server information */
		{
            int count;
			TV_INSERTSTRUCT tvis;

			TreeView_DeleteAllItems(sts.hTreeView);
			TreeView_SetImageList(sts.hTreeView,sts.himl,TVSIL_NORMAL);

			/* Insert Inbound/Outbound items */
			tvis.hParent = NULL;
			tvis.hInsertAfter = TVI_LAST;
			tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;

			tvis.item.iImage = I_IMAGECALLBACK;
			tvis.item.iSelectedImage = I_IMAGECALLBACK;
			tvis.item.pszText = LPSTR_TEXTCALLBACK;

			sts.htviMeshStatus = TreeView_InsertItem(sts.hTreeView,&tvis);
			sts.htviInBound = TreeView_InsertItem(sts.hTreeView,&tvis);
			sts.htviOutBound = TreeView_InsertItem(sts.hTreeView,&tvis);

			for (count = 0; sts.stis[count].servername[0]; count++)
			/* Insert servers */
			{
				/* TEMP: Assume only local servers for now */
				tvis.hInsertAfter = TVI_LAST;
				tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;

				tvis.hParent = (sts.stis[count].bOutBound ? sts.htviOutBound : sts.htviInBound);
				tvis.item.iImage = I_IMAGECALLBACK;
				tvis.item.iSelectedImage = tvis.item.iImage;

				tvis.item.pszText = LPSTR_TEXTCALLBACK;

				sts.stis[count].htvi = TreeView_InsertItem(sts.hTreeView,&tvis);
			}
		}
		break;
		case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT) lParam;

			if (lpmis->CtlID == IDC_LBLOG)
			/* Technical status log */
			{
				lpmis->itemHeight = 20;
				lpmis->itemWidth = 500;
			}
		}
		break;
		case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lParam;
			HBITMAP hBMP, hBMPOld;
			TEXTMETRIC tm;
			HDC hdcMem;

			if (lpdis->CtlID == IDC_LBLOG)
			/* Technical status log */
			{
				char buffer[256];
				int y;

				memset(buffer,0,256);

				if (lpdis->itemID == -1)
					break;

				switch (lpdis->itemAction)
				{
					case ODA_SELECT:
					case ODA_DRAWENTIRE:
					{
						hBMP = hBMPListBox[(unsigned char)SendMessage(lpdis->hwndItem, LB_GETITEMDATA, lpdis->itemID,0)];
						hdcMem = CreateCompatibleDC(lpdis->hDC);
						int buflen = 0;

						hBMPOld = (HBITMAP)SelectObject(hdcMem,hBMP);

						BitBlt(lpdis->hDC,lpdis->rcItem.left, lpdis->rcItem.top, lpdis->rcItem.right - lpdis->rcItem.left, lpdis->rcItem.bottom - lpdis->rcItem.top, hdcMem, 0, 0, SRCCOPY);
						//BitBlt(lpdis->hDC,lpdis->rcItem.left, lpdis->rcItem.top, lpdis->rcItem.right - lpdis->rcItem.left, lpdis->rcItem.bottom - lpdis->rcItem.top, hdcMem, 0, 0, SRCCOPY);

						GetTextMetrics(lpdis->hDC,&tm);

						/* Display text */
                        SendMessage(lpdis->hwndItem,LB_GETTEXT,lpdis->itemID,(LPARAM)buffer);
						buflen = (int)strlen(buffer);

						y = (lpdis->rcItem.bottom + lpdis->rcItem.top - tm.tmHeight) / 2;

						TextOut(lpdis->hDC,20,y,buffer,buflen);

						SelectObject(hdcMem,hBMPOld);
						DeleteDC(hdcMem);
					}
					break;
				}
			}
		}
		break;
		case WM_DESTROY:
			{
				if (hwnd == hMain)
				{
					int count;

					/* Tell our I/O thread the party's over */
					scs.signal = SIGNAL_SHUTDOWN;

					/* ...And wait for it to terminate */
					WaitForSingleObject(hSocketThread,INFINITE);

					/* Free up hash tables */
					Hash_Destroy(&HT_Channel);
					Hash_Destroy(&HT_User);
					Hash_Destroy(&HT_Server);
					
					/* Servers treeview imagelist */
					ImageList_Destroy(sts.himl);

					for (count = 0; count < 4; count++)
					/* Free up icon resources(for tech. status box) */
						DeleteObject(hBMPListBox[count]);

					/* Delete server bitmaps */
					DeleteObject(sts.hBMPConnected);
					DeleteObject(sts.hBMPDisconnected);
					DeleteObject(sts.hBMPOutBound);
					DeleteObject(sts.hBMPInBound);

					/* Delete menus */
					DestroyMenu(hSystrayMenu);

					/* Free our settings information */
					IS_FreeBuffers(&SettingsInfo,TRUE);

					/* And free our RA buffers */
					RA_Cleanup();

					Shell_NotifyIcon(NIM_DELETE,&niData);
					Socket_Cleanup();
					StrBuf_Destroy();
					LL_Destroy();

					PostQuitMessage(0);
				}
			}
		break;

		default:
			return FALSE;
		break;
	}

	return TRUE;
}

int CreateSubWindows(HWND hParent)
/*
** CreateSubWindows()
** Function to create all information windows(Status, Servers, etc), and store em
*/
{
	int err;

	MENUITEMINFO mii;

	/* Status window */
	hStatus = CreateDialog(hInst,MAKEINTRESOURCE(IDD_STATUS),hParent,(DLGPROC)MainDlgProc);
	MoveWindow(hStatus,15,32,330,180,TRUE);
	ShowWindow(hStatus,SW_SHOW);

	SetDlgItemInt(hStatus,IDC_EDITPORT,SettingsInfo.isSecurity.RAPort,TRUE);

	/* Servers */
	hServers = CreateDialog(hInst,MAKEINTRESOURCE(IDD_SERVERS),hParent,(DLGPROC)MainDlgProc);
	MoveWindow(hServers,15,32,330,180,TRUE);
	ShowWindow(hServers,SW_HIDE);

	/* Initialize the servers treeview control */
	sts.hTreeView = GetDlgItem(hMain,IDC_TVSERVERS);
	sts.hTreeView = GetDlgItem(hServers,IDC_TVSERVERS);

	if (!sts.hTreeView)
	{
		err = GetLastError();
	}

	/* Add images */
	sts.himl = ImageList_Create(16,16,FALSE,4,0);

	ImageList_Add(sts.himl,sts.hBMPInBound,(HBITMAP)NULL);
	ImageList_Add(sts.himl,sts.hBMPOutBound,(HBITMAP)NULL);
	ImageList_Add(sts.himl,sts.hBMPConnected,(HBITMAP)NULL);
	ImageList_Add(sts.himl,sts.hBMPDisconnected,(HBITMAP)NULL);
	TreeView_SetImageList(sts.hTreeView,sts.himl,TVSIL_NORMAL);

	/* Initialize server list */
	Serverlist_Initialize();
	SendMessage(hMain,WM_UPDATESERVERS,0,0);

	/* Create server popup menus */
	hServersMenu[SM_CONNECTED] = CreatePopupMenu();
	
	AppendMenu(hServersMenu[SM_CONNECTED],MF_STRING,IDM_SM_CONNECTED,"&Disconnect");










	/* Technical log */
	hLog = CreateDialog(hInst,MAKEINTRESOURCE(IDD_LOG),hParent,(DLGPROC)MainDlgProc);
	MoveWindow(hLog,15,32,330,180,TRUE);
	ShowWindow(hLog,SW_HIDE);

	/* About! */
	hAbout = CreateDialog(hInst,MAKEINTRESOURCE(IDD_ABOUT),hParent,(DLGPROC)MainDlgProc);
	MoveWindow(hAbout,15,32,330,180,TRUE);
	ShowWindow(hAbout,SW_HIDE);

	/* Create taskbar notification area icon */
	hSystrayMenu = CreatePopupMenu();

	/* Make first item default */
	AppendMenu(hSystrayMenu,MF_STRING,IDM_SHOW,"&Show RockIRCX");

	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STATE;
	mii.fState = MFS_DEFAULT;
	SetMenuItemInfo(hSystrayMenu,IDM_SHOW,FALSE,&mii);

	/* Other 2 items */
	AppendMenu(hSystrayMenu,MF_STRING,IDM_CONNECT,"&Connect with remote administration");
	AppendMenu(hSystrayMenu,MF_STRING,IDM_SHUTDOWN,"S&hut down server");

	return 0;
}

int	TechLog_AddEntry(char* text, unsigned char icon)
/*
** TechLog_AddEntry()
** Adds an entry to the technical status log, the drawing is handled in the dlgproc
*/
{
	HWND hListBox = GetDlgItem(hLog,IDC_LBLOG);
	int item;
	unsigned long len = (unsigned long)strlen(text);

	item = (int)SendMessage(hListBox,LB_ADDSTRING,0,(LPARAM)text);
	SendMessage(hListBox,LB_SETITEMDATA,item,(LPARAM)icon);

	/* Store max string length in listbox user data */
	if (len > (unsigned long)GetWindowLong(hListBox,GWL_USERDATA))
		SetWindowLong(hListBox,GWL_USERDATA,len);

	SendMessage(hListBox,LB_SETHORIZONTALEXTENT,35 + (5 * GetWindowLong(hListBox,GWL_USERDATA)),0);
	SendMessage(hListBox,LB_SETCURSEL,SendMessage(hListBox,LB_GETCOUNT,0,0) - 1,0);

	if (icon == BITMAP_ERROR)
		TechLog_Focus();

	InvalidateRect(hListBox,NULL,TRUE);

	return 0;
}

void TechLog_Focus()
/*
** TechLog_Focus()
** Brings immediate attention to status log box and focuses on it
*/
{
	SetActiveWindow(hMain);

	TabCtrl_SetCurSel(GetDlgItem(hMain,IDC_TABMAIN),2);

	ShowWindow(hStatus,SW_HIDE);
	ShowWindow(hServers,SW_HIDE);
	ShowWindow(hLog,SW_SHOW);
	ShowWindow(hAbout,SW_HIDE);
}

void Serverlist_Initialize()
/*
** Serverlist_Initialize()
** This function is called to initially set up the server list
*/
{
	int index = 0;

	ServerList *curr = &SettingsInfo.isServers.serverhead;

	memset(&sts.stis,0,sizeof(sts.stis));
	while (curr->next)
	{
		curr = curr->next;

		/* Copy all server data */
		strcpy(sts.stis[index].servername,curr->name);
		strcpy(sts.stis[index].hostmask,curr->hostmask);
		sts.stis[index].bOutBound = (curr->port != 0 ? TRUE : FALSE);
		sts.stis[index].bConnected = FALSE;
		sts.stis[index].port = curr->port;

		index++;
	}
}

void Serverlist_ChangeStatus(const char *servername, BOOL bConnected)
/*
** Serverlist_ChangeStatus()
** Changes the status for the target server, to connected or disconnected
*/
{
	int count;

	for (count = 0; count < MAX_SERVERCONNECTIONS; count++)
	/* Find our target */
	{
		if (!sts.stis[count].hostmask[0])
			break;

		if (strcmp(sts.stis[count].servername,servername) == 0)
		/* Found it! Update & repaint the treeview */
		{
			sts.stis[count].bConnected = bConnected;
			InvalidateRect(sts.hTreeView,NULL,TRUE);
		}
	}

}