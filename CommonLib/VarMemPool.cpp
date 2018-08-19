#include "VarMemPool.h"
#include "MemoryPool.h"
#include <map>
#include <mutex>
#include "Factory.h"

VarMemPool::VarMemPool(ThreadSafeType type):safeType(type) {
	_poolArr = new IMemPool*[POOL_NUM];
	for (int i = 0; i < POOL_NUM; i++) {
		_poolArr[i] = Factory::createMemPool(
			safeType,
			poolConfig[i].unitSize,
			poolConfig[i].unitNum,
			poolConfig[i].unitNum);
	}
}

VarMemPool::~VarMemPool() {
	for (int i = 0; i < POOL_NUM; i++) {
		delete _poolArr[i];
	}
	delete[] _poolArr;
}

IMemPool* VarMemPool::getPool(int unitSize) {
	int d = 0;
	int u = POOL_NUM - 1;
	VarMemPool_LOG("VarMemPool:getPool:uniSize=%d Begin to Find down_size=%d,up_size=%d\n", unitSize, poolConfig[d].unitSize, poolConfig[u].unitSize);
	while (u - d>1) {
		int m = d + (u - d)*0.5;
		if (poolConfig[m].unitSize >= unitSize) {
			u = m;
		}
		else {
			d = m;
		}
		VarMemPool_LOG("VarMemPool:getPool:uniSize=%d Finding down_size=%d,up_size=%d\n", unitSize, poolConfig[d].unitSize, poolConfig[u].unitSize);
	}
	VarMemPool_LOG("VarMemPool:getPool:uniSize=%d end to Find down_size=%d,up_size=%d\n", unitSize, poolConfig[d].unitSize, poolConfig[u].unitSize);
	return _poolArr[u];
}

void* VarMemPool::Alloc(int unitSize) {
	if (unitSize > poolConfig[POOL_NUM - 1].unitSize) {
		//std::lock_guard<std::mutex> lockGuard(_mu);
		void* addr = malloc(unitSize);
		//_callSystemAlloc[addr] = true;
		VarMemPool_LOG("!!!!!!!!VarMemPool:Alloc:No Use Pool:size:%d addr:%lx!!!!!!\n", unitSize, (uintptr_t)addr);
		return addr;
	}
	if (unitSize <= poolConfig[0].unitSize) {
		void *addr = _poolArr[0]->Alloc();
		VarMemPool_LOG("!!!!!!!!VarMemPool:Alloc:Use First Pool:size:%d addr:%lx!!!!!!\n", unitSize, (uintptr_t)addr);
		return addr;
	}
	IMemPool* pool = getPool(unitSize);
	void *addr = pool->Alloc();
	VarMemPool_LOG("!!!!!!!!!!VarMemPool:Alloc:Find Adapter Pool:size:%d addr:%lx!!!!!!!!\n", unitSize, (uintptr_t)addr);
	return addr;
}

void VarMemPool::Free(void* p, int unitSize) {
	if (unitSize > poolConfig[POOL_NUM - 1].unitSize) {
		VarMemPool_LOG("!!!!!!!!VarMemPool:Free:No Use Pool:size:%d addr:%lx!!!!!!\n", unitSize, (uintptr_t)p);
		//std::lock_guard<std::mutex> lockGuard(_mu);
		//assert(_callSystemAlloc.find(p)!=_callSystemAlloc.end());
		//_callSystemAlloc.erase(p);
		free(p);
		return;
	}
	if (unitSize <= poolConfig[0].unitSize) {
		VarMemPool_LOG("!!!!!!!!VarMemPool:Free:Use First Pool:size:%d addr:%lx!!!!!!\n", unitSize, (uintptr_t)p);
		_poolArr[0]->Free(p);
		return;
	}
	VarMemPool_LOG("!!!!!!!!!!VarMemPool:Free:Find Adapter Pool:size:%d addr:%lx!!!!!!!!\n", unitSize, (uintptr_t)p);
	IMemPool* pool = getPool(unitSize);
	return pool->Free(p);
}

void VarMemPool::sdump() {
	VarMemPool_LOG("VarMemPoolVarMemPoolVarMemPoolVarMemPoolVarMemPoolVarMemPool\n");
	for (int i = 0; i < POOL_NUM; i++) {
		VarMemPool_LOG("----------------VarMemPool idx:%d\n", i);
		MemoryPool_SDump(_poolArr[i]);
	}
	VarMemPool_LOG("***VarMemPoolVarMemPoolVarMemPoolVarMemPoolVarMemPoolVarMemPool***\n");
}

void VarMemPool::dump() {
	VarMemPool_LOG("VarMemPoolVarMemPoolVarMemPoolVarMemPoolVarMemPoolVarMemPool\n");
	for (int i = 0; i < POOL_NUM; i++) {
		VarMemPool_LOG("----------------VarMemPool idx:%d\n", i);
		_poolArr[i]->dump();
	}
	VarMemPool_LOG("***VarMemPoolVarMemPoolVarMemPoolVarMemPoolVarMemPoolVarMemPool***\n");
}

void VarMemPool::GetInfo(MemInfo * memInfo)
{
	for (int i = 0; i < POOL_NUM; i++) {
		_poolArr[i]->GetInfo(memInfo);
	}
}
