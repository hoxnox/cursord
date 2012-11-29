/**@author Nosov Yuri <hoxnox@gmail.com>
 * @date 20120412
 *
 * This header is used to minimize interface differences between BSD sockets and windows sockets.*/

/**@define WINSOCK
 * @brief Defined, if Windows sockets is used.*/

/**@define BSDSOCK
 * @brief Defined, if BSD sockets is used*/

/**@define IS_VALID_SOCK()
 * @brief Check socket state.*/

/**@define GET_LAST_SOCK_ERROR()
 * @brief Get last error code.*/

/**@define SOCK_INIT()
 * @brief Make initialization*/

#ifdef _WINDOWS
#	ifndef WINSOCK
#		define WINSOCK
#	endif
#else
#	ifndef BSDSOCK
#		define BSDSOCK
#	endif
#endif

#ifdef WINSOCK
#include <winsock2.h>
#endif

#ifdef BSDSOCK
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <errno.h>
#include <poll.h>
#endif

#ifndef __NX_SOCKET_H__
#define __NX_SOCKET_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BSDSOCK
typedef int SOCKET;
#define INVALID_SOCKET -1
#	define IS_VALID_SOCK(s) (s >= 0)
#	define GET_LAST_SOCK_ERROR() (errno)
#	define SOCK_INIT()
#endif

#ifdef WINSOCK
#	define IS_VALID_SOCK(s) (s != INVALID_SOCKET)
#	define GET_LAST_SOCK_ERROR() (WSAGetLastError())
#	define SOCK_INIT() \
{ \
	WSADATA wsaData; \
	WSAStartup(MAKEWORD(1, 1), &wsaData); \
}
#	define EINTR         WSAEINTR
#	define EBADF         WSAEBADF
#	define EINVAL        WSAEINVAL
#	define EADDRNOTAVAIL WSAEADDRNOTAVAIL
#	define EADDRINUSE    WSAEADDRINUSE
#	define EHOSTUNREACH  WSAEHOSTUNREACH
#	define ENETUNREACH   WSAENETUNREACH
#	define ECONNREFUSED  WSAECONNREFUSED
#	define ECONNRESET    WSAECONNRESET
#endif

enum {
	IPv4TYPE_A         = 1 << 7, // 1000 0000
	IPv4TYPE_B         = 1 << 6, // 0100 0000
	IPv4TYPE_C         = 1 << 5, // 0010 0000
	IPv4TYPE_RESERVED  = 1 << 4, // 0001 0000
	IPv4TYPE_BROADCAST = 1 << 3, // 0000 1000
	IPv4TYPE_PRIVATE   = 1 << 2, // 0000 0100
	IPv4TYPE_HOST      = 1 << 1, // 0000 0010
	IPv4TYPE_UNKNOWN   = 0
};

int            SetNonBlock(SOCKET sock);
int            SetReusable(SOCKET sock);
int            IPv4Type(const uint32_t ip);
unsigned short GetPort(struct sockaddr* addr);
void*          GetAddr(struct sockaddr* addr);
void           PrintSockInfo(SOCKET sock);
void           CopyStorageToSockaddr(const struct sockaddr_storage * st, struct sockaddr* sa);
void           CopySockaddrToStorage(const struct sockaddr * sa, struct sockaddr_storage* st);
int            MakeSockaddr(struct sockaddr* src,
                            const char * addr,
                            const size_t addrln,
                            const unsigned short port);
int            ResolveSockaddr(struct sockaddr* src,
                               const char * host,
                               const size_t hostln,
                               const unsigned short port);
int            GetFamily(const char* addr, const size_t addrln);

#ifdef __cplusplus
}
#endif

#endif // __NX_SOCKET_H__

