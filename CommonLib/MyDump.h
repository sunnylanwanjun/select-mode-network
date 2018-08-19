#pragma once

#include <Windows.h>
#include <dbghelp.h>

#define DUMP_FILE ".\\WindowsP.dmp"
#include <string>

void CreateDumpFile(LPCSTR lpstrDumpFilePathName, EXCEPTION_POINTERS *pException)
{
	// 创建Dump文件  

	HANDLE hDumpFile = CreateFileA(lpstrDumpFilePathName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	// Dump信息  
	MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
	dumpInfo.ExceptionPointers = pException;
	dumpInfo.ThreadId = GetCurrentThreadId();
	dumpInfo.ClientPointers = TRUE;

	// 写入Dump文件内容  
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);

	CloseHandle(hDumpFile);
}

LONG ApplicationCrashHandler(EXCEPTION_POINTERS *pException)
{
	// 这里弹出一个错误对话框并退出程序  
	char szPath[512];
	GetModuleFileNameA(NULL, szPath, 512);
	char *pChar = strrchr(szPath, '\\');
	*(pChar + 1) = 0;
	std::string strPath = szPath;
	SYSTEMTIME syst;
	GetLocalTime(&syst);
	char strCount[100];
	sprintf_s(strCount, 100, "%d.%.2d.%.2d.%.2d.%.2d.%.2d.%.3d.dmp", syst.wYear -
		2000, syst.wMonth, syst.wDay, syst.wHour, syst.wMinute, syst.wSecond, syst.wMilliseconds);

	strPath += std::string(strCount);
	MakeSureDirectoryPathExists(strPath.c_str());
	CreateDumpFile(strPath.c_str(), pException);
	FatalAppExitA(0, "*** Unhandled Exception! ***");

	return EXCEPTION_EXECUTE_HANDLER;
}