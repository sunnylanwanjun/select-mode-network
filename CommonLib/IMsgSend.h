#ifndef __IMSGSENDH__
#define __IMSGSENDH__
#include "SocketHeader.h"
#include "Msg.h"
#include "Macro.h"
class DLL_API IMsgSend {
public:
	virtual ~IMsgSend() {}
	virtual int SendMsg(MsgHead* msg) = 0;
};
#endif