/*
** RockIRCX.cpp
**
** RockIRCX remote administration utility
**
** This is a seperate program used to change the settings on a RockIRCX server
** installed on any machine. The purpose of this program is to make configuration
** and setup of the server a piece of cake, while also allowing advanced configuration.
**
** Programmer: Zeb Rasco
*/

#define WIN32_LEAN_AND_MEAN
#include "stdafx.h"
#include "RockIRCXRA.h"
#include "..\Common Files\settings.h"
#include "..\Common Files\functions.h"
#include <stdio.h>
#include <time.h>
#include <commdlg.h>
#include <richedit.h>
#include <shlobj.h>
#include <shellapi.h>
#include <windowsx.h>
#include <process.h>

#define MAX_LOADSTRING 100
#define RA_PROTOVERSIONREQUEST				"NV000"
#define RA_PROTOPASSWORDREQUEST				"RQP"
#define RA_PROTOPASSWORDVALID				"YP"
#define RA_PROTOPASSWORDINVALID				"NP"

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING] = "RockIRCXRA";	// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING] = "RockIRCXRA";	// the main window class name
HWND hMain;										// Handle of main dialog
HWND hActive;									// Handle to active dlg
HWND hStatus;									// Handle of status bar
HWND hRegChan;									// Handle of registered channel window
HANDLE hConnThread;								// Handle to connection thread
HWND hConnStatus;								// Handle of status bar on connect dialog
HWND hConnProgress;								// Handle to progress bar on connect dialog
HWND hTreeView;									// Handle of tree view control
HWND hTabGeneral[2];							// Handles to general tab windows
HWND hTabRegChan[3];							// Handles to registered channel tab windows
HWND hWork;										// "Work window", used for temp stuff
char RAPassword[64];							// Remote administration password
static RegChanList RCLDialog;					// Registered channel data for dialog
ConnectInfo ci;
INFOSTRUCTSETTINGS SettingsInfo;
HBITMAP hBMPListBox[4];


static MOTDColors MOTDArray[] =
{
	{ "00 - White", 16777215 },
	{ "01 - Black", 0 },
	{ "02 - Blue", 8323072 },
	{ "03 - Green", 37632 },
	{ "04 - Light red", 255 },
	{ "05 - Brown", 127 },
	{ "06 - Purple", 10223772 },
	{ "07 - Orange", 32764 },
	{ "08 - Yellow", 65535 },
	{ "09 - Light green", 64512 },
	{ "10 - Cyan", 9671424 },
	{ "11 - Light cyan", 16776960 },
	{ "12 - Light blue", 16515072 },
	{ "13 - Pink", 16711935 },
	{ "14 - Grey", 8355711 },
	{ "15 - Light grey", 13816530 },
	{ "", 0 }
};

/* Global linked list heads */
AccountList accthead;
FilterList filterhead;
BanList banhead;
RegChanList RCLHead;
ServerList serverhead;
SpoofedList spoofedhead;
											
TreeTable tt[] = 
/* Information for tree view control */
{
	{ "RockIRCX.com", NULL, "View server status", IDD_STATUS, NULL, 0 },
	{ "I/O control", NULL, "Settings files, I/O data", IDD_IOCTL, NULL, 0 },
	{ "Technical status", NULL, "View technical status report here", IDD_TECH, NULL, 0 },
	{ "General", NULL, "Change general settings here", IDD_GENERAL, NULL, 0 },
	{ "Accounts", NULL, "View/change all accounts on server", IDD_ACCTS, NULL, 0 },
	{ "Banned hosts", NULL, "Check to see whos banned", IDD_BANS, NULL, 0 },
	{ "Channel settings", NULL, "Check out channel settings", IDD_CHANNEL, NULL, 0 },
	{ "User settings", NULL, "See user settings", IDD_USER, NULL, 0 },
	{ "Linked servers", NULL, "Controls network topology", IDD_SERVERS, NULL, 0 },
	{ "Security", NULL, "Use these controls to edit server security", IDD_SECURITY, NULL, 0 },
	{ "Filtering", NULL, "Cuss word filter", IDD_FILTER, NULL, 0 },
	{ "About RockIRCX", NULL, "About RockIRCX", IDD_ABOUT, NULL, 0 },
	{ NULL, NULL, NULL, 0, NULL, 0 }
};

CHECKBOXHANDLES ckitems[] =
/* Which check boxes enable/disable other items? They go here */
{
	{ IDC_CKUSEDNS, NULL, NULL, { IDC_CKCACHEDNS, IDC_EDITCACHEDNS, IDC_VSCACHEDNS, IDC_CBCACHEDNS, 0 } },
	{ IDC_CKCACHEDNS, NULL, NULL, { IDC_EDITCACHEDNS, IDC_VSCACHEDNS, IDC_CBCACHEDNS, 0 } },
	{ IDC_CKMASK, NULL, NULL, { IDC_GRPMASK, IDC_RDMASK1, IDC_RDMASK2, 0 } },
	{ IDC_CKCDMLIMIT, NULL, NULL, { IDC_TXTCDMLIMIT, IDC_EDITCDMLIMIT, IDC_VSCDMLIMIT, 0 } },
	{ IDC_CKCCAUTOJOIN, NULL, NULL, { IDC_EDITCCAUTOJOIN, IDC_LISTCCAUTOJOIN, IDC_BTNCCAUTOJOIN1, IDC_BTNCCAUTOJOIN2, 0 } },
	{ IDC_CKNICKDELAY, NULL, NULL, { IDC_EDITNICKDELAY, IDC_VSNICKDELAY, 0 } },
	{ IDC_CKMSGDELAY, NULL, NULL, { IDC_EDITMSGDELAY, IDC_VSMSGDELAY, 0 } },
	{ IDC_CKSPOOF, NULL, NULL, { IDC_GRPSPOOF, IDC_CKSPOOF23, IDC_CKSPOOF80, IDC_CKSPOOF1080, IDC_CKSPOOF3182, IDC_CKSPOOF8080, IDC_TXTSPOOFPUNISHMENT, IDC_CBSPOOFPUNISHMENT, 0 } },
	{ IDC_CKFILTER, NULL, NULL, { IDC_BTNADDFILTER, IDC_BTNMODFILTER, IDC_BTNDELFILTER, IDC_BTNREFRESHFILTERS, IDC_TXTFILTERALL, IDC_EDITFILTERALL, IDC_GRPFILTERNICK, IDC_EDITFILTERNICK, IDC_LISTFILTERNICK, IDC_GRPFILTERCHAN, IDC_EDITFILTERCHAN, IDC_LISTFILTERCHAN, IDC_GRPFILTERTOPIC, IDC_EDITFILTERTOPIC, IDC_LISTFILTERTOPIC, IDC_TXTNOTE, 0 } },
	{ IDC_CKBANREASON, NULL, NULL, { IDC_EDITBANREASON, 0 } },
	{ IDC_CKBANEXPIRATION, NULL, NULL, { IDC_EDITBANEXPIRATION, IDC_CBBANEXPIRATION, 0 } },
	{ IDC_CKREGCHANMODEL, NULL, NULL, { IDC_EDITREGCHANMODEL, IDC_VSREGCHANMODEL, 0 } },
	{ IDC_CKACCESSREASON, NULL, NULL, { IDC_EDITACCESSREASON, 0 } },
	{ IDC_CKACCESSEXPIRE, NULL, NULL, { IDC_EDITACCESSEXPIRE, IDC_CBACCESSEXPIRE, 0 } },
	{ 0, NULL, NULL, NULL }
};

SCROLLEDITS scrolledits[] =
/* List of all scroll bars attached to edit controls */
{
	{ IDC_EDITCACHEDNS, IDC_VSCACHEDNS, NULL, NULL },
	{ IDC_EDITCDMLIMIT, IDC_VSCDMLIMIT, NULL, NULL },
	{ IDC_EDITCCMAXCHANS, IDC_VSCCMAXCHANS, NULL, NULL },
	{ IDC_EDITUMUSERSPERIP, IDC_VSUMUSERSPERIP, NULL, NULL },
	{ IDC_EDITUMRECVQ, IDC_VSUMRECVQ, NULL, NULL },
	{ IDC_EDITUMSENDQ, IDC_VSUMSENDQ, NULL, NULL },
	{ IDC_EDITPINGDURATION, IDC_VSPINGDURATION, NULL, NULL },
	{ IDC_EDITPINGRESPONSE, IDC_VSPINGRESPONSE, NULL, NULL },
	{ IDC_EDITNICKDELAY, IDC_VSNICKDELAY, NULL, NULL },
	{ IDC_EDITMSGDELAY, IDC_VSMSGDELAY, NULL, NULL },
	{ IDC_EDITCSMAXACCESS, IDC_VSCSMAXACCESS, NULL, NULL },
	{ IDC_EDITMAXNICKLEN, IDC_VSMAXNICKLEN, NULL, NULL },
	{ IDC_EDITMAXCHANLEN, IDC_VSMAXCHANLEN, NULL, NULL },
	{ IDC_EDITMAXTOPICLEN, IDC_VSMAXTOPICLEN, NULL, NULL },
	{ IDC_EDITMAXMSGLEN, IDC_VSMAXMSGLEN, NULL, NULL },
	{ IDC_EDITRAPORT, IDC_VSRAPORT, NULL, NULL },
	{ IDC_EDITBINDPORT, IDC_VSBINDPORT, NULL, NULL },
	{ IDC_EDITREGCHANMODEL, IDC_VSREGCHANMODEL, NULL, NULL },
	{ IDC_EDITINBOUNDPINGFREQUENCY, IDC_VSINBOUNDPINGFREQUENCY, NULL, NULL },
	{ IDC_EDITINBOUNDPINGRESPONSE, IDC_VSINBOUNDPINGRESPONSE, NULL, NULL },
	{ IDC_EDITOUTBOUNDPORT, IDC_VSOUTBOUNDPORT, NULL, NULL },
	{ IDC_EDITCONNECTPORT, IDC_VSCONNECTPORT, NULL, NULL },
	{ 0, 0 }
};

int RefreshButton[] =
{
	IDC_BTNREFRESHACCTS,
	IDC_BTNREFRESHBANS,
	IDC_BTNREFRESHREGCHANS,
	IDC_BTNREFRESHSERVERS,
	IDC_BTNREFRESHFILTERS,
	0
};

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	MainDlgProc(HWND, UINT, WPARAM, LPARAM);
HWND StatusBar_Create(HWND);
void StatusBar_SetText(HWND, char*, int);
char *StatusBar_GetText(HWND, char*);
void TreeView_Create();
int CreateSubWindows(HWND);
int DUtoPixels(int);
void ListView_AddColumn(HWND, int, char*, int);
void Init();
HWND GetSubItemHandle(int);
void AddToEdit(HWND, int);
void GetTextPathname(char*, unsigned int);
void ProgressBar_SetPos(HWND, unsigned int);
void ConnectToServer(VOID*);
void Disconnect();
void UpdateRAInfo(INFOSTRUCTSETTINGS*);
void SaveRAInfo(INFOSTRUCTSETTINGS*);
void ClearLinkedLists();
void ToggleRefresh(BOOL);
int TechLog_AddEntry(char*, unsigned char);
int	IRCColorFromRGB(COLORREF crRGB);

/* List functions */
void			Account_List(AccountList*, HWND, unsigned char);
void			Filter_List(FilterList*, HWND, unsigned int);
void			Banlist_List(BanList*, HWND, BOOL);
void			RCL_List(RegChanList*, HWND);
void			RCL_ListAccess(HWND, RegChanList*);
void			Server_List(ServerList*, HWND);
void			Spoofed_List(SpoofedList*, HWND);
void			SpoofedLogin_List(SpoofedList*, HWND);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;
	INITCOMMONCONTROLSEX icc;

	// Initialize global strings
	MyRegisterClass(hInstance);

	icc.dwICC = ICC_WIN95_CLASSES;
	icc.dwSize = sizeof(icc);
	InitCommonControls();

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_ROCKIRCXRA);

	/* Create connect dialog */
	if (DialogBox(hInst,MAKEINTRESOURCE(IDD_CONNECT),hMain,(DLGPROC)MainDlgProc) == IDC_BTNCONNECTCLOSE)
		DestroyWindow(hMain);
	else
	{
		char buf[256];

		GetWindowText(hMain,buf,256);
		sprintf(buf,"%s[Connected to %s:%d]",buf,ci.hostname,ci.port);

		SetWindowText(hMain,buf);
	}

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!IsDialogMessage(hActive,&msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;

	/* Stop compiler warnings */
	lpCmdLine; hPrevInstance;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)MainDlgProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_ROCKIRCXRA);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
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

	//hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
	//  CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	hWnd = CreateDialog(GetModuleHandle(NULL),MAKEINTRESOURCE(IDD_MAIN),0,(DLGPROC)MainDlgProc);

	/* Create sub-windows */
	CreateSubWindows(hWnd);

	/* Set focus on tree view control */
	SetFocus(hTreeView);

	hMain = hWnd;

	/* TEMP: Remote admin password is "default" */
	strcpy(RAPassword,"default");

	if (!hWnd)
	{
		char buf[256];
		sprintf(buf,"Window creation failed, error: %d",GetLastError());
		MessageBox(NULL,buf,"Status",MB_OK);
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	//ShellExecute(NULL,"open","D:\\ftp\\myprojects\\\RockIRCX\\RockIRCX\\Debug\\RockIRCX.exe",NULL,"D:\\ftp\\myprojects\\\RockIRCX\\RockIRCX\\",SW_MINIMIZE);

	return TRUE;
}

LRESULT CALLBACK MainDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch(Message)
	{
		case WM_INITDIALOG:
		{
			char buf[256];

			GetWindowText(hwnd,buf,256);

			if (strcmp(buf,"RockIRCX Remote Administration") == 0)
			{
				WSADATA wsadata;
				HICON icon;

				/* Create status bar */
				hStatus = StatusBar_Create(hwnd);
				StatusBar_SetText(hStatus,"RockIRCX Remote Administration ready.",0);

				/* Assign treeview handle & populate */
				hTreeView = GetDlgItem(hwnd,IDC_TREE);
				TreeView_Create();

				/* Set icon */
				icon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_ROCKIRCXRA));
				SendMessage(hwnd,WM_SETICON,TRUE,(LPARAM)icon);

				/* Startup winsock */
				WSAStartup(MAKEWORD(1,1),&wsadata);
			}
			else if (strcmp(buf,"Add binding") == 0)
			{
				int count;
				HWND hAddr;

				/* Find binding scroll information */
				for (count = 0; scrolledits[count].edit != IDC_EDITBINDPORT; count++);

				scrolledits[count].hEditBox = GetDlgItem(hwnd,IDC_EDITBINDPORT);
				scrolledits[count].hScrollBar = GetDlgItem(hwnd,IDC_VSBINDPORT);

				hAddr = GetDlgItem(hwnd,IDC_EDITBINDIPADDR);
				SetWindowText(hAddr,"0.0.0.0");
				SetWindowText(scrolledits[count].hEditBox,"6667");
				SetFocus(hAddr);

			}
			else if (strcmp(buf,"Account") == 0)
			/* Add/Modify Account Dialog */
			{
				HWND boxes[4];
				int count, accttab = TabCtrl_GetCurSel(GetSubItemHandle(IDC_TABACCTS));

				boxes[0] = GetDlgItem(hwnd,IDC_EDITACCTUSERNAME);
				boxes[1] = GetDlgItem(hwnd,IDC_EDITACCTHOSTMASK);
				boxes[2] = GetDlgItem(hwnd,IDC_EDITACCTPASSWORD);
				boxes[3] = GetDlgItem(hwnd,IDC_CBACCTLEVEL);

				/* Set text limits to 32 chars */
				for (count = 0; count < 3; count++)
					SendMessage(boxes[count],EM_SETLIMITTEXT,32,0);

				SendMessage(boxes[3],CB_ADDSTRING,0,(LPARAM)"Power user");
				SendMessage(boxes[3],CB_ADDSTRING,0,(LPARAM)"IRCop");
				SendMessage(boxes[3],CB_ADDSTRING,0,(LPARAM)"Administrator");
				SendMessage(boxes[3],CB_SETCURSEL,accttab,0);

				if (strcmp((char*)lParam,"Add account") == 0)
					SetWindowText(hwnd,(char*)lParam);
				else
				{
					HWND hCurr;
					AccountList* curr = (AccountList*)lParam;

					SetWindowText(hwnd,"Modify account");

					hCurr = GetDlgItem(hwnd,IDC_EDITACCTUSERNAME);
					SetWindowText(boxes[0],curr->username);
					SetWindowText(boxes[1],curr->hostmask);
					SetWindowText(boxes[2],curr->password);
					SendMessage(boxes[3],CB_SETCURSEL,curr->level,0);
				}
			}
			else if (strcmp(buf,"Modify filter") == 0)
			/* "Modify filter" dialog */
			{
				FilterList* curr = (FilterList*)lParam;
				HWND hEdit = GetDlgItem(hwnd,IDC_EDITFILTER);
				HWND hCheckBox[] = { GetDlgItem(hwnd,IDC_CKFILTERNICK), GetDlgItem(hwnd,IDC_CKFILTERCHAN), GetDlgItem(hwnd,IDC_CKFILTERTOPIC) }; 
				unsigned int count, flags[] = { FILTER_TYPE_NICKNAME, FILTER_TYPE_CHANNEL, FILTER_TYPE_TOPIC };

                SetWindowText(hEdit,curr->word);

				for (count = 0; count < 3; count++)
				/* Check all corresponding boxes */
					if (curr->type & flags[count])
						SendMessage(hCheckBox[count],BM_CLICK,0,0);
			}
			else if (strcmp(buf,"Ban") == 0)
			/* Ban dialog */
			{
				int count, count1;
				HWND hComboBox[] = { GetDlgItem(hwnd,IDC_CBBANSCOPE), GetDlgItem(hwnd,IDC_CBBANEXPIRATION) };

				for (count = 0; ckitems[count].ckid != 0; count++)
				/* Add behavior for activating/deactivating check boxes */
				{
					if (ckitems[count].ckid == IDC_CKBANREASON || ckitems[count].ckid == IDC_CKBANEXPIRATION)
					{
						ckitems[count].parent = hwnd;
						ckitems[count].ckhwnd = GetDlgItem(ckitems[count].parent,ckitems[count].ckid);

						for (count1 = 0; ckitems[count].id[count1] != 0; count1++)
							EnableWindow(GetDlgItem(ckitems[count].parent,ckitems[count].id[count1]),FALSE);
					}
				}

				/* Fill in comboboxes with information */
				SendMessage(hComboBox[0],CB_ADDSTRING,0,(LPARAM)"Server");
				SendMessage(hComboBox[0],CB_ADDSTRING,1,(LPARAM)"Network");
				SendMessage(hComboBox[0],CB_SETCURSEL,0,0);

				SendMessage(hComboBox[1],CB_ADDSTRING,0,(LPARAM)"Seconds");
				SendMessage(hComboBox[1],CB_ADDSTRING,1,(LPARAM)"Minutes");
				SendMessage(hComboBox[1],CB_ADDSTRING,2,(LPARAM)"Hours");
				SendMessage(hComboBox[1],CB_ADDSTRING,3,(LPARAM)"Days");
				SendMessage(hComboBox[1],CB_ADDSTRING,4,(LPARAM)"Weeks");
				SendMessage(hComboBox[1],CB_SETCURSEL,0,0);

				if (!lParam)
				/* Add ban dialog box */
					SetWindowText(hwnd,"Add ban");
				else
				/* Modify ban dialog box */
				{
					HWND hBoxes[] = { GetDlgItem(hwnd,IDC_EDITBANHOSTMASK), GetDlgItem(hwnd,IDC_EDITBANREASON), GetDlgItem(hwnd,IDC_EDITBANEXPIRATION) };
					BanList* curr = (BanList*)lParam;

					SetWindowText(hwnd,"Modify ban");
					SetWindowText(hBoxes[0],curr->hostmask);
					SendMessage(hComboBox[0],CB_SETCURSEL,(WPARAM)curr->bNetworkBan,0);
					
					if (curr->reason)
					{
						SetWindowText(hBoxes[1],curr->reason);
						SendMessage(GetDlgItem(hwnd,IDC_CKBANREASON),BM_CLICK,0,0);
					}

					if (curr->expiration)
					{
						char buf[32];

						sprintf(buf,"%d",curr->expiration);
						SendMessage(GetDlgItem(hwnd,IDC_CKBANEXPIRATION),BM_CLICK,0,0);
						SetWindowText(hBoxes[2],buf);
						SendMessage(hComboBox[1],CB_SETCURSEL,curr->exptype,0);
					}
				}

			}
			else if (strcmp(buf,"Exception") == 0)
			/* Exception dialog */
			{
				HWND hEdit = GetDlgItem(hwnd,IDC_EDITEXCEPTIONHOSTMASK);

				if (!lParam)
				/* Add exception */
					SetWindowText(hwnd,"Add exception");
				else
				/* Modify exception */
				{
					SetWindowText(hwnd,"Modify exception");
					SetWindowText(hEdit,((BanList*)lParam)->hostmask);
				}
			}
			else if (strcmp(buf,"Registered channel") == 0)
			/* Registered channel dialog */
			{
				int count, count1, nWidth = 275, nHeight = 335;
				TC_ITEM pitem;
				HWND hCB, hListView, hTab = GetDlgItem(hwnd,IDC_TABREGCHAN);

				hRegChan = hwnd;

				/* Set tabs for text mode */
				pitem.mask = TCIF_TEXT;

				/* Add tab pages */
				pitem.pszText = "General";
				TabCtrl_InsertItem(hTab,0,&pitem);

				pitem.pszText = "Access";
				TabCtrl_InsertItem(hTab,1,&pitem);

				pitem.pszText = "Modes";
				TabCtrl_InsertItem(hTab,2,&pitem);

				hTabRegChan[0] = CreateDialog(hInst,MAKEINTRESOURCE(IDD_REGCHANGENERAL),hwnd,(DLGPROC)MainDlgProc);
				MoveWindow(hTabRegChan[0],15,32,nWidth,nHeight,TRUE);
				ShowWindow(hTabRegChan[0],SW_SHOW);

				hTabRegChan[1] = CreateDialog(hInst,MAKEINTRESOURCE(IDD_REGCHANACCESS),hwnd,(DLGPROC)MainDlgProc);
				MoveWindow(hTabRegChan[1],15,32,nWidth,nHeight,TRUE);
				ShowWindow(hTabRegChan[1],SW_HIDE);

				hTabRegChan[2] = CreateDialog(hInst,MAKEINTRESOURCE(IDD_REGCHANMODES),hwnd,(DLGPROC)MainDlgProc);
				MoveWindow(hTabRegChan[2],15,32,nWidth,nHeight,TRUE);
				ShowWindow(hTabRegChan[2],SW_HIDE);

				for (count = 0; ckitems[count].ckid != 0; count++)
				/* Add behavior for activating/deactivating mode limit check box */
				{
					if (ckitems[count].ckid == IDC_CKREGCHANMODEL)
					{
						ckitems[count].parent = hTabRegChan[2];
						ckitems[count].ckhwnd = GetDlgItem(ckitems[count].parent,ckitems[count].ckid);

						for (count1 = 0; ckitems[count].id[count1] != 0; count1++)
							EnableWindow(GetDlgItem(ckitems[count].parent,ckitems[count].id[count1]),FALSE);
					}
				}

				/* Find binding scroll information */
				for (count = 0; scrolledits[count].edit != IDC_EDITREGCHANMODEL; count++);

				scrolledits[count].hEditBox = GetDlgItem(hTabRegChan[2],IDC_EDITREGCHANMODEL);
				scrolledits[count].hScrollBar = GetDlgItem(hTabRegChan[2],IDC_VSREGCHANMODEL);

				SetWindowText(scrolledits[count].hEditBox,"50");

				/* Fill in access buttons */
				SendMessage(GetDlgItem(hTabRegChan[1],IDC_BTNREGCHANADDACCESS),BM_SETIMAGE,(WPARAM)IMAGE_BITMAP,(LPARAM)LoadImage(hInst,MAKEINTRESOURCE(IDB_ADD),IMAGE_BITMAP,0,0,LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT));
				SendMessage(GetDlgItem(hTabRegChan[1],IDC_BTNREGCHANMODACCESS),BM_SETIMAGE,(WPARAM)IMAGE_BITMAP,(LPARAM)LoadImage(hInst,MAKEINTRESOURCE(IDB_MODIFY),IMAGE_BITMAP,0,0,LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT));
				SendMessage(GetDlgItem(hTabRegChan[1],IDC_BTNREGCHANDELACCESS),BM_SETIMAGE,(WPARAM)IMAGE_BITMAP,(LPARAM)LoadImage(hInst,MAKEINTRESOURCE(IDB_DELETE),IMAGE_BITMAP,0,0,LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT));
				SendMessage(GetDlgItem(hTabRegChan[1],IDC_BTNREGCHANREFRESHACCESS),BM_SETIMAGE,(WPARAM)IMAGE_BITMAP,(LPARAM)LoadImage(hInst,MAKEINTRESOURCE(IDB_REFRESH),IMAGE_BITMAP,0,0,LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT));

				/* Add columns to access list view */
				hListView = GetDlgItem(hTabRegChan[1],IDC_LVREGCHANACCESS);

				ListView_AddColumn(hListView,0,"Hostmask",85);
				ListView_AddColumn(hListView,1,"Type",60);
				ListView_AddColumn(hListView,2,"Reason",95);

				/* Set up visibility combobox */
				hCB = GetDlgItem(hTabRegChan[2],IDC_CBREGCHANMODEVISIBILITY);

				SendMessage(hCB,CB_ADDSTRING,0,(LPARAM)"Public");
				SendMessage(hCB,CB_ADDSTRING,1,(LPARAM)"Private");
				SendMessage(hCB,CB_ADDSTRING,2,(LPARAM)"Hidden");
				SendMessage(hCB,CB_ADDSTRING,3,(LPARAM)"Secret");
				SendMessage(hCB,CB_SETCURSEL,0,0);

				memset(&RCLDialog,0,sizeof(RegChanList));

				if (lParam)
				/* Modify registered channel entry */
				{
					RegChanList *curr = (RegChanList*)lParam;

					HWND hEditBox[] = { 
						GetDlgItem(hTabRegChan[0],IDC_EDITREGCHANNAME),
						GetDlgItem(hTabRegChan[0],IDC_EDITREGCHANSUBJECT),
						GetDlgItem(hTabRegChan[0],IDC_EDITREGCHANTOPIC),
						GetDlgItem(hTabRegChan[0],IDC_EDITREGCHANONJOIN),
						GetDlgItem(hTabRegChan[0],IDC_EDITREGCHANONPART),
						GetDlgItem(hTabRegChan[1],IDC_EDITREGCHANOWNERKEY),
						GetDlgItem(hTabRegChan[1],IDC_EDITREGCHANHOSTKEY),
						GetDlgItem(hTabRegChan[1],IDC_EDITREGCHANMEMBERKEY),
						GetDlgItem(hTabRegChan[2],IDC_EDITREGCHANMODEL)
					};
					HWND hCheckBox[] = {
						GetDlgItem(hTabRegChan[2],IDC_CKREGCHANMODEA),
						GetDlgItem(hTabRegChan[2],IDC_CKREGCHANMODEM),
						GetDlgItem(hTabRegChan[2],IDC_CKREGCHANMODET),
						GetDlgItem(hTabRegChan[2],IDC_CKREGCHANMODEW),
						GetDlgItem(hTabRegChan[2],IDC_CKREGCHANMODEI),
						GetDlgItem(hTabRegChan[2],IDC_CKREGCHANMODEN),
						GetDlgItem(hTabRegChan[2],IDC_CKREGCHANMODEU),
						GetDlgItem(hTabRegChan[2],IDC_CKREGCHANMODEX),
						NULL
					};
					unsigned short modeflag[] = {
						ZRA_CHANMODE_AUTHONLY,
						ZRA_CHANMODE_MODERATED,
						ZRA_CHANMODE_ONLYOPSCHANGETOPIC,
						ZRA_CHANMODE_NOWHISPERS,
						ZRA_CHANMODE_INVITEONLY,
						ZRA_CHANMODE_NOEXTERN,
						ZRA_CHANMODE_KNOCK,
						ZRA_CHANMODE_AUDITORIUM,
						0
					};

					SetWindowLongPtr(hwnd,GWL_USERDATA,(LONG)((LONGLONG)curr));
					SetWindowText(hwnd,"Modify registered channel");

					/* Set general information */
					SetWindowText(hEditBox[0],curr->name);
					SetWindowText(hEditBox[1],curr->subject);
					SetWindowText(hEditBox[2],curr->topic);
					SetWindowText(hEditBox[3],curr->onjoin);
					SetWindowText(hEditBox[4],curr->onpart);

					/* Set access information */

					if (curr->accesshead.next)
					/* Create new access entry list */
					{
						AccessList *aCurr = &curr->accesshead;

						while (aCurr->next)
						{
							aCurr = aCurr->next;

							RCL_AddAccess(&RCLDialog,aCurr);
						}
					}

					RCL_ListAccess(GetDlgItem(hTabRegChan[1],IDC_LVREGCHANACCESS),&RCLDialog);

					SetWindowText(hEditBox[5],curr->ownerkey);
					SetWindowText(hEditBox[6],curr->hostkey);
					SetWindowText(hEditBox[7],curr->memberkey);

					/* Set mode information */
					SendMessage(GetDlgItem(hTabRegChan[2],IDC_CBREGCHANMODEVISIBILITY),CB_SETCURSEL,(WPARAM)curr->visibility,0);

					if (curr->limit)
					{
						char buf[32];

						sprintf(buf,"%d",curr->limit);
                        SendMessage(GetDlgItem(hTabRegChan[2],IDC_CKREGCHANMODEL),BM_CLICK,0,0);
						SetWindowText(hEditBox[8],buf);
					}

					for (count = 0; modeflag[count] != 0; count++)
                        if (curr->modeflags & modeflag[count])
							SendMessage(hCheckBox[count],BM_CLICK,0,0);
				}
				else
				{
					SetWindowText(hwnd,"Add registered channel");
					EnableWindow(GetDlgItem(hTabRegChan[1],IDC_BTNREGCHANREFRESHACCESS),FALSE);
				}

				SetFocus(GetDlgItem(hTabRegChan[0],IDC_EDITREGCHANNAME));
				return FALSE;
			}
			else if (strcmp(buf,"Access") == 0)
			{
				int count;
				HWND hComboBox[] = { GetDlgItem(hwnd,IDC_CBACCESSTYPE), GetDlgItem(hwnd,IDC_CBACCESSEXPIRE) };

				SendMessage(hComboBox[0],CB_ADDSTRING,0,(LPARAM)"OWNER");
				SendMessage(hComboBox[0],CB_ADDSTRING,1,(LPARAM)"HOST");
				SendMessage(hComboBox[0],CB_ADDSTRING,2,(LPARAM)"VOICE");
				SendMessage(hComboBox[0],CB_ADDSTRING,3,(LPARAM)"GRANT");
				SendMessage(hComboBox[0],CB_ADDSTRING,4,(LPARAM)"DENY");
				SendMessage(hComboBox[0],CB_SETCURSEL,0,0);

				SendMessage(hComboBox[1],CB_ADDSTRING,0,(LPARAM)"Seconds");
				SendMessage(hComboBox[1],CB_ADDSTRING,1,(LPARAM)"Minutes");
				SendMessage(hComboBox[1],CB_ADDSTRING,2,(LPARAM)"Hours");
				SendMessage(hComboBox[1],CB_SETCURSEL,0,0);

				for (count = 0; ckitems[count].ckid != 0; count++)
				/* Add behavior for activating/deactivating check boxes */
				{
					if (ckitems[count].ckid == IDC_CKACCESSEXPIRE || ckitems[count].ckid == IDC_CKACCESSREASON)
					{
						int count1;

						ckitems[count].parent = hwnd;
						ckitems[count].ckhwnd = GetDlgItem(ckitems[count].parent,ckitems[count].ckid);

						for (count1 = 0; ckitems[count].id[count1] != 0; count1++)
							EnableWindow(GetDlgItem(ckitems[count].parent,ckitems[count].id[count1]),FALSE);
					}
				}

				if (lParam)
				/* Fill in information with access entry data */
				{
					HWND hEditBox[] = { GetDlgItem(hwnd,IDC_EDITACCESSHOSTMASK), GetDlgItem(hwnd,IDC_EDITACCESSREASON), GetDlgItem(hwnd,IDC_EDITACCESSEXPIRE) };
					HWND hCheckBox[] = { GetDlgItem(hwnd,IDC_CKACCESSREASON), GetDlgItem(hwnd,IDC_CKACCESSEXPIRE) };

					AccessList *curr = (AccessList*)lParam;

					SetWindowText(hwnd,"Modify access entry");

					SetWindowText(hEditBox[0],curr->hostmask);
					SendMessage(hComboBox[0],CB_SETCURSEL,(WPARAM)curr->type,0);

					if (curr->reason)
					{
						SendMessage(hCheckBox[0],BM_CLICK,0,0);
						SetWindowText(hEditBox[1],curr->reason);
					}
					if (curr->expire)
					{
						char buf[16];

						SendMessage(hCheckBox[1],BM_CLICK,0,0);
						sprintf(buf,"%d",curr->expire);
						SetWindowText(hEditBox[2],buf);
						SendMessage(hComboBox[1],CB_SETCURSEL,(WPARAM)curr->exptype,0);
					}

				}
				else
				/* New access entry */
					SetWindowText(hwnd,"Add access entry");
			}
			else if (strcmp(buf,"Server details") == 0)
			/* Server details dialog */
			{				
				HWND hEditBox[] = { GetDlgItem(hwnd,IDC_EDITINBOUNDNAME), GetDlgItem(hwnd,IDC_EDITINBOUNDHOSTNAME), GetDlgItem(hwnd,IDC_EDITINBOUNDPINGFREQUENCY), GetDlgItem(hwnd,IDC_EDITINBOUNDPINGRESPONSE), GetDlgItem(hwnd,IDC_EDITINBOUNDPASSWORD), GetDlgItem(hwnd,IDC_EDITOUTBOUNDPORT) };
				HWND hCBEncryption = GetDlgItem(hwnd,IDC_CBSERVERENCRYPTION);
				int count;

				ComboBox_AddString(hCBEncryption,"None");
				ComboBox_AddString(hCBEncryption,"SSL");
				ComboBox_AddString(hCBEncryption,"AES");
				ComboBox_SetCurSel(hCBEncryption,0);

				SetWindowText(hEditBox[2],"90");
				SetWindowText(hEditBox[3],"90");

				SetWindowText(hEditBox[5],"6667");

				/* Set default to inbound server */
				Button_SetCheck(GetDlgItem(hwnd,IDC_RDSERVERINBOUND),TRUE);
				Button_SetCheck(GetDlgItem(hwnd,IDC_RDSERVEROUTBOUND),FALSE);
				EnableWindow(GetDlgItem(hwnd,IDC_TXTOUTBOUNDPORT),FALSE);
				EnableWindow(GetDlgItem(hwnd,IDC_EDITOUTBOUNDPORT),FALSE);
				EnableWindow(GetDlgItem(hwnd,IDC_VSOUTBOUNDPORT),FALSE);

				for (count = 0; scrolledits[count].edit != NULL; count++)
				{
					if (scrolledits[count].edit == IDC_EDITINBOUNDPINGFREQUENCY || scrolledits[count].edit == IDC_EDITINBOUNDPINGRESPONSE || scrolledits[count].edit == IDC_EDITOUTBOUNDPORT)
					{
						scrolledits[count].hEditBox = GetDlgItem(hwnd,scrolledits[count].edit);
						scrolledits[count].hScrollBar = GetDlgItem(hwnd,scrolledits[count].scrollbar);
					}
				}

				if (!lParam)
				/* Add server */
				{
					SetWindowText(hwnd,"Add new server");
				}
				else
				/* Modify server */
				{
					ServerList *curr = (ServerList*)lParam;
					char buf[256];

					SetWindowText(hwnd,"Edit server details");

					SetWindowText(hEditBox[0],curr->name);
					SetWindowText(hEditBox[1],curr->hostmask);
					SetWindowText(hEditBox[4],curr->password);

					/* Set encryption type */
					ComboBox_SetCurSel(hCBEncryption,curr->encryption);

					if (curr->port)
					/* Outbound server */
					{
						SendMessage(GetDlgItem(hwnd,IDC_RDSERVEROUTBOUND),BM_CLICK,0,0);
						
						sprintf(buf,"%d",curr->port);
						SetWindowText(hEditBox[5],buf);
					}
					else
					/* Inbound server */
					{
						SendMessage(GetDlgItem(hwnd,IDC_RDSERVERINBOUND),BM_CLICK,0,0);
						
						sprintf(buf,"%d",curr->ping_frequency); SetWindowText(hEditBox[2],buf);
						sprintf(buf,"%d",curr->ping_response); SetWindowText(hEditBox[3],buf);
					}

				
				}
            }
			else if (strcmp(buf,"Spoofed hosts detected") == 0)
			/* Spoofed hosts detected dialog */
			{
				HWND hListView = GetDlgItem(hwnd,IDC_LVDETECTEDHOSTS);

				ListView_AddColumn(hListView,0,"Hostname",125);
				ListView_AddColumn(hListView,1,"Port(s)",50);
				ListView_AddColumn(hListView,2,"Logins",50);
				ListView_AddColumn(hListView,3,"Last attempt",125);

				Spoofed_List(&spoofedhead, hListView);
			}

			else if (strcmp(buf,"Details for host") == 0)
			/* Details for spoofed host dialog */
			{
				HWND hEditBox[] = { GetDlgItem(hwnd,IDC_EDITDETAILSHOSTMASK), GetDlgItem(hwnd,IDC_EDITDETAILSSPOOFEDPORTS), GetDlgItem(hwnd,IDC_EDITDETAILSSERVER), GetDlgItem(hwnd,IDC_EDITDETAILSPORT) };
				HWND hTxtTotal = GetDlgItem(hwnd,IDC_TXTDETAILSTOTAL);
				HWND hListBox = GetDlgItem(hwnd,IDC_LISTDETAILSLOGINS);
				SpoofedList *curr = (SpoofedList*)lParam;
				char buf[128], temp[16];
				int count;

                SpoofedLogin_List(curr,hListBox);
				SendMessage(hListBox,LB_SETCURSEL,0,0);

				SetWindowText(hEditBox[0],curr->loginhead.next->hostmask);
				
				for (count = 0; curr->port[count] != 0; count++)
				{
					if (count == 0)
						sprintf(buf,"%d",curr->port[count]);
					else
					{
						sprintf(temp,", %d",curr->port[count]);
						strcat(buf,temp);
					}
				}
				SetWindowText(hEditBox[1],buf);
				SetWindowText(hEditBox[2],curr->loginhead.next->server);
				sprintf(buf,"%d",curr->loginhead.next->port);
				SetWindowText(hEditBox[3],buf);

				sprintf(buf,"%d",SpoofedLogin_Logins(curr));
				SetWindowText(hTxtTotal,buf);
			}
			else if (strcmp(buf,"Report for host") == 0)
			/* "Report for host" dialog box */
			{
				char *windowbuf = (char*)calloc(1,256);
				int length;
				SpoofedList *sTarget = (SpoofedList*)lParam;
				SpoofedLoginLog *curr = &sTarget->loginhead;
				char buf[256];

				windowbuf[0] = 0;

				while (curr->next != NULL)
				{
					curr = curr->next;

					SpoofedLogin_Report(sTarget,curr,buf);
					strcat(buf,"\r\n");

					length = (int)strlen(windowbuf);
					windowbuf = (char*)(realloc(windowbuf,strlen(buf) + strlen(windowbuf) + 1));
					windowbuf[length] = 0;
					strcat(windowbuf,buf);
				}

				SetWindowText(GetDlgItem(hwnd,IDC_EDITREPORT),windowbuf);
				free(windowbuf);

				SetFocus(GetDlgItem(hwnd,IDC_BTNREPORTCOPY));

				return FALSE;
			}
			else if (strcmp(buf,"Export detected hosts") == 0)
			/* Export detected hosts dialog */
			{
				char buf[MAX_PATH];

				GetTextPathname(buf,MAX_PATH);

				SetWindowText(GetDlgItem(hwnd,IDC_EDITEXPORTFOLDER),buf);
				SendMessage(GetDlgItem(hwnd,IDC_BTNEXPORTFOLDER),BM_SETIMAGE,(WPARAM)IMAGE_BITMAP,(LPARAM)LoadImage(hInst,MAKEINTRESOURCE(IDB_FOLDER),IMAGE_BITMAP,0,0,LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT));
				SendDlgItemMessage(hwnd,IDC_RDEXPORTONEFILE,BM_CLICK,0,0);
			}
			else if (strcmp(buf,"Connect to remote server") == 0)
			/* Connect dialog */
			{
				RECT rect;
				int count;

				/* Set window handle for connection info & socket to 0(not connected)*/
				ci.hConnect = hwnd;
				ci.socket = 0;
				ci.refresh = 0;

				/* Add editbox scrolling capability */
				for (count = 0; scrolledits[count].edit != NULL; count++)
				{
					if (scrolledits[count].edit == IDC_EDITCONNECTPORT)
					{
						scrolledits[count].hEditBox = GetDlgItem(hwnd,scrolledits[count].edit);
						scrolledits[count].hScrollBar = GetDlgItem(hwnd,scrolledits[count].scrollbar);
					}
				}

				/* Create status bar */
				hConnStatus = StatusBar_Create(hwnd);
				StatusBar_SetText(hConnStatus,"Ready.",0);

				/* Create progress bar */
				GetClientRect(hConnStatus,&rect);
				hConnProgress = CreateWindowEx(NULL,PROGRESS_CLASS,NULL,WS_CHILD | WS_VISIBLE, rect.right - 75, rect.top + 4, 70, rect.bottom - rect.top - 6,hConnStatus,NULL,hInst,NULL);

				/* Set default information */
				SetDlgItemText(hwnd,IDC_EDITCONNECTHOSTNAME,"127.0.0.1");
				SetDlgItemText(hwnd,IDC_EDITCONNECTPORT,"6932");
			}
		}
		break;
		case WM_NOTIFY:
		{
			int idCtrl = (int) wParam;
			LPNMHDR pnmh = (LPNMHDR) lParam;

			switch (pnmh->idFrom)
			{
				case IDC_LVDETECTEDHOSTS:
				{                    
					if (pnmh->code == LVN_KEYDOWN)
					{
						LV_KEYDOWN* lv_keydown = (LV_KEYDOWN*)lParam;

						if (lv_keydown->wVKey == VK_DELETE)
						/* DEL key pressed, click the "Delete spoofed hosts" button */
							SendMessage(GetDlgItem(hwnd,IDC_BTNDETECTEDHOSTSDELETE),BM_CLICK,0,0);
					}
					else if (pnmh->code == LVN_GETDISPINFO)
					/* System is requesting info for registered channel entry sub-items */
					{
						LV_DISPINFO *plvdi = (LV_DISPINFO*)lParam;
						SpoofedList *curr = (SpoofedList*)plvdi->item.lParam;
						char buf[256], temp[32];

						switch (plvdi->item.iSubItem)
						/* Column #1 = hostname, #2 = Port(s), #3 = Logins, #4 = Last attempt */
						{
							case 0:
								plvdi->item.pszText = curr->hostname;
							break;
							case 1:
							{
								int count;

								for (count = 0; curr->port[count] != 0; count++)
								{
									if (count == 0)
										sprintf(buf,"%d",curr->port[count]);
									else
									{
										sprintf(temp,", %d",curr->port[count]);
										strcat(buf,temp);
									}
								}
								plvdi->item.pszText = buf;
							}
							break;
							case 2:
							{
								sprintf(buf,"%d",SpoofedLogin_Logins(curr));
								plvdi->item.pszText = buf;
							}
							break;
							case 3:
							{
								char targetbuf[64];

								time_t tm = SpoofedLogin_Last(curr);
								strcpy(buf,ctime(&tm));
								buf[strlen(buf) - 1] = 0;
								ConvertTime(buf,targetbuf);
								plvdi->item.pszText = targetbuf;
							}
							break;
						}
						plvdi->item.mask = plvdi->item.mask | LVIF_DI_SETITEM;
					}
				}
				break;
				case IDC_LVINBOUND:
					if (pnmh->code == LVN_KEYDOWN)
					{
						LV_KEYDOWN* lv_keydown = (LV_KEYDOWN*)lParam;

						if (lv_keydown->wVKey == VK_DELETE)
						/* DEL key pressed, click the "Delete inbound server " button */
							SendMessage(GetSubItemHandle(IDC_BTNDELSERVER),BM_CLICK,0,0);
					}
					else if (pnmh->code == LVN_GETDISPINFO)
					/* System is requesting info for server entry sub-items */
					{
						LV_DISPINFO* plvdi = (LV_DISPINFO*)lParam;
						ServerList* curr = (ServerList*)plvdi->item.lParam;
						char buf[32];

						switch (plvdi->item.iSubItem)
						/* Column #1 = server name, Column #2 = expected IP/hostname, Column #3 = ping frequency/response OR port */
						{
							case 0:
								plvdi->item.pszText = curr->name;
							break;
							case 1:
								plvdi->item.pszText = curr->hostmask;
							break;
							case 2:
							{
								sprintf(buf,"%d",curr->port);
								plvdi->item.pszText = buf;
							}
							break;
						}
						plvdi->item.mask = plvdi->item.mask | LVIF_DI_SETITEM;
					}
					else if (pnmh->code == NM_DBLCLK)
						SendDlgItemMessage(GetParent(pnmh->hwndFrom),IDC_BTNMODSERVER,BM_CLICK,0,0);
				break;
				case IDC_LVREGCHAN:
				{                    
					if (pnmh->code == LVN_KEYDOWN)
					{
						LV_KEYDOWN* lv_keydown = (LV_KEYDOWN*)lParam;

						if (lv_keydown->wVKey == VK_DELETE)
						/* DEL key pressed, click the "Delete registered channel" button */
							SendMessage(GetSubItemHandle(IDC_BTNDELREGCHAN),BM_CLICK,0,0);
					}
					else if (pnmh->code == LVN_GETDISPINFO)
					/* System is requesting info for registered channel entry sub-items */
					{
						LV_DISPINFO* plvdi = (LV_DISPINFO*)lParam;
						RegChanList* curr = (RegChanList*)plvdi->item.lParam;

						switch (plvdi->item.iSubItem)
						/* Column #1 = channel name, Column #2 = topic */
						{
							case 0:
								plvdi->item.pszText = curr->name;
							break;
							case 1:
								plvdi->item.pszText = curr->topic;
							break;
						}
						plvdi->item.mask = plvdi->item.mask | LVIF_DI_SETITEM;
					}
					else if (pnmh->code == NM_DBLCLK)
						SendDlgItemMessage(GetParent(pnmh->hwndFrom),IDC_BTNMODREGCHAN,BM_CLICK,0,0);
				}
				break;
				case IDC_LVREGCHANACCESS:
				{                    
					if (pnmh->code == LVN_KEYDOWN)
					{
						LV_KEYDOWN* lv_keydown = (LV_KEYDOWN*)lParam;

						if (lv_keydown->wVKey == VK_DELETE)
						/* DEL key pressed, click the "Delete access entry" button */
							SendMessage(GetSubItemHandle(IDC_BTNREGCHANDELACCESS),BM_CLICK,0,0);
					}
					else if (pnmh->code == LVN_GETDISPINFO)
					/* System is requesting info for access entry sub-items */
					{
						LV_DISPINFO* plvdi = (LV_DISPINFO*)lParam;
						AccessList* curr = (AccessList*)plvdi->item.lParam;
						int level[] = { ZRA_ACCESSLEVEL_OWNER, ZRA_ACCESSLEVEL_HOST, ZRA_ACCESSLEVEL_VOICE, ZRA_ACCESSLEVEL_GRANT, ZRA_ACCESSLEVEL_DENY };
						char *levelstr[] = { "OWNER", "HOST", "VOICE", "GRANT", "DENY" };

						switch (plvdi->item.iSubItem)
						/* Column #1 = hostmask, Column #2 = type, Column #3 = reason */
						{
							case 0:
								plvdi->item.pszText = curr->hostmask;
							break;
							case 1:
							{
								int count;

								for (count = 0; count < 5; count++)
								{
									if (curr->type == level[count])
									{
										plvdi->item.pszText = levelstr[count];
										break;
									}
								}
							}
							break;
							case 2:
								plvdi->item.pszText = curr->reason;
							break;

						}
						plvdi->item.mask = plvdi->item.mask | LVIF_DI_SETITEM;
					}
					else if (pnmh->code == NM_DBLCLK)
						SendDlgItemMessage(GetParent(pnmh->hwndFrom),IDC_BTNREGCHANMODACCESS,BM_CLICK,0,0);
				}
				break;
				case IDC_LVACCTS:
				{                    
					if (pnmh->code == LVN_KEYDOWN)
					{
						LV_KEYDOWN* lv_keydown = (LV_KEYDOWN*)lParam;

						if (lv_keydown->wVKey == VK_DELETE)
						/* DEL key pressed, click the "Delete account" button */
							SendMessage(GetSubItemHandle(IDC_BTNDELACCT),BM_CLICK,0,0);
					}
					else if (pnmh->code == LVN_GETDISPINFO)
					/* System is requesting info for account list sub-items */
					{
						LV_DISPINFO* plvdi = (LV_DISPINFO*)lParam;
						AccountList* curr = (AccountList*)plvdi->item.lParam;
						char buf[8];

						switch (plvdi->item.iSubItem)
						/* Column #1 = username, Column #2 = hostmask, and Column #3 = logins */
						{
							case 0:
								plvdi->item.pszText = curr->username;
							break;
							case 1:
								plvdi->item.pszText = curr->hostmask;
							break;
							case 2:
							{
								sprintf(buf,"%d",curr->logins);
								plvdi->item.pszText = buf;
							}
						}

						plvdi->item.mask = plvdi->item.mask | LVIF_DI_SETITEM;
					}
					else if (pnmh->code == NM_DBLCLK)
						SendDlgItemMessage(GetParent(pnmh->hwndFrom),IDC_BTNMODACCT,BM_CLICK,0,0);
				}
				break;
				case IDC_LVBANS:
				{                    
					if (pnmh->code == LVN_KEYDOWN)
					{
						LV_KEYDOWN* lv_keydown = (LV_KEYDOWN*)lParam;

						if (lv_keydown->wVKey == VK_DELETE)
						/* DEL key pressed, click the "Delete ban" button */
							SendMessage(GetSubItemHandle(IDC_BTNDELBAN),BM_CLICK,0,0);
					}
					else if (pnmh->code == LVN_GETDISPINFO)
					/* System is requesting info for account list sub-items */
					{
						LV_DISPINFO* plvdi = (LV_DISPINFO*)lParam;
						BanList* curr = (BanList*)plvdi->item.lParam;
						char buf[8];

						switch (plvdi->item.iSubItem)
						/* Column #1 = hostmask, Column #2 = scope, and Column #3 = reason */
						{
							case 0:
								plvdi->item.pszText = curr->hostmask;
							break;
							case 1:
							{
								sprintf(buf,"%s",curr->bNetworkBan ? "Network" : "Server");
								plvdi->item.pszText = buf;
							}
							break;
							case 2:
								plvdi->item.pszText = curr->reason;
						}

						plvdi->item.mask = plvdi->item.mask | LVIF_DI_SETITEM;
					}
					else if (pnmh->code == NM_DBLCLK)
						SendDlgItemMessage(GetParent(pnmh->hwndFrom),IDC_BTNMODBAN,BM_CLICK,0,0);
				}
				break;
				case IDC_LVEXCEPTIONS:
				{
					if (pnmh->code == NM_DBLCLK)
						SendDlgItemMessage(GetParent(pnmh->hwndFrom),IDC_BTNMODBAN,BM_CLICK,0,0);
				}
				break;
				case IDC_TREE:
					if (pnmh->code == TVN_SELCHANGED)
					/* Treeview selection has changed */
					{
						int count, idold = -1, idnew = -1;
						NM_TREEVIEW* pnmtv = (NM_TREEVIEW*)lParam;

						for (count = 0; tt[count].text != NULL; count++)
						/* Obtain indexes for new & old tree table structures */
						{
							if (tt[count].hItem == pnmtv->itemOld.hItem)
								idold = count;
							else if (tt[count].hItem == pnmtv->itemNew.hItem)
								idnew = count;
						}

						if (idnew == -1 || idold == -1)
							break;

						/* Code to switch windows & status */
						StatusBar_SetText(hStatus,tt[idnew].status,0);

						hActive = tt[idnew].hwnd;
						ShowWindow(tt[idold].hwnd,SW_HIDE);
						ShowWindow(tt[idnew].hwnd,SW_SHOW);
					}
				break;
				case IDC_TABREGCHAN:
				{
					int tab = TabCtrl_GetCurSel(pnmh->hwndFrom);
					
					if (pnmh->code == TCN_SELCHANGE)
					/* Selection is changing */
					{
						int count;

						for (count = 0; count < 3; count++)
						{
							if (count == tab)
								ShowWindow(hTabRegChan[count],SW_SHOW);
							else
								ShowWindow(hTabRegChan[count],SW_HIDE);
						}
					}
				}
				break;
				case IDC_TABGENERAL:
				{
					int tab = TabCtrl_GetCurSel(pnmh->hwndFrom);

					if (pnmh->code == TCN_SELCHANGE)
					/* Selection is changing */
					{
						ShowWindow(hTabGeneral[tab == 0 ? 0 : 1],SW_SHOW);
						ShowWindow(hTabGeneral[tab == 0 ? 1 : 0],SW_HIDE);
					}
				}
				break;
				case IDC_TABBANS:
				{
					int tab = TabCtrl_GetCurSel(pnmh->hwndFrom);

					if (pnmh->code == TCN_SELCHANGE)
					/* Hide the appropriate control */
					{
						HWND hCurr = tt[5].hwnd, hBanList = GetDlgItem(hCurr,IDC_LVBANS), hExceptionList = GetDlgItem(hCurr,IDC_LVEXCEPTIONS);

						ShowWindow(tab == 0 ? hBanList : hExceptionList,SW_SHOW);
						ShowWindow(tab == 0 ? hExceptionList : hBanList,SW_HIDE);
					}
				}
				break;
				case IDC_TABACCTS:
				/* Accounts tab is changing */
				{
					int tab = TabCtrl_GetCurSel(pnmh->hwndFrom);

					if (pnmh->code == TCN_SELCHANGE)
					{
						HWND hList = GetSubItemHandle(IDC_LVACCTS);

						Account_List(&accthead, hList,(unsigned char)tab);
					}
				}
				break;

				case IDC_EDITMOTD:
				{
					if (pnmh->code == EN_MSGFILTER)
					{
						MSGFILTER *msgf = (MSGFILTER*)(lParam);

						if (msgf->msg == WM_KEYDOWN)
						{
							if (GetAsyncKeyState(VK_CONTROL))
							/* If user presses control B or control U, bold or underline the selected text */
							{
								if (msgf->wParam == 'B' || msgf->wParam == 'b')
									SendDlgItemMessage(GetParent(pnmh->hwndFrom),IDC_BTNMOTDBOLD,BM_CLICK,0,0);
								else if (msgf->wParam == 'U' || msgf->wParam == 'u')
									SendDlgItemMessage(GetParent(pnmh->hwndFrom),IDC_BTNMOTDUNDERLINE,BM_CLICK,0,0);
							}							
						}
					}
				}
				break;
				case IDC_EDITFILTERNICK:
				break;
				default:
					break;
			}
		}
		break;
		case WM_COMMAND:
		{
			int wNotifyCode = HIWORD(wParam);
			int wID = LOWORD(wParam);
			HWND hFrom = (HWND) lParam;

			if (wNotifyCode == BN_CLICKED)
			{
				int count;

				switch (wID)
				{
					case IDC_RDSERVERINBOUND:
					{
						EnableWindow(GetDlgItem(GetParent(hFrom),IDC_TXTPINGFREQUENCY),TRUE);
						EnableWindow(GetDlgItem(GetParent(hFrom),IDC_EDITINBOUNDPINGFREQUENCY),TRUE);
						EnableWindow(GetDlgItem(GetParent(hFrom),IDC_VSINBOUNDPINGFREQUENCY),TRUE);
						EnableWindow(GetDlgItem(GetParent(hFrom),IDC_EDITINBOUNDPINGRESPONSE),TRUE);
						EnableWindow(GetDlgItem(GetParent(hFrom),IDC_VSINBOUNDPINGRESPONSE),TRUE);

						EnableWindow(GetDlgItem(GetParent(hFrom),IDC_TXTOUTBOUNDPORT),FALSE);
						EnableWindow(GetDlgItem(GetParent(hFrom),IDC_EDITOUTBOUNDPORT),FALSE);
						EnableWindow(GetDlgItem(GetParent(hFrom),IDC_VSOUTBOUNDPORT),FALSE);
					}
					break;
					case IDC_RDSERVEROUTBOUND:
					{
						EnableWindow(GetDlgItem(GetParent(hFrom),IDC_TXTOUTBOUNDPORT),TRUE);
						EnableWindow(GetDlgItem(GetParent(hFrom),IDC_EDITOUTBOUNDPORT),TRUE);
						EnableWindow(GetDlgItem(GetParent(hFrom),IDC_VSOUTBOUNDPORT),TRUE);

						EnableWindow(GetDlgItem(GetParent(hFrom),IDC_TXTPINGFREQUENCY),FALSE);
						EnableWindow(GetDlgItem(GetParent(hFrom),IDC_EDITINBOUNDPINGFREQUENCY),FALSE);
						EnableWindow(GetDlgItem(GetParent(hFrom),IDC_VSINBOUNDPINGFREQUENCY),FALSE);
						EnableWindow(GetDlgItem(GetParent(hFrom),IDC_EDITINBOUNDPINGRESPONSE),FALSE);
						EnableWindow(GetDlgItem(GetParent(hFrom),IDC_VSINBOUNDPINGRESPONSE),FALSE);
					}
					break;
					case IDC_BTNIOAPPLY:
					/* Clicked "Apply" on "I/O Control" dialog */
					{
						char *buffer = NULL;
						unsigned int len, retval;

						/* Disable refresh/apply buttons */
						EnableWindow(GetDlgItem(hwnd,wID),FALSE);
						EnableWindow(GetDlgItem(hwnd,IDC_BTNIOREFRESH),FALSE);

						/* Save dialog info */
						SaveRAInfo(&SettingsInfo);

						len = IS_SaveToBuffer(&buffer,&SettingsInfo);

						/* Re-allocate 7(3 + SIZE_LONG) additional bytes for protocol header */
						buffer = (char*)realloc(buffer,len + 3 + SIZE_LONG);

						memmove(&buffer[3 + SIZE_LONG],buffer,len);
						buffer[0] = 'U'; buffer[1] = 'P'; buffer[2] = 'D';
						InsertLong(buffer,3,htonl(len));

						len += 3 + SIZE_LONG;

						if ((retval = send(ci.socket,buffer,len,0)) == SOCKET_ERROR)
						/* Error in send */
						{
							int err = WSAGetLastError();

							if (err != WSAEWOULDBLOCK)
							/* Temp: error in send */
							{
								char buf[256], errbuf[256];

								GetErrorString(errbuf,err);

								sprintf(buf,"Unable to send data to server(Error %d: %s)",err,errbuf);
								MessageBox(hwnd,buf,"Error",MB_OK | MB_ICONERROR);

								Disconnect();
							}
						}
						else
						/* Data sent OK, free up all buffers */
						{
							free(buffer);
							IS_FreeBuffers(&SettingsInfo,FALSE);
						}
					}
					break;
					case IDC_BTNIOREFRESH:
					/* Clicked "Refresh" on "I/O control" dialog */
					{
						if (MessageBox(hwnd,"Are you sure you wish to discard your changes and reload the server data?","Cancel changes?",MB_ICONQUESTION | MB_YESNO) == IDYES)
						/* User clicked "Yes" */
						{
							char *buffer = "RQD";
							int retval, len = (int)strlen(buffer);

							/* Disable refresh/apply buttons */
							EnableWindow(GetDlgItem(hwnd,IDC_BTNIOAPPLY),FALSE);
							EnableWindow(GetDlgItem(hwnd,wID),FALSE);

							if ((retval = send(ci.socket,buffer,len,0)) == SOCKET_ERROR)
							/* Error in send */
							{
								int err = WSAGetLastError();

								if (err != WSAEWOULDBLOCK)
								/* Temp: error in send */
								{
									char buf[256], errbuf[256];

									GetErrorString(errbuf,err);

									sprintf(buf,"Unable to issue request to download server data(Error %d: %s)",err,errbuf);
									MessageBox(hwnd,buf,"Error",MB_OK | MB_ICONERROR);

									Disconnect();
								}
							}
						}
					}
					break;

					case IDC_BTNCONNECT:
					/* Clicked "Connect" on "Connect to remote server" dialog */
					{
						HWND hEditBox[] = { GetDlgItem(hwnd,IDC_EDITCONNECTHOSTNAME), GetDlgItem(hwnd,IDC_EDITCONNECTPORT), GetDlgItem(hwnd,IDC_EDITCONNECTPASSWORD), NULL };
						char buf[256];
						int count;
						BOOL bDisable = TRUE;

						for (count = 0; hEditBox[count] != 0; count++)
						/* Make sure no fields are empty */
						{
							GetWindowText(hEditBox[count],buf,256);

							if (!buf[0])
							{
								bDisable = FALSE;
								SetFocus(hEditBox[count]);
							}
						}

						if (bDisable)				
						{
							GetWindowText(hEditBox[0],buf,256);

							/* Get hostname */
							ci.hConnect = hwnd;
							ci.bTerminate = FALSE;
							strcpy(ci.hostname,buf);

							/* Password */
							GetWindowText(hEditBox[1],buf,256);
							ci.port = (unsigned short)atoi(buf);

							/* And port */
							GetWindowText(hEditBox[2],buf,256);
							strcpy(ci.password,buf);

							EnableWindow(hFrom,FALSE);

							hConnThread = (HANDLE)_beginthread(ConnectToServer,0,NULL);
						}
					}
					break;
					case IDC_BTNCONNECTCLOSE:
					/* Clicked "Close" on "Connect to server" dialog */
					{
						EndDialog(hwnd,wID);
					}
					break;
					case IDC_BTNRAPASSWORD:
					/* Clicked "Change password" in security window */
					{
						DialogBox(hInst,MAKEINTRESOURCE(IDD_RAPASSWORD),hwnd,(DLGPROC)MainDlgProc);
					}
					break;
					case IDC_BTNPASSWORDOK:
					/* Clicked "OK" on "Remote administration password" dialog */
					{
						HWND hEditBox[] = { GetDlgItem(hwnd,IDC_EDITPASSWORDOLD), GetDlgItem(hwnd,IDC_EDITPASSWORDNEW), GetDlgItem(hwnd,IDC_EDITPASSWORDNEW2) };
						char op[256], np[256], vnp[256];
						char *buf[] = { op, np, vnp };
						BOOL bExit = TRUE;
						int count;

						for (count = 0; count < 3; count++)
						/* Make sure no boxes are empty */
						{
							GetWindowText(hEditBox[count],buf[count],256);

							if (!buf[count][0])
							{
								SetFocus(hEditBox[count]);
								bExit = FALSE;
								break;
							}
						}

						if (bExit)
						/* No empty boxes */
						{
							if (strcmp(np,vnp) != 0)
							/* New & verified passwords do not match */
							{
								MessageBox(hwnd,"The new passwords do not match.","Error",MB_OK | MB_ICONERROR);
								break;
							}
							else
							{
								if (strcmp(RAPassword,op) == 0)
								/* Passwords match */
								{
									strcpy(RAPassword,np);
								}
								else
								/* Passwords do not match */
								{
									MessageBox(hwnd,"The old password is incorrect.","Error",MB_OK | MB_ICONERROR);
									break;
								}
							}

							EndDialog(hwnd,wID);
						}
					}
					break;
					case IDC_BTNPASSWORDCANCEL:
					/* Clicked "Cancel" on "Remote administration password" dialog */
					{
						EndDialog(hwnd,wID);
					}
					break;
					case IDC_BTNCCAUTOJOIN1:
					/* Clicked "->" in channel autojoin group box on user window */
					{
						HWND hEditBox = GetDlgItem(hwnd,IDC_EDITCCAUTOJOIN);
						HWND hListBox = GetDlgItem(hwnd,IDC_LISTCCAUTOJOIN);
						char buf[128];

						GetWindowText(hEditBox,buf,128);

						if (buf[0])
						/* Box was not empty */
						{
							int sel = (int)SendMessage(hListBox,LB_FINDSTRINGEXACT,-1,(LPARAM)buf);

							if (sel == LB_ERR)
							/* Add string to autojoin entries */
							{
								ListBox_AddString(hListBox,buf);
							}
							else
							/* Already on autojoin! duh */
							{
								SendMessage(hListBox,LB_SETCURSEL,sel,0);
							}

							SetWindowText(hEditBox,"");
						}

					}
					break;
					case IDC_BTNCCAUTOJOIN2:
					/* Clicked "Delete autojoin entry" on user window */
					{
						HWND hListBox = GetDlgItem(hwnd,IDC_LISTCCAUTOJOIN);
						int sel = (int)SendMessage(hListBox,LB_GETCURSEL,0,0),
							total = (int)SendMessage(hListBox,LB_GETCOUNT,0,0);

                        if (sel != LB_ERR)
						/* Delete selected entry */
						{
							ListBox_DeleteString(hListBox,sel);

							if (total > sel)
								sel = total - 1;

							SendMessage(hListBox,LB_SETCURSEL,sel,0);								
						}

					}
					break;
					case IDC_BTNDETECTEDHOSTSDELETE:
					case IDC_BTNDETECTEDHOSTSCLEAR:
					/* Have these 2 display a message box, for now
					TODO: Make these delete/clear entries for active spoofed list */
					{
						MessageBox(hwnd,"Never click me again!","hehe",MB_OK);
					}
					break;
					case IDC_BTNDETECTEDHOSTSOK:
					case IDC_BTNDETECTEDHOSTSCANCEL:
					/* Just have these 2 close the dialog, for now
					TODO: Make OK save list, and cancel bring back old list */
					{
						EndDialog(hwnd,wID);
					}
					break;
					case IDC_BTNEXPORTFOLDER:
					/* Clicked "Browse for folder" on "Export spoofed hosts" dialog */
					{
						BROWSEINFO bi = { 0 };
						bi.lpszTitle = "Please select folder for export file(s)...";
						bi.hwndOwner = hwnd;
						bi.ulFlags = BIF_RETURNONLYFSDIRS;
						LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

						if (pidl)
						/* User chose a folder */
						{
							char path[MAX_PATH];
							IMalloc *imalloc = NULL;

							if (SHGetPathFromIDList(pidl,path))
							{
								strcat(path,"\\");
								SetDlgItemText(hwnd,IDC_EDITEXPORTFOLDER,path);
							}

							/* Free memory used */

                            if (SUCCEEDED(SHGetMalloc(&imalloc)))
							{
								imalloc->Free(pidl);
								imalloc->Release();
							}
						}
						
					}
					break;
					case IDC_BTNEXPORTOK:
					/* Clicked "OK" on "Export hosts" dialog */
					{
						char tbuf[2][32], buf[MAX_PATH], portbuf[32], hostbuf[256];
						char *filebuf = (char*)malloc(1), *strptr;
						unsigned int portcount = 0, length, count, state = (unsigned int)SendDlgItemMessage(hwnd,IDC_RDEXPORTONEFILE,BM_GETSTATE,0,0);
						time_t tm;
						FILE *fp;
						BOOL bExists = TRUE;
						SpoofedList *curr = &spoofedhead;

						GetWindowText(GetDlgItem(hwnd,IDC_EDITEXPORTFOLDER),buf,MAX_PATH);
						CreateDirectory(buf,NULL);

						*filebuf = 0;

						if (state & BST_CHECKED)
						/* Create 1 file for hosts */
						{
							while (curr->next)
							{
								curr = curr->next;

								for (count = 0; curr->port[count] ; count++)
								/* Add strings for each port */
								{
									if (!count)
										sprintf(hostbuf,"%s:%d\r\n",curr->hostname,curr->port[count]);
									else
									{
										strcat(hostbuf,curr->hostname);
										sprintf(portbuf,":%d\r\n",curr->port[count]); strcat(hostbuf,portbuf);
									}
									portcount++;
								}

								filebuf = (char*)(realloc(filebuf,strlen(filebuf) + strlen(hostbuf) + 1));
								strcat(filebuf,hostbuf);
							}

							tm = time(NULL);
							strcpy(tbuf[0],ctime(&tm));
							tbuf[0][strlen(tbuf[0]) - 1] = 0;
							ConvertTime(tbuf[0],tbuf[1]);

							/* Cutoff time, only need date */
							strptr = strrchr(tbuf[1],32);
							strptr[0] = 0;

							strcat(buf,"portlist(");
							strcat(buf,tbuf[1]);
							strcat(buf,").txt");

							/* Determine if file already exists */
							SetLastError(0);
							length = (unsigned int)strlen(buf);

							for (count = 0; ; count++)
							/* Find suitable filename */
							{
								if (count)
								{
									buf[length - 4] = 0;
									sprintf(portbuf," #%d.txt",count);
									strcat(buf,portbuf);
								}

								if (!(fp = fopen(buf,"r")))
								/* Unable to open file */
								{
									//int err = GetLastError();

									//if (err == 2)
									/* Cannot find file */
									break;
								}
								else
									fclose(fp);
							}

							if (fp = fopen(buf,"w"))
							/* Write data to file */
							{
								char temp[MAX_PATH + 100];
								fwrite(filebuf,strlen(filebuf),1,fp);
								fclose(fp);

								sprintf(temp,"All %d Hostname/Port entries have been exported to %s",portcount,buf);
								MessageBox(hwnd,temp,"Export successful",MB_OK);
							}
                            
							free(filebuf);
						}
						else
						/* Create different files for each host port */
						{
							char buf[MAX_PATH], msgboxbuf[MAX_PATH * 5], *filebuf[6], portbuf[32], hostbuf[256], tbuf[2][32];
							unsigned int count, portctr, portcount[6] = { 0, 0, 0, 0, 0 }, portptr[] = { 23, 80, 1080, 3128, 8080 };
							FILE *fp;
							time_t tm;
							SpoofedList *curr = &spoofedhead;

							GetWindowText(GetDlgItem(hwnd,IDC_EDITEXPORTFOLDER),buf,MAX_PATH);
							CreateDirectory(buf,NULL);

							for (count = 0; count < 5; count++)
							/* Initialize file data buffers */
							{
								filebuf[count] = (char*)malloc(1);
								filebuf[count][0] = 0;
							}

							while (curr->next)
							{
								curr = curr->next;
                                
								for (count = 0; curr->port[count]; count++)
								/* Go through port list */
								{
									sprintf(hostbuf,"%s\r\n",curr->hostname);

									/* Find appropriate file buffer */
									for (portctr = 0; curr->port[count] != portptr[portctr]; portctr++);

									filebuf[portctr] = (char*)realloc(filebuf[portctr],strlen(filebuf[portctr]) + strlen(portbuf) + 1);
									strcat(filebuf[portctr],hostbuf);
									portcount[portctr]++;
								}
							}

							/* Set beginning of message box text buffer */
							sprintf(msgboxbuf,"The following entry numbers/files have been written to the directory %s:\r\n\r\n",buf);

							tm = time(NULL);
							strcpy(tbuf[0],ctime(&tm));
							tbuf[0][strlen(tbuf[0]) - 1] = 0;
							ConvertTime(tbuf[0],tbuf[1]);

							/* Cutoff time, only need date */
							strptr = strrchr(tbuf[1],32);
							strptr[0] = 0;

							strcat(buf,"portlist");

							/* Determine if file already exists */
							SetLastError(0);
							length = (unsigned int)strlen(buf);

							for (count = 0; count < 5; count++)
							/* Write only files which we need */
							{
								if (portcount[count])
								/* Include this file */
								{
									int count1;

									for (count1 = 0 ; ; count1++)
									{
										buf[length] = 0;

										if (count1)
										/* Append numeral to filename */
										{
											sprintf(portbuf,"%d(%s) #%d.txt",portptr[count],tbuf[1],count1);
											strcat(buf,portbuf);
										}
										else
										/* Create initial filename, ex: portlist23(11/16/03).txt */
										{
											sprintf(portbuf,"%d(%s).txt",portptr[count],tbuf[1]);
											strcat(buf,portbuf);
										}

										if (!(fp = fopen(buf,"r")))
										/* Unable to open file */
										{
											char *strptr = strrchr(buf,92);

											strptr++;

											sprintf(hostbuf,"%d %s written to: %s\r\n",portcount[count],portcount[count] == 1 ? "Entry" : "Entries",strptr);
											strcat(msgboxbuf,hostbuf);
											break;
										}
										else
											fclose(fp);
									}

									if (fp = fopen(buf,"w"))
									/* Write data to file */
									{
										fwrite(filebuf[count],strlen(filebuf[count]),1,fp);
										fclose(fp);
									}

								}

							}

							/* All done here */
							for (count = 0; count < 5; count++)
								free(filebuf[count]);

							MessageBox(hwnd,msgboxbuf,"Export successful",MB_OK);
						}

						EndDialog(hwnd,wID);
						
					}
					break;
					case IDC_BTNEXPORTCANCEL:
					/* Clicked "Cancel" on "Export spoofed hosts" dialog */
					{
						EndDialog(hwnd,wID);
					}
					break;

					case IDC_BTNDETAILSREPORT:
					/* Clicked "Create report" on "Detected host details" dialog */
					{
						SpoofedList* curr;
						char hostmask[128];
						char *hostname;

						GetWindowText(GetDlgItem(hwnd,IDC_EDITDETAILSHOSTMASK),hostmask,128);
                        
						hostname = strstr(hostmask,"@");
						hostname++;
						curr = Spoofed_Find(&spoofedhead, hostname);

						DialogBoxParam(hInst,MAKEINTRESOURCE(IDD_SPOOFEDREPORT),hwnd,(DLGPROC)MainDlgProc,(LPARAM)curr);
					}
					break;
					case IDC_BTNREPORTCLOSE:
					{
						EndDialog(hwnd,wID);
					}
					break;
					case IDC_BTNREPORTCOPY:
					/* Clicked "Copy" on "Detected host report" dialog */
					{
						HWND hEditBox = GetDlgItem(hwnd,IDC_EDITREPORT);

						SendMessage(hEditBox,EM_SETSEL,0,(LPARAM)-1);
						SendMessage(hEditBox,WM_COPY,0,0);
						SetFocus(hFrom);
					}
					break;
					case IDC_BTNREPORTSAVEAS:
					/* Clicked "Save As..." on "Detected host report" dialog */
					{
						OPENFILENAME ofn;
						char szFileName[MAX_PATH] = "";

						ZeroMemory(&ofn, sizeof(ofn));

						ofn.lStructSize = sizeof(ofn);
						ofn.hwndOwner = hwnd;
						ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
						ofn.lpstrFile = szFileName;
						ofn.lpstrInitialDir = "Desktop";
						ofn.nMaxFile = MAX_PATH;
						ofn.lpstrDefExt = "txt";
						ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

						if (GetSaveFileName(&ofn))
						/* Save to this file */
						{
							HWND hEditBox = GetDlgItem(hwnd,IDC_EDITREPORT);
							int count, last13 = 0, length = GetWindowTextLength(hEditBox);
							char *pszText = (char*)calloc(1,length + 1);
							FILE *fp;

							GetWindowText(hEditBox,pszText,length);

							for (count = 0; count < length; count++)
							{
								if (pszText[count] == 13)
									last13 = count;
							}

							if (last13 == 0)
								last13 = length;

							pszText[last13] = 0;

							length = (int)strlen(pszText);

							if((fp=fopen(ofn.lpstrFile,"w")) != NULL)
							/* Write information to file */
							{
								fwrite(pszText,length + 1,1,fp);
								fclose(fp);
							}
						}	
					}
					break;
					case IDC_BTNDETECTEDHOSTSVIEW:
					/* Clicked "View details" on "Detected hosts" dialog */
					{
						hWork = GetDlgItem(hwnd,IDC_LVDETECTEDHOSTS);

						if (ListView_GetSelectedCount(hWork) > 0)
						{
							SpoofedList* curr;
							char hostname[64];
							int sel = ListView_GetNextItem(hWork,-1,LVNI_FOCUSED);

							ListView_GetItemText(hWork,sel,0,hostname,64);

							curr = Spoofed_Find(&spoofedhead, hostname);

							DialogBoxParam(hInst,MAKEINTRESOURCE(IDD_SPOOFEDDETAILS),hwnd,(DLGPROC)MainDlgProc,(LPARAM)curr);
						}
					}
					break;
					case IDC_BTNDETAILSCLOSE:
					/* Clicked "Close" on "Details for host" dialog */
					{
						EndDialog(hwnd,wID);
					}
					break;

					case IDC_BTNSPOOF:
					/* Clicked "View detected hosts" */
					{
						SpoofedList *curr = &spoofedhead;
						DialogBox(hInst,MAKEINTRESOURCE(IDD_SPOOFDETECTED),hwnd,(DLGPROC)MainDlgProc);

						/* Temp: clear allocated list */
						while (curr = spoofedhead.next)
						{
							SpoofedLogin_DeleteList(curr);
							Spoofed_Delete(&spoofedhead, curr);
						}
					}
					break;
					case IDC_BTNDETECTEDHOSTSEXPORT:
					/* Clicked "Export list" on "Spoofed hosts" dialog */
					{
						DialogBox(hInst,MAKEINTRESOURCE(IDD_SPOOFEDEXPORT),hwnd,(DLGPROC)MainDlgProc);
					}
					break;
					case IDC_BTNADDSERVER:
					/* Clicked "Add inbound/outbound server" */
					{
						DialogBox(hInst,MAKEINTRESOURCE(IDD_INBOUND),GetParent(hFrom),(DLGPROC)MainDlgProc);

					}
					break;
					case IDC_BTNMODSERVER:
					/* Clicked "Modify inbound/outbound server" */
					{
						hWork = GetDlgItem(GetParent(hFrom),IDC_LVINBOUND);

						if (ListView_GetSelectedCount(hWork) > 0)
						{
							ServerList* curr;
							char name[64], portbuf[64];
							int sel = ListView_GetNextItem(hWork,-1,LVNI_FOCUSED);
							unsigned short port;

							ListView_GetItemText(hWork,sel,0,name,64);
							ListView_GetItemText(hWork,sel,2,portbuf,64);

							curr = Server_Find(&serverhead, name);

							DialogBoxParam(hInst,MAKEINTRESOURCE(IDD_INBOUND),GetParent(hFrom),(DLGPROC)MainDlgProc,(LPARAM)curr);
						}
					}
					break;
					case IDC_BTNDELSERVER:
					/* Clicked "Delete inbound/outbound server" */
					{
						HWND hParent = GetParent(hFrom);
						HWND hListView = GetDlgItem(hParent,IDC_LVINBOUND);
						BOOL bOutBound = FALSE;
						int sel, total;

						if (!IsWindowVisible(hListView))
						/* Delete outbound server */
						{
							hListView = GetDlgItem(hParent,IDC_LVOUTBOUND);
							bOutBound = TRUE;
						}

						sel = ListView_GetNextItem(hListView,-1,LVNI_FOCUSED);

                        if (sel != -1)
						/* Item selected, delete it */
						{
							char buf[256];
							ServerList* curr;

							ListView_GetItemText(hListView,sel,0,buf,256);

							curr = Server_Find(&serverhead, buf);
							Server_Delete(&serverhead, curr);
							Server_List(&serverhead, hListView);

							total = ListView_GetItemCount(hListView);

							if (sel >= total)
								sel = total - 1;

							ListView_SetItemState(hListView,sel,LVIS_SELECTED | LVIS_FOCUSED,LVIS_SELECTED | LVIS_FOCUSED);
							SetFocus(hListView);
						}
					}
					break;
					case IDC_BTNREFRESHSERVERS:
					/* Clicked "Refresh inbound/outbound servers" */
					{
						HWND hParent = GetParent(hFrom);
                        HWND hListView = GetDlgItem(hParent,IDC_LVINBOUND);
						BOOL bOutBound = FALSE;
						char buffer[20], sbtext[256];
						int retval, len, 
							nDisable[] =
						/* List of controls to disable */
						{
							IDC_BTNADDSERVER,
							IDC_BTNDELSERVER,
							IDC_BTNMODSERVER,
							IDC_BTNREFRESHSERVERS,
							0
						};

						if (!IsWindowVisible(hListView))
						/* Outbound servers list is active */
						{
							hListView = GetDlgItem(hParent,IDC_LVOUTBOUND);
							bOutBound = TRUE;
						}

						/* Get current status bar text */
						StatusBar_GetText(hStatus,sbtext);
						StatusBar_SetText(hStatus,"Obtaining linked server information...",0);

						/* Disable all relevant windows */
						ci.refresh = REFRESH_SERVERS;
						EnableWindow(hTreeView,FALSE);
						for (retval = 0; nDisable[retval]; retval++)
							EnableWindow(GetDlgItem(hParent,nDisable[retval]),FALSE);

						/* Create string to send to server */
						strcpy(buffer,"RQS");
						len = 3;

						if ((retval = send(ci.socket,buffer,len,0)) == SOCKET_ERROR)
						/* Error in send */
						{
							int err = WSAGetLastError();

							if (err != WSAEWOULDBLOCK)
							/* Error in send */
							{
								char buf[256], errbuf[256];

								GetErrorString(errbuf,err);

								sprintf(buf,"Unable to send command to obtain list of linked servers(Error %d: %s)",err,errbuf);
								MessageBox(hwnd,buf,"Error",MB_OK | MB_ICONERROR);

								StatusBar_SetText(hStatus,sbtext,0);

								/* Re-enable all relevant windows */
								EnableWindow(hTreeView,TRUE);
								for (retval = 0; nDisable[retval]; retval++)
									EnableWindow(GetDlgItem(hParent,nDisable[retval]),TRUE);

								Disconnect();
							}
						}
					}
					break;
					case IDC_BTNINBOUNDOK:
					/* Clicked "OK" on inbound server dialog */
					{
						HWND hWork = GetSubItemHandle(IDC_LVINBOUND), hEditBox[] = { GetDlgItem(hwnd,IDC_EDITINBOUNDNAME), GetDlgItem(hwnd,IDC_EDITINBOUNDHOSTNAME), GetDlgItem(hwnd,IDC_EDITOUTBOUNDPORT), GetDlgItem(hwnd,IDC_EDITINBOUNDPINGFREQUENCY), GetDlgItem(hwnd,IDC_EDITINBOUNDPINGRESPONSE), GetDlgItem(hwnd,IDC_EDITINBOUNDPASSWORD), NULL };
						ServerList sData;
						char ping[2][64];
						char port[64];
						char *buffers[] = { sData.name, sData.hostmask, port, ping[0], ping[1], sData.password };
						int count;
						BOOL bEndDialog = TRUE;
						BOOL bOutbound = FALSE;

						memset(&sData,0,sizeof(ServerList));

						for (count = 0; hEditBox[count] != NULL; count++)
						{
							GetWindowText(hEditBox[count],buffers[count],64);

							if (!buffers[count][0])
							{
								SetFocus(hEditBox[count]);
								bEndDialog = FALSE;
								break;
							}
						}

						/* Encryption */
						sData.encryption = ComboBox_GetCurSel(GetDlgItem(hwnd,IDC_CBSERVERENCRYPTION));

						if (Button_GetCheck(GetDlgItem(hwnd,IDC_RDSERVERINBOUND)))
						/* Inbound server */
						{
							if ( bEndDialog && (!(sData.ping_frequency = (unsigned short)atoi(ping[0])) || !(sData.ping_response = (unsigned short)atoi(ping[1]))))
							/* Invalid ping frequency/response time */
							{
								MessageBox(hwnd,"You must specify a valid ping frequency and maximum response time.","Unable to add inbound server",MB_OK | MB_ICONERROR);
								bEndDialog = FALSE;
							}

							sData.port = 0;
						}
						else
						{
							if ( bEndDialog && !(sData.port = (unsigned short)atoi(port)))
							/* Invalid port specified */
							{
								MessageBox(hwnd,"You must specify a valid port.","Unable to add outbound server",MB_OK | MB_ICONERROR);
								bEndDialog = FALSE;
							}

							sData.ping_frequency = sData.ping_response = 0;
						}


						if (bEndDialog)
						/* OK to close dialog box */
						{
							char buf[256];

							GetWindowText(hwnd,buf,256);

							if (strcmp(buf,"Add new server") == 0)
							/* Add inbound server */
							{
								if (Server_Add(&serverhead, &sData) == -1)
								{
									MessageBox(hwnd,"A linked server with the specified name already exists.","Unable to add inbound server",MB_OK | MB_ICONERROR);
									break;
								}
							}
							else
							/* Modify inbound server */
							{
								ServerList* curr;
								int sel = ListView_GetNextItem(hWork,-1,LVNI_FOCUSED);

								ListView_GetItemText(hWork,sel,0,buf,256);

								curr = Server_Find(&serverhead, buf);

								if (Server_Modify(&serverhead, curr,&sData) == -1)
								{
									MessageBox(hwnd,"A different linked server with the specified name currently exists.","Unable to add inbound server",MB_OK | MB_ICONERROR);
									SetFocus(hEditBox[0]);
									break;
								}

							}

							Server_List(&serverhead, hWork);
							EndDialog(hwnd,wID);
						}
					}
					break;
					case IDC_BTNINBOUNDCANCEL:
					/* Clicked "Cancel" on inbound server dialog */
					{
						EndDialog(hwnd,wID);
					}
					break;
					case IDC_BTNREGCHANADDACCESS:
					/* Clicked "Add access entry" on registered channel dialog */
					{
						hWork = GetDlgItem(hTabRegChan[1],IDC_LVREGCHANACCESS);
						DialogBox(hInst,MAKEINTRESOURCE(IDD_ACCESS),hwnd,(DLGPROC)MainDlgProc);
					}
					break;
					case IDC_BTNREGCHANMODACCESS:
					/* Clicked "Modify access entry" on registered channel dialog */
					{
						hWork = GetDlgItem(hTabRegChan[1],IDC_LVREGCHANACCESS);

						if (ListView_GetSelectedCount(hWork) > 0)
						/* Load dialog box */
						{
							AccessList* curr;
							char hostmask[64];
							int sel = ListView_GetNextItem(hWork,-1,LVNI_FOCUSED);

							ListView_GetItemText(hWork,sel,0,hostmask,64);

							curr = RCL_FindAccess(&RCLDialog,hostmask);

							DialogBoxParam(hInst,MAKEINTRESOURCE(IDD_ACCESS),hwnd,(DLGPROC)MainDlgProc,(LPARAM)curr);
						}
					}
					break;
					case IDC_BTNREGCHANDELACCESS:
					/* Clicked "Delete access entry" */
					{
						HWND hParent = GetParent(hFrom);
						HWND hListView = GetDlgItem(hParent,IDC_LVREGCHANACCESS);
						int sel, total;

						sel = ListView_GetNextItem(hListView,-1,LVNI_FOCUSED);

                        if (sel != -1)
						/* Item selected, delete it */
						{
							char buf[256];
							AccessList* curr;

							ListView_GetItemText(hListView,sel,0,buf,256);

							curr = RCL_FindAccess(&RCLDialog,buf);
							RCL_DeleteAccess(&RCLDialog,curr);
							RCL_ListAccess(hListView,&RCLDialog);

							total = ListView_GetItemCount(hListView);

							if (sel >= total)
								sel = total - 1;

							ListView_SetItemState(hListView,sel,LVIS_SELECTED | LVIS_FOCUSED,LVIS_SELECTED | LVIS_FOCUSED);
							SetFocus(hListView);
						}
					}
					break;
					case IDC_BTNREGCHANREFRESHACCESS:
					/* Clicked "Refresh access entry" */
					{
						HWND hTab = GetDlgItem(hRegChan,IDC_TABREGCHAN),
							hBtnOK = GetDlgItem(hRegChan,IDC_BTNREGCHANOK),
							hBtnCancel = GetDlgItem(hRegChan,IDC_BTNREGCHANCANCEL),
							hRCListView = GetSubItemHandle(IDC_LVREGCHAN), 
							hParent = GetParent(hFrom), 
							hListView = GetDlgItem(hParent,IDC_LVREGCHANACCESS);
						char buf[256], buffer[256];
						int retval, len, RCsel = ListView_GetNextItem(hRCListView,-1,LVNI_FOCUSED),
							nDisable[] =
						/* List of controls to disable */
						{
							IDC_BTNREGCHANADDACCESS,
							IDC_BTNREGCHANMODACCESS,
							IDC_BTNREGCHANDELACCESS,
							IDC_BTNREGCHANREFRESHACCESS,
							0
						};

						/* Disable all relevant windows */
						ci.refresh = REFRESH_ACCESS;
						EnableWindow(hTab,FALSE);
						EnableWindow(hBtnOK,FALSE);
						EnableWindow(hBtnCancel,FALSE);
						ToggleClose(hRegChan,FALSE);
						for (retval = 0; nDisable[retval]; retval++)
							EnableWindow(GetDlgItem(hParent,nDisable[retval]),FALSE);

						/* Create string to send to server */
						strcpy(buffer,"RQRCA");
						ListView_GetItemText(hRCListView,RCsel,0,buf,256);
						InsertShort(buffer,5,htons((unsigned short)strlen(buf)));
						memcpy(&buffer[5 + SIZE_SHORT],buf,strlen(buf));
						len = (int)strlen(buf) + SIZE_SHORT + 5;

						if ((retval = send(ci.socket,buffer,len,0)) == SOCKET_ERROR)
						/* Error in send */
						{
							int err = WSAGetLastError();

							if (err != WSAEWOULDBLOCK)
							/* Error in send */
							{
								char buf[256], errbuf[256];

								GetErrorString(errbuf,err);

								sprintf(buf,"Unable to send command to obtain access list(Error %d: %s)",err,errbuf);
								MessageBox(hwnd,buf,"Error",MB_OK | MB_ICONERROR);

								/* Re-enable all relevant windows */
								EnableWindow(hTab,TRUE);
								EnableWindow(hBtnOK,TRUE);
								EnableWindow(hBtnCancel,TRUE);
								ToggleClose(hRegChan,TRUE);
								for (retval = 0; nDisable[retval]; retval++)
									EnableWindow(GetDlgItem(hParent,nDisable[retval]),TRUE);
								
								/* Disable refresh button */
								EnableWindow(GetDlgItem(hParent,IDC_BTNREGCHANREFRESHACCESS),FALSE);

								Disconnect();
							}
						}
					}
					break;
					case IDC_BTNACCESSOK:
					/* Clicked "OK" on access entry dialog */
					{
						AccessList aData;
						char hostmask[64], reason[256], expire[32];
						char *buffers[] = { hostmask, reason, expire };
						HWND hBoxes[] = { GetDlgItem(hwnd,IDC_EDITACCESSHOSTMASK), GetDlgItem(hwnd,IDC_EDITACCESSREASON), GetDlgItem(hwnd,IDC_EDITACCESSEXPIRE) };
						HWND hCheckBox[] = { GetDlgItem(hwnd,IDC_CKACCESSREASON), GetDlgItem(hwnd,IDC_CKACCESSEXPIRE) };
						BOOL bEndDialog = TRUE, bChecked[] = { FALSE, FALSE };
						char buf[256];
						int count, mask;

						for (count = 0; count < 2; count++)
						/* See if boxes are checked */
						{
							mask = (unsigned int)SendMessage(hCheckBox[count],BM_GETSTATE,0,0);

							if (mask & BST_CHECKED)
							/* Box is checked */
								bChecked[count] = TRUE;
						}

						for (count = 0; count < 3; count++)
						/* Stop if required fields are empty */
						{
							GetWindowText(hBoxes[count],buffers[count],256);

							if (count > 0)
							{
								if (bChecked[count - 1] && !buffers[count][0])
								/* Edit box is empty but check box is checked */
								{
									SetFocus(hBoxes[count]);
									bEndDialog = FALSE;
									break;
								}
							}
							else
							{
								if (!buffers[count][0])
								/* Edit box is empty */
								{
									SetFocus(hBoxes[count]);
									bEndDialog = FALSE;
									break;
								}
							}
						}

						if (bEndDialog)
						/* Proceed with work on access entry */
						{
							HWND hComboBox[] = { GetDlgItem(hwnd,IDC_CBACCESSTYPE), GetDlgItem(hwnd,IDC_CBACCESSEXPIRE) };

							strcpy(aData.hostmask,hostmask);

							aData.type = (unsigned char)SendMessage(hComboBox[0],CB_GETCURSEL,0,0);

							if (!bChecked[0])
								aData.reason = NULL;
							else
								aData.reason = reason;
							
							if (!bChecked[1])
							{
								aData.expire = 0;
								aData.exptype = 0;
							}
							else
							{
								aData.expire = atol(expire);
								aData.exptype = (unsigned char)SendMessage(hComboBox[1],CB_GETCURSEL,0,0);
							}
                            
							GetWindowText(hwnd,buf,256);

							if (strcmp(buf,"Add access entry") == 0)
							/* Add access entry */
							{
								if (RCL_AddAccess(&RCLDialog,&aData) == -1)
								/* Entry already exists */
								{
									MessageBox(hwnd,"An access entry with the specified hostmask currently exists.","Unable to add access entry",MB_OK | MB_ICONERROR);
									break;
								}
							}
							else
							/* Modify access entry */
							{
								AccessList* curr;
								char buf[256];
								int sel = ListView_GetNextItem(hWork,-1,LVNI_FOCUSED);

								ListView_GetItemText(hWork,sel,0,buf,256);

								curr = RCL_FindAccess(&RCLDialog,buf);

								if (RCL_ModifyAccess(&RCLDialog,curr,&aData) == -1)
								/* Access entry exists under same hostmask */
								{
									MessageBox(hwnd,"A different access entry with the specified hostmask currently exists.","Unable to add access entry",MB_OK | MB_ICONERROR);
									break;
								}
							}
                            
							RCL_ListAccess(hWork,&RCLDialog);
							EndDialog(hwnd,wID);
						}
					}
					break;
					case IDC_BTNACCESSCANCEL:
					/* Clicked "Cancel" on access entry dialog */
					{
						EndDialog(hwnd,wID);
					}
					break;
					case IDC_BTNADDREGCHAN:
					/* Clicked "Add registered channel" */
					{						
						int result = (int)DialogBox(hInst,MAKEINTRESOURCE(IDD_REGCHANNEL),hwnd,(DLGPROC)MainDlgProc);

						if (result != IDC_BTNREGCHANOK)
						/* Dialog cancelled/closed */
						{
							while (RCLDialog.accesshead.next)
								RCL_DeleteAccess(&RCLDialog,RCLDialog.accesshead.next);
						}
					}
					break;
					case IDC_BTNMODREGCHAN:
					/* Clicked "Modify registered channel" */
					{
						HWND hListView = GetDlgItem(GetParent(hFrom),IDC_LVREGCHAN);

						if (ListView_GetSelectedCount(hListView) > 0)
						/* Load dialog box */
						{
							RegChanList* curr;
							char name[64];
							int result, sel = ListView_GetNextItem(hListView,-1,LVNI_FOCUSED);

							ListView_GetItemText(hListView,sel,0,name,64);

							curr = RCL_Find(&RCLHead, name);

							result = (int)DialogBoxParam(hInst,MAKEINTRESOURCE(IDD_REGCHANNEL),hwnd,(DLGPROC)MainDlgProc,(LPARAM)curr);

							if (result != IDC_BTNREGCHANOK)
							/* Dialog cancelled/closed */
							{
								while (RCLDialog.accesshead.next)
									RCL_DeleteAccess(&RCLDialog,RCLDialog.accesshead.next);
							}
						}
					}
					break;
					case IDC_BTNDELREGCHAN:
					/* Clicked "Delete registered channel" */
					{
						HWND hParent = GetParent(hFrom);
						HWND hListView = GetDlgItem(hParent,IDC_LVREGCHAN);
						int sel, total;

						sel = ListView_GetNextItem(hListView,-1,LVNI_FOCUSED);

                        if (sel != -1)
						/* Item selected, delete it */
						{
							char buf[64];
							RegChanList* curr;

							ListView_GetItemText(hListView,sel,0,buf,64);

							curr = RCL_Find(&RCLHead, buf);
							RCL_Delete(&RCLHead, curr);
							RCL_List(&RCLHead, hListView);

							total = ListView_GetItemCount(hListView);

							if (sel >= total)
								sel = total - 1;

							ListView_SetItemState(hListView,sel,LVIS_SELECTED | LVIS_FOCUSED,LVIS_SELECTED | LVIS_FOCUSED);
							SetFocus(hListView);
						}
					}
					break;
					case IDC_BTNREFRESHREGCHANS:
					/* Clicked "Refresh registered channels" */
					{
						HWND hParent = GetParent(hFrom);
						HWND hListView = GetDlgItem(hFrom,IDC_LVREGCHAN);
						char buffer[20], sbtext[256];
						int retval, len, 
							nDisable[] =
						/* List of controls to disable */
						{
							IDC_BTNADDREGCHAN,
							IDC_BTNDELREGCHAN,
							IDC_BTNMODREGCHAN,
							IDC_BTNREFRESHREGCHANS,
							0
						};

						/* Get current status bar text */
						StatusBar_GetText(hStatus,sbtext);
						StatusBar_SetText(hStatus,"Obtaining registered channel information...",0);

						/* Disable all relevant windows */
						ci.refresh = REFRESH_CHANS;
						EnableWindow(hTreeView,FALSE);
						for (retval = 0; nDisable[retval]; retval++)
							EnableWindow(GetDlgItem(hParent,nDisable[retval]),FALSE);

						/* Create string to send to server */
						strcpy(buffer,"RQRC");
						len = 4;

						if ((retval = send(ci.socket,buffer,len,0)) == SOCKET_ERROR)
						/* Error in send */
						{
							int err = WSAGetLastError();

							if (err != WSAEWOULDBLOCK)
							/* Error in send */
							{
								char buf[256], errbuf[256];

								GetErrorString(errbuf,err);

								sprintf(buf,"Unable to send command to obtain registered channel information(Error %d: %s)",err,errbuf);
								MessageBox(hwnd,buf,"Error",MB_OK | MB_ICONERROR);

								StatusBar_SetText(hStatus,sbtext,0);

								/* Re-enable all relevant windows */
								EnableWindow(hTreeView,TRUE);
								for (retval = 0; nDisable[retval]; retval++)
									EnableWindow(GetDlgItem(hParent,nDisable[retval]),TRUE);

								Disconnect();
							}
						}
					}
					break;
					case IDC_BTNREGCHANOK:
					/* Clicked "OK" on registered channel dialog */
					{
						HWND hEditBox[] = { 
							GetDlgItem(hTabRegChan[0],IDC_EDITREGCHANNAME),
							GetDlgItem(hTabRegChan[0],IDC_EDITREGCHANSUBJECT),
							GetDlgItem(hTabRegChan[0],IDC_EDITREGCHANTOPIC),
							GetDlgItem(hTabRegChan[0],IDC_EDITREGCHANONJOIN),
							GetDlgItem(hTabRegChan[0],IDC_EDITREGCHANONPART),
							GetDlgItem(hTabRegChan[1],IDC_EDITREGCHANOWNERKEY),
							GetDlgItem(hTabRegChan[1],IDC_EDITREGCHANHOSTKEY),
							GetDlgItem(hTabRegChan[1],IDC_EDITREGCHANMEMBERKEY),
							GetDlgItem(hTabRegChan[2],IDC_EDITREGCHANMODEL),
							NULL
						};

						BOOL bEditBoxNeeded[] = { TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE };

						HWND hCheckBox[] = {
							GetDlgItem(hTabRegChan[2],IDC_CKREGCHANMODEL),
							GetDlgItem(hTabRegChan[2],IDC_CKREGCHANMODEA),
							GetDlgItem(hTabRegChan[2],IDC_CKREGCHANMODEM),
							GetDlgItem(hTabRegChan[2],IDC_CKREGCHANMODET),
							GetDlgItem(hTabRegChan[2],IDC_CKREGCHANMODEW),
							GetDlgItem(hTabRegChan[2],IDC_CKREGCHANMODEI),
							GetDlgItem(hTabRegChan[2],IDC_CKREGCHANMODEN),
							GetDlgItem(hTabRegChan[2],IDC_CKREGCHANMODEU),
							GetDlgItem(hTabRegChan[2],IDC_CKREGCHANMODEX),
							NULL
						};
						unsigned short modeflag[] = {
							ZRA_CHANMODE_AUTHONLY,
							ZRA_CHANMODE_MODERATED,
							ZRA_CHANMODE_ONLYOPSCHANGETOPIC,
							ZRA_CHANMODE_NOWHISPERS,
							ZRA_CHANMODE_INVITEONLY,
							ZRA_CHANMODE_NOEXTERN,
							ZRA_CHANMODE_KNOCK,
							ZRA_CHANMODE_AUDITORIUM,
							0
						};
						HWND hParent;
						char buf[2048];
                        int count, count2;
						BOOL bEndDialog = TRUE;

						for (count = 0; hEditBox[count] != NULL; count++)
						/* Go through edit boxes and see if one is empty */
						{
							hParent = GetParent(hEditBox[count]);
							GetWindowText(hEditBox[count],buf,2048);

							if (!buf[0] && bEditBoxNeeded[count])
							{
								for (count2 = 0; count2 < 3; count2++)
									if (hParent == hTabRegChan[count2])
									/* Set window to active */
									{
										ShowWindow(hTabRegChan[count2],SW_SHOW);
										TabCtrl_SetCurSel(GetDlgItem(hwnd,IDC_TABREGCHAN),count2);
										SetFocus(hEditBox[count]);
										bEndDialog = FALSE;
									}
									else
										ShowWindow(hTabRegChan[count2],SW_HIDE);

								break;
							}
						}

						if (bEndDialog)
						{
							int mask, maxsize[] = { 64, 64, 512, 1024, 1024, 64, 64, 64, 12 };
							char topic[512], onjoin[1024], onpart[1024], limit[12];
							char *buffers[9];

							buffers[0] = RCLDialog.name;
							buffers[1] = RCLDialog.subject;
							buffers[2] = RCLDialog.topic = topic;
							buffers[3] = RCLDialog.onjoin = onjoin;
							buffers[4] = RCLDialog.onpart = onpart;
							buffers[5] = RCLDialog.ownerkey;
							buffers[6] = RCLDialog.hostkey;
							buffers[7] = RCLDialog.memberkey;
							buffers[8] = limit;

							for (count = 0; hEditBox[count] != NULL; count++)
							/* Assign buffers to dialog RCL structure */
							{
                                GetWindowText(hEditBox[count],buffers[count],maxsize[count]);

								mask = (int)SendMessage(hCheckBox[count],BM_GETSTATE,0,0);

								if (mask & BST_CHECKED)
								/* Box is checked */
								{
									if (!count)
									/* Limit box is checked */
										RCLDialog.limit = 1;
									else
										RCLDialog.modeflags |= modeflag[count - 1];
								}
								else if (!count)
								/* Limit box is not checked */
								{
									RCLDialog.limit = 0;
								}
							}

							if (RCLDialog.limit)
								RCLDialog.limit = atol(buffers[8]);

							RCLDialog.visibility = (unsigned char)SendMessage(GetDlgItem(hTabRegChan[2],IDC_CBREGCHANMODEVISIBILITY),CB_GETCURSEL,0,0);

							GetWindowText(hwnd,buf,2048);

							if (strcmp(buf,"Add registered channel") == 0)
							/* Adding a new registered channel */
							{
								if (RCL_Add(&RCLHead, &RCLDialog) == -1)
								/* Channel exists */
								{
									MessageBox(hwnd,"A registered channel with the specified name currently exists.","Unable to add registered channel",MB_OK | MB_ICONERROR);
									break;
								}
								RCLDialog.accesshead.next = NULL;
							}
							else
							/* Modifying an existing registered channel */
							{
								HWND hListView = GetSubItemHandle(IDC_LVREGCHAN);

								if (ListView_GetSelectedCount(hListView) > 0)
								{
									RegChanList *curr;
									AccessList aOld;
									char name[64];
									int sel = ListView_GetNextItem(hListView,-1,LVNI_FOCUSED);

									ListView_GetItemText(hListView,sel,0,name,64);

									curr = RCL_Find(&RCLHead, name);
									aOld = curr->accesshead;

                                    if (RCL_Modify(&RCLHead, curr,&RCLDialog) == -1)
									/* Attempting to modify to existing name */
									{
										MessageBox(hwnd,"A different registered channel with the specified name currently exists.","Unable to modify registered channel",MB_OK | MB_ICONERROR);
										break;
									}

									RCLDialog.accesshead = aOld;

									while (RCLDialog.accesshead.next)
									{
										RCL_DeleteAccess(&RCLDialog,RCLDialog.accesshead.next);
									}

								}

							}

							RCL_List(&RCLHead, GetSubItemHandle(IDC_LVREGCHAN));
							EndDialog(hwnd,wID);
						}
					}
					break;
					case IDC_BTNREGCHANCANCEL:
					/* Clicked "Cancel" on registered channel dialog */
					{
						RCLDialog.accesshead.next = NULL;
						EndDialog(hwnd,wID);
					}
					break;

					case IDC_BTNADDBAN:
					/* Clicked "Add ban/exception" */
					{
						hWork = GetDlgItem(GetParent(hFrom),IDC_LVBANS);
						if (IsWindowVisible(hWork))
						/* Add ban dialog */
							DialogBox(hInst,MAKEINTRESOURCE(IDD_BANDIALOG),GetParent(hFrom),(DLGPROC)MainDlgProc);
						else
						/* Add exception dialog */
						{
							hWork = GetDlgItem(GetParent(hFrom),IDC_LVEXCEPTIONS);
							DialogBox(hInst,MAKEINTRESOURCE(IDD_EXCEPTIONDIALOG),GetParent(hFrom),(DLGPROC)MainDlgProc);
						}
					}
					break;
					case IDC_BTNMODBAN:
					/* Clicked "Modify ban/exception" */
					{
						BOOL bException = FALSE;

						hWork = GetDlgItem(GetParent(hFrom),IDC_LVBANS);

						if (!IsWindowVisible(hWork))
						{
							bException = TRUE;
							hWork = GetDlgItem(GetParent(hFrom),IDC_LVEXCEPTIONS);
						}

						if (ListView_GetSelectedCount(hWork) > 0)
						{
							BanList* curr;
							char hostmask[64];
							int sel = ListView_GetNextItem(hWork,-1,LVNI_FOCUSED);

							ListView_GetItemText(hWork,sel,0,hostmask,64);

							curr = Banlist_Find(&banhead, hostmask,bException);

							if (bException)
							/* Load "Modify exception" dialog */
								DialogBoxParam(hInst,MAKEINTRESOURCE(IDD_EXCEPTIONDIALOG),GetParent(hFrom),(DLGPROC)MainDlgProc,(LPARAM)curr);
							else
							/* Load "Modify ban" dialog */
								DialogBoxParam(hInst,MAKEINTRESOURCE(IDD_BANDIALOG),GetParent(hFrom),(DLGPROC)MainDlgProc,(LPARAM)curr);
						}
					}
					break;
					case IDC_BTNDELBAN:
					/* Clicked "Delete ban/exception" */
					{
						HWND hParent = GetParent(hFrom);
						HWND hListView = GetDlgItem(hParent,IDC_LVBANS);
						BOOL bException = FALSE;
						int sel, total;

						if (!IsWindowVisible(hListView))
						/* Delete exception */
						{
							hListView = GetDlgItem(hParent,IDC_LVEXCEPTIONS);
							bException = TRUE;
						}

						sel = ListView_GetNextItem(hListView,-1,LVNI_FOCUSED);

                        if (sel != -1)
						/* Item selected, delete it */
						{
							char buf[256];
							BanList* curr;

							ListView_GetItemText(hListView,sel,0,buf,256);

							curr = Banlist_Find(&banhead, buf, bException);
							Banlist_Delete(&banhead, curr);
							Banlist_List(&banhead, hListView, bException);

							total = ListView_GetItemCount(hListView);

							if (sel >= total)
								sel = total - 1;

							ListView_SetItemState(hListView,sel,LVIS_SELECTED | LVIS_FOCUSED,LVIS_SELECTED | LVIS_FOCUSED);
							SetFocus(hListView);
						}
					}
					break;
					case IDC_BTNREFRESHBANS:
					/* Clicked "Refresh bans/exceptions" */
					{
						HWND hParent = GetParent(hFrom);
                        HWND hListView = GetDlgItem(hParent,IDC_LVBANS);
						BOOL bException = FALSE;
						char buffer[20], sbtext[256];
						int retval, len, 
							nDisable[] =
						/* List of controls to disable */
						{
							IDC_BTNADDBAN,
							IDC_BTNDELBAN,
							IDC_BTNMODBAN,
							IDC_BTNREFRESHBANS,
							0
						};

						if (!IsWindowVisible(hListView))
						/* Exceptions list is active */
						{
							hListView = GetDlgItem(hParent,IDC_LVEXCEPTIONS);
							bException = TRUE;
						}

						/* Get current status bar text */
						StatusBar_GetText(hStatus,sbtext);
						StatusBar_SetText(hStatus,"Obtaining ban/exception information...",0);

						/* Disable all relevant windows */
						ci.refresh = REFRESH_BANS;
						EnableWindow(hTreeView,FALSE);
						for (retval = 0; nDisable[retval]; retval++)
							EnableWindow(GetDlgItem(hParent,nDisable[retval]),FALSE);

						/* Create string to send to server */
						strcpy(buffer,"RQB");
						len = 3;

						if ((retval = send(ci.socket,buffer,len,0)) == SOCKET_ERROR)
						/* Error in send */
						{
							int err = WSAGetLastError();

							if (err != WSAEWOULDBLOCK)
							/* Error in send */
							{
								char buf[256], errbuf[256];

								GetErrorString(errbuf,err);

								sprintf(buf,"Unable to send command to obtain ban/exception information(Error %d: %s)",err,errbuf);
								MessageBox(hwnd,buf,"Error",MB_OK | MB_ICONERROR);

								StatusBar_SetText(hStatus,sbtext,0);

								/* Re-enable all relevant windows */
								EnableWindow(hTreeView,TRUE);
								for (retval = 0; nDisable[retval]; retval++)
									EnableWindow(GetDlgItem(hParent,nDisable[retval]),TRUE);

								Disconnect();
							}
						}
					}
					break;
					case IDC_BTNBANOK:
					/* Clicked "OK" on ban dialog */
					{
						char buf[256];
						char hostmask[128], reason[256], expirationbuf[16];
						char *buffers[] = { &hostmask[0], &reason[0], &expirationbuf[0] };
						int count;
						unsigned int mask;
						HWND hComboBox[] = { GetDlgItem(hwnd,IDC_CBBANSCOPE), GetDlgItem(hwnd,IDC_CBBANEXPIRATION) };
						HWND hBoxes[] = { GetDlgItem(hwnd,IDC_EDITBANHOSTMASK), GetDlgItem(hwnd,IDC_EDITBANREASON), GetDlgItem(hwnd,IDC_EDITBANEXPIRATION) };
						HWND hCheckBox[] = { GetDlgItem(hwnd,IDC_CKBANREASON), GetDlgItem(hwnd,IDC_CKBANEXPIRATION) };
						BOOL bEndDialog = TRUE, bChecked[] = { FALSE, FALSE };

						for (count = 0; count < 2; count++)
						/* See if boxes are checked */
						{
							mask = (unsigned int)SendMessage(hCheckBox[count],BM_GETSTATE,0,0);

							if (mask & BST_CHECKED)
							/* Box is checked */
								bChecked[count] = TRUE;
						}

						for (count = 0; count < 3; count++)
						/* Stop if required fields are empty */
						{
							GetWindowText(hBoxes[count],buffers[count],256);

							if (count > 0)
							{
								if (bChecked[count - 1] && !buffers[count][0])
								/* Edit box is empty but check box is checked */
								{
									SetFocus(hBoxes[count]);
									bEndDialog = FALSE;
									break;
								}
							}
							else
							{
								if (!buffers[count][0])
								/* Edit box is empty */
								{
									SetFocus(hBoxes[count]);
									bEndDialog = FALSE;
									break;
								}
							}
						}

						if (bEndDialog)
						/* Everything OK, copy data to structure fields */
						{
							char *ptrreason;
							int expiration = 0, exptype = 0;
							BOOL bNetworkBan = (BOOL)SendMessage(hComboBox[0],CB_GETCURSEL,0,0);

							/* Assign values if boxes are checked */
							if (bChecked[0])
								ptrreason = reason;
							else
								ptrreason = NULL;

							if (bChecked[1])
							{
								expiration = atol(expirationbuf);
								exptype = (unsigned char)SendMessage(hComboBox[1],CB_GETCURSEL,0,0);
							}

							GetWindowText(hwnd,buf,256);

							if (strcmp(buf,"Add ban") == 0)
							/* Add ban */
							{
								if (Banlist_Add(&banhead, hostmask,ptrreason,bNetworkBan,FALSE,expiration,exptype) == -1)
								/* Ban currently exists */
								{
									MessageBox(hwnd,"A ban entry with the specified hostmask currently exists.","Unable to add ban entry",MB_OK | MB_ICONERROR);
									SetFocus(hBoxes[0]);
									break;
								}
							}
							else
							/* Modify ban */
							{
								BanList* curr;
								char buf[256];
								int sel = ListView_GetNextItem(hWork,-1,LVNI_FOCUSED);

								ListView_GetItemText(hWork,sel,0,buf,256);

								curr = Banlist_Find(&banhead, buf,FALSE);

								if (Banlist_Modify(&banhead, curr,hostmask,ptrreason,bNetworkBan,expiration,exptype) == -1)
								/* Ban entry exists under same username */
								{
									MessageBox(hwnd,"A different ban entry with the specified hostmask currently exists.","Unable to add ban entry",MB_OK | MB_ICONERROR);
									break;
								}
							}

							Banlist_List(&banhead, hWork,FALSE);
							EndDialog(hwnd,wID);
						}
					}
					break;
					case IDC_BTNBANCANCEL:
					/* Clicked "Cancel" on ban dialog */
					{
						EndDialog(hwnd,wID);
					}
					break;
					case IDC_BTNEXCEPTIONOK:
					/* Clicked "OK" on exception dialog */
					{
                        HWND hEdit = GetDlgItem(hwnd,IDC_EDITEXCEPTIONHOSTMASK);
						char buf[256], hostmask[64];

						GetWindowText(hEdit,hostmask,64);

						if (!hostmask[0])
						/* Hostmask field is empty */
						{
							SetFocus(hEdit);
							break;
						}
						else
						{
							GetWindowText(hwnd,buf,256);

							if (strcmp(buf,"Modify exception") == 0)
							/* Modify exception */
							{
								BanList* curr;
								int sel = ListView_GetNextItem(hWork,-1,LVNI_FOCUSED);

								ListView_GetItemText(hWork,sel,0,buf,256);

								curr = Banlist_Find(&banhead, buf,TRUE);

								if (Banlist_Modify(&banhead, curr,hostmask,NULL,FALSE,0,0) == -1)
								{
									MessageBox(hwnd,"A different ban exception with the specified hostmask currently exists.","Unable to add exception",MB_OK | MB_ICONERROR);
									SetFocus(hEdit);
									break;
								}
							}
							else
							/* Add exception */
							{
								if (Banlist_Add(&banhead, hostmask, NULL, FALSE, TRUE, 0, 0) == -1)
								{
									MessageBox(hwnd,"A ban exception with the specified hostmask currently exists.","Unable to add exception",MB_OK | MB_ICONERROR);
									SetFocus(hEdit);
									break;
								}
							}
						}

						Banlist_List(&banhead, hWork, TRUE);
						EndDialog(hwnd,wID);

						
					}
					break;
					case IDC_BTNEXCEPTIONCANCEL:
					/* Clicked "Cancel" on exception dialog */
					{
						EndDialog(hwnd,wID);
					}
					break;

					case IDC_BTNADDFILTER:
					/* Clicked "Add filter" */
					{
						char buf[32];
						HWND hEditAll = GetSubItemHandle(IDC_EDITFILTERALL);

						GetWindowText(hEditAll,buf,32);

						if (buf[0] != 0)
						/* Add to all filters */
						{
							Filter_Add(&filterhead, buf, FILTER_TYPE_ALL);
							Filter_List(&filterhead, GetParent(hFrom), FILTER_TYPE_ALL);

							SetWindowText(hEditAll,"");
						}
						else
						/* Add only specific filters */
						{
							HWND parent = GetParent(hFrom);
							HWND hEdit[] = { GetDlgItem(parent,IDC_EDITFILTERNICK), GetDlgItem(parent,IDC_EDITFILTERCHAN), GetDlgItem(parent,IDC_EDITFILTERTOPIC) };
							DWORD flags[] = { FILTER_TYPE_NICKNAME, FILTER_TYPE_CHANNEL, FILTER_TYPE_TOPIC };
							int count;

							for (count = 0; count < 3; count++)
							/* Loop through edit boxes and add ones filled */
							{
								GetWindowText(hEdit[count],buf,32);

								if (buf[0])
								/* Edit box contains information, add & clear box */
								{
									Filter_Add(&filterhead, buf, flags[count]);
									SetWindowText(hEdit[count],"");
								}
							}

							Filter_List(&filterhead, parent, FILTER_TYPE_ALL);
						}

					}
					break;

					case IDC_BTNMODFILTER:
					/* Clicked "Modify filter" */
					{
						HWND parent = GetParent(hFrom);
						HWND hListBox[] = { GetDlgItem(parent,IDC_LISTFILTERNICK), GetDlgItem(parent,IDC_LISTFILTERCHAN), GetDlgItem(parent,IDC_LISTFILTERTOPIC) };
						int count, sel;

						for (count = 0; count < 3; count++)
						{
							if ((sel = (int)SendMessage(hListBox[count],LB_GETCURSEL,0,0)) != LB_ERR)
							/* Found filter to modify */
							{
								char buf[32];
								FilterList* curr;

								SendMessage(hListBox[count],LB_GETTEXT,sel,(LPARAM)buf);

								curr = Filter_Find(&filterhead, buf);

								if (curr)
								/* Load "Modify filter" dialog box */
								{
									hWork = hListBox[count];
									DialogBoxParam(hInst,MAKEINTRESOURCE(IDD_FILTERDIALOG),GetParent(hFrom),(DLGPROC)MainDlgProc,(LPARAM)curr);
								}
							}
						}
					}
					break;
					case IDC_BTNDELFILTER:
					/* Clicked "Delete filter" */
					{
						HWND parent = GetParent(hFrom);
						HWND hListBox[] = { GetDlgItem(parent,IDC_LISTFILTERNICK), GetDlgItem(parent,IDC_LISTFILTERCHAN), GetDlgItem(parent,IDC_LISTFILTERTOPIC) };
						DWORD flags[] = { FILTER_TYPE_NICKNAME, FILTER_TYPE_CHANNEL, FILTER_TYPE_TOPIC };
						int count, sel, retval;
						char buf[32];
						
						for (count = 0; count < 3; count++)
						{
							if ((sel = (int)SendMessage(hListBox[count],LB_GETCURSEL,0,0)) != LB_ERR)
							/* Found filter to delete */
							{
								FilterList* target = NULL;

								/* Get message text & delete string from listbox */
								SendMessage(hListBox[count],LB_GETTEXT,sel,(LPARAM)buf);
								target = Filter_Find(&filterhead, buf);

								if (target)
								{
									int newsel = 0;

									Filter_Delete(&filterhead, buf, flags[count]);
									retval = (int)SendMessage(hListBox[count],LB_DELETESTRING,sel,0);

									if (sel >= retval)
										newsel = retval - 1;
									else
										newsel = sel;

									SendMessage(hListBox[count],LB_SETCURSEL,newsel,0);
								}
							}
						}
					}
					break;

					case IDC_BTNREFRESHFILTERS:
					/* Clicked "Refresh filters" */
					{
						HWND hParent = GetParent(hFrom);
						HWND hListBox[] = { GetDlgItem(hParent,IDC_LISTFILTERNICK), GetDlgItem(hParent,IDC_LISTFILTERCHAN), GetDlgItem(hParent,IDC_LISTFILTERTOPIC), NULL };
						char buffer[20], sbtext[256];
						int retval, len, 
							nDisable[] =
						/* List of controls to disable */
						{
							IDC_BTNADDFILTER,
							IDC_BTNDELFILTER,
							IDC_BTNMODFILTER,
							IDC_BTNREFRESHFILTERS,
							IDC_CKFILTER,
							0
						};

						/* Get current status bar text */
						StatusBar_GetText(hStatus,sbtext);
						StatusBar_SetText(hStatus,"Obtaining filtering information...",0);

						/* Disable all relevant windows */
						ci.refresh = REFRESH_FILTERS;
						EnableWindow(hTreeView,FALSE);
						for (retval = 0; nDisable[retval]; retval++)
							EnableWindow(GetDlgItem(hParent,nDisable[retval]),FALSE);

						/* Create string to send to server */
						strcpy(buffer,"RQF");
						len = 3;

						if ((retval = send(ci.socket,buffer,len,0)) == SOCKET_ERROR)
						/* Error in send */
						{
							int err = WSAGetLastError();

							if (err != WSAEWOULDBLOCK)
							/* Error in send */
							{
								char buf[256], errbuf[256];

								GetErrorString(errbuf,err);

								sprintf(buf,"Unable to send command to obtain filter list(Error %d: %s)",err,errbuf);
								MessageBox(hwnd,buf,"Error",MB_OK | MB_ICONERROR);

								StatusBar_SetText(hStatus,sbtext,0);

								/* Re-enable all relevant windows */
								EnableWindow(hTreeView,TRUE);
								for (retval = 0; nDisable[retval]; retval++)
									EnableWindow(GetDlgItem(hParent,nDisable[retval]),TRUE);

								Disconnect();
							}
						}
					}
					break;

					case IDC_BTNFILTEROK:
					/* Clicked "OK" on "Modify filter" dialog box */
					{
						HWND hParent = hWork;
						HWND hEditBox = GetDlgItem(hwnd,IDC_EDITFILTER);
						HWND hCheckBox[] = { GetDlgItem(hwnd,IDC_CKFILTERNICK), GetDlgItem(hwnd,IDC_CKFILTERCHAN), GetDlgItem(hwnd,IDC_CKFILTERTOPIC) }; 
						FilterList* target;
						char buf[32], targetbuf[32];
						unsigned int mask, type = 0, count, 
							sel = (unsigned int)SendMessage(hParent,LB_GETCURSEL,0,0),
							flags[] = { FILTER_TYPE_NICKNAME, FILTER_TYPE_CHANNEL, FILTER_TYPE_TOPIC };

						GetWindowText(hEditBox,buf,32);
						SendMessage(hParent,LB_GETTEXT,sel,(LPARAM)targetbuf);

						for (count = 0; count < 3; count++)
						{
							mask = (unsigned int)SendMessage(hCheckBox[count],BM_GETSTATE,0,0);

							if (mask & BST_CHECKED)
							/* Checkbox is marked, add flag */
								type |= flags[count];
						}

						if (!type)
						/* No filter type selected */
						{
							MessageBox(hwnd,"Please select a filter type","Error",MB_OK | MB_ICONERROR);
							break;
						}
						else
						/* Let's modify the filter already */
						{
							target = Filter_Find(&filterhead, targetbuf);
							if (Filter_Modify(&filterhead, target,buf,type) == -1)
							{
								MessageBox(hwnd,"Cannot modify to currently existing filter","Error",MB_OK | MB_ICONERROR);
								SetFocus(hEditBox);
								break;
							}
							else
								Filter_List(&filterhead, GetParent(hParent),FILTER_TYPE_ALL);

							EndDialog(hwnd,wID);
						}
					}
					break;

					case IDC_BTNFILTERCANCEL:
					/* Clicked "Cancel" on "Modify filter" dialog box */
					{
						EndDialog(hwnd,wID);
					}
					break;

					case IDC_BTNADDACCT:
					/* Clicked "Add account */
					{
						DialogBoxParam(hInst,MAKEINTRESOURCE(IDD_ACCTDIALOG), hMain, (DLGPROC)MainDlgProc,(LPARAM)"Add account");
					}
					break;

					case IDC_BTNMODACCT:
					/* Clicked "Modify account */
					{
						HWND hList = GetSubItemHandle(IDC_LVACCTS);

						if (ListView_GetSelectedCount(hList) > 0)
						{
							AccountList* curr;
							char username[32];
							int sel = ListView_GetNextItem(hList,-1,LVNI_FOCUSED);

							ListView_GetItemText(hList,sel,0,username,32);

							curr = Account_Find(&accthead, username);

							/* Set param of dialog box initialization to pointer of account info structure */
							DialogBoxParam(hInst,MAKEINTRESOURCE(IDD_ACCTDIALOG), hwnd, (DLGPROC)MainDlgProc,(LPARAM)curr);
						}
					}
					break;
					case IDC_BTNDELACCT:
					/* Clicked "Delete account" */
					{
						HWND hList = GetSubItemHandle(IDC_LVACCTS);

						if (ListView_GetSelectedCount(hList) > 0)
						{
							AccountList* curr;
							char username[32];
							unsigned char level;
							int total, sel = ListView_GetNextItem(hList,-1,LVNI_FOCUSED);

							ListView_GetItemText(hList,sel,0,username,32);

							curr = Account_Find(&accthead, username);
							level = curr->level;
							Account_Delete(&accthead, curr);
							Account_List(&accthead, hList, level);

							total = ListView_GetItemCount(hList);

							if (sel >= total)
								sel = total - 1;

							ListView_SetItemState(hList,sel,LVIS_SELECTED | LVIS_FOCUSED,LVIS_SELECTED | LVIS_FOCUSED);
							SetFocus(hList);
						}
					}
					break;

					case IDC_BTNREFRESHACCTS:
					/* Clicked "Refresh accounts" */
					{
						HWND hParent = GetParent(hFrom);
						HWND hListView = GetDlgItem(hFrom,IDC_LVREGCHAN);
						char buffer[20], sbtext[256];
						int retval, len, 
							nDisable[] =
						/* List of controls to disable */
						{
							IDC_BTNADDACCT,
							IDC_BTNDELACCT,
							IDC_BTNMODACCT,
							IDC_BTNREFRESHACCTS,
							0
						};

						/* Get current status bar text */
						StatusBar_GetText(hStatus,sbtext);
						StatusBar_SetText(hStatus,"Obtaining account information...",0);

						/* Disable all relevant windows */
						ci.refresh = REFRESH_ACCTS;
						EnableWindow(hTreeView,FALSE);
						for (retval = 0; nDisable[retval]; retval++)
							EnableWindow(GetDlgItem(hParent,nDisable[retval]),FALSE);

						/* Create string to send to server */
						strcpy(buffer,"RQA");
						len = 3;

						if ((retval = send(ci.socket,buffer,len,0)) == SOCKET_ERROR)
						/* Error in send */
						{
							int err = WSAGetLastError();

							if (err != WSAEWOULDBLOCK)
							/* Error in send */
							{
								char buf[256], errbuf[256];

								GetErrorString(errbuf,err);

								sprintf(buf,"Unable to send command to obtain account information(Error %d: %s)",err,errbuf);
								MessageBox(hwnd,buf,"Error",MB_OK | MB_ICONERROR);

								StatusBar_SetText(hStatus,sbtext,0);

								/* Re-enable all relevant windows */
								EnableWindow(hTreeView,TRUE);
								for (retval = 0; nDisable[retval]; retval++)
									EnableWindow(GetDlgItem(hParent,nDisable[retval]),TRUE);

								Disconnect();
							}
						}
					}
					break;

					case IDC_BTNADDBINDING:
					/* Clicked "Add binding" */
					{
						DialogBox(hInst,MAKEINTRESOURCE(IDD_ADDBINDING), hMain, (DLGPROC)MainDlgProc);
					}
					break;

					case IDC_BTNDELBINDING:
					/* Clicked "Delete binding" */
					{
						HWND bindlist = GetSubItemHandle(IDC_LISTBINDINGS);
						Binding_Delete((int)SendMessage(bindlist,LB_GETCURSEL,0,0));
						SetFocus(bindlist);
						SendMessage(bindlist,LB_SETCURSEL,0,0);
					}
					break;
					case IDC_BTNBINDOK:
					/* Clicked "OK" on "Add binding" dialog */
					{
						char addr[16];
						char port[8];

						GetDlgItemText(hwnd,IDC_EDITBINDIPADDR,addr,16);
						GetDlgItemText(hwnd,IDC_EDITBINDPORT,port,8);

						if (Binding_Add(addr,(unsigned short)atoi(port)) == -1)
						{
							MessageBox(hwnd,"Binding already exists","Error",MB_OK | MB_ICONERROR);
							return FALSE;
						}

						EndDialog(hwnd,wID);
					}
					break;
					case IDC_BTNBINDCANCEL:
					/* Clicked "Cancel" on "Add binding" dialog */
					{
						EndDialog(hwnd,wID);
					}
					break;
					case IDC_BTNACCTOK:
					/* Clicked "OK" on account dialog */
					{
						HWND hList = GetSubItemHandle(IDC_LVACCTS);
						char buf[256];
						struct AccountList newinfo;

						GetWindowText(GetDlgItem(hwnd,IDC_EDITACCTUSERNAME),newinfo.username,32);
						GetWindowText(GetDlgItem(hwnd,IDC_EDITACCTHOSTMASK),newinfo.hostmask,32);
						GetWindowText(GetDlgItem(hwnd,IDC_EDITACCTPASSWORD),newinfo.password,32);
						newinfo.level = (unsigned char)SendMessage(GetDlgItem(hwnd,IDC_CBACCTLEVEL),CB_GETCURSEL,0,0);
						newinfo.logins = 0;

						GetWindowText(hwnd,buf,256);

						if (newinfo.username[0] == 0 || newinfo.hostmask[0] == 0 || newinfo.password[0] == 0)
						/* Make sure all fields are valid */
							break;

						else if (strcmp(buf,"Add account") == 0)
						/* Add new account */
						{
							if (Account_Add(&accthead, &newinfo) == -1)
							/* Account already exists */
							{
								MessageBox(hwnd,"An account with the specified username currently exists.","Unable to add account",MB_OK | MB_ICONERROR);
								break;
							}
							else
							{
								HWND hTab = GetSubItemHandle(IDC_TABACCTS);

								SendMessage(hTab,TCM_SETCURSEL,(WPARAM)newinfo.level,0);
								Account_List(&accthead, hList, newinfo.level);
							}
						}
						else
						/* Modify existing account */
						{
							AccountList* curr;
							char buf[256];
							int sel = ListView_GetNextItem(hList,-1,LVNI_FOCUSED);

							ListView_GetItemText(hList,sel,0,buf,256);

							curr = Account_Find(&accthead, buf);

							if (Account_Modify(&accthead, curr, &newinfo) == -1)
							/* Account exists under same username */
							{
								MessageBox(hwnd,"A different account with the specified username currently exists.","Unable to add account",MB_OK | MB_ICONERROR);
								break;
							}
							else
							{
								HWND hTab = GetSubItemHandle(IDC_TABACCTS);
								
								SendMessage(hTab,TCM_SETCURSEL,(WPARAM)newinfo.level,0);
								Account_List(&accthead, hList, newinfo.level);
							}
						}

						EndDialog(hwnd,wID);
					}
					break;
					case IDC_BTNACCTCANCEL:
					/* Clicked "Cancel" on account dialog */
					{
						EndDialog(hwnd,wID);
					}
					break;
					case IDC_BTNMOTDBOLD:
					/* Clicked bold button on MOTD dialog */
					{
						int nState = Button_GetState(hFrom);
						HWND hParent = GetParent(hFrom);
						CHARFORMAT2 cf2;

						memset(&cf2,0,sizeof(CHARFORMAT2));

						cf2.cbSize = sizeof(CHARFORMAT2);
						cf2.dwMask = CFM_BOLD;

						SendMessage(GetDlgItem(GetParent(hFrom),IDC_EDITMOTD),EM_GETCHARFORMAT,(WPARAM)SCF_SELECTION,(LPARAM)&cf2);

						if ((cf2.dwMask & CFM_BOLD) && (cf2.dwEffects & CFE_BOLD))
						/* Unbold */
							cf2.dwEffects = CFE_AUTOBACKCOLOR;
						else
						/* Bold */
							cf2.dwEffects = CFE_BOLD | CFE_AUTOBACKCOLOR;

						cf2.cbSize = sizeof(CHARFORMAT2);
						cf2.dwMask = CFM_BOLD;

						SendMessage(GetDlgItem(hParent,IDC_EDITMOTD), EM_SETCHARFORMAT, (WPARAM)(UINT)SCF_SELECTION, (LPARAM)&cf2);
						SetFocus(GetDlgItem(hParent,IDC_EDITMOTD));
					}
					break;
					case IDC_BTNMOTDUNDERLINE:
					/* Clicked underline button on MOTD dialog */
					{
						int nState = Button_GetState(hFrom);
						HWND hParent = GetParent(hFrom);
						CHARFORMAT2 cf2;

						memset(&cf2,0,sizeof(CHARFORMAT2));

						cf2.cbSize = sizeof(CHARFORMAT2);
						cf2.dwMask = CFM_UNDERLINE;

						SendMessage(GetDlgItem(GetParent(hFrom),IDC_EDITMOTD),EM_GETCHARFORMAT,(WPARAM)SCF_SELECTION,(LPARAM)&cf2);

						if ((cf2.dwMask & CFM_UNDERLINE) && (cf2.dwEffects & CFE_UNDERLINE))
						/* Un-underline */
							cf2.dwEffects = CFE_AUTOBACKCOLOR;
						else
						/* Underline */
							cf2.dwEffects = CFE_UNDERLINE | CFE_AUTOBACKCOLOR;

						cf2.cbSize = sizeof(CHARFORMAT2);
						cf2.dwMask = CFM_UNDERLINE;

						SendMessage(GetDlgItem(hParent,IDC_EDITMOTD), EM_SETCHARFORMAT, (WPARAM)(UINT)SCF_SELECTION, (LPARAM)&cf2);
						SetFocus(GetDlgItem(hParent,IDC_EDITMOTD));
					}
					break;
					default:
					{
						for (count = 0; ckitems[count].ckid != 0; count++)
						/* See if clicked button was a check box */
						{
							if (hFrom == ckitems[count].ckhwnd)
							{
								int count2;
								unsigned int mask = (unsigned int)SendMessage(hFrom,BM_GETSTATE,0,0);
								BOOL enable = FALSE;

								if (mask & BST_CHECKED)
									enable = TRUE;

								for (count2 = 0; ckitems[count].id[count2] != 0; count2++)
									EnableWindow(GetDlgItem(ckitems[count].parent,ckitems[count].id[count2]),enable);
		                        
								if (hFrom == ckitems[0].ckhwnd && enable == TRUE)
								/* If its "Use DNS", disable others if needed */
								{
									mask = (unsigned int)SendMessage(ckitems[1].ckhwnd,BM_GETSTATE,0,0);
									enable = FALSE;

									if (mask & BST_CHECKED)
										enable = TRUE;

									for (count2 = 0; ckitems[1].id[count2] != 0; count2++)
										EnableWindow(GetDlgItem(ckitems[1].parent,ckitems[1].id[count2]),enable);									
								}
							}
						}
					}
				}
			}
			else if (wNotifyCode == LBN_SELCHANGE)
			/* Listbox selection changed */
			{
				if (wID == IDC_LISTFILTERNICK || wID == IDC_LISTFILTERCHAN || wID == IDC_LISTFILTERTOPIC)
				/* Filter list box selection changed */
				{
					HWND parent = GetParent(hFrom);
					HWND hListBox[] = { GetDlgItem(parent,IDC_LISTFILTERNICK), GetDlgItem(parent,IDC_LISTFILTERCHAN), GetDlgItem(parent,IDC_LISTFILTERTOPIC) };
					HWND hEditBox[] = { GetDlgItem(parent,IDC_EDITFILTERNICK), GetDlgItem(parent,IDC_EDITFILTERCHAN), GetDlgItem(parent,IDC_EDITFILTERTOPIC) };
					int count1;

					int indexes[][3] = { { 0, 1, 2 }, { 1, 0, 2 }, { 2, 0, 1 } };

					for (count1 = 0; count1 < 3; count1++)
					{
						if (hFrom == hListBox[count1])
						/* Found corresponding handle */
						{
							SetFocus(hEditBox[count1]);
							SendMessage(hListBox[indexes[count1][1]],LB_SETCURSEL,(WPARAM)-1,0);
							SendMessage(hListBox[indexes[count1][2]],LB_SETCURSEL,(WPARAM)-1,0);
						}
					}


				}

				else if (wID == IDC_LISTDETAILSLOGINS)
				/* Selection changed on "Spoofed hostname details" login entry list */
				{
					HWND hEditBox[] = { GetDlgItem(hwnd,IDC_EDITDETAILSHOSTMASK), GetDlgItem(hwnd,IDC_EDITDETAILSSPOOFEDPORTS), GetDlgItem(hwnd,IDC_EDITDETAILSSERVER), GetDlgItem(hwnd,IDC_EDITDETAILSPORT) };
					SpoofedList *sTarget;
					SpoofedLoginLog *curr;
					char buf[128];
					char *hostptr;
					unsigned short sel;

					GetWindowText(hEditBox[0],buf,128);
					hostptr = strstr(buf,"@"); hostptr++;

					sTarget = Spoofed_Find(&spoofedhead, hostptr);
					sel = (unsigned short)SendMessage(hFrom,LB_GETCURSEL,0,0);
					curr = SpoofedLogin_Find(sTarget,sel);

					SetWindowText(hEditBox[0],curr->hostmask);
					SetWindowText(hEditBox[2],curr->server);

					sprintf(buf,"%d",curr->port);
					SetWindowText(hEditBox[3],buf);
				}
			}
			else if (wNotifyCode == CBN_SELENDOK)
			/* Combobox change notifications */
			{
				int nIndex = ComboBox_GetCurSel(hFrom);

				if (wID == IDC_CBMOTDCOLOR)
				/* Color combobox on MOTD screen */
				{
					int nState = Button_GetState(hFrom);
					HWND hParent = GetParent(hFrom);
					CHARFORMAT2 cf2;

					memset(&cf2,0,sizeof(CHARFORMAT2));

					cf2.cbSize = sizeof(CHARFORMAT2);
					cf2.dwMask = CFM_COLOR;

					cf2.dwEffects = CFE_AUTOBACKCOLOR;

					cf2.crTextColor = MOTDArray[nIndex].crRGB;

					SendMessage(GetDlgItem(hParent,IDC_EDITMOTD), EM_SETCHARFORMAT, (WPARAM)(UINT)SCF_SELECTION, (LPARAM)&cf2);
					SetFocus(GetDlgItem(hParent,IDC_EDITMOTD));
				}
			}
			else if (wNotifyCode == LBN_DBLCLK)
			{
				if (wID == IDC_LISTFILTERNICK || wID == IDC_LISTFILTERCHAN || wID == IDC_LISTFILTERTOPIC)
					SendDlgItemMessage(GetParent(hFrom),IDC_BTNMODFILTER,BM_CLICK,0,0);

				break;
			}
			else if (wID == IDOK)
			/* Generic WM_COMMAND messages go here */
			{
				if (hFrom == tt[10].hwnd)
				{
					ODS("Test!");

					return 0;
				}
			}
		}
		break;
		case WM_VSCROLL:
		{
			int count = 0, nScrollCode = (int)LOWORD(wParam);
			HWND hwndScrollBar = (HWND)lParam;

			for (count = 0; scrolledits[count].edit != 0; count++)
			/* Loop through scrollbars and find match */
			{
				if (scrolledits[count].hScrollBar == hwndScrollBar)
				{
					if (nScrollCode == SB_LINEUP || nScrollCode == SB_LINEDOWN)
					{
						AddToEdit(scrolledits[count].hEditBox,(nScrollCode == SB_LINEUP) ? 1 : -1);
						break;
					}
				}
			}
		}
		break;
		case WM_GETMINMAXINFO:
		{
			char buf[256];

			GetWindowText(hwnd,buf,256);

			if (strcmp(buf,"Report for host") == 0)
			/* Report view window info being requested */
			{
				LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;

				lpmmi->ptMinTrackSize.x = 375;
				lpmmi->ptMinTrackSize.y = 200;
			}
		}
		break;
		case WM_SIZE:
		{
			char buf[256];

			GetWindowText(hwnd,buf,256);

			if (strcmp(buf,"Report for host") == 0)
			/* Report view window is being resized */
			{
				unsigned long wWidth, wHeight;
				HWND hButton[] = { GetDlgItem(hwnd,IDC_BTNREPORTCOPY), GetDlgItem(hwnd,IDC_BTNREPORTSAVEAS), GetDlgItem(hwnd,IDC_BTNREPORTCLOSE) };
				HWND hEditBox = GetDlgItem(hwnd,IDC_EDITREPORT);
				HWND hGrpBox = GetDlgItem(hwnd,IDC_GRPREPORT);
				RECT rect;

				GetWindowRect(hwnd,&rect);
				wWidth = rect.right - rect.left;
				wHeight = rect.bottom - rect.top;

				MoveWindow(hGrpBox,11,11,wWidth - 29, wHeight - 87,FALSE);
				MoveWindow(hEditBox,21,28,wWidth - 54 + 4, wHeight - 122 + 4,FALSE);
				MoveWindow(hButton[0],int(wWidth * .20),wHeight - 68,75,23,FALSE);
				MoveWindow(hButton[1],int(wWidth * .4),wHeight - 68,75,23,FALSE);
				MoveWindow(hButton[2],int(wWidth * .6),wHeight - 68,75,23,FALSE);

				/* Repaint window after resize */
				InvalidateRect(hwnd,NULL,TRUE);
				UpdateWindow(hwnd);
			}
		}
		break;
		case WM_UPDATECONNECTINFO:
		{
			if (ci.bTerminate)
			/* There was a problem */
			{
				MessageBox(hwnd,ci.status,"Error",MB_OK | MB_ICONERROR);
				EnableWindow(GetDlgItem(hwnd,IDC_BTNCONNECT),TRUE);
				StatusBar_SetText(hConnStatus,"Ready.",0);
			}
			else
			/* Everything OK, update status bar */
			{
				StatusBar_SetText(hConnStatus,ci.status,0);
			}
		}
		break;
		case WM_RASOCKET:
		/*
		** NOTE: Whenever a socket has incoming data, we can assume that the data
		** is only (1) packet, this is because the protocol requires acknowledgement.
		*/
		{
			SOCKET s = (SOCKET)wParam;
			int	retval, nError = WSAGETSELECTERROR(lParam), nEvent = WSAGETSELECTEVENT(lParam);

			if (s == ci.socket)
			/* RA connection socket */
			{
				switch (nEvent)
				{
					case FD_READ:
					{
						char *recvbuf = NULL;
						char buf[512];
						int recvlen = 0, err;
						BOOL bExitRecv = FALSE;

						/* Disable subsequent FD_READ's, we'll read all the data ourselves */
						WSAAsyncSelect(s,hwnd,Message,FD_WRITE | FD_CLOSE);

						while (!bExitRecv)
						{
							if ((retval = recv(s,buf,512,0)) != SOCKET_ERROR)
							/* Data read */
							{
								/* Expand buffer & add */
								recvbuf = (char*)realloc(recvbuf,recvlen + retval + 1);
								memcpy(&recvbuf[recvlen],buf,retval);

								recvlen += retval;
							}
							else
							/* Error in read */
							{
								err = WSAGetLastError();

								if (err == WSAEWOULDBLOCK)
								/* We're finished reading */
								{
									bExitRecv = TRUE;
								}
								else
								/* Error in recv() */
								{
									char msgbuf[512];

									GetErrorString(buf,err);
									sprintf(msgbuf,"Async recv() failed(Error %d: %s)",err,buf);

									MessageBox(hwnd,msgbuf,"Status",MB_OK | MB_ICONERROR);
								}
							}
						}

						recvbuf[recvlen] = 0;

						/* Process all data read from loop */
						if (strcmp(recvbuf,"RUPD") == 0)
						/* Server data has been updated, response given */
						{
							MessageBox(hwnd,"The server reports the data has been recieved and saved.","Got server response",MB_OK | MB_ICONINFORMATION);

							/* Enable I/O control buttons */
							EnableWindow(GetDlgItem(tt[1].hwnd,IDC_BTNIOAPPLY),TRUE);
							EnableWindow(GetDlgItem(tt[1].hwnd,IDC_BTNIOREFRESH),TRUE);
						}
						else if (recvbuf[0] == 'R' && recvbuf[1] == 'D')
						{
							ClearLinkedLists();
							IS_LoadFromBuffer(&recvbuf[2 + SIZE_LONG],&SettingsInfo);
							UpdateRAInfo(&SettingsInfo);

							MessageBox(hwnd,"The server has sent all the information.","Got server response",MB_OK | MB_ICONINFORMATION);

							/* Enable I/O control buttons */
							EnableWindow(GetDlgItem(tt[1].hwnd,IDC_BTNIOAPPLY),TRUE);
							EnableWindow(GetDlgItem(tt[1].hwnd,IDC_BTNIOREFRESH),TRUE);
						}
						else if (recvbuf[0] == 'R' && recvbuf[1] == 'S')
						/* Linked servers data */
						{
							unsigned short slen;
							int ttid, count, nEnable[] =
							/* List of controls to enable */
							{
								IDC_BTNADDSERVER,
								IDC_BTNDELSERVER,
								IDC_BTNMODSERVER,
								IDC_BTNREFRESHSERVERS,
								0
							};

							/* Get length of server data */
							GetShort(recvbuf,2,&slen); slen = ntohs(slen);

							/* Get TT index of linked servers dialog */
							for (ttid = 0; tt[ttid].id != IDD_SERVERS; ttid++);

							/* Clear linked servers */
							while (serverhead.next)
								Server_Delete(&serverhead,serverhead.next);

							/* Create and display new list */
							if (slen)
								IS_CreateList(&serverhead,&recvbuf[2 + SIZE_SHORT],BUFFER_TYPE_SERVER);

                            Server_List(&serverhead,GetSubItemHandle(IDC_LVINBOUND));
                            
							/* Set everything back to normal */
							EnableWindow(hTreeView,TRUE);
							for (count = 0; nEnable[count]; count++)
								EnableWindow(GetDlgItem(tt[ttid].hwnd,nEnable[count]),TRUE);
							StatusBar_SetText(hStatus,tt[ttid].status,0);
							ci.refresh = 0;
						}

						else if (recvbuf[0] == 'R' && recvbuf[1] == 'F')
						/* Filter data */
						{
							unsigned short slen;
							int ttid, count, nEnable[] =
							/* List of controls to enable */
							{
								IDC_BTNADDFILTER,
								IDC_BTNDELFILTER,
								IDC_BTNMODFILTER,
								IDC_BTNREFRESHFILTERS,
								IDC_CKFILTER,
								0
							};

							/* Get length of filter data */
							GetShort(recvbuf,2,&slen); slen = ntohs(slen);

							/* Get TT index of filter dialog */
							for (ttid = 0; tt[ttid].id != IDD_FILTER; ttid++);

							/* Clear filter list */
							while (filterhead.next)
								Filter_Delete(&filterhead,filterhead.next->word,filterhead.next->type);

							/* Create and display new list */
							if (slen)
								IS_CreateList(&filterhead,&recvbuf[2 + SIZE_SHORT],BUFFER_TYPE_FILTER);

							Filter_List(&filterhead,tt[ttid].hwnd,FILTER_TYPE_ALL);
                            
							/* Set everything back to normal */
							EnableWindow(hTreeView,TRUE);
							for (count = 0; nEnable[count]; count++)
								EnableWindow(GetDlgItem(tt[ttid].hwnd,nEnable[count]),TRUE);
							StatusBar_SetText(hStatus,tt[ttid].status,0);
							ci.refresh = 0;
						}
						else if (recvbuf[0] == 'R' && recvbuf[1] == 'R' && recvbuf[2] == 'C' && recvbuf[3] != 'A')
						/* Registered channel data */
						{
							unsigned short slen;
							int ttid, count, nEnable[] =
							/* List of controls to enable */
							{
								IDC_BTNADDREGCHAN,
								IDC_BTNDELREGCHAN,
								IDC_BTNMODREGCHAN,
								IDC_BTNREFRESHREGCHANS,
								0
							};

							/* Get length of RCL data */
							GetShort(recvbuf,3,&slen); slen = ntohs(slen);

							/* Get TT index of channel dialog */
							for (ttid = 0; tt[ttid].id != IDD_CHANNEL; ttid++);

							/* Clear RCL list */
							while (RCLHead.next)
								RCL_Delete(&RCLHead,RCLHead.next);

							/* Create and display new list */
							if (slen)
								IS_CreateList(&RCLHead,&recvbuf[3 + SIZE_SHORT],BUFFER_TYPE_RCL);

							RCL_List(&RCLHead,GetDlgItem(tt[ttid].hwnd,IDC_LVREGCHAN));
                            
							/* Set everything back to normal */
							EnableWindow(hTreeView,TRUE);
							for (count = 0; nEnable[count]; count++)
								EnableWindow(GetDlgItem(tt[ttid].hwnd,nEnable[count]),TRUE);
							StatusBar_SetText(hStatus,tt[ttid].status,0);
							ci.refresh = 0;
						}

						else if (recvbuf[0] == 'R' && recvbuf[1] == 'B')
						/* Ban/exception information being refreshed */
						{
							unsigned short slen;
							int ttid, count, nEnable[] =
							/* List of controls to enable */
							{
								IDC_BTNADDBAN,
								IDC_BTNDELBAN,
								IDC_BTNMODBAN,
								IDC_BTNREFRESHBANS,
								0
							};

							/* Get length of ban/exception data */
							GetShort(recvbuf,2,&slen); slen = ntohs(slen);

							/* Get TT index of ban/exception dialog */
							for (ttid = 0; tt[ttid].id != IDD_BANS; ttid++);

							/* Clear ban/exception list */
							while (banhead.next)
								Banlist_Delete(&banhead,banhead.next);

							/* Create and display new list */
							if (slen)
								IS_CreateList(&banhead,&recvbuf[2 + SIZE_SHORT],BUFFER_TYPE_BAN);

							Banlist_List(&banhead,GetDlgItem(tt[ttid].hwnd,IDC_LVBANS),FALSE);
							Banlist_List(&banhead,GetDlgItem(tt[ttid].hwnd,IDC_LVEXCEPTIONS),TRUE);
                            
							/* Set everything back to normal */
							EnableWindow(hTreeView,TRUE);
							for (count = 0; nEnable[count]; count++)
								EnableWindow(GetDlgItem(tt[ttid].hwnd,nEnable[count]),TRUE);
							StatusBar_SetText(hStatus,tt[ttid].status,0);
							ci.refresh = 0;
						}

						else if (recvbuf[0] == 'R' && recvbuf[1] == 'A')
						/* Account information being refreshed */
						{
							unsigned short slen;
							int ttid, count, nEnable[] =
							/* List of controls to enable */
							{
								IDC_BTNADDACCT,
								IDC_BTNDELACCT,
								IDC_BTNMODACCT,
								IDC_BTNREFRESHACCTS,
								0
							};

							/* Get length of account data */
							GetShort(recvbuf,2,&slen); slen = ntohs(slen);

							/* Get TT index of account dialog */
							for (ttid = 0; tt[ttid].id != IDD_ACCTS; ttid++);

							/* Clear account list */
							while (accthead.next)
								Account_Delete(&accthead,accthead.next);

							/* Create and display new list */
							if (slen)
								IS_CreateList(&accthead,&recvbuf[2 + SIZE_SHORT],BUFFER_TYPE_ACCOUNT);

							Account_List(&accthead,GetDlgItem(tt[ttid].hwnd,IDC_LVACCTS),TabCtrl_GetCurSel(GetSubItemHandle(IDC_TABACCTS)));
                            
							/* Set everything back to normal */
							EnableWindow(hTreeView,TRUE);
							for (count = 0; nEnable[count]; count++)
								EnableWindow(GetDlgItem(tt[ttid].hwnd,nEnable[count]),TRUE);
							StatusBar_SetText(hStatus,tt[ttid].status,0);
							ci.refresh = 0;
						}
						else if (recvbuf[0] == 'R' && recvbuf[1] == 'R' && recvbuf[2] == 'C' && recvbuf[3] == 'A')
						/* Server is sending channel access information */
						{
							HWND hTab = GetDlgItem(hRegChan,IDC_TABREGCHAN),
								hBtnOK = GetDlgItem(hRegChan,IDC_BTNREGCHANOK),
								hBtnCancel = GetDlgItem(hRegChan,IDC_BTNREGCHANCANCEL),
								hRCListView = GetSubItemHandle(IDC_LVREGCHAN), 
								hParent = hTabRegChan[1], 
								hListView = GetDlgItem(hParent,IDC_LVREGCHANACCESS);

							unsigned int WndPtr = GetWindowLongPtr(hRegChan,GWL_USERDATA);
							unsigned short slen;
							int count, nEnable[] =
							/* List of controls to enable */
							{
								IDC_BTNREGCHANADDACCESS,
								IDC_BTNREGCHANDELACCESS,
								IDC_BTNREGCHANMODACCESS,
								IDC_BTNREGCHANREFRESHACCESS,
								0
							};

							if (recvbuf[4] == 'N' && recvbuf[5] == 'C')
							/* Channel not found */
								MessageBox(hwnd,"The server reports this channel is not registered.\r\n\r\nTry going to the \"I/O control\" option and applying your changes first.","Error",MB_OK | MB_ICONERROR);
							else
							{
								/* Get number of access entries */
								GetShort(recvbuf,4 + SIZE_SHORT,&slen); slen = ntohs(slen);

								if (slen)
								/* There are access entries */
									IS_CreateList(&RCLDialog,&recvbuf[4],BUFFER_TYPE_ACCESS);

								RCL_ListAccess(hListView,&RCLDialog);
							}

							/* Set everything back to normal */
							ToggleClose(hRegChan,TRUE);
							EnableWindow(hTab,TRUE);
							EnableWindow(hBtnOK,TRUE);
							EnableWindow(hBtnCancel,TRUE);
							for (count = 0; nEnable[count]; count++)
								EnableWindow(GetDlgItem(hTabRegChan[1],nEnable[count]),TRUE);
							ci.refresh = 0;
						}
						free(recvbuf);

						/* Re-enable FD_READ event */
						WSAAsyncSelect(s,hwnd,Message,FD_WRITE | FD_READ | FD_CLOSE);
					}
					break;
					case FD_CLOSE:
						{
							int count, nEnable[20], ttlookup = 0, ttid;

							MessageBox(hwnd,"The connection to the server has been closed.","Error",MB_OK | MB_ICONERROR);

							memset(nEnable,0,sizeof(nEnable));

							switch (ci.refresh)
							{
								case REFRESH_SERVERS:
									ttlookup = IDD_SERVERS;
									nEnable[0] = IDC_BTNADDSERVER;
									nEnable[1] = IDC_BTNDELSERVER;
									nEnable[2] = IDC_BTNMODSERVER;
									nEnable[3] = IDC_BTNREFRESHSERVERS;
								break;
								case REFRESH_FILTERS:
									ttlookup = IDD_FILTER;
									nEnable[0] = IDC_BTNADDFILTER;
									nEnable[1] = IDC_BTNDELFILTER;
									nEnable[2] = IDC_BTNMODFILTER;
									nEnable[3] = IDC_BTNREFRESHFILTERS;
									nEnable[4] = IDC_CKFILTER;
								break;
								case REFRESH_CHANS:
									ttlookup = IDD_CHANNEL;
									nEnable[0] = IDC_BTNADDREGCHAN;
									nEnable[1] = IDC_BTNDELREGCHAN;
									nEnable[2] = IDC_BTNMODREGCHAN;
									nEnable[3] = IDC_BTNREFRESHREGCHANS;
								break;
								case REFRESH_BANS:
									ttlookup = IDD_BANS;
									nEnable[0] = IDC_BTNADDBAN;
									nEnable[1] = IDC_BTNDELBAN;
									nEnable[2] = IDC_BTNMODBAN;
									nEnable[3] = IDC_BTNREFRESHBANS;
								break;
								case REFRESH_ACCTS:
									ttlookup = IDD_ACCTS;
									nEnable[0] = IDC_BTNADDACCT;
									nEnable[1] = IDC_BTNDELACCT;
									nEnable[2] = IDC_BTNMODACCT;
									nEnable[3] = IDC_BTNREFRESHACCTS;
								break;
								case REFRESH_ACCESS:
								/* Special case, access being refreshed */
								{
									HWND hTab = GetDlgItem(hRegChan,IDC_TABREGCHAN),
										hBtnOK = GetDlgItem(hRegChan,IDC_BTNREGCHANOK),
										hBtnCancel = GetDlgItem(hRegChan,IDC_BTNREGCHANCANCEL);

									nEnable[0] = IDC_BTNREGCHANADDACCESS;
									nEnable[1] = IDC_BTNREGCHANMODACCESS;
									nEnable[2] = IDC_BTNREGCHANDELACCESS;

									ToggleClose(hRegChan,TRUE);
									EnableWindow(hTab,TRUE);
									EnableWindow(hBtnOK,TRUE);
									EnableWindow(hBtnCancel,TRUE);
									for (count = 0; nEnable[count]; count++)
										EnableWindow(GetDlgItem(hTabRegChan[1],nEnable[count]),TRUE);

									Disconnect();

									SetActiveWindow(hRegChan);

									return TRUE;
								}
								break;
							}

							ci.refresh = 0;

							/* Get TT index */
							for (ttid = 0; tt[ttid].id && tt[ttid].id != ttlookup; ttid++);

							if (tt[ttid].id)
							/* A refresh is currently in progress */
							{
								HWND hParent = tt[ttid].hwnd;

								/* Get new status bar text */
								StatusBar_SetText(hStatus,tt[ttid].status,0);

								/* Enable treeview & corresponding controls */
								EnableWindow(hTreeView,TRUE);
								
								for (count = 0; nEnable[count]; count++)
                                    EnableWindow(GetDlgItem(hParent,nEnable[count]),TRUE);
							}

							Disconnect();
						}
					break;
				}
			}
		}
		break;
		case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT) lParam;

			if (lpmis->CtlID == IDC_LISTREPORT)
			/* Technical status log */
				lpmis->itemHeight = 20;
		}
		break;
		case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lParam;
			HBITMAP hBMP, hBMPOld;
			TEXTMETRIC tm;
			HDC hdcMem;

			if (lpdis->CtlID == IDC_LISTREPORT)
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
						HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
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
			else if (lpdis->CtlID == IDC_CBMOTDCOLOR)
			/* Color MOTD ComboBox */
			{
				char szBuf[256];
				HBRUSH lBackBrush = NULL, hSysClrBrush = NULL;
				RECT rcSolid;

				SendMessage(lpdis->hwndItem,CB_GETLBTEXT,lpdis->itemID,(LPARAM)szBuf);

				if (lpdis->itemState & ODS_SELECTED)
				{
					lBackBrush = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
					SetBkColor(lpdis->hDC,GetSysColor(COLOR_HIGHLIGHT));
					SetTextColor(lpdis->hDC,lpdis->itemData);
				}
				else
				{
					lBackBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
					SetBkColor(lpdis->hDC,GetSysColor(COLOR_WINDOW));
					SetTextColor(lpdis->hDC,lpdis->itemData);
				}

				/* Fill in solid color sample */
				hSysClrBrush = CreateSolidBrush(lpdis->itemData);

				rcSolid.left = lpdis->rcItem.left;
				rcSolid.right = lpdis->rcItem.left + 20;
				rcSolid.top = lpdis->rcItem.top + 2 ;
				rcSolid.bottom = lpdis->rcItem.bottom - 2;

				FillRect(lpdis->hDC,&(lpdis->rcItem),lBackBrush);
				FillRect(lpdis->hDC,&rcSolid,hSysClrBrush);

				TextOut(lpdis->hDC,lpdis->rcItem.left + 24,lpdis->rcItem.top + 1,szBuf,strlen(szBuf));
			} 
		}
		break;

		case WM_CLOSE:
			if (hwnd == hMain)
				DestroyWindow(hwnd);
			else if (hwnd == ci.hConnect)
			/* Closing connection info dialog */
			{
				if (ci.bReady)
				{
					EndDialog(hwnd,0);
				}
				else
				/* Not connected, close remote admin */
				{
					EndDialog(hwnd,0);
					DestroyWindow(hMain);
				}
			}
			else
				EndDialog(hwnd,0);
			break;
		case WM_DESTROY:
			if (hwnd == hMain)
			{
				WSACleanup();
				PostQuitMessage(0);
			}
			break;
		case WM_PAINT:
			return FALSE;
			break;

		default:
			return FALSE;
	}

	return TRUE;
}

HWND StatusBar_Create(HWND parent)
{
	HWND retval;
	int statwidths[] = { ZRA_LENGTH, -1 };

	retval = CreateWindowEx(WS_EX_CLIENTEDGE, STATUSCLASSNAME, NULL,
        WS_CHILD | WS_VISIBLE , 0, 0, 0, 0,
        parent, NULL, GetModuleHandle(NULL), NULL);

    SendMessage(retval, SB_SETPARTS, sizeof(statwidths) / sizeof(int), (LPARAM)statwidths);

	return retval;
}

void StatusBar_SetText(HWND sbHandle, char *str, int part)
/*
** StatusBar_SetText()
** Changes the status text to string
*/
{
	SendMessage(sbHandle, SB_SETTEXT, part, (LPARAM)str);
}

char *StatusBar_GetText(HWND sbHandle, char *buffer)
/*
** StatusBar_GetText()
** This function retrieves the text from the specified status bar and fills the buffer
** Returns: Pointer to result
** NOTE: buffer is assumed to be initialized and have the proper amount of chars
*/
{
	SendMessage(sbHandle,SB_GETTEXT,0,(LPARAM)buffer);

	return buffer;
}

void TreeView_Create()
/*
** TreeView_Create()
** Fills treeview with items from tree table
*/
{
	TV_INSERTSTRUCT tvis;
	
	int count;

	for (count = 0; tt[count].text != NULL; count++)
	/* Map tree entries from table */
	{
		if (count == 0)
		{
			tvis.hParent = NULL;
			tvis.hInsertAfter = TVI_LAST;
			tvis.item.mask = TVIF_TEXT;
		}
		else
		{
			tvis.hParent = tt[0].hItem;
		}

		tvis.item.pszText = tt[count].text;
		tvis.item.cchTextMax = (int)strlen(tt[count].text);
		tt[count].hItem = TreeView_InsertItem(hTreeView,&tvis);
	}
}

int CreateSubWindows(HWND parent)
/*
** CreateSubWindows()
** Function to create all information windows(Status, I/O control, etc), and store em
*/
{
	int count;
	DWORD dwStyle;
	HTREEITEM sel = (HTREEITEM)SendMessage(hTreeView,TVM_GETNEXTITEM,0,0);
	HWND hTab, hCurr, hLocal;
	TC_ITEM pitem;

	/* All tab control items will be in text */
	pitem.mask = TCIF_TEXT;

	/* Load rich edit control library */
	LoadLibrary("RICHED20.DLL");

	for (count = 0; tt[count].id != 0; count++)
	{
		tt[count].hwnd = CreateDialog(hInst,MAKEINTRESOURCE(tt[count].id),parent,(DLGPROC)MainDlgProc);
		MoveWindow(tt[count].hwnd,172,13,512,411,TRUE);

		ShowWindow(tt[count].hwnd,SW_HIDE);

		if (sel == tt[count].hItem)
			ShowWindow(tt[count].hwnd,SW_SHOW);
		else
			ShowWindow(tt[0].hwnd,SW_SHOW);
	}

	ShowWindow(tt[0].hwnd,SW_SHOW);
	hActive = tt[0].hwnd;

	/* Initialize tab controls */
	hCurr = tt[3].hwnd;

	hTab = GetDlgItem(hCurr,IDC_TABGENERAL);
	
	pitem.pszText = "Settings";
	TabCtrl_InsertItem(hTab,0,&pitem);

	pitem.pszText = "MOTD";
	TabCtrl_InsertItem(hTab,1,&pitem);

	hTabGeneral[0] = CreateDialog(hInst,MAKEINTRESOURCE(IDD_GENERALSETTINGS),hCurr,(DLGPROC)MainDlgProc);
	MoveWindow(hTabGeneral[0],15,32,465,350,TRUE);
	ShowWindow(hTabGeneral[0],SW_SHOW);

	hTabGeneral[1] = CreateDialog(hInst,MAKEINTRESOURCE(IDD_GENERALMOTD),hCurr,(DLGPROC)MainDlgProc);
	MoveWindow(hTabGeneral[1],15,32,465,350,TRUE);
	ShowWindow(hTabGeneral[1],SW_HIDE);

	/* Set MOTD color dialog box */
	hLocal = GetDlgItem(hTabGeneral[1],IDC_CBMOTDCOLOR);

	for (count = 0; MOTDArray[count].szDisplayStr[0]; count++)
	{
		ComboBox_AddString(hLocal,MOTDArray[count].szDisplayStr);
		ComboBox_SetItemData(hLocal,count,MOTDArray[count].crRGB);
	}

	/* Set default to 01 - black */
	ComboBox_SetCurSel(hLocal,1);

	/* Set events for rich edit control */
	hLocal = GetDlgItem(hTabGeneral[1],IDC_EDITMOTD);
	SendMessage(hLocal,EM_SETEVENTMASK,0,(LPARAM)(ENM_KEYEVENTS | ENM_SELCHANGE | ENM_DROPFILES | ENM_DRAGDROPDONE));

	/* Load technical log bitmaps */
	hBMPListBox[BITMAP_OK] = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_OK));
	hBMPListBox[BITMAP_INFO] = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_INFO));
	hBMPListBox[BITMAP_WARNING] = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_WARNING));
	hBMPListBox[BITMAP_ERROR] = LoadBitmap(hInst,MAKEINTRESOURCE(IDB_ERROR));

	/* Fill in General tab controls */
	SendMessage(GetDlgItem(hTabGeneral[1],IDC_BTNMOTDSAVE),BM_SETIMAGE,(WPARAM)IMAGE_BITMAP,(LPARAM)LoadImage(hInst,MAKEINTRESOURCE(IDB_SAVE),IMAGE_BITMAP,0,0,LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT));
	SendMessage(GetDlgItem(hTabGeneral[1],IDC_BTNMOTDLOAD),BM_SETIMAGE,(WPARAM)IMAGE_BITMAP,(LPARAM)LoadImage(hInst,MAKEINTRESOURCE(IDB_LOAD),IMAGE_BITMAP,0,0,LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT));
	SendMessage(GetDlgItem(hTabGeneral[1],IDC_BTNMOTDBOLD),BM_SETIMAGE,(WPARAM)IMAGE_BITMAP,(LPARAM)LoadImage(hInst,MAKEINTRESOURCE(IDB_BOLD),IMAGE_BITMAP,0,0,LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT));
	SendMessage(GetDlgItem(hTabGeneral[1],IDC_BTNMOTDUNDERLINE),BM_SETIMAGE,(WPARAM)IMAGE_BITMAP,(LPARAM)LoadImage(hInst,MAKEINTRESOURCE(IDB_UNDERLINE),IMAGE_BITMAP,0,0,LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT));

	/* Accounts tab */
	hCurr = tt[4].hwnd;

	hTab = GetDlgItem(hCurr,IDC_TABACCTS);
	
	pitem.pszText = "Power users";
	TabCtrl_InsertItem(hTab,0,&pitem);

	pitem.pszText = "IRCops";
	TabCtrl_InsertItem(hTab,1,&pitem);

	pitem.pszText = "Administrators";
	TabCtrl_InsertItem(hTab,2,&pitem);

	/* Set up accounts list view control */
	hCurr = GetDlgItem(hCurr,IDC_LVACCTS);
	ListView_AddColumn(hCurr,0,"Account name",95);
	ListView_AddColumn(hCurr,1,"Hostmask",105);
	ListView_AddColumn(hCurr,2,"Logins",200);

	dwStyle = (DWORD)SendMessage(hCurr,LVM_GETEXTENDEDLISTVIEWSTYLE,0,0);
	SendMessage(hCurr,LVM_SETEXTENDEDLISTVIEWSTYLE,0,dwStyle | LVS_EX_FULLROWSELECT);

	/* Bans/Exceptions window */
	hCurr = tt[5].hwnd;

	hTab = GetDlgItem(hCurr,IDC_TABBANS);

	pitem.pszText = "Bans";
	TabCtrl_InsertItem(hTab,0,&pitem);

	pitem.pszText = "Exceptions";
	TabCtrl_InsertItem(hTab,1,&pitem);

	/* Set up ban/exception list view controls */
	hCurr = GetDlgItem(tt[5].hwnd,IDC_LVBANS);
	ListView_AddColumn(hCurr,0,"Hostmask",100);
	ListView_AddColumn(hCurr,1,"Scope",100);
	ListView_AddColumn(hCurr,2,"Reason",300);
	dwStyle = (DWORD)SendMessage(hCurr,LVM_GETEXTENDEDLISTVIEWSTYLE,0,0);
	SendMessage(hCurr,LVM_SETEXTENDEDLISTVIEWSTYLE,0,dwStyle | LVS_EX_FULLROWSELECT);

	hCurr = GetDlgItem(tt[5].hwnd,IDC_LVEXCEPTIONS);
	ListView_AddColumn(hCurr,0,"Hostmask",100);
	dwStyle = (DWORD)SendMessage(hCurr,LVM_GETEXTENDEDLISTVIEWSTYLE,0,0);
	SendMessage(hCurr,LVM_SETEXTENDEDLISTVIEWSTYLE,0,dwStyle | LVS_EX_FULLROWSELECT);

	/* Set up registered channels */
	hCurr = GetDlgItem(tt[6].hwnd,IDC_LVREGCHAN);
	ListView_AddColumn(hCurr,0,"Name",150);
	ListView_AddColumn(hCurr,1,"Topic",200);
	dwStyle = (DWORD)SendMessage(hCurr,LVM_GETEXTENDEDLISTVIEWSTYLE,0,0);
	SendMessage(hCurr,LVM_SETEXTENDEDLISTVIEWSTYLE,0,dwStyle | LVS_EX_FULLROWSELECT);

	/* Linked servers window */

	/* Add server types to server type combo box */
	hCurr = GetDlgItem(tt[8].hwnd,IDC_CBNETWORKTYPE);
	ComboBox_AddString(hCurr,"Star");
	ComboBox_AddString(hCurr,"Ring");
	ComboBox_AddString(hCurr,"Mesh");
	ComboBox_SetCurSel(hCurr,0);
	
	/* Add columns to listview controls */
    hCurr = GetDlgItem(tt[8].hwnd,IDC_LVINBOUND);
	ListView_AddColumn(hCurr,0,"Name",150);
	ListView_AddColumn(hCurr,1,"Address",125);
	ListView_AddColumn(hCurr,2,"Outbound port",150);

	dwStyle = (DWORD)SendMessage(hCurr,LVM_GETEXTENDEDLISTVIEWSTYLE,0,0);
	SendMessage(hCurr,LVM_SETEXTENDEDLISTVIEWSTYLE,0,dwStyle | LVS_EX_FULLROWSELECT);

	//hCurr = GetDlgItem(tt[8].hwnd,IDC_LVOUTBOUND);
	//ListView_AddColumn(hCurr,0,"Name",200);
	//ListView_AddColumn(hCurr,1,"Address",150);
	//ListView_AddColumn(hCurr,2,"Port",100);
	//dwStyle = (DWORD)SendMessage(hCurr,LVM_GETEXTENDEDLISTVIEWSTYLE,0,0);
	//SendMessage(hCurr,LVM_SETEXTENDEDLISTVIEWSTYLE,0,dwStyle | LVS_EX_FULLROWSELECT);

	Init();
	//curr->item[13] = CreateCtl(&curr->hItem,IDLV_CHANNELS,ZRA_CS_LISTVIEW,5,125,265,95,NULL,WS_VSCROLL | WS_HSCROLL | LVS_NOSORTHEADER | LVS_REPORT | LVS_ALIGNLEFT | LVS_SINGLESEL | LVS_SHOWSELALWAYS);
	//curr->item[5] = CreateCtl(&curr->hItem,IDLV_ACCTS,ZRA_CS_LISTVIEW,15,55,250,150,NULL,WS_VSCROLL | WS_HSCROLL | LVS_NOSORTHEADER | LVS_REPORT | LVS_ALIGNLEFT | LVS_SINGLESEL | LVS_SHOWSELALWAYS);
	return FALSE;
}

int DUtoPixels(int dialogUnits)
{
     // The conversion of DUs to pixels is complex if the font associated with
     // the control is not the Fixed System font (we use MS Sans Serif).
     // In our case, the dialog base unit is the average width of the
     // MS Sans Serif font.
     // Basic conversion code came from the Win32 SDK help on GetDialogBaseUnits().
     // Notes about fonts is in Win32 SDK help on MapDialogRect().

     TEXTMETRIC tm;
	 HFONT hFont = (HFONT)SendMessage(hMain,WM_GETFONT,0,0);
	 HDC hDC = CreateCompatibleDC(NULL);
	 int dialogBaseX;

	 SelectObject(hDC,hFont);
     GetTextMetrics(hDC,&tm);

     dialogBaseX = tm.tmAveCharWidth;

	 DeleteDC(hDC);

     return (dialogUnits * dialogBaseX) / 4;
}

void ListView_AddColumn(HWND hwnd, int index, char* coltext, int width)
/*
** ListView_AddColumn()
** This function adds a column on the specified listview
*/
{
	int retval;
	LV_COLUMN lvcolumn;

	memset(&lvcolumn,0,sizeof(lvcolumn));

	lvcolumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lvcolumn.iSubItem = index;
	lvcolumn.cx = width;
	lvcolumn.pszText = coltext;

	retval = ListView_InsertColumn(hwnd,index,&lvcolumn);
	
}

void Init()
/*
** Init()
** Fills in all button images, list view columns, basically makes the thing come to life
*/
{
	/* Step #1, fill in all buttons with bitmaps */
	BtnInfoTable btns[] = {
		{
			LoadImage(hInst,MAKEINTRESOURCE(IDB_ADD),IMAGE_BITMAP,0,0,LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT),
			{ IDC_BTNADDACCT, IDC_BTNADDBAN, IDC_BTNADDREGCHAN, IDC_BTNADDSERVER, IDC_BTNADDFILTER, 0 }
		},
		{
			LoadImage(hInst,MAKEINTRESOURCE(IDB_MODIFY),IMAGE_BITMAP,0,0,LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT),
			{ IDC_BTNMODACCT, IDC_BTNMODBAN, IDC_BTNMODREGCHAN, IDC_BTNMODSERVER, IDC_BTNMODFILTER, 0 }
		},
		{
			LoadImage(hInst,MAKEINTRESOURCE(IDB_DELETE),IMAGE_BITMAP,0,0,LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT),
			{ IDC_BTNDELACCT, IDC_BTNDELBAN, IDC_BTNDELREGCHAN, IDC_BTNDELSERVER, IDC_BTNDELFILTER, IDC_BTNCCAUTOJOIN2, 0 }
		},
		{
			LoadImage(hInst,MAKEINTRESOURCE(IDB_REFRESH),IMAGE_BITMAP,0,0,LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT),
			{ IDC_BTNREFRESHACCTS, IDC_BTNREFRESHBANS, IDC_BTNREFRESHREGCHANS, IDC_BTNREFRESHSERVERS, IDC_BTNREFRESHFILTERS, 0 }
		},
		{ NULL, NULL }
	};

	int count1, count2;
	HWND btn = NULL, hCurr = NULL;

	for (count1 = 0; btns[count1].button != NULL; count1++)
	{
		for (count2 = 0; btns[count1].btnid[count2] != 0; count2++)
		{
			btn = GetSubItemHandle(btns[count1].btnid[count2]);

			SendMessage(btn,BM_SETIMAGE,(WPARAM)IMAGE_BITMAP,(LPARAM)btns[count1].button);
		}
	}

	/* Step #2, initialize check boxes */
	ckitems[0].parent = ckitems[1].parent = ckitems[2].parent = hTabGeneral[0];
	ckitems[3].parent = tt[6].hwnd;
	ckitems[4].parent = ckitems[5].parent = ckitems[6].parent = tt[7].hwnd;
	ckitems[7].parent = tt[9].hwnd;
	ckitems[8].parent = tt[10].hwnd;

	for (count1 = 0; ckitems[count1].ckid != 0; count1++)
	/* Fill structures with handles to check boxes */
	{
		ckitems[count1].ckhwnd = GetDlgItem(ckitems[count1].parent,ckitems[count1].ckid);
		
		/* Uncheck all boxes */
		for (count2 = 0; ckitems[count1].id[count2] != 0; count2++)
			EnableWindow(GetDlgItem(ckitems[count1].parent,ckitems[count1].id[count2]),FALSE);
	}

	/* Step #3, Fill in all information */

	/* Server action combobox */
	hCurr = GetSubItemHandle(IDC_CBSERVERACTION);
	SendMessage(hCurr,CB_ADDSTRING,0,(LPARAM)"Start");
	SendMessage(hCurr,CB_ADDSTRING,0,(LPARAM)"Restart");
	SendMessage(hCurr,CB_ADDSTRING,0,(LPARAM)"Halt");
	SendMessage(hCurr,CB_ADDSTRING,0,(LPARAM)"Shutdown");
	SendMessage(hCurr,CB_SETCURSEL,0,0);

	/* DNS cache info */
	hCurr = GetSubItemHandle(IDC_CBCACHEDNS);
	SendMessage(hCurr,CB_ADDSTRING,0,(LPARAM)"Seconds");
	SendMessage(hCurr,CB_ADDSTRING,0,(LPARAM)"Minutes");
	SendMessage(hCurr,CB_ADDSTRING,0,(LPARAM)"Hours");
	SendMessage(hCurr,CB_SETCURSEL,0,0);

	/* IP masking radio button */
	hCurr = GetSubItemHandle(IDC_RDMASK1);
	SendMessage(hCurr,BM_SETCHECK,BST_CHECKED,0);

	/* Consequence for spoofed IP combobox */
	hCurr = GetSubItemHandle(IDC_CBSPOOFPUNISHMENT);
	SendMessage(hCurr,CB_ADDSTRING,0,(LPARAM)"Do nothing, just log");
	SendMessage(hCurr,CB_ADDSTRING,0,(LPARAM)"Issue warning");
	SendMessage(hCurr,CB_ADDSTRING,0,(LPARAM)"Ban from server");
	SendMessage(hCurr,CB_ADDSTRING,0,(LPARAM)"Ban from network");
	SendMessage(hCurr,CB_SETCURSEL,0,0);

	/* Set all edit boxes to 0 */
	for (count1 = 0; scrolledits[count1].edit != 0; count1++)
	{
		scrolledits[count1].hEditBox = GetSubItemHandle(scrolledits[count1].edit);
		scrolledits[count1].hScrollBar = GetSubItemHandle(scrolledits[count1].scrollbar);
		SetWindowText(scrolledits[count1].hEditBox,"0");
	}

	/* Step #4, initialize linked lists */
	accthead.next = NULL;
	banhead.next = NULL;
	filterhead.next = NULL;
	RCLHead.next = NULL;
	serverhead.next = NULL;
	spoofedhead.next = NULL;

	memset(&ci,0,sizeof(ci));
}

HWND GetSubItemHandle(int id)
/*
** GetSubItemHandle()
** Loops through windows and finds the control, and returns the handle to it
*/
{
	int count = 0;
	HWND hResult =  NULL;

	if ((hResult = GetDlgItem(hTabGeneral[0],id)) == NULL)
	{
		if ((hResult = GetDlgItem(hTabGeneral[1],id)) != NULL)
			return hResult;
	}
	else
		return hResult;

	for (count = 0; tt[count].hwnd != NULL; count++)
		if ( (hResult = GetDlgItem(tt[count].hwnd,id)) != NULL)
			break;


	return hResult;
}

void AddToEdit(HWND editbox, int value)
/*
** AddToEdit()
** Adds contents of specified edit box to specified value
*/
{
	char buffer[16];
	short newval;

	GetWindowText(editbox,buffer,16);
	newval = (short)atoi(buffer);
	newval += (short)value;
	
	if (newval < 0)
		newval = 0;

	sprintf(buffer,"%d",newval);

	SetWindowText(editbox,buffer);
	SetFocus(editbox);
}

void GetTextPathname(char* pszBuffer, unsigned int uBuflen)
/*
** GetTextPathname()
** Fills in specified buffer with pathname to text directory
*/
{
	char buf[MAX_PATH];
	char *strptr;

	GetModuleFileName(NULL,buf,MAX_PATH);

	strptr = strrchr(buf,92);
	strptr[0] = 0;

	strcat(buf,"\\text\\");
	if (strlen(buf) > uBuflen)
		buf[uBuflen] = 0;

	strcpy(pszBuffer,buf);
}

void ProgressBar_SetPos(HWND hProgress, unsigned int pos)
/*
** ProgressBar_SetPos()
** Sets target progress bar to new position specified
*/
{
	SendMessage(hProgress,PBM_SETPOS,(WPARAM)pos,0);
	InvalidateRect(hProgress,NULL,FALSE);
	UpdateWindow(hProgress);

}

void ConnectToServer(VOID* pvoid)
/*
** ConnectToServer()
** Secondary thread procedure which handles all socket work for connection
*/
{
	SOCKET s;
	unsigned short len;
	unsigned long uladdr, retval;
	char buf[256], *data, connip[32], sendbuf[1024], recvbuf[1024];
	struct hostent *pHE;
	SOCKADDR_IN sin;	

	memset(&sin,0,sizeof(SOCKADDR_IN));
    uladdr = inet_addr(ci.hostname);

    if (uladdr == INADDR_NONE)
	/* Hostname needs to be resolved */
	{
		sprintf(ci.status,"Obtaining IP address for hostname %s...",ci.hostname);
		UpdateConnectInfo(ci.hConnect);

		pHE = gethostbyname(ci.hostname);

		if (!pHE)
		/* Unable to resolve */
		{
			sprintf(ci.status,"Unable to resolve IP address for hostname %s",ci.hostname);
			ci.bTerminate = TRUE;
			UpdateConnectInfo(ci.hConnect);
			return;
		}
		else
		/* Successfully resolved hostname */
		{
			uladdr = *((unsigned long*)pHE->h_addr_list[0]);

			sin.sin_addr.s_addr = uladdr;

			strcpy(ci.ipaddr,inet_ntoa(sin.sin_addr));
			sprintf(ci.status,"Connecting to %s(%s)...",ci.hostname,ci.ipaddr);
			UpdateConnectInfo(ci.hConnect);
			strcpy(connip,ci.ipaddr);
		}
	}
	else
	/* IP address given for hostname */
	{
		sprintf(ci.status,"Connecting to %s...",ci.hostname);
		UpdateConnectInfo(ci.hConnect);
		strcpy(connip,ci.hostname);
	}

	/* Create connection socket */
	s = ci.socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(connip);
	sin.sin_port = htons(ci.port);

	retval = connect(s,(sockaddr*)&sin,sizeof(struct sockaddr));

	if (retval == SOCKET_ERROR)
	/* Unable to connect */
	{
		int err = WSAGetLastError();
		GetErrorString(buf,err);
		sprintf(ci.status,"Unable to connect to remote host %s:%d(Error %d: %s)",ci.hostname,ci.port,err,buf);
		ci.bTerminate = TRUE;
		UpdateConnectInfo(ci.hConnect);
		return;
	}

	/* Socket connected, initiate communication */
	strcpy(ci.status,"Socket connected, initiating communication...");
	UpdateConnectInfo(ci.hConnect);

	/* Step #1, Send version information */
	/* Temp: Use version 0.8 for beta testing */
	sendbuf[0] = 'V'; sendbuf[1] = 0; sendbuf[2] = 0; sendbuf[3] = 8;
	send(s,sendbuf,4,0);

	if ((retval = recv(s,recvbuf,5,0)) == SOCKET_ERROR)
	/* Error in recieving version data */
	{
		sprintf(ci.status,"Unable to complete version verification, Error: %d",WSAGetLastError());
		ci.bTerminate = TRUE;
		UpdateConnectInfo(ci.hConnect);
		return;
	}
	else
	{
		recvbuf[retval] = 0;

		if (recvbuf[0] == 'N')
		/* Version information incorrect */
		{
			sprintf(ci.status,"The server reports you have an incompatible version of the remote administration utility, version required: %d.%d",(unsigned int)recvbuf[2],ntohs(*((unsigned short*)&recvbuf[3])));
			ci.bTerminate = TRUE;
			UpdateConnectInfo(ci.hConnect);
			return;
		}
		else if (strcmp(recvbuf,"RQP") == 0)
		/* Version information correct, server is requesting password */
		{
			sprintf(ci.status,"Sending password...",(unsigned int)recvbuf[2],(*((unsigned short*)&recvbuf[3])));
			UpdateConnectInfo(ci.hConnect);
		}
	}

	/*
	** Step #2, Send password information
	** Password packet: RP + length of password(u_short) + password string
	*/
	sendbuf[0] = 'R'; sendbuf[1] = 'P';

	len = (unsigned short)strlen(ci.password);
	InsertShort(sendbuf,2,htons(len));
	memcpy(&sendbuf[2 + SIZE_SHORT],ci.password,len);
	len += SIZE_SHORT + 2;
    
	send(s,sendbuf,len,0);

	if ((retval = recv(s,recvbuf,2,0)) == SOCKET_ERROR)
	/* Error recieving password info */
	{
		int err = WSAGetLastError();
		GetErrorString(buf,err);

		sprintf(ci.status,"Unable to recv() password verification(Error %d: %s)",err,buf);
		ci.bTerminate = TRUE;
		UpdateConnectInfo(ci.hConnect);
		return;
	}
	else
	{
		recvbuf[retval] = 0;

		if (strcmp(recvbuf,RA_PROTOPASSWORDINVALID) == 0)
		/* Password is invalid */
		{
			sprintf(ci.status,"The server reports the password specified is invalid.",WSAGetLastError());
			ci.bTerminate = TRUE;
			UpdateConnectInfo(ci.hConnect);
			return;
		}
		else if (strcmp(recvbuf,RA_PROTOPASSWORDVALID) == 0)
		/* Password valid */
		{
			strcpy(ci.status,"Password valid.");
			UpdateConnectInfo(ci.hConnect);
		}
	}

	/* Step #3, Password verification complete, line now open for commands */

	/* Send command to download server information */
	strcpy(sendbuf,"RQD");
	len = (unsigned short)strlen(sendbuf);

	send(s,sendbuf,len,0);

	/* First 6 bytes are our information about our information */
	if ((retval = recv(s,recvbuf,6,0)) == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		GetErrorString(buf,err);

		sprintf(ci.status,"Unable to download server information header(Error %d: %s)",err,buf);
		ci.bTerminate = TRUE;
		UpdateConnectInfo(ci.hConnect);
		return;
	}
	else
	{
		unsigned long dlen;

		GetLong(recvbuf,2,&dlen); dlen = ntohl(dlen);

		data = (char*)malloc(dlen);

        if ((retval = recv(s,data,dlen,0)) == SOCKET_ERROR)
		/* Error in transmit of server data */
		{
			int err = WSAGetLastError();
			GetErrorString(buf,err);

			sprintf(ci.status,"Unable to recv() server data(Error %d: %s)",err,buf);
			ci.bTerminate = TRUE;
			UpdateConnectInfo(ci.hConnect);
			return;
		}
		else
		/* Recieved data */
		{
			ClearLinkedLists();
			IS_LoadFromBuffer(data,&SettingsInfo);

			ci.bReady = TRUE;
			EndDialog(ci.hConnect,0);
		}
	}

	/* Update dialog with information */
	UpdateRAInfo(&SettingsInfo);

	/* This thread is finished, so make connection socket asynchronous */
	SetNonBlocking(s);
	WSAAsyncSelect(s,hMain,WM_RASOCKET,FD_READ | FD_WRITE | FD_CLOSE);

	_endthread();
}

void Disconnect()
/*
** Disconnect()
** This function is called whenever the connection to the remote server is terminated
*/
{
	closesocket(ci.socket);
	ci.socket = 0;
	ToggleRefresh(FALSE);
	SetWindowText(hMain,"RockIRCX Remote Administration[Not connected]");

	/* Disable I/O server communication buttons */
	EnableWindow(GetDlgItem(tt[1].hwnd,IDC_BTNIOAPPLY),FALSE);
	EnableWindow(GetDlgItem(tt[1].hwnd,IDC_BTNIOREFRESH),FALSE);
}

void UpdateRAInfo(INFOSTRUCTSETTINGS* is)
/*
** UpdateRAInfo()
** Updates the RA dialog with the information specified in the info structure
*/
{
	char buf[256];
	HWND hCurr;
	int nTemp, nCount;
	unsigned short port;

	/* TODO: All boxes must be checked/unchecked properly */

	for (nTemp = 0; ckitems[nTemp].ckid; nTemp++)
	/* Uncheck all boxes */
	{
		if (BoxChecked(ckitems[nTemp].parent,ckitems[nTemp].ckid))
			SendDlgItemMessage(ckitems[nTemp].parent,ckitems[nTemp].ckid,BM_CLICK,0,0);
        
		/* Last one in list */
		if (ckitems[nTemp].ckid == IDC_CKFILTER)
			break;
	}
	/* Status window items */
	hCurr = tt[0].hwnd;

	/* User count */
	sprintf(buf,"%s:",is->isGeneral.servername);
	SetDlgItemText(hCurr,IDC_TXTSERVERNAME,buf);
	SetDlgItemInt(hCurr,IDC_EDITLOCALUSERS,is->isStatus.localusers,FALSE);
	SetDlgItemInt(hCurr,IDC_EDITLOCALUSERSMAX,is->isStatus.localmax,FALSE);
	SetDlgItemInt(hCurr,IDC_EDITGLOBALUSERS,is->isStatus.globalusers,FALSE);
	SetDlgItemInt(hCurr,IDC_EDITGLOBALUSERSMAX,is->isStatus.globalmax,FALSE);

	/* Accounts */
	sprintf(buf,"%d/%d",is->isStatus.powerusers,Account_GetTotal(&SettingsInfo.isAccounts.accthead,ACCOUNT_LEVEL_POWERUSER));
	SetDlgItemText(hCurr,IDC_TXTPOWERUSERS,buf);
	sprintf(buf,"%d/%d",is->isStatus.ircops,Account_GetTotal(&SettingsInfo.isAccounts.accthead,ACCOUNT_LEVEL_IRCOP));
	SetDlgItemText(hCurr,IDC_TXTIRCOPS,buf);
	sprintf(buf,"%d/%d",is->isStatus.admins,Account_GetTotal(&SettingsInfo.isAccounts.accthead,ACCOUNT_LEVEL_ADMIN));
	SetDlgItemText(hCurr,IDC_TXTADMINS,buf);

	/* Server status */
	SetDlgItemInt(hCurr,IDC_EDITUPTIME,is->isStatus.uptime,FALSE);

	/* Channel information */
	SetDlgItemInt(hCurr,IDC_TXTDYNLOCAL,is->isStatus.localchannels,FALSE);
	SetDlgItemInt(hCurr,IDC_TXTDYNGLOBAL,is->isStatus.globalchannels,FALSE);
	SetDlgItemInt(hCurr,IDC_TXTREGLOCAL,is->isStatus.localregchannels,FALSE);
	SetDlgItemInt(hCurr,IDC_TXTREGGLOBAL,is->isStatus.globalregchannels,FALSE);

	/* I/O control */
	hCurr = tt[1].hwnd;

	SetDlgItemInt(hCurr,IDC_TXTTOTALDESCRIPTORS,is->isIOControl.totaldescriptors,FALSE);
	SetDlgItemInt(hCurr,IDC_TXTLISTENINGSOCKETS,is->isIOControl.listeningsockets,FALSE);
	SetDlgItemInt(hCurr,IDC_TXTTOTALCONNECTIONS,is->isIOControl.totalconnections,FALSE);

	sprintf(buf,"%.1fKB",(float)is->isIOControl.datasent / 1024);
	SetDlgItemText(hCurr,IDC_TXTDATASENT,buf);
	sprintf(buf,"%.1fKB",(float)is->isIOControl.datarecvd / 1024);
	SetDlgItemText(hCurr,IDC_TXTDATARECIEVED,buf);

	/* General information */
	hCurr = hTabGeneral[0];

	/* Server */
    SetDlgItemText(hCurr,IDC_EDITNETWORKNAME,is->isGeneral.networkname);
	SetDlgItemText(hCurr,IDC_EDITSERVERNAME,is->isGeneral.servername);
	SetDlgItemInt(hCurr,IDC_EDITSERVERID,is->isGeneral.serverid,FALSE);
	SetDlgItemText(hCurr,IDC_EDITSERVERDESC,is->isGeneral.serverdescription);

	/* Administrative information */
	SetDlgItemText(hCurr,IDC_EDITLOC1,is->isGeneral.adminloc1);
	SetDlgItemText(hCurr,IDC_EDITLOC2,is->isGeneral.adminloc2);
	SetDlgItemText(hCurr,IDC_EDITADMINEMAIL,is->isGeneral.adminemail);

	/* Bindings */

	/* Clear old */
	while (SendDlgItemMessage(hCurr,IDC_LISTBINDINGS,LB_GETCOUNT,0,0))
		Binding_Delete(0);

	/* Add new */
	for (nTemp = 0; nTemp < is->isGeneral.bindingnum; nTemp++)
	{
		BindingBuf_GetInfo(is->isGeneral.bindingbuf,nTemp,buf,&port);
		Binding_Add(buf,port);
	}

	/* DNS/IP masking */

	if (is->isGeneral.dns_lookup)
		SendDlgItemMessage(hCurr,IDC_CKUSEDNS,BM_CLICK,0,0);
	if (is->isGeneral.dns_cache)
		SendDlgItemMessage(hCurr,IDC_CKCACHEDNS,BM_CLICK,0,0);

	SetDlgItemInt(hCurr,IDC_EDITCACHEDNS,is->isGeneral.dns_cachetime,FALSE);
	SendDlgItemMessage(hCurr,IDC_CBCACHEDNS,CB_SETCURSEL,is->isGeneral.dns_cachetimeduration,0);

	if (is->isGeneral.dns_masktype)
	{
		SendDlgItemMessage(hCurr,IDC_CKMASK,BM_CLICK,0,0);

		if (is->isGeneral.dns_masktype == 1)
			CheckBox(GetDlgItem(hCurr,IDC_RDMASK1),TRUE);
		else
			CheckBox(GetDlgItem(hCurr,IDC_RDMASK2),TRUE);
	}
	else
		CheckBox(GetDlgItem(hCurr,IDC_RDMASK1),TRUE);

	/* MOTD tab */
	hCurr = hTabGeneral[1];

	SetDlgItemText(hCurr,IDC_EDITMOTD,"");

	if (is->isGeneral.motdbuffer)
	/* Process the MOTD and make the IRC color/bold/underline codes show up on the rich edit control */
	{
		SETTEXTEX st;
		CHARFORMAT2 cb2;
		char szAdd[32], szTemp[32];
		int nCode;
		BOOL bUnderline = FALSE, bBold = FALSE;

		szAdd[1] = 0;

		st.codepage = CP_ACP;
		st.flags = ST_SELECTION;

		cb2.cbSize = sizeof(cb2);

		for (nCount = 0; is->isGeneral.motdbuffer[nCount]; nCount++)
		{
			nCode = 1;

			szAdd[0] = is->isGeneral.motdbuffer[nCount];

			if (szAdd[0] != 10)
			/* Skip LF */
			{
				cb2.dwEffects = CFE_AUTOCOLOR;
				cb2.dwMask = NULL;

				if (szAdd[0] == 2)
				/* Bold */
				{
					bBold = (bBold ? FALSE : TRUE);

					cb2.dwMask |= CFM_BOLD;

					if (bBold)
						cb2.dwEffects |= CFE_BOLD;

					nCode = 2;
				}
				else if (szAdd[0] == 31)
				/* Underline */
				{
					bUnderline = (bUnderline ? FALSE : TRUE);

					cb2.dwMask |= CFM_UNDERLINE;

					if (bUnderline)
						cb2.dwEffects |= CFE_UNDERLINE;

					nCode = 31;
				}

				if (szAdd[0] == 3)
				/* Color code */
				{
					cb2.dwMask |= CFM_COLOR;

					if (is->isGeneral.motdbuffer[nCount + 1] >= '0' &&
						is->isGeneral.motdbuffer[nCount + 1] <= '9')
					/* Change color */
					{
						strncpy(szTemp,&is->isGeneral.motdbuffer[nCount + 1],2);
						szTemp[2] = 0;
						nCode = atoi(szTemp);

						cb2.crTextColor = MOTDArray[nCode].crRGB;
						cb2.dwEffects &= ~CFE_AUTOCOLOR;

						nCount = nCount + 2;
					}
					else
					/* Remove color */
						nCode = 3;
				}

				if (nCode == 1)
				/* Add the character to the control */
				{
					SendDlgItemMessage(hCurr,IDC_EDITMOTD,EM_SETSEL,(WPARAM)(-1),(LPARAM)(-1));
					SendDlgItemMessage(hCurr,IDC_EDITMOTD,EM_SETTEXTEX,(WPARAM)&st,(LPARAM)&szAdd);
				}
				else
				/* Change formatting */
					SendDlgItemMessage(hCurr,IDC_EDITMOTD,EM_SETCHARFORMAT,(WPARAM)SCF_SELECTION,(LPARAM)&cb2);
			}
			else
			{
				bUnderline = FALSE;
				bBold = FALSE;

				cb2.dwMask = CFM_COLOR | CFM_BOLD | CFM_UNDERLINE;
				cb2.dwEffects = CFE_AUTOCOLOR;
				SendDlgItemMessage(hCurr,IDC_EDITMOTD,EM_SETCHARFORMAT,(WPARAM)SCF_SELECTION,(LPARAM)&cb2);
			}
		}
		//SetDlgItemText(hCurr,IDC_EDITMOTD,is->isGeneral.motdbuffer);
	}

	/* Channel settings */
	hCurr = tt[6].hwnd;

	if (is->isChannel.modeflag)
	/* Set default modes */
	{
		unsigned char modeflag[] = {
			ZRA_CHANMODE_MODERATED,
			ZRA_CHANMODE_NOEXTERN,
			ZRA_CHANMODE_ONLYOPSCHANGETOPIC,
			ZRA_CHANMODE_KNOCK,
			ZRA_CHANMODE_NOWHISPERS,
			0
		};
		int modeckbox[] = { 
			IDC_CKCDMMODERATED,
			IDC_CKCDMNOEXTERN,
			IDC_CKCDMONLYOPSSETTOPIC,
			IDC_CKCDMKNOCK,
			IDC_CKCDMNOWHISPERS,
			0
		};
		int count;

		for (count = 0; modeflag[count]; count++)
		/* Check appropriate boxes */
		{
			if (is->isChannel.modeflag & modeflag[count])
				CheckDlgBox(hCurr,modeckbox[count],TRUE);
			else
				CheckDlgBox(hCurr,modeckbox[count],FALSE);
		}
	}

	else
	{
		int modeckbox[] = { 
			IDC_CKCDMMODERATED,
			IDC_CKCDMNOEXTERN,
			IDC_CKCDMONLYOPSSETTOPIC,
			IDC_CKCDMKNOCK,
			IDC_CKCDMNOWHISPERS,
			0
		};
		int count;

		for (count = 0; modeckbox[count]; count++)
		/* Check appropriate boxes */
		{
			CheckDlgBox(hCurr,modeckbox[count],FALSE);
		}
	}

	if (is->isChannel.defaultlimit)
		SendDlgItemMessage(hCurr,IDC_CKCDMLIMIT,BM_CLICK,0,0);

	SetDlgItemInt(hCurr,IDC_EDITCDMLIMIT,is->isChannel.defaultlimit,FALSE);

	/* User settings */
	hCurr = tt[7].hwnd;

	CheckDlgBox(hCurr,IDC_CKCCCREATEDYN,is->isUser.dyn_create);
	CheckDlgBox(hCurr,IDC_CKCCJOINDYN,is->isUser.dyn_join);

	/* Clear autojoin list */
	SendDlgItemMessage(hCurr,IDC_LISTCCAUTOJOIN,LB_RESETCONTENT,0,0);

	if (is->isUser.autojoinbuffer)
	{	
		int count, chans = numtok(is->isUser.autojoinbuffer,44);
		HWND hListBox = GetDlgItem(hCurr,IDC_LISTCCAUTOJOIN);

		char toadd[1024];

		SendDlgItemMessage(hCurr,IDC_CKCCAUTOJOIN,BM_CLICK,0,0);

		/* Populate autojoin list */
		for (count = 0; count < chans; count++)
		{
			/* Get string tokens from autojoin buffer & add em */
			ListBox_AddString(hListBox,gettok(toadd,is->isUser.autojoinbuffer,count + 1,44));
		}
	}

	SetDlgItemInt(hCurr,IDC_EDITCCMAXCHANS,is->isUser.maxchannels,FALSE);
	SetDlgItemInt(hCurr,IDC_EDITUMUSERSPERIP,is->isUser.max_perip,FALSE);
	SetDlgItemInt(hCurr,IDC_EDITUMRECVQ,is->isUser.max_recvq,FALSE);
	SetDlgItemInt(hCurr,IDC_EDITUMSENDQ,is->isUser.max_sendq,FALSE);
	SetDlgItemInt(hCurr,IDC_EDITPINGDURATION,is->isUser.ping_duration,FALSE);
	SetDlgItemInt(hCurr,IDC_EDITPINGRESPONSE,is->isUser.ping_response,FALSE);

	if (is->isUser.nickdelay)
		SendDlgItemMessage(hCurr,IDC_CKNICKDELAY,BM_CLICK,0,0);

	SetDlgItemInt(hCurr,IDC_EDITNICKDELAY,is->isUser.nickdelay,FALSE);

	if (is->isUser.msgdelay)
		SendDlgItemMessage(hCurr,IDC_CKMSGDELAY,BM_CLICK,0,0);

	SetDlgItemInt(hCurr,IDC_EDITMSGDELAY,is->isUser.msgdelay,FALSE);

	/* Security settings */
	hCurr = tt[9].hwnd;

	CheckDlgBox(hCurr,IDC_CKCSNICKINCHAN,is->isSecurity.preventnickchginchan);
	CheckDlgBox(hCurr,IDC_CKCSQUITSASPARTS,is->isSecurity.showquitsasparts);
	SetDlgItemInt(hCurr,IDC_EDITCSMAXACCESS,is->isSecurity.max_access,FALSE);

	if (is->isSecurity.spoofflag)
	{
		unsigned char flag[] =
		{
			SPOOF_FLAG23,
			SPOOF_FLAG80,
			SPOOF_FLAG1080,
			SPOOF_FLAG3182,
			SPOOF_FLAG8080
		};
		unsigned int ckbox[] =
		{
			IDC_CKSPOOF23,
			IDC_CKSPOOF80,
			IDC_CKSPOOF1080,
			IDC_CKSPOOF3182,
			IDC_CKSPOOF8080,
			0
		};
		int count;

		SendDlgItemMessage(hCurr,IDC_CKSPOOF,BM_CLICK,0,0);

		for (count = 0; ckbox[count]; count++)
		/* Check corresponding ports to scan */
		{
			if (is->isSecurity.spoofflag & flag[count])
				CheckDlgBox(hCurr,ckbox[count],TRUE);
			else
				CheckDlgBox(hCurr,ckbox[count],FALSE);
		}

		SendDlgItemMessage(hCurr,IDC_CBSPOOFPUNISHMENT,CB_SETCURSEL,is->isSecurity.spoofconsequence,0);
	}

	if (is->isFilter.enabled)
		SendDlgItemMessage(tt[10].hwnd,IDC_CKFILTER,BM_CLICK,0,0);

	SetDlgItemInt(hCurr,IDC_EDITMAXNICKLEN,is->isSecurity.max_access,FALSE);
	SetDlgItemInt(hCurr,IDC_EDITMAXCHANLEN,is->isSecurity.max_chanlen,FALSE);
	SetDlgItemInt(hCurr,IDC_EDITMAXTOPICLEN,is->isSecurity.max_topiclen,FALSE);
	SetDlgItemInt(hCurr,IDC_EDITMAXMSGLEN,is->isSecurity.max_msglen,FALSE);
	SetDlgItemInt(hCurr,IDC_EDITRAPORT,is->isSecurity.RAPort,FALSE);

	/* Done with all normal stuff, setup lists */
	accthead.next = is->isAccounts.accthead.next;
	banhead.next = is->isBans.banhead.next;
	filterhead.next = is->isFilter.filterhead.next;
	RCLHead.next = is->isChannel.RCLHead.next;
	serverhead.next = is->isServers.serverhead.next;
	spoofedhead.next = is->isSecurity.spoofedhead.next;

	/* Now, list lists */
	Account_List(&accthead,GetSubItemHandle(IDC_LVACCTS),0);
	Banlist_List(&banhead,GetSubItemHandle(IDC_LVBANS),FALSE);
	Banlist_List(&banhead,GetSubItemHandle(IDC_LVEXCEPTIONS),TRUE);
	RCL_List(&RCLHead,GetSubItemHandle(IDC_LVREGCHAN));
	Server_List(&serverhead,GetSubItemHandle(IDC_LVINBOUND));
	Filter_List(&filterhead,tt[10].hwnd,FILTER_TYPE_ALL);
}

void SaveRAInfo(INFOSTRUCTSETTINGS *is)
/*
** SaveRAInfo()
** Saves information from RA dialog to the specified settings structure
*/
{
	char buf[256];
	HWND hCurr;
	GETTEXTLENGTHEX gtl;
	GETTEXTEX		gt;
	int nTemp;
	int count;

	memset(is,0,sizeof(*is));

	/* Status window items */
	hCurr = tt[0].hwnd;

	/* User count */
	is->isStatus.localmax = GetDlgItemInt(hCurr,IDC_EDITLOCALUSERSMAX,NULL,FALSE);
	is->isStatus.globalmax = GetDlgItemInt(hCurr,IDC_EDITGLOBALUSERSMAX,NULL,FALSE);

	/* General information */
	hCurr = hTabGeneral[0];

	/* Server */
    GetDlgItemText(hCurr,IDC_EDITNETWORKNAME,is->isGeneral.networkname,sizeof(is->isGeneral.networkname));
	GetDlgItemText(hCurr,IDC_EDITSERVERNAME,is->isGeneral.servername,sizeof(is->isGeneral.servername));
	is->isGeneral.serverid = GetDlgItemInt(hCurr,IDC_EDITSERVERID,NULL,FALSE);
	GetDlgItemText(hCurr,IDC_EDITSERVERDESC,is->isGeneral.serverdescription,sizeof(is->isGeneral.serverdescription));
    
	/* Administrative information */
	GetDlgItemText(hCurr,IDC_EDITLOC1,is->isGeneral.adminloc1,sizeof(is->isGeneral.adminloc1));
	GetDlgItemText(hCurr,IDC_EDITLOC2,is->isGeneral.adminloc2,sizeof(is->isGeneral.adminloc2));
	GetDlgItemText(hCurr,IDC_EDITADMINEMAIL,is->isGeneral.adminemail,sizeof(is->isGeneral.adminemail));

	/* Bindings */
	nTemp = (int)SendDlgItemMessage(hCurr,IDC_LISTBINDINGS,LB_GETCOUNT,0,0);

	if (nTemp)
	/* There are bindings to add */
	{
		unsigned short port;
		char ip[32];

		for (count = 0; count < nTemp; count++)
		{
			SendDlgItemMessage(hCurr,IDC_LISTBINDINGS,LB_GETTEXT,count,(LPARAM)buf);

			/* Copy all chars up to the ':' into IP buffer, and copy port */
			strcpy(ip,buf);
			(strchr(ip,':'))[0] = 0;
			strcpy(buf,strchr(buf,':'));
			port = (unsigned short)atoi(&buf[1]);

			BindingBuf_Add(&is->isGeneral.bindingbuf,&is->isGeneral.bindingnum,ip,port);
		}
	}

	/* DNS/IP masking */

	if (BoxChecked(hCurr,IDC_CKUSEDNS))
	/* Get DNS info */
	{
		is->isGeneral.dns_lookup = TRUE;

		if (is->isGeneral.dns_cache = BoxChecked(hCurr,IDC_CKCACHEDNS))
		/* Cache lookups? */
		{
			is->isGeneral.dns_cachetime = GetDlgItemInt(hCurr,IDC_EDITCACHEDNS,NULL,FALSE);
			is->isGeneral.dns_cachetimeduration = (unsigned char)SendDlgItemMessage(hCurr,IDC_CBCACHEDNS,CB_GETCURSEL,0,0);
		}
	}

	if (BoxChecked(hCurr,IDC_CKMASK))
	/* IP/DNS masking enabled? */
	{
		if (BoxChecked(hCurr,IDC_RDMASK1))
			is->isGeneral.dns_masktype = 1;
		else if (BoxChecked(hCurr,IDC_RDMASK2))
			is->isGeneral.dns_masktype = 2;
	}

	/* MOTD tab */
	hCurr = hTabGeneral[1];

	/* Get length of MOTD in buffer */
	memset(&gtl,0,sizeof(gtl));

	gtl.flags = GTL_CLOSE | GTL_USECRLF | GTL_NUMCHARS;

	nTemp = (int)SendDlgItemMessage(hCurr,IDC_EDITMOTD,EM_GETTEXTLENGTHEX,(WPARAM)&gtl,0);

	if (nTemp)
	/* MOTD buffer was not empty */
	{
		TCHAR *szBuf = (TCHAR*)calloc(sizeof(TCHAR),nTemp + 1024), szTemp[256];
		CHARRANGE crr;
		CHARFORMAT2 cb2;
		int nTemp2, nMotdIndex = 0, nColorPos = 0, nBoldPos = 0, nUnderlinePos = 0;
		COLORREF crPrevColor = 0;
		BOOL bLastSpace = FALSE;

		/* Get MOTD text */
		gt.cb = 2;
		gt.flags = GT_SELECTION;
		gt.codepage	= CP_ACP;
		gt.lpDefaultChar = NULL;
		gt.lpUsedDefChar = NULL;

		cb2.cbSize = sizeof(CHARFORMAT2);
		cb2.dwMask = CFM_COLOR | CFM_BOLD | CFM_UNDERLINE;

		/* */
		for (count = 0; count < nTemp; count++)
		{
			crr.cpMin = count;
			crr.cpMax = count + 1;
			SendDlgItemMessage(hCurr,IDC_EDITMOTD,EM_EXSETSEL,0,(LPARAM)&crr);
			SendDlgItemMessage(hCurr,IDC_EDITMOTD,EM_GETTEXTEX,(WPARAM)&gt,(LPARAM)szTemp);
			SendDlgItemMessage(hCurr,IDC_EDITMOTD,EM_GETCHARFORMAT,(WPARAM)SCF_SELECTION,(LPARAM)&cb2);

			if (szTemp[0] == 13)
			/* CR encountered */
			{
				nBoldPos = 0;
				nUnderlinePos = 0;
				nColorPos = 0;

				/* Add CRLF */
				szBuf[nMotdIndex++] = 13;
				szBuf[nMotdIndex++] = 10;

				bLastSpace = FALSE;
			}
			else
			{
				if (cb2.dwEffects & CFE_BOLD)
				/* Character is bold */
				{
					if (nBoldPos == 0)
					/* ASCII character 2 for bold */
					{
						szBuf[nMotdIndex++] = 2;
						nBoldPos = count + 1;
					}
				}
				else
				/* Character is not bold */
				{
					if (nBoldPos != 0)
					/* End of bold string */
					{
						szBuf[nMotdIndex++] = 2;
						nBoldPos = 0;
					}
				}
				if (cb2.dwEffects & CFE_UNDERLINE)
				/* Character is underlined */
				{
					if (nUnderlinePos == 0)
					/* ASCII character 31 for underline */
					{
						szBuf[nMotdIndex++] = 31;
						nUnderlinePos = count + 1;
					}
				}
				else
				/* Character is not underlined */
				{
					if (nUnderlinePos != 0)
					/* End of underline string */
					{
						szBuf[nMotdIndex++] = 31;
						nUnderlinePos = 0;
					}
				}
				if (cb2.crTextColor)
				/* ASCII character 3 for bold */
				{
					if (cb2.crTextColor != crPrevColor || nColorPos == 0)
					/* Different color, mark position */
					{
						nTemp2 = IRCColorFromRGB(cb2.crTextColor);

						szBuf[nMotdIndex++] = 3;
						sprintf(&szBuf[nMotdIndex],"%2.2d",nTemp2);

						nMotdIndex = nMotdIndex + 2;

						nColorPos = count + 1;
						crPrevColor = cb2.crTextColor;
					}
				}
				else
				{
					if (nColorPos != 0)
					/* Take away color */
					{
						szBuf[nMotdIndex++] = 3;
					}
				}

				if (szTemp[0] == 32)
				/* Fill in alt 160 ( ) where more than one space appears */
				{
					if (bLastSpace == TRUE)
						szBuf[nMotdIndex++] = 160;
					else
					{
						szBuf[nMotdIndex++] = szTemp[0];
						bLastSpace = TRUE;
					}
				}
				else
				{
					szBuf[nMotdIndex++] = szTemp[0];
					bLastSpace = FALSE;
				}

				
			}
		}

		is->isGeneral.motdbuffer = (char*)realloc(is->isGeneral.motdbuffer,strlen(szBuf) + 1);
		strcpy(is->isGeneral.motdbuffer,szBuf);

		free(szBuf);
	}

	/* Channel settings */
	hCurr = tt[6].hwnd;
    
	if (hCurr)
	/* Get default modes */
	{
		unsigned char modeflag[] = {
			ZRA_CHANMODE_MODERATED,
			ZRA_CHANMODE_NOEXTERN,
			ZRA_CHANMODE_ONLYOPSCHANGETOPIC,
			ZRA_CHANMODE_KNOCK,
			ZRA_CHANMODE_NOWHISPERS,
			0
		};
		int modeckbox[] = { 
			IDC_CKCDMMODERATED,
			IDC_CKCDMNOEXTERN,
			IDC_CKCDMONLYOPSSETTOPIC,
			IDC_CKCDMKNOCK,
			IDC_CKCDMNOWHISPERS,
			0
		};
		int count;

		for (count = 0; modeflag[count]; count++)
		/* Check appropriate boxes */
		{
			if (BoxChecked(hCurr,modeckbox[count]))
			/* Box is checked */
				is->isChannel.modeflag |= modeflag[count];
		}
	}

	if (BoxChecked(hCurr,IDC_CKCDMLIMIT))
	/* Default limit? */
		is->isChannel.defaultlimit = GetDlgItemInt(hCurr,IDC_EDITCDMLIMIT,NULL,FALSE);

	/* User settings */
	hCurr = tt[7].hwnd;

	if (BoxChecked(hCurr,IDC_CKCCCREATEDYN))
		is->isUser.dyn_create = TRUE;
	if (BoxChecked(hCurr,IDC_CKCCJOINDYN))
		is->isUser.dyn_join = TRUE;

	if (BoxChecked(hCurr,IDC_CKCCAUTOJOIN))
	{
		char abuffer[1024];
		int len = 0, count, total = (int)SendDlgItemMessage(hCurr,IDC_LISTCCAUTOJOIN,LB_GETCOUNT,0,0);

		abuffer[0] = 0;

		for (count = 0; count < total; count++)
		{
			SendDlgItemMessage(hCurr,IDC_LISTCCAUTOJOIN,LB_GETTEXT,count,(LPARAM)abuffer);
			strcat(abuffer,",");
			len += (int)strlen(abuffer);

			is->isUser.autojoinbuffer = (char*)realloc(is->isUser.autojoinbuffer,len + 1);

			if (!count)
			/* First string, add a '0' at beginning */
				is->isUser.autojoinbuffer[0] = 0;

			strcat(is->isUser.autojoinbuffer,abuffer);
		}

		if (abuffer[0])
			is->isUser.autojoinbuffer[len] = is->isUser.autojoinbuffer[len - 1] = 0;
	}

	is->isUser.maxchannels = GetDlgItemInt(hCurr,IDC_EDITCCMAXCHANS,NULL,FALSE);
	is->isUser.max_perip = GetDlgItemInt(hCurr,IDC_EDITUMUSERSPERIP,NULL,FALSE);
	is->isUser.max_recvq = GetDlgItemInt(hCurr,IDC_EDITUMRECVQ,NULL,FALSE);
	is->isUser.max_sendq = GetDlgItemInt(hCurr,IDC_EDITUMSENDQ,NULL,FALSE);
	is->isUser.ping_duration = GetDlgItemInt(hCurr,IDC_EDITPINGDURATION,NULL,FALSE);
	is->isUser.ping_response = GetDlgItemInt(hCurr,IDC_EDITPINGRESPONSE,NULL,FALSE);

	if (BoxChecked(hCurr,IDC_CKNICKDELAY))
		is->isUser.nickdelay = GetDlgItemInt(hCurr,IDC_EDITNICKDELAY,NULL,FALSE);

	if (BoxChecked(hCurr,IDC_CKMSGDELAY))
		is->isUser.msgdelay = GetDlgItemInt(hCurr,IDC_EDITMSGDELAY,NULL,FALSE);

	/* Server settings */
	is->isServers.networktype = ComboBox_GetCurSel(GetSubItemHandle(IDC_CBNETWORKTYPE)) + 1;

	/* Security settings */
	hCurr = tt[9].hwnd;

	is->isSecurity.preventnickchginchan = BoxChecked(hCurr,IDC_CKCSNICKINCHAN);
	is->isSecurity.showquitsasparts = BoxChecked(hCurr,IDC_CKCSQUITSASPARTS);

	is->isSecurity.max_access = GetDlgItemInt(hCurr,IDC_EDITCSMAXACCESS,NULL,FALSE);

	if (BoxChecked(hCurr,IDC_CKSPOOF))
	{
		unsigned char flag[] =
		{
			SPOOF_FLAG23,
			SPOOF_FLAG80,
			SPOOF_FLAG1080,
			SPOOF_FLAG3182,
			SPOOF_FLAG8080
		};
		unsigned int ckbox[] =
		{
			IDC_CKSPOOF23,
			IDC_CKSPOOF80,
			IDC_CKSPOOF1080,
			IDC_CKSPOOF3182,
			IDC_CKSPOOF8080,
			0
		};
		int count;

		for (count = 0; ckbox[count]; count++)
		/* Check corresponding ports to scan */
		{
			if (BoxChecked(hCurr,ckbox[count]))
				is->isSecurity.spoofflag |= flag[count];
		}

		is->isSecurity.spoofconsequence = (unsigned char)SendDlgItemMessage(hCurr,IDC_CBSPOOFPUNISHMENT,CB_GETCURSEL,0,0);
	}

	is->isSecurity.max_nicklen = GetDlgItemInt(hCurr,IDC_EDITMAXNICKLEN,NULL,FALSE);
	is->isSecurity.max_chanlen = GetDlgItemInt(hCurr,IDC_EDITMAXCHANLEN,NULL,FALSE);
	is->isSecurity.max_topiclen = GetDlgItemInt(hCurr,IDC_EDITMAXTOPICLEN,NULL ,FALSE);
	is->isSecurity.max_msglen = GetDlgItemInt(hCurr,IDC_EDITMAXMSGLEN,NULL,FALSE);
	is->isSecurity.RAPort = GetDlgItemInt(hCurr,IDC_EDITRAPORT,NULL,FALSE);
	strcpy(is->isSecurity.RAPassword,RAPassword);

	/* Filtering enabled? */
	hCurr = tt[10].hwnd;
	is->isFilter.enabled = BoxChecked(hCurr,IDC_CKFILTER);

	/* Done with all normal stuff, setup lists & list buffers */
	is->isAccounts.accthead.next = accthead.next;
	IS_CreateBuffer(&is->isAccounts.accthead,&is->isAccounts.acctbuffer,BUFFER_TYPE_ACCOUNT);
	is->isBans.banhead.next = banhead.next;
	IS_CreateBuffer(&is->isBans.banhead,&is->isBans.banbuffer,BUFFER_TYPE_BAN);
	is->isFilter.filterhead.next = filterhead.next;
	IS_CreateBuffer(&is->isFilter.filterhead,&is->isFilter.filterbuffer,BUFFER_TYPE_FILTER);
	is->isChannel.RCLHead.next = RCLHead.next;
	IS_CreateBuffer(&is->isChannel.RCLHead,&is->isChannel.RCLbuffer,BUFFER_TYPE_RCL);
	is->isServers.serverhead.next = serverhead.next;
	IS_CreateBuffer(&is->isServers.serverhead,&is->isServers.serverbuffer,BUFFER_TYPE_SERVER);
	is->isSecurity.spoofedhead.next = spoofedhead.next;
	IS_CreateBuffer(&is->isSecurity.spoofedhead,&is->isSecurity.spoofedbuffer,BUFFER_TYPE_SPOOFED);
}

void Account_List(AccountList *accthead, HWND hListView, unsigned char level)
/*
** Account_List()
** Lists all accounts for specified level
*/
{
	AccountList* curr = accthead;
	int count = 0;

	ListView_DeleteAllItems(hListView);

	while (curr->next != NULL)
	{
		curr = curr->next;

		if (curr->level == level)
		/* Matching level, add entry to list */
		{
			LV_ITEM lvitem;

			lvitem.mask = LVIF_TEXT | LVIF_PARAM;
			lvitem.pszText = LPSTR_TEXTCALLBACK;
			lvitem.iItem = ++count;
			lvitem.iSubItem = 0;
			lvitem.lParam = (LPARAM)curr;

			ListView_InsertItem(hListView,&lvitem);
		}
	}
}

void Filter_List(FilterList *filterhead, HWND hParent, unsigned int type)
/*
** Filter_List()
** Populates filter list box(es) with all matching items
*/
{
	FilterList* curr = filterhead;

	HWND hListBox[] = { GetDlgItem(hParent,IDC_LISTFILTERNICK), GetDlgItem(hParent,IDC_LISTFILTERCHAN), GetDlgItem(hParent,IDC_LISTFILTERTOPIC) };
	unsigned int flag[] = { FILTER_TYPE_NICKNAME, FILTER_TYPE_CHANNEL, FILTER_TYPE_TOPIC };
	int count;

	for (count = 0; count < 3; count++)
	/* Clear matching list boxes */
	{
		if (type & flag[count])
			SendMessage(hListBox[count],LB_RESETCONTENT,0,0);
	}

	while (curr->next != NULL)
	{
		curr = curr->next;

		for (count = 0; count < 3; count++)
		{
			if (curr->type & flag[count])
			/* List item */
			{
				SendMessage(hListBox[count],LB_ADDSTRING,0,(LPARAM)curr->word);
			}
		}
	}
}

void Banlist_List(BanList* banhead, HWND hListView, BOOL bException)
/*
** BanList_List()
** Lists contents of ban/exception lists into corresponding list view boxes
*/
{
	BanList* curr = banhead;
	int count = 0;
	LV_ITEM lvitem;

	ListView_DeleteAllItems(hListView);

	while (curr->next != NULL)
	{
		curr = curr->next;

		if (curr->bException == bException)
		/* Add entry to list */
		{

			if (!bException)
			{
				lvitem.mask = LVIF_TEXT | LVIF_PARAM;
				lvitem.pszText = LPSTR_TEXTCALLBACK;
				lvitem.iItem = count++;
				lvitem.iSubItem = 0;
				lvitem.lParam = (LPARAM)curr;
			}
			else
			{
				lvitem.mask = LVIF_TEXT;
				lvitem.pszText = curr->hostmask;
				lvitem.iItem = count++;
				lvitem.iSubItem = 0;

			}

			ListView_InsertItem(hListView,&lvitem);
		}
	}
}

void RCL_List(RegChanList *RCLHead, HWND hListView)
/*
** RCL_List()
** Lists all entries for the registered channel list
*/
{
	RegChanList* curr = RCLHead;
	int count = 0;
	LV_ITEM lvitem;

	ListView_DeleteAllItems(hListView);

	while (curr->next != NULL)
	{
		curr = curr->next;

		lvitem.mask = LVIF_TEXT | LVIF_PARAM;
		lvitem.pszText = LPSTR_TEXTCALLBACK;
		lvitem.iItem = count++;
		lvitem.iSubItem = 0;
		lvitem.lParam = (LPARAM)curr;
		
		ListView_InsertItem(hListView,&lvitem);
	}
}


void RCL_ListAccess(HWND hListView, RegChanList* RCLTarget)
/*
** RCL_ListAccess()
** Lists all access entries for target registered channel entry in specified list box
*/
{
	AccessList* curr = &RCLTarget->accesshead;
	int count = 0;
	LV_ITEM lvitem;

	ListView_DeleteAllItems(hListView);

	while (curr->next != NULL)
	{
		curr = curr->next;

		lvitem.mask = LVIF_TEXT | LVIF_PARAM;
		lvitem.pszText = LPSTR_TEXTCALLBACK;
		lvitem.iItem = count++;
		lvitem.iSubItem = 0;
		lvitem.lParam = (LPARAM)curr;
		
		ListView_InsertItem(hListView,&lvitem);
	}
}



void Server_List(ServerList *serverhead, HWND hListView)
/*
** Server_List()
** Lists specified server types in target list view control
*/
{
	ServerList* curr = serverhead;
	int count = 0;
	LV_ITEM lvitem;

	ListView_DeleteAllItems(hListView);

	while (curr->next != NULL)
	{
		curr = curr->next;

		lvitem.mask = LVIF_TEXT | LVIF_PARAM;
		lvitem.pszText = LPSTR_TEXTCALLBACK;
		lvitem.iItem = count++;
		lvitem.iSubItem = 0;
		lvitem.lParam = (LPARAM)curr;
		
		ListView_InsertItem(hListView,&lvitem);
	}

	/* Set combobox at top while we're at it */
	ComboBox_SetCurSel(GetSubItemHandle(IDC_CBNETWORKTYPE),SettingsInfo.isServers.networktype - 1);
}

void Spoofed_List(SpoofedList* spoofedhead, HWND hListView)
/*
** Spoofed_List()
** Goes through list of spoofed hosts and puts them all in the list view control specified
*/
{
	SpoofedList* curr = spoofedhead;
	int count = 0;
	LV_ITEM lvitem;

	ListView_DeleteAllItems(hListView);

	while (curr->next != NULL)
	{
		curr = curr->next;

		lvitem.mask = LVIF_TEXT | LVIF_PARAM;
		lvitem.pszText = LPSTR_TEXTCALLBACK;
		lvitem.iItem = count++;
		lvitem.iSubItem = 0;
		lvitem.lParam = (LPARAM)curr;
		
		ListView_InsertItem(hListView,&lvitem);
	}
}

void SpoofedLogin_List(SpoofedList *sTarget, HWND hListBox)
/*
** SpoofedLogin_List()
** Goes through list of login attempts for specified spoofed host
*/
{
	SpoofedLoginLog *curr = &sTarget->loginhead;
	char buf[32];
	time_t tm;

	while (curr->next)
	{
		curr = curr->next;

		tm = curr->time;
		ConvertTime(ctime(&tm),buf);

		SendMessage(hListBox,LB_ADDSTRING,(WPARAM)curr->index,(LPARAM)buf);
	}
}

void ClearLinkedLists()
/*
** ClearLinkedLists()
** This function does exactly what it says, clears all linked lists for program
*/
{
	while (accthead.next)
		Account_Delete(&accthead,accthead.next);

	while (banhead.next)
		Banlist_Delete(&banhead,banhead.next);

	while (RCLHead.next)
		RCL_Delete(&RCLHead,RCLHead.next);

	while (serverhead.next)
		Server_Delete(&serverhead,serverhead.next);

	while (filterhead.next)
		Filter_Delete(&filterhead,filterhead.next->word,filterhead.next->type);
}

void ToggleRefresh(BOOL bEnable)
/*
** DisableRefresh()
** This function goes through and enables/disables all refresh buttons
*/
{
	int count;

	for (count = 0; RefreshButton[count]; count++)
		EnableWindow(GetSubItemHandle(RefreshButton[count]),bEnable);
}

int	TechLog_AddEntry(char* text, unsigned char icon)
/*
** TechLog_AddEntry()
** Adds an entry to the technical status log, the drawing is handled in the dlgproc
*/
{
	HWND hListBox = GetSubItemHandle(IDC_LISTREPORT);
	int item;
	long len = (long)strlen(text);

	item = (int)SendMessage(hListBox,LB_ADDSTRING,0,(LPARAM)text);
	SendMessage(hListBox,LB_SETITEMDATA,item,(LPARAM)icon);

	/* Store max string length in listbox user data */
	if (len > GetWindowLong(hListBox,GWL_USERDATA))
		SetWindowLong(hListBox,GWL_USERDATA,len);

	SendMessage(hListBox,LB_SETHORIZONTALEXTENT,35 + (5 * GetWindowLong(hListBox,GWL_USERDATA)),0);
	SendMessage(hListBox,LB_SETCURSEL,SendMessage(hListBox,LB_GETCOUNT,0,0) - 1,0);

	return 0;
}

int	IRCColorFromRGB(COLORREF crRGB)
/*
** IRCColorFromRGB()
** Converts the RGB color into the corresponding IRC color
** Returns -1 if the color is invalid
*/
{
	int nCount;

	for (nCount = 0; MOTDArray[nCount].szDisplayStr[0]; nCount++)
	{
		if (MOTDArray[nCount].crRGB == crRGB)
			return nCount;
	}

	return -1;
}