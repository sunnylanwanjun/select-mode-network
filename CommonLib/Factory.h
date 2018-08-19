#ifndef __FACTORYH__
#define __FACTORYH__
#include "IQueue.h"
#include "Common.h"
#include "Macro.h"
#include "MemoryPool.h"
class Factory {
public:
	template<typename T>
	static IQueue<T>* createQueue(ThreadSafeType safeType, USHORT poolSize) {
		switch (safeType){
		case ThreadSafeType::Safe:
			return new SafeQueue<T>(poolSize);
			break;
		case ThreadSafeType::UnSafe:
			return new MyQueue<T>(poolSize);
			break;
		default:
			assert(false);
			break;
		}
		return nullptr;
	}
	
	static IMemPool* createMemPool(ThreadSafeType safeType, USHORT nUnitSize,
		USHORT nInitUnitNum,
		USHORT nGrowUnitNum) {
		switch (safeType) {
		case ThreadSafeType::Safe:
			return new SafeMemPool(nUnitSize, nInitUnitNum, nGrowUnitNum);
			break;
		case ThreadSafeType::UnSafe:
			return new MemoryPool(nUnitSize, nInitUnitNum, nGrowUnitNum);
			break;
		default:
			assert(false);
			break;
		}
		return nullptr;
	}
};
#endif