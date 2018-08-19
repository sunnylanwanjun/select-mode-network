// ServerTest.cpp : Defines the entry point for the console application.
//

#include "Server01.h"
#include "Server02.h"
#include "ConfigHelp.h"
#include "Macro.h"

#ifdef _WIN32
#include "MyDump.h"
#endif

int main()
{
#ifdef _WIN32
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);
#endif

	ConfigHelp configHelp;
	configHelp.init("ServerConfig.txt");
	int _serverType = configHelp.getInt("ServerType");
	LOG("Launch Server Type:%d\n", _serverType);
	switch (_serverType) {
	case 1: {
		Server01 server;
		break;
	}
	case 2: {
		Server02 server;
		break;
	}
	default:
		LOG("Not Found Adapter Server Launch\n");
		break;
	}
#ifdef _WIN32
	system("Pause");
#endif
    return 0;
}

