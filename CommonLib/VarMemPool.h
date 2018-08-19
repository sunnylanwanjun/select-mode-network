#ifndef __VARMEMPOOLH__
#define __VARMEMPOOLH__
#include "MemoryPool.h"
#include <map>
#include <mutex>
#include "Common.h"

class DLL_API VarMemPool {
//#define DEF_POOL_NUM 7
public:
	static const int POOL_NUM = 7;
private:
	struct PoolConfig {
		int unitSize;
		int unitNum;
	};

	IMemPool** _poolArr;
	//std::mutex _mu;
	//std::map<void*, bool> _callSystemAlloc;

	const PoolConfig poolConfig[POOL_NUM] = {

	{ 16   ,1000, },
	{ 32   ,1000, },
	{ 64   ,1000, },
	{ 128  ,1000, },
	{ 256  ,1000, },
	{ 512  ,1000,  },
	{ 1024 ,1000,  },

	/*{ 16   ,2, },
	{ 32   ,2, },
	{ 64   ,2, },
	{ 128  ,2, },
	{ 256  ,2, },
	{ 512  ,2, },
	{ 1024 ,2, },*/

	/*{ 8   ,2, },
	{ 16  ,2, },
	{ 32  ,2, },
	{ 64  ,2, },*/

	};
private:
	IMemPool * getPool(int unitSize);
public:
	VarMemPool(ThreadSafeType safeType);
	~VarMemPool();
	void* Alloc(int unitSize);
	void Free(void* p, int unitSize);
	void sdump();
	void dump();
	void GetInfo(MemInfo* memInfo);
public:
	ThreadSafeType safeType;
};
#endif