#pragma once

#include <vector>

#include "../WK_BASE/Log.h"

#include "Mediator.h"

class Transmit
{
	static const std::size_t SOCKET_BUF_SIZE = 4096;
public:
	Transmit(std::shared_ptr<Mediator> _pMediator)
		:pMediator_(_pMediator)
	{
	};

	virtual ~Transmit() = default;

public:
	void Start(int _num);
	void Terminate();

	void ClientDisconnect(int _socket);
	void ClientRecvData(int _socket);
	void ClientSendData(int _socket);

	virtual void ClientBind(int _socket) = 0;
	virtual void ClientBindSend(int _socket) = 0;

private:
	virtual void TransmitFun(int _index) = 0;

protected:
	atomic<bool> IsContinue_;
	size_t ThreadCount_;

	shared_ptr<Mediator> pMediator_;

private:
	vector<shared_ptr<thread>>  ThreadVec_;
};

