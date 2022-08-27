#pragma once

#include <atomic>
#include <cstring>
#include <future>
#include <memory>
#include <fstream>
#include <map>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "../WK_BASE/STL.h"
#include "../WK_BASE/Log.h"

class Client
{
	static const size_t SOCKET_BUF_SIZE = 4096;
public:
	Client(Client const&) = delete;
	Client(Client&&) = delete;
	Client& operator=(Client const&) = delete;
	Client& operator=(Client&&) = delete;

	Client() = delete;

public:
	Client(int _socket)
		:socket_(_socket)
		, IP_(0)
		, port_(0)
		, IsDisconnected_(true)
		, LastRecvTime_(time(0))
		, LastSendTime_(time(0))
		, ConnectTime_(time(0))
	{}

	virtual ~Client()
	{
		RecvPipe_.Clear();
		SendPipe_.Clear();
	}

	size_t PushRecvData(char* _buf, int _len)
	{
		return RecvPipe_.Push(_buf, _len);
	}

	size_t PopRecvData(char* _buf, int _len)
	{
		return RecvPipe_.Pop(_buf, _len);
	}

	size_t PushSendData(char* _buf, int _len)
	{
		return SendPipe_.Push(_buf, _len);
	}

	size_t PopSendData(char* _buf, int _len)
	{
		return SendPipe_.Pop(_buf, _len);
	}

	void Connect(uint32_t _IP, uint16_t _port)
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

	void Disconnect()
	{
		IP_ = 0;
		port_ = 0;

		IsDisconnected_ = true;
	}

	size_t RecvData()
	{
		char* buf = new char[SOCKET_BUF_SIZE];
		memset(buf, 0, SOCKET_BUF_SIZE);

		size_t recvRet(0), recvCount(0);

		do
		{
			recvRet = recv(socket_, buf, SOCKET_BUF_SIZE, 0);

			if (recvRet > 0)
			{
				RecvPipe_.Push(buf, recvRet);
				recvCount += recvRet;
			}

			memset(buf, 0, SOCKET_BUF_SIZE);
		}
		while (recvRet > 0);

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

	size_t SendData()
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

	bool IsDisconnect()
	{
		return IsDisconnected_;
	}

	long InactiveTime()
	{
		return time(0) - LastRecvTime_;
	}

	long ConnectedTime()
	{
		return time(0) - ConnectTime_;
	}

private:
	bool CheckSocket()
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

private:
	int socket_;

	uint32_t IP_;
	uint16_t port_;

	Pipeline RecvPipe_;
	Pipeline SendPipe_;

	std::atomic<bool> IsDisconnected_;

	time_t LastRecvTime_;
	time_t LastSendTime_;
	time_t ConnectTime_;
};

using PClient = std::shared_ptr<Client>;

class Mediator;

class ClientManager
{
	const int INACTIVE_TIME_OUT = 180;

public:
	ClientManager(ClientManager const&) = delete;
	ClientManager(ClientManager&&) = delete;
	ClientManager& operator=(ClientManager const&) = delete;
	ClientManager& operator=(ClientManager&&) = delete;

public:
	ClientManager(shared_ptr<Mediator> _pMediator)
		:IsContinue_(true)
		, pMediator_(_pMediator)
		, CheckThread_(new thread(bind(&ClientManager::CheckThreadFun, this)))
	{

	}
	virtual ~ClientManager()
	{
		IsContinue_ = false;
	}

public:
	PClient GetClient(int _socket)
	{
		PClient client{ nullptr };

		auto it = ClientMap_.find(_socket);

		if (it != ClientMap_.end())
		{
			client = it->second;
		}

		return client;
	}

	void AddClient(int _socket, PClient _client)
	{
		DelClient(_socket);

		if (_client != nullptr)
		{
			ClientMap_.insert(make_pair(_socket, _client));
		}
	}

	void DelClient(int _socket)
	{
		PClient client = GetClient(_socket);

		if (client != nullptr)
		{
			client->Disconnect();
		}
	}

	void ClientConnect(int _socket, uint32_t _IP, uint16_t _port)
	{
		PClient client = GetClient(_socket);

		if (client == nullptr)
		{
			client = make_shared<Client>(_socket);
		}

		client->Connect(_IP, _port);

		AddClient(_socket, client);
	}

	void ClientDisconnect(int _socket)
	{
		DelClient(_socket);
	}

	void ClientRecvData(int _socket)
	{
		PClient client = GetClient(_socket);

		if (client != nullptr)
			client->RecvData();
	}

	void ClientSendData(int _socket)
	{
		PClient client = GetClient(_socket);

		if (client != nullptr)
			client->SendData();
	}

	void CheckClient()
	{
		auto iter = ClientMap_.begin();
		while (iter != ClientMap_.end())
		{
			if (iter->second->IsDisconnect())
			{
				LOGI("Socket %d Is Disconnected", iter->first);
				iter = ClientMap_.erase(iter);
			}
			else if (iter->second->InactiveTime() > INACTIVE_TIME_OUT)
			{
				LOGI("Socket %d Is Inactive Time Out", iter->first);
				ClientDisconnect(iter->first);
				iter = ClientMap_.erase(iter);
			}
			else
			{
				long connected = iter->second->ConnectedTime();
				long inactive = iter->second->InactiveTime();

				LOGI("Socket %d Connected Time [%d:%02d:%02d]  Inactive Time [%d:%02d:%02d]",
					iter->first,
					connected / 3600, connected / 60 % 60, connected % 60,
					inactive / 3600, inactive / 60 % 60, inactive % 60);

				++iter;
			}
		}
	}

	void CheckThreadFun()
	{
		LOGI("Check Thread Start !");

		while (IsContinue_)
		{
			unique_lock<mutex> locker(CheckThreadMutex_);
			if (CheckThreadCondition_.wait_for(locker, chrono::seconds(10)) == cv_status::timeout)
			{
				CheckClient();
			}
		}

		LOGI("Check Thread Exit !");
	}

private:
	std::atomic<bool> IsContinue_;

	std::map<int, PClient> ClientMap_;

	shared_ptr<Mediator> pMediator_;

	std::unique_ptr<std::thread> CheckThread_;
	std::mutex CheckThreadMutex_;
	std::condition_variable CheckThreadCondition_;
};
