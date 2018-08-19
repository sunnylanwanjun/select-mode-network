#ifndef __SOCKETHEADERH__
#define __SOCKETHEADERH__

#ifndef FD_SETSIZE
#define FD_SETSIZE 5000
#endif

#ifdef _WIN32
#include <Windows.h>
#include <WinSock2.h>
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#define SOCKET int
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#endif
#endif