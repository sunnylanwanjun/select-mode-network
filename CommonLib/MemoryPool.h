#ifndef __MEMORYPOOLH__
#define __MEMORYPOOLH__
#include <vector>
#include "Macro.h"
#include <assert.h>
#include <mutex>
#include <map>
#include "IMemPool.h"

struct DLL_API MemoryBlock
{
	ULONGLONG       _nSize;
	USHORT          _nFree;
	USHORT          _nFirst;
	USHORT			_nUnitSize;
	USHORT          _nUnitNum;
	MemoryBlock*    _pPre;
	MemoryBlock*    _pNext;
	MemoryBlock*	_this;
	char            _aData[1];

	static void* operator new(size_t, USHORT nUnitSize, USHORT nUnitNum);
	static void  operator delete(void *p, size_t);

	MemoryBlock(USHORT nUnitSize, USHORT nUnitNum);
	~MemoryBlock();
	void reset();
	void* popUnit();
	void pushUnit(void* addr);
};

class DLL_API MemoryPool:public IMemPool
{
private:
	MemoryBlock *   _pBlock;
	MemoryBlock *   _initHead;
	USHORT          _nUnitSize;
	USHORT          _nInitUnitNum;
	USHORT          _nGrowUnitNum;
public:
	MemoryPool(USHORT nUnitSize,
		USHORT nInitUnitNum,
		USHORT nGrowUnitNum);
	virtual ~MemoryPool();
	virtual void* Alloc();
	virtual void Free(void* p);
	virtual void FreeAll();
	virtual void GetInfo(MemInfo* info);
	virtual void dumpBlock(MemoryBlock* curBlock);
	virtual void sdump();
	virtual void dump();
	virtual void dispose();
};

class DLL_API SafeMemPool:public MemoryPool {
	std::mutex _mu;
public:
	SafeMemPool(USHORT nUnitSize,
		USHORT nInitUnitNum,
		USHORT nGrowUnitNum);
	virtual ~SafeMemPool();
	virtual void* Alloc();
	virtual void Free(void* p);
	virtual void GetInfo(MemInfo* info);
};
#endif