#include "MemoryPool.h"
#include <vector>
#include <assert.h>
#include <mutex>
#include <map>
#include <string>

void* MemoryBlock::operator new(size_t, USHORT nUnitSize, USHORT nUnitNum)
{
	//多出的2个字节用来存放寻址block地址的偏移地址
	return ::operator new(sizeof(MemoryBlock) + nUnitSize * nUnitNum);
}

void  MemoryBlock::operator delete(void *p, size_t)
{
	::operator delete (p);
}

MemoryBlock::MemoryBlock(USHORT nUnitSize, USHORT nUnitNum) {
	_nUnitNum = nUnitNum;
	_nUnitSize = nUnitSize;
	_nSize = _nUnitNum * _nUnitSize;
	_pNext = nullptr;
	_pPre = nullptr;
	_this = this;
	reset();
}

MemoryBlock::~MemoryBlock() {

}

void MemoryBlock::reset() {
	_nFree = _nUnitNum;
	_nFirst = 0;
	for (int i = 0; i < _nUnitNum; i++) {
		uintptr_t addr = (uintptr_t)_aData + i * _nUnitSize;
		*((USHORT*)addr) = i;
		*((USHORT*)(addr + 2)) = i + 1;
	}
}

void* MemoryBlock::popUnit() {
	assert(_nFirst < _nUnitNum);
	_nFree--;
	//每个单元往后偏移两个字节，才是真正的分配地址，两个字节用来存储block寻址
	auto retVal = _aData + _nFirst * _nUnitSize + 2;
	if (_nFree <= 0) {
		_nFirst = -1;
	}
	else {
		_nFirst = *((USHORT*)retVal);
	}
	return retVal;
}

void MemoryBlock::pushUnit(void* addr) {
	//分配地址往前两个字节，才是单元地址，两个字节用来存储block寻址
	uintptr_t offset = (uintptr_t)addr - 2 - (uintptr_t)_aData;
	int number = offset / _nUnitSize;
	assert(number < _nUnitNum);
	*((USHORT*)addr) = _nFirst;
	_nFirst = number;
	_nFree++;
}

MemoryPool::MemoryPool(USHORT nUnitSize,
	USHORT nInitUnitNum,
	USHORT nGrowUnitNum)
	:_nUnitSize(nUnitSize + 2),
	_nInitUnitNum(nInitUnitNum),
	_nGrowUnitNum(nGrowUnitNum) {
	assert(_nUnitSize >= 4);
	assert(nInitUnitNum >= 1);
	assert(nGrowUnitNum >= 1);
	_pBlock = new(_nUnitSize, nInitUnitNum) MemoryBlock(_nUnitSize, nInitUnitNum);
	_initHead = _pBlock;
}

MemoryPool::~MemoryPool() {
	dispose();
}

void MemoryPool::dispose() {
	MemoryBlock *curBlock = _pBlock;
	int deleteIdx = 0;
	while (curBlock) {
		MemoryBlock *nextBlock = curBlock->_pNext;
		MemoryPool_LOG("delete block idx:%d\n", deleteIdx++);
		dumpBlock(curBlock);
		//会先去调用析构函数，再去调用重载的operator delete
		delete curBlock;
		curBlock = nextBlock;
	}
	_pBlock = nullptr;
}

//第一个都没有空间，说明需要重新分配空间，因为我们总是把有空间的block移动到最前面
void* MemoryPool::Alloc() {
	/*MemoryBlock* freeBlock = _pBlock;
	while (freeBlock&&freeBlock->_nFree <= 0) {
		freeBlock = freeBlock->_pNext;
	}
	if (freeBlock) {
		return freeBlock->popUnit();
	}*/
	if (_pBlock->_nFree>0) {
		return _pBlock->popUnit();
	}
	else {
		MemoryPool_LOG("!!!!!!!!!!!!!!!!!!MemoryPool:Alloc another block:%d!!!!!!!!!!!!!!\n", _nUnitSize);
		//先分配内存，再调用构造函数
		MemoryBlock* newBlock = new(_nUnitSize, _nGrowUnitNum) MemoryBlock(_nUnitSize, _nGrowUnitNum);
		newBlock->_pNext = _pBlock;
		newBlock->_pPre = nullptr;
		_pBlock->_pPre = newBlock;
		_pBlock = newBlock;
		return _pBlock->popUnit();
	}
}

void MemoryPool::FreeAll() {
	MemoryPool_LOG("!!!!!!!!MemoryPool:FreeAll Bolock!!!!!!!!\n");
	MemoryBlock *curBlock = _pBlock;
	int freeIdx = 0;
	while (curBlock) {
		curBlock->reset();
		MemoryPool_LOG("free block idx:%d\n", freeIdx++);
		dumpBlock(curBlock);
		curBlock = curBlock->_pNext;
	}
}

void MemoryPool::Free(void* p) {
	uintptr_t addrVal = (uintptr_t)p - 2;
	USHORT unitIdx = *((USHORT*)addrVal);
	uintptr_t blockAddr = addrVal - unitIdx * _nUnitSize - sizeof(uintptr_t);
	MemoryBlock* freeBlock = *((MemoryBlock**)blockAddr);
	assert(freeBlock != nullptr);
	freeBlock->pushUnit(p);

	//是否可以释放该内存块
	if (freeBlock->_nFree == freeBlock->_nUnitNum) {
		//如果该内存块是初始的内存块，即使为空，也不能删除
		if (freeBlock != _initHead) {
			//该内存块不是头节点，则直接删除
			if (freeBlock->_pPre != nullptr) {
				freeBlock->_pPre->_pNext = freeBlock->_pNext;
				if (freeBlock->_pNext != nullptr) {
					freeBlock->_pNext->_pPre = freeBlock->_pPre;
				}
				delete freeBlock;
				return;
			}
			//该内存块是头节点，则删除的同时，还必须寻找一个新的头节点
			else {
				assert(freeBlock->_pNext != nullptr);
				_pBlock = freeBlock->_pNext;
				_pBlock->_pPre = nullptr;
				delete freeBlock;
				return;
			}
		}
	}

	//是否已经是头节点，如果不是，且当前头节点没有可用空间，那么将该节点移至头部
	//可加块下次分配可用空间的寻址
	if (_pBlock->_nFree <= 0 && freeBlock->_pPre != nullptr) {
		freeBlock->_pPre->_pNext = freeBlock->_pNext;
		if (freeBlock->_pNext != nullptr) {
			freeBlock->_pNext->_pPre = freeBlock->_pPre;
		}

		freeBlock->_pNext = _pBlock;
		freeBlock->_pPre = nullptr;
		_pBlock->_pPre = freeBlock;
		_pBlock = freeBlock;
	}
}

void MemoryPool::dumpBlock(MemoryBlock* curBlock) {
	MemoryPool_LOG("_nSize:%llu _nFree:%d _nFirst:%d _nUnitSize:%d _nUnitNum:%d _pPre:%lx _pNext:%lx _aData:%lx\n",
		curBlock->_nSize,
		curBlock->_nFree,
		curBlock->_nFirst,
		curBlock->_nUnitSize,
		curBlock->_nUnitNum,
		(uintptr_t)curBlock->_pPre,
		(uintptr_t)curBlock->_pNext,
		(uintptr_t)curBlock->_aData
	);
}

void MemoryPool::sdump() {
	MemoryPool_LOG("MemoryPoolMemoryPoolMemoryPoolMemoryPoolMemoryPoolMemoryPool\n");
	int blockIdx = 0;
	MemoryBlock *curBlock = _pBlock;
	while (curBlock) {
		blockIdx++;
		MemoryPool_LOG(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>block:%d\n", blockIdx);
		MemoryPool_LOG("_nSize:%llu _nFree:%d _nFirst:%d _nUnitSize:%d _nUnitNum:%d _pPre:%lx _pNext:%lx _aData:%lx\n",
			curBlock->_nSize,
			curBlock->_nFree,
			curBlock->_nFirst,
			curBlock->_nUnitSize,
			curBlock->_nUnitNum,
			(uintptr_t)curBlock->_pPre,
			(uintptr_t)curBlock->_pNext,
			(uintptr_t)curBlock->_aData
		);
		curBlock = curBlock->_pNext;
	}
	MemoryPool_LOG("==================total block is:%d================\n", blockIdx);
}

void MemoryPool::dump() {
	MemoryPool_LOG("MemoryPoolMemoryPoolMemoryPoolMemoryPoolMemoryPoolMemoryPool\n");
	int blockIdx = 0;
	MemoryBlock *curBlock = _pBlock;
	while (curBlock) {
		blockIdx++;
		MemoryPool_LOG(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>block:%d\n", blockIdx);
		MemoryPool_LOG("_nSize:%llu _nFree:%d _nFirst:%d _nUnitSize:%d _nUnitNum:%d _pPre:%lx _pNext:%lx _aData:%lx\n",
			curBlock->_nSize,
			curBlock->_nFree,
			curBlock->_nFirst,
			curBlock->_nUnitSize,
			curBlock->_nUnitNum,
			(uintptr_t)curBlock->_pPre,
			(uintptr_t)curBlock->_pNext,
			(uintptr_t)curBlock->_aData
		);
		MemoryPool_LOG("---------------------------------\n");
		char buf[64];
		char* addr = curBlock->_aData;
		std::string unitInfo = "";
		for (int i = 0; i < curBlock->_nUnitNum; i++) {
			unitInfo = "";
			for (int j = 0; j < curBlock->_nUnitSize; j++) {
				//sprintf(buf, "%x(%u)(%c)|", (unsigned char)*addr, (unsigned char)*addr, (unsigned char)*addr);
				sprintf(buf, "%u|", (unsigned char)*addr);
				unitInfo += buf;
				addr++;
			}
			MemoryPool_LOG("idx:%d val:%s\n--------\n", i, unitInfo.c_str());
		}
		curBlock = curBlock->_pNext;
	}
	MemoryPool_LOG("==================total block is:%d================\n", blockIdx);
}

void MemoryPool::GetInfo(MemInfo* info) {
	assert(info != nullptr);
	MemoryBlock *curBlock = _pBlock;
	while (curBlock) {
		info->totalMem = info->totalMem + curBlock->_nSize;
		info->freeMem = info->freeMem + curBlock->_nFree*curBlock->_nUnitSize;
		info->totalUnit = info->totalUnit + curBlock->_nUnitNum;
		info->freeUnit = info->freeUnit + curBlock->_nFree;
		info->blockNum++;
		curBlock = curBlock->_pNext;
	}
}

SafeMemPool::SafeMemPool(USHORT nUnitSize,
	USHORT nInitUnitNum,
	USHORT nGrowUnitNum) :
	MemoryPool(nUnitSize, nInitUnitNum, nGrowUnitNum) {
}

SafeMemPool::~SafeMemPool() {
	
}

void* SafeMemPool::Alloc() {
	_mu.lock();
	void* ret = MemoryPool::Alloc();
	_mu.unlock();
	return ret;
}

void SafeMemPool::Free(void* p) {
	_mu.lock();
	MemoryPool::Free(p);
	_mu.unlock();
}

void SafeMemPool::GetInfo(MemInfo* memInfo) {
	_mu.lock();
	MemoryPool::GetInfo(memInfo);
	_mu.unlock();
}