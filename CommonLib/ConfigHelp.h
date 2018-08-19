#ifndef __CONFIGHELPERH__
#define __CONFIGHELPERH__
#include <string>
#include <map>
#include "Macro.h"
class DLL_API ConfigHelp {
	std::map <std::string, std::string> _record;
public:
	ConfigHelp();
	~ConfigHelp();
	int init(const char* configName);
	std::string getString(const char* key);
	int getInt(const char* key);
	void dump();
};
#endif
