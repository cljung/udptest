#ifndef __INC_SOCKETDEF__
#define __INC_SOCKETDEF__
/*/////////////////////////////////////////////////////////////////////// */
/* Microsoft Visual C++ */
#ifdef _MSC_VER

#pragma message("socketdef.h - Windows/Intel compilation...")

#include <stdio.h>
#include <time.h>
#define WS_VERSION_REQD    0x0202
#include <winsock2.h>
#pragma comment(lib, "wsock32.lib")
#include <signal.h>
#include <tchar.h>

/* defs so we can code in UNIX/Linux style under Windows */
#define write(h,p,l)		send(h,p,l,0)
#define read(h,p,l)		recv(h,p,l,0)
#define close(h)		closesocket(h)
#define ioctl(h)		ioctlsocket(h)
#define socklen_t		int

/*/////////////////////////////////////////////////////////////////////// */
/* OpenVMS  */
#elif (defined __vms) /* OpenVMS */

#pragma message("socketdef.h - OpenVMS/Alpha compilation...")

#ifdef _VMS_WINDOWS_
#define WS_VERSION_REQD    0x0100
#include <vms_dcom.h>
#include <windows.h>
#include <windu_stdlib.h>
#ifdef __WINSOCK_
#include <winsock.h>
#endif
#else  // _VMS_WINDOWS
#include <stdlib.h>
#include <types.h>
#include <time.h>
/* defs so we can code in winsock style under Linux/UNIX */
//#define send(h,p,l,f)		write(h,p,l)
//#define recv(h,p,l)		read(h,p,l)
#endif // _VMS_WINDOWS

#include <stdio.h>
#include <descrip.h>        /* VMS descriptor stuff */
#include <in.h>             /* internet system Constants and structures. */
#include <inet.h>           /* Network address info. */
#include <ioctl.h>          /* I/O Ctrl DEFS */
#include <iodef.h>          /* I/O FUNCTION CODE DEFS */
#include <lib$routines.h>   /* LIB$ RTL-routine signatures. */
#include <netdb.h>          /* Network database library info. */
#include <signal.h>         /* UNIX style Signal Value Definitions */
#include <socket.h>         /* TCP/IP socket definitions. */
#include <ssdef.h>          /* SS$_<xyz> sys ser return stati <8-) */
#include <starlet.h>        /* Sys ser calls */
#include <string.h>         /* String handling function definitions */
#ifdef __VMS_TCPIP_INETDEF
#include <tcpip$inetdef.h>    /* TCPIP network definitions */
#endif
#include <unixio.h>         /* Prototypes for UNIX emulation functions */
#include <stropts.h>         /* ioctl() function */
//#include <signal.h>

#define closesocket(h)		close(h)
#define ioctlsocket(h,p,l)	ioctl(h,p,l)

#ifndef INVALID_SOCKET
#define INVALID_SOCKET		-1
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR		-1
#endif

#define SOCKET				int
//typedef struct sockaddr SOCKADDR;
//typedef struct sockaddr *PSOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr_in *PSOCKADDR_IN;

/* Functions not supported by the OpenVMS C runtime (strangly enough!) */
#define stricmp		vms_stricmp
#define strnicmp	vms_strnicmp
#define strlwr		vms_strlwr
#define strupr		vms_strupr

/*/////////////////////////////////////////////////////////////////////// */
/* Linux assumed   */
#else /* Linux/UNIX includes */

#pragma message("socketdef.h - Linux compilation...")

#ifndef __LINUX__
#define __LINUX__
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <pwd.h>
#include <signal.h>
#include <memory.h>
#include <ctype.h>
#include <netdb.h>

/* defs so we can code in winsock style under Linux/UNIX */
#define send(h,p,l,f)		write(h,p,l)
#define recv(h,p,l,f)		read(h,p,l)
#define closesocket(h)		close(h)
#define ioctlsocket(h,x,y)	ioctl(h,x,y)

#define INVALID_SOCKET		-1
#define SOCKET_ERROR		-1
#define SOCKET				int
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr *PSOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr_in *PSOCKADDR_IN;

#define stricmp		vms_stricmp
#define strnicmp	vms_strnicmp
#define strlwr		vms_strlwr
#define strupr		vms_strupr

#endif
/*/////////////////////////////////////////////////////////////////////// */

/* common */
#ifndef SOCKADDR_LEN 
#define SOCKADDR_LEN sizeof(struct sockaddr)
#endif

// This struct does not appear in Linux header files, so
// we define them here with "_x_" as a prefix
// We use it to access the ip-address b1..b4 values
struct _x_in_addr {
        union {
                struct { u_char s_b1,s_b2,s_b3,s_b4; } S_un_b;
                struct { u_short s_w1,s_w2; } S_un_w;
                u_long S_addr;
        } S_un;
};

struct _x_sockaddr_in {
        short   sin_family;
        u_short sin_port;
        struct  _x_in_addr sin_addr;
        char    sin_zero[8];
};

#endif // __INC_SOCKETDEF__
