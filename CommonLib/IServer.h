#ifndef __ISERVERH__
#define __ISERVERH__
#include "SocketHeader.h"
#include "Msg.h"
#include <map>
class IServer {
public:
	virtual int Bind(unsigned long addr,unsigned short port) = 0;
	virtual int Listen(int backlog) = 0;
	virtual void Close() = 0;
	virtual bool IsRun() = 0;
	virtual int OnRun() = 0;
};
#endif