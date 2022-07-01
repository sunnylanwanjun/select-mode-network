#include "ClientSocket.h"
#include "VarMemPool.h"
#include "Factory.h"

ClientSocket * ClientSocket::Create(SOCKET sock, IMsgHandle * msgHandle, VarMemPool & memeryPool)
{
	ClientSocket* ret = new ClientSocket(sock,msgHandle,memeryPool);
	ret->_refCount = 1;
	return ret;
}

void ClientSocket::Release()
{
	_refCount--;
	
}

ClientSocket::ClientSocket(SOCKET sock, IMsgHandle* msgHandle, VarMemPool& memeryPool) :
#ifdef DEBUG_SERVER_PERFORMANCE
	_testMsgTimes(0),
	_testSendTimes(0),
	_testSendQueueSize(0),
	_testRecvTimes(0),
#endif
	_msgHandle(msgHandle),
	_clientSock(sock),
	_retainSize(0),
	_memeryPool(memeryPool),
	_sendQueue(nullptr),
	_refCount(0){
	_safeType = _memeryPool.safeType;
	_ioSend.setBuffer(_sendBuf, sizeof(_sendBuf));
	_sendQueue = Factory::createQueue<QMSend>(_safeType, SERVER_SEND_QUEUE_SIZE);
}

ClientSocket::~ClientSocket() {
	Close();
	delete _sendQueue;
	_sendQueue = nullptr;
}

void ClientSocket::Close() {
	if (_safeType == ThreadSafeType::Safe) {
		_closeMu.lock();
	}
	if (_clientSock != INVALID_SOCKET) {
		closesocket(_clientSock);
		_clientSock = INVALID_SOCKET;
		ClearQueue();
		ClearBuffer();
	}
	if (_safeType == ThreadSafeType::Safe) {
		_closeMu.unlock();
	}
}

void ClientSocket::ClearQueue() {
	while (_sendQueue->hasData()) {

#ifdef DEBUG_SERVER_PERFORMANCE
		if (_testSendQueueSize != nullptr)
			(*_testSendQueueSize)--;
#endif

		QMSend sendData = _sendQueue->pop();
		MsgHead* msg = (MsgHead*)sendData.data;
		_memeryPool.Free(msg, sendData.size);
	}
}

void ClientSocket::ClearBuffer() {
	_ioSend.setBuffer(_sendBuf, sizeof(_sendBuf));
	_oMsg.setBuffer(0, 0);
	_oRecv.setBuffer(0, 0);
}

/*
这里解释为什么返回0表示与客户端断开连接
当服务器或者客户端主动关闭一个socket的时候，这时如果去recv或者send
都会返回0，表示socket已经关闭，这涉及到tcp四次分手的过程

1 客户端closesocket，发送一个fin1包给服务器，客户端状态为fin_wait_1

2 服务器接到此包后,发送一个ack1包给客户端，服务器状态为close_wait

3 客户端收到ack1包后，因为还需要等待服务器closesocket，状态为fin_wait_2

4 此时服务器对应的通道为可读状态，当去recv时，返回0，从而主动关闭socket，
发送fin2给客户端，状态为last_ack

5 客户端收到fin2后，发送一个ack2给服务器，状态为time_wait

6 服务器收到ack2后，状态为closed，分手结束

*/
int ClientSocket::ProcessorRecv() {
#ifdef DEBUG_SERVER_PERFORMANCE
	//测试使用
	if (_testRecvTimes != nullptr) {
		(*_testRecvTimes)++;
	}
#endif
	int nlen = (int)recv(_clientSock, _recvBuf + _retainSize, sizeof(_recvBuf) - _retainSize, 0);
	if (nlen <= 0) {
		//LOG("ERROR:recv msg failed sock:%d\n", _clientSock);
		Close();
		return -1;
	}

	_oRecv.setBuffer(_recvBuf, nlen + _retainSize);
	//LOG("bbbbbbbbbbbbbbbbbb processorRecv recv buf len is:%d,retain len is:%d,total len is:%d\n", nlen, _retainSize,nlen+_retainSize);
	_retainSize = 0;
	int packageNum = 0;
	while (true) {
		//残留数据不足读取一个包头
		int canReadSize = _oRecv.canRead();
		if (canReadSize < sizeof(MsgHead)) {
			//LOG("111111111111111111 processorRecv recv buff no enough len:%d,need len:%d\n", canReadSize,sizeof(MsgHead));
			break;
		}
		unsigned short  msgLen = _oRecv.readUShort();
		if (msgLen > PACKAGE_MAX_SIZE) {
			// assert(msgLen<=PACKAGE_MAX_SIZE);
			LOG("ERROR:ClientSocket ProcessorRecv msgLen(%d) > PACKAGE_MAX_SIZE(%d), socket:%d\n", msgLen, PACKAGE_MAX_SIZE, _clientSock);
			Close();
			return -1;
		}
		_oRecv.moveReadPos(-2);
		//不够读取一个完整包
		if (msgLen > canReadSize) {
			//LOG("222222222222222222 processorRecv recv buff no enough len:%d,need len:%d\n", canReadSize,msgLen);
			break;
		}

#ifdef DEBUG_SERVER_PERFORMANCE
		//测试使用
		if (_testMsgTimes != nullptr) {
			(*_testMsgTimes)++;
		}
#endif

		memset(_package, 0, sizeof(_package));
		_oRecv.readBytes(_package, msgLen);
		_msgHandle->OnMsgHandle(this, (MsgHead*)_package);
		packageNum++;



	}
	//将剩余数据往前挪动
	_oRecv.moveDataToFront();
	_retainSize = _oRecv.canRead();
	//LOG("eeeeeeeeeeeeeeeeee processorRecv handle package num is:%d,retian msg len:%d\n", packageNum, _retainSize);

	return 0;
}

int ClientSocket::SendData(char* data, USHORT dataLen) {
	if (_clientSock == INVALID_SOCKET) {
		return -1;
	}

#ifdef DEBUG_SERVER_PERFORMANCE
	// TODO: MayBe Unsafe
	if (_testSendQueueSize != nullptr)
		(*_testSendQueueSize)++;
	if (_testSendTimes != nullptr)
		(*_testSendTimes)++;
#endif

	char* package = (char*)_memeryPool.Alloc(dataLen);
	memcpy(package, data, dataLen);
	_sendQueue->push({ dataLen,(char*)package });
	return 0;
}

int ClientSocket::SendMsg(MsgHead* msg) {
	if (_clientSock == INVALID_SOCKET) {
		return -1;
	}
	return SendData((char*)msg, msg->size);
}

int ClientSocket::SendToKernelBuf() {
	//判断当前能否发送数据
	fd_set fdWrite;
	FD_ZERO(&fdWrite);
	FD_SET(_clientSock, &fdWrite);
	timeval t = { 0,10 };
	int n = select(_clientSock + 1, 0, &fdWrite, 0, &t);
	if (n < 0) {
		LOG("ERROR:select write failed socket:%d\n", _clientSock);
		Close();
		return -1;
	}
	if (FD_ISSET(_clientSock, &fdWrite)) {
		FD_CLR(_clientSock, &fdWrite);
		int totalLen = _ioSend.canRead();
		int sendLen = send(_clientSock, _sendBuf, totalLen, 0);
		if (sendLen <= 0) {
			LOG("ERROR:send msg failed socket:%d\n", _clientSock);
			Close();
			return -1;
		}
		_ioSend.moveReadPos(sendLen);
		_ioSend.moveDataToFront();
		return 1;
	}
	return 0;
}

int ClientSocket::ProcessorSend() {
	if (_clientSock == INVALID_SOCKET)
		return -1;
	do {
		do {
			//先判断发送缓冲区有没有残留数据没发出去
			int canWrite = _ioSend.canWrite();
			if (canWrite <= 0) {
				//尝试发送数据
				int n = SendToKernelBuf();
				if (n < 0)return -1;//网络出错
				//暂时不能写
				if (n == 0) {
					if (_sendQueue->size()>SERVER_SEND_QUEUE_MAX_ACCUMULATE_COUNT) {
						LOG("error 11111:client %d send queue accumulate to many,size is:%d",_clientSock, _sendQueue->size());
						Close();
						return -1;
					}
					return 0;
				}
			}
			//再判断当前是否还有数据在OBuffer里面
			int canRead = _oMsg.canRead();
			if (canRead <= 0) {
				//OBuffer里如果没有数据，说明第一次进来这里，不用释放资源
				if (_oMsg.getBuffer() != nullptr) {

#ifdef DEBUG_SERVER_PERFORMANCE
					if (_testSendQueueSize != nullptr)
						(*_testSendQueueSize)--;
#endif

					QMSend qmSend = _sendQueue->pop();
					_memeryPool.Free(qmSend.data, qmSend.size);
					_oMsg.setBuffer(0, 0);
				}
				break;
			}
			int writeLen = canRead < canWrite ? canRead : canWrite;
			_oMsg.readBytes(&_ioSend, writeLen);
		} while (true);
		if (!_sendQueue->hasData())
			break;
		QMSend qmSend = _sendQueue->front();
		_oMsg.setBuffer((char*)qmSend.data, qmSend.size);
	} while (true);

	//当缓冲区有数据时，把数据发送出去，走到这，说明消息队列里面的消息都被取出
	int canRead = _ioSend.canRead();
	if (canRead > 0) {
		int n = SendToKernelBuf();
		if (n < 0)return -1;//网络出错
		if (n == 0) {
			if (_sendQueue->size()>SERVER_SEND_QUEUE_MAX_ACCUMULATE_COUNT) {
				LOG("error 22222:client %d send queue accumulate to many,size is:%d", _clientSock, _sendQueue->size());
				Close();
				return -1;
			}
		}
	}
	return 0;
}