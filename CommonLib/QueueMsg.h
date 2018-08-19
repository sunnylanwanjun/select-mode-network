#ifndef __QUEUEMSGH__
#define __QUEUEMSGH__
#include "SocketHeader.h"
#include "Macro.h"
#include "Msg.h"

enum EClientState {
	Leave,
	Join,
};

struct QMClientState {
	UCHAR type;
	SOCKET clientSock;
};

struct QMSend {
	USHORT size;
	char*  data;
};

////////////////////////////////////////
//废弃
//#define QMSIZE 32
//enum QMType {
//	QMError,
//	ClientIn,
//	ClientOut,
//	ClientMsg,
//};

//struct QMHead {
//	USHORT handleType;
//	USHORT size;
//	void Init() {
//		handleType = QMType::QMError;
//		size = sizeof(QMHead);
//	}
//};

//struct QMClientIn:public QMHead {
//	SOCKET clientSock;
//	void Init(){
//		handleType = QMType::ClientIn;
//		size = sizeof(QMClientIn);
//
//		clientSock = INVALID_SOCKET;
//	}
//};

//struct QMClientOut:public QMHead {
//	USHORT handleId;
//	SOCKET clientSock;
//	USHORT clientNum;
//	void Init() {
//		handleType = QMType::ClientOut;
//		size = sizeof(QMClientOut);
//
//		clientNum = 0;
//		handleId = 0;
//		clientSock = INVALID_SOCKET;
//	}
//};

//struct QMClientMsg :public QMHead {
//	ULONG  clientNum;
//	USHORT handleId;
//	SOCKET clientSock;
//	MsgHead* msg;
//	void Init() {
//		handleType = QMType::ClientMsg;
//		size = sizeof(QMClientMsg);
//
//		clientSock = INVALID_SOCKET;
//		msg = nullptr;
//		handleId = 0;
//		clientNum = 0;
//	}
//};
#endif