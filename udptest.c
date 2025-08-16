#define UDPLIB_IMPLEMENTATION
#include "udplib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

static int8_t	check_args(int argc, char **argv)
{
	if (argc < 4)
		return (-1);
	if (!(strcmp(argv[1], "host") == 0 || strcmp(argv[1], "conn") == 0))
		return (-1);
	int8_t mode = (strcmp(argv[1], "host") == 0) ? 0 : 1;
	if (mode  == 1 && argc != 5)
		return (-1);
	return (mode);
}

static udpsocket_t initialize_connection(udpaddr *addr, uint8_t mode, char **argv)
{
	udpsocket_t socket = udp_socket();

	if (socket == UDP_INVALID_SOCKET)
		return (UDP_INVALID_SOCKET);
	if (mode == 0)
	{
		if (!udp_initaddr(addr, NULL, atoi(argv[2])))
			return (udp_close(socket), UDP_INVALID_SOCKET);
		if (udp_bind(socket, addr) == UDP_SOCKET_ERROR)
			return (udp_close(socket), UDP_INVALID_SOCKET);
	}
	else
	{
		if (!udp_initaddr(addr, argv[2], atoi(argv[3])))
				return (udp_close(socket), UDP_INVALID_SOCKET);
	}
	return (socket);
}

int	main(int argc, char **argv)
{
	udpsocket_t	socket;
	udpaddr		addr;
	int8_t		mode;

	mode = check_args(argc, argv);
	if (mode == -1)
		return (fprintf(stderr, "Usage: %s <host/conn> |addr| <port> <msg>\n", argv[0]), -1);

	socket = initialize_connection(&addr, mode, argv);
	if (socket == UDP_INVALID_SOCKET)
		return (fprintf(stderr, "Something went wrong, error: %d\n", udp_geterror()), -2);

	char buff[256];
	buff[255] = 0;
	udpaddr incoming;
	if (mode == 0) //host mode
	{
		if (udp_recvfrom(socket, buff, sizeof(buff)-1, &incoming) == UDP_SOCKET_ERROR)
			return (fprintf(stderr, "recvfrom error: %d\n", udp_geterror()), -3);
		fprintf(stdout, "[%s:%d] %s\n", udp_getip(&incoming), udp_getport(&incoming), buff);
		if (udp_sendto(socket, &incoming, argv[3], strlen(argv[3])+1) == UDP_SOCKET_ERROR)
			return (fprintf(stderr, "sendto error: %d\n", udp_geterror()), -4);
	}
	else //conn mode
	{
		if (udp_sendto(socket, &addr, argv[4], strlen(argv[4])+1) == UDP_SOCKET_ERROR)
			return (fprintf(stderr, "sendto error: %d\n", udp_geterror()), -4);
		if (udp_recvfrom(socket, buff, sizeof(buff)-1, &incoming) == UDP_SOCKET_ERROR)
			return (fprintf(stderr, "recvfrom error: %d\n", udp_geterror()), -3);
		fprintf(stdout, "[%s:%d] %s\n", udp_getip(&incoming), udp_getport(&incoming), buff);
	}
	udp_close(socket);
	return (0);
}

// Copile with
// 	- Linux: gcc -Wall -Wextra -Werror udptest.c -o udptest
// 	- Windows: x86_64-w64-mingw32-gcc -Wall -Wextra -Werror udptest.c -o udptest.exe -lws2_32
