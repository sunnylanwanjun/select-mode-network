#ifndef __IOBUFFER__
#define __IOBUFFER__
#include <assert.h>
#include <string.h>
#include "Macro.h"
class IOBuffer;
//写缓冲
class DLL_API IBuffer {
private:
	char *_buf;
	int _pos;
	int _endPos;
public:
	IBuffer() :_buf(0), _pos(0),_endPos(-1) {
	}
	void setBuffer(char* buf,int bufSize) {
		_buf = buf;
		_pos = 0;
		_endPos = bufSize-1;
	}
	void reset() {
		_pos = 0;
	}
	int canWrite() {
		return _endPos - _pos + 1;
	}
	int hasWrite() {
		return _pos;
	}
	void writeBytes(const char* buf,int len) {
		assert(canWrite() >= len);
		memcpy(_buf+_pos,buf,len);
		_pos += len;
	}
	void writeUShort(USHORT data) {
		assert(canWrite() >= 2);
		*((USHORT*)(_buf + _pos)) = data;
		_pos += 2;
	}
	void seek(int pos) {
		assert(pos<=_endPos&&pos>=0);
		_pos = pos;
	}
};

class DLL_API IOBuffer {
private:
	char *_buf;
	int _endPos;
	int _curWritePos;
	int _curReadPos;
public:
	IOBuffer() :_buf(0), _endPos(-1), _curWritePos(0), _curReadPos(0) {
	}
	~IOBuffer() {
	}
	void setBuffer(char* buf, int len) {
		_buf = buf;
		_endPos = len-1;
		_curWritePos = 0;
		_curReadPos = 0;
	}
	void writeBytes(const char* buf, int len) {
		assert(canWrite() >= len);
		memcpy(_buf + _curWritePos, buf, len);
		_curWritePos += len;
	}
	void reset() {
		_curWritePos = 0;
		_curReadPos = 0;
	}
	int canWrite() {
		return _endPos - _curWritePos + 1;
	}
	int hasWrite() {
		return _curWritePos;
	}
	int canRead() {
		return _curWritePos - _curReadPos;
	}
	int hasRead() {
		return _curReadPos;
	}
	unsigned short readUShort() {
		assert(canRead() >= 2);
		unsigned short ret = *((unsigned short*)(_buf + _curReadPos));
		_curReadPos += 2;
		return ret;
	}
	char* readBytes(int len) {
		assert(canRead() >= len);
		char* ret = _buf + _curReadPos;
		_curReadPos += len;
		return ret;
	}
	void readBytes(char* dst, int len) {
		assert(canRead() >= len);
		memcpy(dst, _buf+_curReadPos, len);
		_curReadPos += len;
	}
	void readBytes(IBuffer* iBuf, int len) {
		assert(canRead() >= len);
		iBuf->writeBytes(_buf+_curReadPos, len);
		_curReadPos += len;
	}
	void seekReadPos(int pos) {
		_curReadPos = pos;
	}
	int getReadPos() {
		return _curReadPos;
	}
	void moveReadPos(int len) {
		assert(canRead() >= len);
		_curReadPos += len;
	}
	void moveDataToFront() {
		int canReadSize = canRead();
		if (canReadSize <= 0) {
			_curReadPos = 0;
			_curWritePos = 0;
			return;
		}
		memmove(_buf, _buf + _curReadPos, canReadSize);
		_curReadPos = 0;
		_curWritePos = canReadSize;
	}
};

//读缓冲
class DLL_API OBuffer {
private:
	char *_buf;
	int _pos;
	int _endPos;
public:
	OBuffer() :_buf(0), _pos(0), _endPos(-1) {
	}
	void setBuffer(char* buf, int bufSize) {
		_buf = buf;
		_pos = 0;
		_endPos = bufSize - 1;
	}
	char* getBuffer() {
		return _buf;
	}
	int getBufferSize() {
		return _endPos + 1;
	}
	void reset() {
		_pos = 0;
	}
	int canRead() {
		return _endPos - _pos + 1;
	}
	int hasRead() {
		return _pos;
	}
	unsigned short readUShort() {
		assert(canRead() >= 2);
		unsigned short ret = *((unsigned short*)(_buf + _pos));
		_pos += 2;
		return ret;
	}
	char* readBytes(int len) {
		assert(canRead() >= len);
		char* ret = _buf + _pos;
		_pos += len;
		return ret;
	}
	void readBytes(char* dst,int len) {
		assert(canRead() >= len);
		memcpy(dst, _buf+_pos, len);
		_pos += len;
	}
	void readBytes(IBuffer* iBuf,int len) {
		assert(canRead() >= len);
		iBuf->writeBytes(_buf+_pos, len);
		_pos += len;
	}
	void readBytes(IOBuffer* iBuf, int len) {
		assert(canRead() >= len);
		iBuf->writeBytes(_buf+_pos, len);
		_pos += len;
	}
	void moveReadPos(int m) {
		_pos += m;
	}
	void moveDataToFront() {
		int canReadSize = canRead();
		if (canReadSize <= 0) {
			_pos = 0;
			_endPos = -1;
			return;
		}
		memmove(_buf, _buf + _pos, canReadSize);
		_pos = 0;
		_endPos = _pos + canReadSize - 1;
	}
};
#endif