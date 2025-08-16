#ifndef UDPLIB_H
# define UDPLIB_H

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
typedef SOCKET udpsocket_t;
typedef int socklen_t;
#else
# include <sys/socket.h>// sockaddr_in socket sendto sockaddr bind recvfrom socklen_t
# include <arpa/inet.h>// inet_pton htons ntohs
# include <unistd.h>// close
typedef int udpsocket_t;
#endif// _WIN32
# include <stddef.h>// NULL
# include <errno.h> // errno

# define UDP_INVALID_SOCKET 0xffff
# define UDP_SOCKET_ERROR -1

typedef struct s_udpaddr
{
	struct sockaddr_in	sockaddr;
}	udpaddr;

int				udp_initaddr(udpaddr *dst, const char *host, unsigned short port);
udpsocket_t		udp_socket(void);
int				udp_sendto(int socket, udpaddr *addr, const void *msg, size_t len);
int				udp_bind(int socket, udpaddr *addr);
int				udp_recvfrom(int socket, void *buf, size_t len, udpaddr *in);
unsigned short	udp_getport(udpaddr *addr);
char			*udp_getip(udpaddr *addr);
int				udp_close(int socket);
int				udp_geterror(void);
#endif // UDPLIB_H

#ifdef UDPLIB_IMPLEMENTATION
#ifdef _WIN32
static int __udplib_winsock_initialized = 0;
static void __udplib_cleanup(void)
{
	if (__udplib_winsock_initialized)
	{
		WSACleanup();
		__udplib_winsock_initialized = 0;
	}
}
static int __udplib_initwinsock(void)
{
	if (!__udplib_winsock_initialized)
	{
		WSADATA wsa;
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
			return (0);
		atexit(__udplib_cleanup);
		__udplib_winsock_initialized = 1;
	}
	return (1);
}
#endif // _WIN32

int	udp_initaddr(udpaddr *dst, const char *host, unsigned short port)
{
	dst->sockaddr.sin_family = AF_INET;
	dst->sockaddr.sin_port = htons(port);
	if (host == NULL)
		dst->sockaddr.sin_addr.s_addr = INADDR_ANY;
	else if (inet_pton(AF_INET, host, &(dst->sockaddr.sin_addr)) <= 0)
		return (0);
	return (1);
}

udpsocket_t	udp_socket(void)
{
#ifdef _WIN32
	if (!__udplib_initwinsock()) return (UDP_INVALID_SOCKET);
	udpsocket_t ret = socket(AF_INET, SOCK_DGRAM, 0);
	if (ret == INVALID_SOCKET)
		return (UDP_INVALID_SOCKET);
	return (ret);

#else
	udpsocket_t ret = socket(AF_INET, SOCK_DGRAM, 0);
	if (ret == -1)
		return (UDP_INVALID_SOCKET);
	return (ret);
#endif
}

int	udp_sendto(int socket, udpaddr *addr, const void *msg, size_t len)
{
#ifdef _WIN32
	return (sendto(socket, (const char *)msg, (int)len, 0, (struct sockaddr *)&(addr->sockaddr), sizeof(addr->sockaddr)));
#else
	return (sendto(socket, msg, len, 0, (struct sockaddr *)&(addr->sockaddr), sizeof(addr->sockaddr)));
#endif
}

int		udp_bind(int socket, udpaddr *addr)
{
	return (bind(socket, (struct sockaddr *)&(addr->sockaddr), sizeof(addr->sockaddr)));
}

int	udp_recvfrom(int socket, void *buf, size_t len, udpaddr *in)
{
	struct sockaddr_in inaddr;
	socklen_t inaddr_len = sizeof(inaddr);

#ifdef _WIN32
	int bytes_recived = recvfrom(socket, (char*)buf, (int)len, 0, (struct sockaddr *)&inaddr, &inaddr_len);
#else
	ssize_t bytes_recived = recvfrom(socket, buf, len, 0, (struct sockaddr *)&inaddr, &inaddr_len);
#endif
	if (inaddr_len != sizeof(inaddr) || bytes_recived < 0)
		return (-1);
	in->sockaddr = inaddr;
	return (bytes_recived);
}

unsigned short	udp_getport(udpaddr *addr)
{
	return (ntohs(addr->sockaddr.sin_port));
}

char			*udp_getip(udpaddr *addr)
{
	return (inet_ntoa(addr->sockaddr.sin_addr));
}

int				udp_close(int socket)
{
#ifdef _WIN32
	return (closesocket(socket));
#else
	return (close(socket));
#endif
}

int	udp_geterror(void)
{
#ifdef _WIN32
	return (WSAGetLastError());
#else
	return (errno);
#endif
}
#endif // UDPLIB_IMPLEMENTATION

/*
 *  
 * ----- USAGE -----
 *  ONLY ONE of the source files must have something like:
 *
 *  #define UDPLIB_IMPLEMENTATION
 *  #include "udplib.h"
 *
 *  On the rest of the proyect you sould include the library like any other one.
 *  To compile in linux use gcc (or any other compiler I guess), no additional linking
 *  is required.
 *  For windows I used x86_64-w64-mingw32-gcc and linked with -lws2_32
 * */
