#include "Client.h"

Client::Client(int _socket)
	:socket_(_socket)
	, IP_(0)
	, port_(0)
	, IsDisconnected_(true)
	, LastRecvTime_(time(0))
	, LastSendTime_(time(0))
	, ConnectTime_(time(0))
{}

Client::~Client()
{
	RecvPipe_.Clear();
	SendPipe_.Clear();
}

size_t Client::PushRecvData(char* _buf, int _len)
{
	return RecvPipe_.Push(_buf, _len);
}

size_t Client::PopRecvData(char* _buf, int _len)
{
	return RecvPipe_.Pop(_buf, _len);
}

size_t Client::PushSendData(char* _buf, int _len)
{
	return SendPipe_.Push(_buf, _len);
}

size_t Client::PopSendData(char* _buf, int _len)
{
	return SendPipe_.Pop(_buf, _len);
}

void Client::Connect(uint32_t _IP, uint16_t _port)
{
	IP_ = _IP;
	port_ = _port;

	ConnectTime_ = time(0);
	LastRecvTime_ = time(0);
	LastSendTime_ = time(0);

	RecvPipe_.Clear();
	SendPipe_.Clear();

	IsDisconnected_ = false;
}

void Client::Disconnect()
{
	IP_ = 0;
	port_ = 0;

	IsDisconnected_ = true;
}

size_t Client::RecvData()
{
	char* buf = new char[SOCKET_BUF_SIZE];
	memset(buf, 0, SOCKET_BUF_SIZE);

	int recvRet(0), recvCount(0);

	while ((recvRet = recv(socket_, buf, SOCKET_BUF_SIZE, 0)) > 0)
	{
		if (recvRet > 0)
		{
			RecvPipe_.Push(buf, recvRet);
			recvCount += recvRet;
		}

		memset(buf, 0, SOCKET_BUF_SIZE);
	}

	delete[] buf;

	if (recvRet == 0)
	{
		LOGI("%d Socket Disconnect", socket_);
		Disconnect();
	}
	else if (recvRet == -1 && errno != EAGAIN)
	{
		LOGE("Recv Fr %d Socket Error:(%d)%s", socket_, errno, strerror(errno));

		Disconnect();
	}
	else if (recvCount > 0)
	{
		LOGI("Recv Fr %d Socket Count %d Bytes", socket_, recvCount);
	}

	return recvCount;
}

size_t Client::SendData()
{
	char* buf = new char[SOCKET_BUF_SIZE];

	int sendRet(0), sendCount(0);
	size_t popLen(0);
	while (!SendPipe_.IsEmpty())
	{
		memset(buf, 0, SOCKET_BUF_SIZE);
		popLen = SendPipe_.Pop(buf, SOCKET_BUF_SIZE);

		if (popLen > 0 && socket_ > 0 && CheckSocket())
		{
			sendRet = send(socket_, buf, popLen, 0);

			if (sendRet > 0)
			{
				LOGI("Send To %d Socket %d Bytes", socket_, sendRet);
				sendCount += sendRet;
			}
			else if (errno != EAGAIN)
			{
				LOGE("Send To %d Socket Error:(%d)%s", socket_, errno, strerror(errno));
				Disconnect();

				break;
			}
		}
	}

	delete[] buf;

	if (sendCount > 0)
		LOGI("Send To %d Socket Count %d Bytes", socket_, sendCount);

	return sendCount;
}

bool Client::IsDisconnect()
{
	return IsDisconnected_;
}

long Client::InactiveTime()
{
	return time(0) - LastRecvTime_;
}

long Client::ConnectedTime()
{
	return time(0) - ConnectTime_;
}

bool Client::CheckSocket()
{
	bool result(false);

	if (socket_ <= 0)
		return result;

	struct tcp_info info;
	int len = sizeof(info);

	memset(&info, 0, sizeof(info));
	getsockopt(socket_, IPPROTO_TCP, TCP_INFO, &info, (socklen_t*)&len);

	result = (info.tcpi_state == 1);

	return result;
}