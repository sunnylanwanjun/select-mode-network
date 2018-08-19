#ifndef __IMSGHANDLEH__
#define __IMSGHANDLEH__
#include "IMsgSend.h"
#include "Macro.h"
class DLL_API IMsgHandle {
public:
	virtual ~IMsgHandle() {}
	virtual void OnMsgHandle(IMsgSend* msgSender, MsgHead* msg) = 0;
};
#endif