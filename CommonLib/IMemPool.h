#ifndef __IMEMPOOLH__
#define __IMEMPOOLH__
#include "Macro.h"

struct MemInfo {
	UINT totalMem;
	UINT freeMem;
	USHORT totalUnit;
	USHORT freeUnit;
	USHORT blockNum;
};

class IMemPool {
public:
	virtual ~IMemPool() {};
	virtual void* Alloc() = 0;
	virtual void Free(void* p) = 0;
	virtual void FreeAll() = 0;
	virtual void GetInfo(MemInfo* info) = 0;
	virtual void sdump() = 0;
	virtual void dump() = 0;
	virtual void dispose() = 0;
};
#endif