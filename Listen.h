#pragma once

#include <cstring>
#include <cerrno>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include "../WK_BASE/Log.h"

#include "Mediator.h"

class ListenSocket
{
public:
	ListenSocket() = delete;

	ListenSocket(const char* _ip, uint16_t _port);
	virtual ~ListenSocket();

	int GetSocket();

private:
	int ListenSocket_;
};


class Listen
{
public:
	Listen(shared_ptr<Mediator> _pMediator);
	virtual ~Listen() = default;

	void Start(const char* _IP, uint16_t _port);
	void Terminate();

	void ClientConnect(int _socket, int _ip, uint16_t _port);

	virtual void ClientBindRecv(int _socket) = 0;

protected:
	shared_ptr<ListenSocket> pListenSocket_;
	atomic<bool> IsContinue_;

	shared_ptr<Mediator> pMediator_;

private:
	virtual void ListenThreadFun() = 0;

	shared_ptr<thread> pListenThread_;
};

