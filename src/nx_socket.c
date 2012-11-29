/**@author Kim Merder <hoxnox@gmail.com>
 * @date 20121015 11:39:14
 * @copyright Kim Merder */

#include <nx_socket.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <stdio.h>

/**@brief Set nonblocking mode*/
int SetNonBlock(SOCKET sock)
{
	int flags;
	if( (flags = fcntl(sock, F_GETFL, 0)) < 0 )
		return -1;
	if( fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0 )
		return -2;
	return 1;
}

/**@brief set reusable*/
int SetReusable(SOCKET sock)
{
	int on = 1, off = 0;
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))
		return -1;
#ifdef WINSOCK
	// windows allow to bind the same port
	if(setsockopt(*sock, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, &on, sizeof(on)))
		return -1;
#endif
	/*
	if(setsockopt(*sock, SOL_SOCKET, SO_REUSEPORT, &off, sizeof(off)))
		return -1;
		*/
	return 1;
}

/**@brief get ipv4 type
 * @param ip - uint32_t ip representation in network byte order*/
int IPv4Type(const uint32_t ip)
{
	int result = 0;
	if( (ip & (uint32_t)0xff) != 0)
	{
		if((ip & 0x80)== 0)
		{
			result |= IPv4TYPE_A;
			if((ip & (uint32_t)0xffffff80) != 0)
			{
				if((ip & (uint32_t)0xffffff00) == 0xffffff00)
				{
					result |= IPv4TYPE_BROADCAST;
				}
				else
				{
					result |= IPv4TYPE_HOST;
					if((ip & 0x8f) == 0x8f)
						result |= IPv4TYPE_LOCAL;
				}
			}
		}
		else if((ip & 0xc0) == 0x80)
		{
			result |= IPv4TYPE_B;
			if((ip & (uint32_t)0xffff0040) != 0)
			{
				if((ip & (uint32_t)0xfffff000) == 0xfffff000)
				{
					result |= IPv4TYPE_BROADCAST;
				}
				else
				{
					result |= IPv4TYPE_HOST;
					if((ip & 0xfbf) == 0xfbf)
						result |= IPv4TYPE_LOCAL;
				}
			}
		}
		else if((ip & 0xe0) == 0xc0) 
		{
			result |= IPv4TYPE_C;
			if((ip & (uint32_t)0xff000020) != 0)
			{
				if((ip & (uint32_t)0xff000000) == 0xff000000)
				{
					result |= IPv4TYPE_BROADCAST;
				}
				else
				{
					result |= IPv4TYPE_HOST;
					if((ip & 0xffffdf) == 0xffffdf)
						result |= IPv4TYPE_LOCAL;
				}
			}
		}
		else if( ((ip & 0xf0) == 0xe0) )
			result |= IPv4TYPE_BROADCAST;
		else if( ((ip & 0xf0) == 0xf0) )
			result |= IPv4TYPE_RESERVED;
	}
	return result;
}

void* GetAddr(struct sockaddr* addr)
{
	if(addr == NULL)
		return NULL;
	if(addr->sa_family == AF_INET)
		return &(((struct sockaddr_in *)addr)->sin_addr);
	if(addr->sa_family == AF_INET6)
		return &(((struct sockaddr_in6*)addr)->sin6_addr);
	return NULL;
}

void PrintSockInfo(SOCKET sock)
{
	struct sockaddr_storage addr;
	socklen_t addrln = sizeof(addr);
	char tmp[50];
	memset(&addr, 0, sizeof(addr));
	memset(tmp, 0, sizeof(tmp));

	printf("SOCKINFO:\n", sock);
	if(!IS_VALID_SOCK(sock))
	{
		printf("Not valid socket\n");
		return;
	}
	printf("socket: %d\n", sock);
	if(getsockname(sock, (struct sockaddr*)&addr, &addrln) != 0)
	{
		printf("  bind: not binded\n");
		return;
	}
	if(inet_ntop(addr.ss_family, GetAddr((struct sockaddr*)&addr), tmp, sizeof(tmp)) == NULL)
	{
		printf("  bind: bind address corrupted\n");
		return;
	}
	printf("  bind: %s:%d", tmp, GetPort((struct sockaddr*)&addr));
}

unsigned short GetPort(struct sockaddr* addr)
{
	if(addr->sa_family == AF_INET)
	{
		return ((struct sockaddr_in*)addr)->sin_port;
	}
	else if(addr->sa_family == AF_INET6)
	{
		return ((struct sockaddr_in6*)addr)->sin6_port;
	}
	else
		return 0;
}

void CopyStorageToSockaddr(const struct sockaddr_storage * st, struct sockaddr* sa)
{
	sa->sa_family = st->ss_family;
	if(st->ss_family == AF_INET)
	{
		((struct sockaddr_in*)sa)->sin_addr = ((const struct sockaddr_in*)st)->sin_addr;
		((struct sockaddr_in*)sa)->sin_port = ((const struct sockaddr_in*)st)->sin_port;
	}
	else if(sa->sa_family == AF_INET6)
	{
		((struct sockaddr_in6*)sa)->sin6_addr = ((const struct sockaddr_in6*)st)->sin6_addr;
		((struct sockaddr_in6*)sa)->sin6_port = ((const struct sockaddr_in6*)st)->sin6_port;
	}
}

void CopySockaddrToStorage(const struct sockaddr * sa, struct sockaddr_storage* st)
{
	st->ss_family = sa->sa_family;
	if(sa->sa_family == AF_INET)
	{
		((struct sockaddr_in*)st)->sin_addr = ((const struct sockaddr_in*)sa)->sin_addr;
		((struct sockaddr_in*)st)->sin_port = ((const struct sockaddr_in*)sa)->sin_port;
	}
	else if(sa->sa_family == AF_INET6)
	{
		((struct sockaddr_in6*)st)->sin6_addr = ((const struct sockaddr_in6*)sa)->sin6_addr;
		((struct sockaddr_in6*)st)->sin6_port = ((const struct sockaddr_in6*)sa)->sin6_port;
	}
}

/**@brief fill sockaddr_storage structure from parameters values
 * @param src - source, where to save data
 * @param addr - dotted notation of IP address
 * @param addrln - the addr length
 * @param port - port number (network byte order)
 * @return negative on error, 0 on success*/
int MakeSockaddr(struct sockaddr* src,
                 const char * addr,
                 const size_t addrln,
                 const unsigned short port)
{
	char * tmp = (char*)malloc((addrln + 1)*sizeof(char));
	memset(tmp, 0, addrln + 1);
	memcpy(tmp, addr, addrln);
	if(src == NULL || addr == NULL || addrln == 0)
		return -1;
	src->sa_family = GetFamily(addr, addrln);
	if( inet_pton(src->sa_family, tmp, GetAddr(src)) != 1 )
		return -2;
	((struct sockaddr_in6*)src)->sin6_port = port;
	return 0;
}

/**@brief fill sockaddr_storage structure from parameters values
 * @param src - source, where to save data
 * @param host - hostname (try to resolve host)
 * @param hostln - the hostname length
 * @param port - port number (network byte order)
 * @return negative on error, 0 on success*/
int ResolveSockaddr(struct sockaddr* src,
                    const char * host,
                    const size_t hostln,
                    const unsigned short port)
{
	// user getaddrinfo
	return 0;
}

/**@brief get address family by string representation
 * @param addr - address
 * @param addrln - the address length
 * @return AF_INET, if addr may be IPv4, AF_INET6 if IPv6 and -1 on error
 *
 * TODO: For now, this function just search for ':' sign, if it is here, we
 * expect IPv6 address, otherwise - IPv4.*/
int GetFamily(const char* addr, const size_t addrln)
{
	if(addr == NULL || addrln == 0)
		return 0;
	int i;
	for(i = 0; i < addrln; ++i)
		if(addr[i] == ':')
			return AF_INET6;
	return AF_INET;
}

