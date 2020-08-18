/*
** functions.cpp
** Contains all generic-use functions for RockIRCX
*/

#include "functions.h"

static char tokenbuf[4096];

int SetNonBlocking(SOCKET s)
/*
** SetNonBlocking()
** This function sets a socket as non-blocking, will return 0 if successful
*/
{
	unsigned long DontBlock = 1;

	return ioctlsocket(s,FIONBIO,&DontBlock);
}

void ConvertTime(char* ctime, char* targetbuf)
/*
** ConvertTime()
** Takes a time format returned by ctime() and converts it to MM/DD/YY HH:MM:SS format
*/
{
	// Wed Jan 02 02:03:55 1980


	int count;
	int hour;
	char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	char buf[32], mm[4], dd[4], yy[4], hh[4], nn[4], ss[4];
	BOOL bPM = FALSE;

	memcpy(buf,&ctime[4],3);
	buf[3] = 0;

	for (count = 0; count < 12; count++)
	/* Add month to buffer */
	{
		if (strcmp(buf,months[count]) == 0)
		/* Found match */
		{
			sprintf(mm,"%02d",count + 1);
		}
	}
    
	memcpy(buf,&ctime[8],2);
	buf[2] = 0;
	sprintf(dd,"%02d",atoi(buf));

	memcpy(buf,&ctime[11],2);
	hour = atoi(buf);
	if (hour > 12)
	{
		hour -= 12;
		bPM = TRUE;
	}
	sprintf(hh,"%d",hour);

	memcpy(buf,&ctime[14],2);
	sprintf(nn,"%02d",atoi(buf));

	memcpy(buf,&ctime[17],2);
	sprintf(ss,"%02d",atoi(buf));

	memcpy(buf,&ctime[22],2);
	strcpy(yy,buf);

	sprintf(targetbuf,"%s-%s-%s %s:%s:%s%s",mm,dd,yy,hh,nn,ss,bPM == TRUE ? "PM" : "AM");
}

void GetErrorString(char *errbuffer, unsigned int error)
/*
** GetErrorString()
** Retreives the system error string and fills the specified buffer
*/
{
	LPVOID lpMsgBuf;
	int nCount;

	if (error >= WSAEINTR && error <= WSANO_DATA)
	/* Winsock error */
	{
		for (nCount = 0; nCount < NUMERRORTABLEENTRIES; nCount++)
		{
			if (error == error_table[nCount].nErrorCode)
			{
				strcpy(errbuffer,error_table[nCount].pszShortDescription);
				break;
			}
		}
	}
	else
	{
		if (!FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,
			0,
			NULL ))

			strcpy(errbuffer,"Unknown Error.");

		else
		{
			/* Copy buffer & take out CRLF */
			strcpy(errbuffer,((LPCTSTR)lpMsgBuf));
			(strstr(errbuffer,"\r\n"))[0] = 0;

			// Free the buffer.
			LocalFree( lpMsgBuf );
		}
	}
}

int index(const char *buffer, char value, int indexnbr)
/*
** index()
** Returns the position of the specified character, indexnbr being the target occurance
** Returns -1 if indexnbr > # of occurances
** NOTE: if indexnbr == 0, will return the total # of occurances
*/
{
	int count, retval = 0;

	for (count = 0; buffer[count]; count++)
	/* Search through string */
	{
		if (buffer[count] == value)
		{
			retval++;
			
			if (retval == indexnbr)
				break;
		}
	}

	return retval;
}

void ToggleClose(HWND hwnd, BOOL bEnable)
/*
** ToggleClose()
** This function will enable/disable the "Close" button for the specified window
*/
{
	HMENU hSysMenu = GetSystemMenu(hwnd,FALSE);

	if (!bEnable)
		EnableMenuItem(hSysMenu,SC_CLOSE,MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
	else
		EnableMenuItem(hSysMenu,SC_CLOSE,MF_BYCOMMAND | MF_ENABLED);

}

char *tokenrange(char *targetbuf, const char *str, int token, int numtoks, int seperator)
/*
** tokenrange()
** This function will get several tokens from a string
** Returns: Pointer to buffer containing tokens
*/
{
	BOOL bDone = FALSE;
	int ntok = numtok(str,seperator), count = 0;
	char buf[MAXSTR], sep[2] = { (char)seperator, 0 };

	targetbuf[0] = 0;

	if ((token + numtoks) > ntok)
	{
		numtoks = ntok - token + 1;
	}

	while (!bDone)
	{
		/* Add token to buffer */
        strcat(targetbuf,gettok(buf,str,token + count,seperator));
		count++;

		if (!numtoks && count == (ntok - token + 1))
			bDone = TRUE;
		else if (numtoks && count >= numtoks)
			bDone = TRUE;
		else
			strcat(targetbuf,sep);

	}

	return targetbuf;
}
char *gettok(char *targetbuf, const char* str, int token, int seperator)
/*
** gettok()
** This function will find a specific token in a string
** Returns: Pointer to buffer containing token
*/
{
	int count = 0;
	char sep[2] = { (char)seperator, 0 };
	//char *copy = (char*)(malloc(strlen(str) + 1));
	char *copy = tokenbuf;
	char *result = targetbuf;
	char *p, *strptr = (char*)str;

	/* Create copy of string to be modified by strtok() */
	strcpy(copy,str);

	p = strtok(copy,sep);
	//p = strtok(strptr,sep);

	while (p != NULL)
	{
		count++;

		if (count == token)
		/* Found token */
		{
			strcpy(result,p);
			//free(copy);
			return result;
		}
		else
		/* Keep on searchin' */
		{
			p = strtok(NULL,sep);
		}
	}

	//free(copy);
	return p;
}

int numtok(const char* str,int seperator)
/* 
This function will return the number of tokens in a string
*/
{

	int tFirst = 0, 
		tLast = 0, 
		tFound = 0, 
		tIndex = 0, 
		length,
		ch;

	length = (int)strlen(str);

	for (tFirst = 0; tFirst <= length; tFirst++)
	{
		ch = str[tFirst];

		if (ch != seperator)
		{
			break;
		}
	}

	if (tFirst == length)
	{
		return 1;
	}
	else
	{
		tFound++;

		for (tIndex = tFirst; tIndex <= length; tIndex++)
		{
			ch = str[tIndex];

			if (ch == seperator && tLast != seperator && tIndex != (length - 1))
			{
				tFound++;
			}

			tLast = ch;
		}
	}

	return tFound;
}

BOOL Valid_Username(char *username, BOOL bMakeValid)
/*
** Valid_Ident()
** Checks to see if the specified NULL-terminated ident is correct, if not then
** it can optionally be made correct by the bMakeValid flag.
** Returns: TRUE if ident(or newly corrected ident) is OK, otherwise FALSE
*/
{
	int count;

	for (count = 0; username[count]; count++)
	/* Find those bad boys */
	{
		if (username[count] < 32)
		/* Bad char */
		{
			if (bMakeValid)
				username[count] = '_';
			else
				return FALSE;
		}
	}

	return TRUE;
}

BOOL Valid_Nickname(const char *nickname)
/*
** Valid_Nickname()
** Checks to see if the specified NULL-terminated nickname is valid, and returns TRUE if it is
*/
{
	int count;
	char ch;

	for (count = 0; nickname[count]; count++)
	/* Find those bad char's */
	{
		ch = nickname[count];

		if ((ch != '-' && ch < 48) || ch >= 127 || (ch > 57 && ch <= 64) ||
			(count == 0 && ch <= 64))
		/*
		** Nicknames may not contain the following:
		** 1) Any character less then ascii code 48(Except '-')
		** 2) Any character greater then or equal to ascii code 127
		** 3) Any character greater then ascii code 57 and less then or equal to 64
		** 4) A starting character less then ascii code 64
		*/

			return FALSE;
	}

	return TRUE;
}

BOOL Valid_Channelname(const char *szChannelname)
/*
** Valid_Channelname()
** Checks if the specified NULL-terminated string is a valid channel name
*/
{
	
	
	if (szChannelname[0] == '&' || szChannelname[0] == '#' || (szChannelname[0] == '%' && szChannelname[1] == '#') || (szChannelname[0] == '%' && szChannelname[1] == '&'))
	/* Channel has a correct prefix */
	{
		if (strchr(szChannelname,','))
			return FALSE;

		return TRUE;
	}

	return FALSE;
}

void Validate_AccessHostmask(const char *szInput, char *szOutput)
/*
** Validate_AccessHostmask
** Takes the target string and makes sure it is access-friendly
** Specifics:
** String occuring before any special character from beginning will be the nickname
** String between a '!' and another seperator (or null) will be the username, counting only the right-most one
** String between a '@' and another seperator (or null) will be the hostmask
** String between a '$' and another seperator (or null) will be the server name
*/
{
	int nCount = 0, nOutputPos = 0, nStage = 1;
	char szOutputParts[4][64], *szString;
	int nLen = strlen(szInput);

	char ch;

	memset(&szOutputParts,0,sizeof(szOutputParts));

	/* Find the server name */
	szString = (char*)strrchr(szInput,'$');

	if (szString)
	{
		for (nCount = 1;; nCount++)
		{
			ch = szString[nCount];

			if (ch == '$' || ch == '!' || ch == '@' || ch == 0)
			{
				strncpy(szOutputParts[3],&szString[1],nCount - 1);
				break;
			}
		}
	}
	else
	{
		szOutputParts[3][0] = '*';
		szOutputParts[3][1] = 0;
	}

	/* Find the nickname */
	szString = (char*)szInput;

	if (szString[0] != '!' && szString[0] != '@' && szString[0] != '$')
	{
		for (nCount = 0;; nCount++)
		{
			ch = szString[nCount];

			if (ch == '$' || ch == '!' || ch == '@' || ch == 0)
			{
				strncpy(szOutputParts[0],&szString[0],nCount);
				break;
			}
		}
	}
	else
	{
		szOutputParts[0][0] = '*';
		szOutputParts[0][1] = 0;
	}

	/* Find the username */
	szString = (char*)strrchr(szInput,'!');

	if (szString)
	{
		for (nCount = 1;; nCount++)
		{
			ch = szString[nCount];

			if (ch == '$' || ch == '!' || ch == '@' || ch == 0)
			{
				strncpy(szOutputParts[1],&szString[1],nCount - 1);
				break;
			}
		}
	}
	else
	{
		szOutputParts[1][0] = '*';
		szOutputParts[1][1] = 0;
	}

	/* Find the hostmask */
	szString = (char*)strrchr(szInput,'@');

	if (szString)
	{
		for (nCount = 1;; nCount++)
		{
			ch = szString[nCount];

			if (ch == '$' || ch == '!' || ch == '@' || ch == 0)
			{
				strncpy(szOutputParts[2],&szString[1],nCount - 1);
				break;
			}
		}
	}
	else
	{
		szOutputParts[2][0] = '*';
		szOutputParts[2][1] = 0;
	}

	/* Replace anything NULL with an asterisk */
	for (nCount = 0; nCount < 4; nCount++)
	{
		if (!szOutputParts[nCount][0])
		{
			szOutputParts[nCount][0] = '*';
			szOutputParts[nCount][1] = 0;
		}
	}


	strcpy(szOutput,szOutputParts[0]);
	strcat(szOutput,"!");
	strcat(szOutput,szOutputParts[1]);
	strcat(szOutput,"@");
	strcat(szOutput,szOutputParts[2]);

	strcat(szOutput,"$");
	strcat(szOutput,szOutputParts[3]);
}

void ODS(const char *string, ...)
/*
** ODS()												(Output Debug String)
** Outputs a string to the debugger
*/
{
	va_list vArgs;
	char buf[1024];

	va_start(vArgs,string);
	vsprintf(buf,string,vArgs);
	va_end(vArgs);

	strcat(buf,"\r\n");

	/* Output string */
	OutputDebugString(buf);
}

char *IPFromLong(unsigned int ip)
/*
** IPFromLong()
** Converts a long int to a string IP address
*/
{
	struct in_addr in;

	in.s_addr = ip;

	return inet_ntoa(in);
}