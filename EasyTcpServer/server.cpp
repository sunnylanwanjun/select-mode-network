#include "EasyTcpServer.hpp"
#include<thread>

bool g_bRun = true;
void cmdThread()
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			printf("exit cmdThread\n");
			break;
		}
		else {
			printf("not support command.\n");
		}
	}
}

class MyServer : public EasyTcpServer
{
public:

	//只会被一个线程触发 安全
	virtual void OnNetJoin(ClientSocket* pClient)
	{
		_clientCount++;
		//printf("client<%d> join\n", pClient->sockfd());
	}
	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
	virtual void OnNetLeave(ClientSocket* pClient)
	{
		_clientCount--;
		//printf("client<%d> leave\n", pClient->sockfd());
	}
	//cellServer 4 多个线程触发 不安全
	//如果只开启1个cellServer就是安全的
	virtual void OnNetMsg(ClientSocket* pClient, DataHeader* header)
	{
		_recvCount++;
		switch (header->cmd)
		{
		case CMD_LOGIN:
		{

			Login* login = (Login*)header;
			//printf("收到客户端<Socket=%d>请求：CMD_LOGIN,数据长度：%d,userName=%s PassWord=%s\n", cSock, login->dataLength, login->userName, login->PassWord);
			//忽略判断用户密码是否正确的过程
			LoginResult ret;
			pClient->SendData(&ret);
		}
		break;
		case CMD_LOGOUT:
		{
			Logout* logout = (Logout*)header;
			//printf("收到客户端<Socket=%d>请求：CMD_LOGOUT,数据长度：%d,userName=%s \n", cSock, logout->dataLength, logout->userName);
			//忽略判断用户密码是否正确的过程
			//LogoutResult ret;
			//SendData(cSock, &ret);
		}
		break;
		default:
		{
			printf("<socket=%d>recv unknow msg,data len:%d\n", pClient->sockfd(), header->dataLength);
			//DataHeader ret;
			//SendData(cSock, &ret);
		}
		break;
		}
	}
private:

};

int main()
{

	MyServer server;
	server.InitSocket();
	server.Bind(nullptr, 4567);
	server.Listen(5);
	server.Start(2);

	//启动UI线程
	std::thread t1(cmdThread);
	t1.detach();

	while (g_bRun)
	{
		server.OnRun();
		//printf("空闲时间处理其它业务..\n");
	}
	server.Close();
	printf("has exit.\n");
	getchar();
	return 0;
}