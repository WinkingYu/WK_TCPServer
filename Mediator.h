#pragma once

#include "Client.h"
#include "Listen.h"

class Mediator
{
public:
	Mediator(Mediator const&) = delete;
	Mediator(Mediator&&) = delete;
	Mediator& operator=(Mediator const&) = delete;
	Mediator& operator=(Mediator&&) = delete;

public:
	Mediator() = default;
	~Mediator() = default;

	void SetClientManager(shared_ptr<ClientManager> _pManager)
	{
		pClientManager_ = _pManager;
	}

	void ClientConnect(int _socket, uint32_t _IP, uint16_t _port)
	{
		pClientManager_->ClientConnect(_socket, _IP, _port);
	}

private:
	shared_ptr<ClientManager> pClientManager_;
};

