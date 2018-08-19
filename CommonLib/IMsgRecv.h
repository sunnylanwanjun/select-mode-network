#ifndef __IMSGRECVH__
#define __IMSGRECVH__
#include "Macro.h"
class DLL_API IMsgRecv {
public:
	virtual ~IMsgRecv() {}
	virtual int ProcessorRecv() = 0;
};
#endif