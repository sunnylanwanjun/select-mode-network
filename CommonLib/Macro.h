#ifndef __MACROH__
#define __MACROH__

#ifndef _WIN32
#define closesocket(sock) close(sock)
#endif
#ifdef _WIN32
#define sleep(time) Sleep(time)
#define LOG(fmt,...) printf(fmt,__VA_ARGS__)
#else
#define LOG(fmt,args...) printf(fmt,##args)
#endif

#ifdef _WIN32
#ifndef DLL_API
#define DLL_API __declspec(dllexport)
#endif
#else
#define DLL_API
#endif

typedef unsigned short USHORT;
typedef unsigned long ULONG;
typedef unsigned long long ULONGLONG;
typedef unsigned int UINT;
typedef unsigned char UCHAR;

#define DEBUG_SERVER_PERFORMANCE
#define DEBUG_CLIENT_PERFORMANCE

#define PACKAGE_MAX_SIZE 2048

#define SERVER_RECV_BUFFER_SIZE PACKAGE_MAX_SIZE*10
#define SERVER_SEND_BUFFER_SIZE PACKAGE_MAX_SIZE*10
#define SERVER_SEND_QUEUE_SIZE 1000
#define SERVER_CLIENT_STATE_QUEUE_SIZE 1000

#define CLIENT_RECV_BUFFER_SIZE PACKAGE_MAX_SIZE*10
#define CLIENT_SEND_BUFFER_SIZE PACKAGE_MAX_SIZE*10
#define CLIENT_SEND_QUEUE_SIZE 100
#define CLIENT_RECV_QUEUE_SIZE 100

#define SERVER_SEND_QUEUE_MAX_ACCUMULATE_COUNT 10000

#define CLIENT_SEND_QUEUE_MAX_ACCUMULATE_COUNT 10000
#define CLIENT_RECV_QUEUE_MAX_ACCUMULATE_COUNT 10000

#if 0
	#define MemoryPool_Dump(obj) (obj)->dump();
	#define MemoryPool_SDump(obj) (obj)->sdump();
	#define MemoryPool_LOG LOG

	#define VarMemPool_Dump(obj) (obj)->dump();
	#define VarMemPool_SDump(obj) (obj)->sdump();
	#define VarMemPool_LOG LOG

	#define MyQueue_Dump(obj) (obj)->dump();
	#define MsgQueue_Dump(obj) (obj)->dump();
#else
	#define MemoryPool_Dump(obj)
	#define MemoryPool_SDump(obj)
	#define MemoryPool_LOG

	#define VarMemPool_Dump(obj)
	#define VarMemPool_SDump(obj)
	#define VarMemPool_LOG

	#define MyQueue_Dump(obj)
	#define MsgQueue_Dump(obj)
#endif

#define MsgQueue_Max 10000
#endif