#if (defined _MSC_VER) /*|| (defined _VMS_WINDOWS_)*/
#pragma warning(disable: 4996)
#define __WSA_CALLS
#define getpid	GetCurrentProcessId
#endif

#include "socketdef.h"

#define IPADDR_BROADCAST	"255.255.255.255"

char			gszOSType[40] = { "" };
char			gszParam[64] = { 0 };
static char		gszPort[32] = { 0 };
static bool		gfServerMode = false;
static bool		gfClientMode = false;
static char		gszDestination[15 + 1] = { "127.0.0.1" };
static char*	gpszMsg = 0;
static bool		gfShutdown = false;
static int		gnExitCode = 0;
static char		gszExitMessage[128] = { "" };
static int		gnPortNbr = 5080;
static int		gfCtrlC = 0;

typedef struct tagCLIENTSOCKADDR {
	int					hSocket;
	SOCKADDR_IN			sockaddr;
} CLIENTSOCKADDR;

////////////////////////////////////////////////////////////////////////////
//
void print_syntax(void)
{
	printf("\n"
		"%s\n"
		"syntax: udptest [-c|--client|-s|--server] [-p|--port Nbr] [-i|--ip ipaddr] [-m|--msg m]\n"
		"\n"
		"where\n"
		"\n"
		"s             start in server/listening mode\n"
		"c             start in client/sending mode\n"
		"p             udp port to use (default is 5080)\n"
		"i             ip address to send to - OR - 255.255.255.255 to broadcast\n"
		"m             message to send\n"
		"\n"
		, gszOSType
	);
}
#if (defined __vms) || (defined __LINUX__)	/* OpenVMS or Linux*/
////////////////////////////////////////////////////////////////////////////
// OpenVMS and Linux are missing these C-Runtime functions
int vms_stricmp(const char *psz1, const char *psz2)
{
	char	ch1, ch2;
	int		nDiff = 'a' - 'A';

	/* while both not null */
	while (*psz1 && *psz2)
	{
		ch1 = *psz1;
		ch2 = *psz2;
		if (ch1 >= 'a' && ch1 <= 'z')
			ch1 -= nDiff;
		if (ch2 >= 'a' && ch2 <= 'z')
			ch2 -= nDiff;
		if (ch1 != ch2)
			return (int)(ch1 - ch2);
		psz1++;
		psz2++;
	}
	if (!*psz1 && *psz2)
		return (int)-(*psz2); /* string2 is greater */

	if (*psz1 && !*psz2)
		return (int)(*psz1); /* string1 is greater */

	return 0; /* is equal */
}
int vms_strnicmp(const char *psz1, const char *psz2, size_t cMatch)
{
	char	ch1, ch2;
	size_t	n = 0;
	int		nDiff = 'a' - 'A';

	/* while both not null */
	while (n <= cMatch && *psz1 && *psz2)
	{
		ch1 = *psz1;
		ch2 = *psz2;
		if (ch1 >= 'a' && ch1 <= 'z')
			ch1 -= nDiff;
		if (ch2 >= 'a' && ch2 <= 'z')
			ch2 -= nDiff;
		if (ch1 != ch2)
			return (int)(ch1 - ch2);
		n++;
		psz1++;
		psz2++;
	}
	if (n < cMatch)
	{
		if (!*psz1 && *psz2)
			return (int)-(*psz2); /* string2 is greater */

		if (*psz1 && !*psz2)
			return (int)(*psz1); /* string1 is greater */
	}
	return 0; /* is equal */
}
char * vms_strlwr(char *pszBuf)
{
	char	*pch = pszBuf;
	int		nDiff = 'a' - 'A';

	if (!pszBuf)
		return pszBuf;

	while (*pch)
	{
		if (*pch >= 'A' && *pch <= 'Z')
			*pch += nDiff;

		pch++;
	}
	return pszBuf;
}
char *vms_strupr(char *pszBuf)
{
	char	*pch = pszBuf;
	int		nDiff = 'a' - 'A';

	if (!pszBuf)
		return pszBuf;

	while (*pch)
	{
		if (*pch >= 'a' && *pch <= 'z')
			*pch -= nDiff;

		pch++;
	}
	return pszBuf;
}
#endif	// OpenVMS or Linux
////////////////////////////////////////////////////////////////////////////
//
void StartUdpServer()
{
	SOCKADDR_IN		sockaddr, sockaddr2;
	socklen_t		nLen = 0;
	char			achBuffer[1024];
	int				nRcv;
	int	udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(gnPortNbr);
	if (!stricmp(gszDestination, IPADDR_BROADCAST))
	{
		int	fBroadcast = 1;
		setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, (char *)&fBroadcast, sizeof(int));
		sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else
	{
		sockaddr.sin_addr.s_addr = inet_addr(gszDestination);
	}
	memset(sockaddr.sin_zero, 0, sizeof(sockaddr.sin_zero));

	int n = bind(udpSocket, (const struct sockaddr *)&sockaddr, sizeof(SOCKADDR_IN));
	if (n == SOCKET_ERROR)
	{
		closesocket(udpSocket);
		printf("bind() error %d\n", n);
		return;
	}

	printf("listening to ip addr %s port %d\n", gszDestination, gnPortNbr);
	while (!gfShutdown)
	{
		nLen = sizeof(sockaddr2);
		nRcv = recvfrom(udpSocket, achBuffer, sizeof(achBuffer), 0, (SOCKADDR*)&sockaddr2, (socklen_t*)&nLen);
		if (nRcv > 0)
		{
			printf("from %s: %*.*s\n", inet_ntoa(sockaddr2.sin_addr), nRcv, nRcv, achBuffer);
		}
	}
	closesocket(udpSocket);
}
////////////////////////////////////////////////////////////////////////////
//
void SendUdpMessage(char *pszDestination, char *pszMessage)
{
	SOCKADDR_IN		sockaddr, sockaddr2;
	int	udpSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	sockaddr.sin_family = AF_INET;
	if (!stricmp(pszDestination, IPADDR_BROADCAST))
	{
		int	fBroadcast = 1;
		setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, (char *)&fBroadcast, sizeof(int));
		sockaddr.sin_addr.s_addr = inet_addr(gszDestination); // htonl (INADDR_BROADCAST);
	}
	else
	{
		sockaddr.sin_addr.s_addr = inet_addr(gszDestination);
	}
	sockaddr.sin_port = htons(gnPortNbr);
	memset(sockaddr.sin_zero, 0, sizeof(sockaddr.sin_zero));

	printf("sending to %s: %s\n", pszDestination, pszMessage);
	int nRet = sendto(udpSocket, (const char*)pszMessage, strlen(pszMessage), 0, (const struct sockaddr *)&sockaddr, SOCKADDR_LEN);

	closesocket(udpSocket);
}
/////////////////////////////////////////////////////////////////////
// 
int IsProcessShutdown(void)
{
	//printf( "IsProcessShutdown() = %d\n", (int)gfShutdown );
	if (gfShutdown)
		return 1;
	else return 0;
}
/////////////////////////////////////////////////////////////////////
// 
int SetProcessShutdownFlag(int nRc, const char *pszExitMessage)
{
	gfShutdown = true;
	gnExitCode = nRc;
	char *msg = (char*)"process exits";
	if (pszExitMessage)
		msg = (char*)pszExitMessage;
	sprintf(gszExitMessage, "pid=0x%0X, rc=%d: %s"
		, getpid()
		, nRc, msg);
	return nRc;
}
////////////////////////////////////////////////////////////////////////////
//
void signal_handler(int sig)
{
	printf("\nCtrl+C pressed - exiting (sig=%d)...\n", sig);
	SetProcessShutdownFlag(1, (const char*)"Ctrl+C pressed");
	SendUdpMessage( gszDestination, (char*)"Ctrl+C - time to exit");
}
////////////////////////////////////////////////////////////////////////////
//
int check_param(char *pszParam, char *pszValue)
{
	int	nRc = 1;

	if (*pszParam == '-' && *(pszParam+1) == '-')
		pszParam += 2;

	if (*pszParam == '-' || *pszParam == '/')
		pszParam++;

	if ((!stricmp(pszParam, "p") || !stricmp(pszParam, "port")) && pszValue)
	{
		gnPortNbr = atoi(pszValue);
	}
	if (!stricmp(pszParam, "c") || !stricmp(pszParam, "client"))
	{
		gfServerMode = false;
		gfClientMode = true;
	}
	if (!stricmp(pszParam, "s") || !stricmp(pszParam, "server"))
	{
		gfServerMode = true;
		gfClientMode = false;
	}
	if ((!stricmp(pszParam, "i") || !stricmp(pszParam, "ip") ) && pszValue)
		strcpy(gszDestination, pszValue);
	if ((!stricmp(pszParam, "msg") || !stricmp(pszParam, "m")) && pszValue)
		gpszMsg = pszValue;

	return nRc;
}
////////////////////////////////////////////////////////////////////////////
//
int global_init(int argc, char *argv[])
{
	int		n;
#ifdef _MSC_VER
	strcpy( gszOSType, "Windows" );
#elif (defined __vms) /* OpenVMS */
	strcpy(gszOSType, "OpenVMS" );
#elif (defined __APPLE__) /* Mac includes */
	strcpy(gszOSType, "Mac OS" );
#elif (defined __LINUX__) /* Linux/UNIX includes */
	strcpy(gszOSType, "Linux" );
#else
	strcpy(gszOSType, "Other OS" );
#endif

	if (argc >= 2 && (!strcmp(argv[1], "/?") || !strcmp(argv[1], "-?") || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help") ))
	{
		print_syntax();
		exit(0);
	}

	SetProcessShutdownFlag(0, 0);
	gfShutdown = false;

#ifdef __WSA_CALLS
	// initialize WinSocket under Windows
	WSADATA     wsaData;
	int nRc = WSAStartup(WS_VERSION_REQD, &wsaData);
#endif

	signal(SIGINT, signal_handler);

	for (n = 1; n < argc; n++)
	{
		if (n + 1 < argc)
		{
			if (*argv[n + 1] != '-')
			{
				if (check_param(argv[n], argv[n + 1]))
					n++;
			}
			else check_param(argv[n], 0);
		}
		else check_param(argv[n], 0);

		if (IsProcessShutdown())
			break;
	}

	return 0;
}
////////////////////////////////////////////////////////////////////////////
//
int global_exit(void)
{
#ifdef __WSA_CALLS
	WSACleanup();
#endif
	return 1;
}
////////////////////////////////////////////////////////////////////////////
//
int main(int argc, char *argv[])
{
	int		nRc = 0;

	global_init(argc, argv);

	if (!gfShutdown)
	{
		if (gfServerMode)
		{
			StartUdpServer();
		}
		else
		if (gfClientMode)
		{
			SendUdpMessage( gszDestination, gpszMsg );
		}
	}

	global_exit();

	exit(gnExitCode);
	return nRc;
}
