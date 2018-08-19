// ClientTest.cpp : Defines the entry point for the console application.
//
#include "ConfigHelp.h"
#include "Client01.h"
#include "Client02.h"
#include "Client03.h"
#include "Client04.h"
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
	configHelp.init("ClientConfig.txt");
	int _clientType = configHelp.getInt("ClientType");
	LOG("Launch Client Type:%d\n", _clientType);
	switch (_clientType) {
	case 1: {
		Client01 client;
		break;
	}
	case 2: {
		Client02 client;
		break;
	}
	case 3:{
		Client03 client;
		break;
	}
	case 4: {
		Client04 client;
		break;
	}
	default:
		LOG("Not Found Adapter Client Launch\n");
		break;
	}
#ifdef _WIN32
	system("Pause");
#endif
    return 0;
}

