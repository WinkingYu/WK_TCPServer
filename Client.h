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
	Client(int _socket);

	virtual ~Client();

	size_t PushRecvData(char* _buf, int _len);
	size_t PopRecvData(char* _buf, int _len);
	size_t PushSendData(char* _buf, int _len);
	size_t PopSendData(char* _buf, int _len);

	void Connect(uint32_t _IP, uint16_t _port);
	void Disconnect();

	size_t RecvData();
	size_t SendData();

	bool IsDisconnect();

	long InactiveTime();
	long ConnectedTime();

private:
	bool CheckSocket();

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
