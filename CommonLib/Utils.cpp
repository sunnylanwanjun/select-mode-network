#include "Utils.h"
#include "Macro.h"
int strcmp_ex(const char* str0, const char* str1, int cmpLen) {
	if (cmpLen == -1) {
		cmpLen = strlen(str1);
	}
	if (strlen(str0) < cmpLen || strlen(str1) < cmpLen) {
		return -1;
	}
	int i = 0;
	while (++i <= cmpLen&&*str0++ == *str1++);
	if (i == cmpLen+1)
		return 0;
	else
		return -1;
}

std::vector<std::string> strsplit(const std::string& src, const std::string& split) {
	std::vector<std::string> res;
	std::size_t begPos = 0;
	std::size_t findPos = 0;
	int len = src.length();
	int splitLen = split.length();
	while (begPos<len && (findPos = src.find(split, begPos)) != std::string::npos) {
		if (begPos == findPos) {
			begPos += splitLen;
			continue;
		}
		res.push_back(src.substr(begPos, findPos - begPos));
		begPos = findPos + splitLen;
	}
	if (begPos<len) {
		res.push_back(src.substr(begPos, len - begPos));
	}
	return res;
}

//返回拷贝字节数
int writeBuffer(char* dst, int dstSize, const MsgHead* msg) {
	memset(dst, 0, dstSize);
	//不足发送一个包头
	if (dstSize < sizeof(MsgHead)) {
		return 0;
	}
	int srcSize = msg->size;
	int cpyLen = dstSize > srcSize ? srcSize : dstSize;
	memcpy(dst, msg, cpyLen);
	LOG("writeBuffer size:%d,msgId:%d\n", cpyLen, msg->msgId);
	return cpyLen;
}

void strtrim(std::string& src,const std::string& mark) {
	size_t pos;
	size_t delNum = mark.length();
	while ((pos = src.find(mark))!=src.npos) {
		src.erase(pos, delNum);
	}
}