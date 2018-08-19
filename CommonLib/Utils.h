#ifndef __UTILS__
#define __UTILS__
#include <string>
#include <vector>
#include <string>
#include "Msg.h"
#include "Macro.h"
int DLL_API strcmp_ex(const char* str0, const char* str1, int cmpLen=-1);
std::vector<std::string> DLL_API strsplit(const std::string& src, const std::string& split);
int DLL_API writeBuffer(char* dst, int dstSize, const MsgHead* msg);
void DLL_API strtrim(std::string& src, const std::string& mark);
#endif