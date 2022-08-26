#pragma once

#include "Client.h"
#include "Listen.h"
#include "Transmit.h"

class Transmit;
class Listen;

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

	void SetClientManager(shared_ptr<ClientManager> _pManager);
	void SetListen(shared_ptr<Listen> _pListen);
	void SetTransmit(shared_ptr<Transmit> _pTransmit);
	void ClientConnect(int _socket, uint32_t _IP, uint16_t _port);
	void ClientDisconnect(int _socket);
	void ClientRecvData(int _socket);
	void ClientSendData(int _socket);
	void ClientBind(int _socket);

private:
	shared_ptr<ClientManager> pClientManager_;
	shared_ptr<Listen> pListen_;
	shared_ptr<Transmit> pTransmit_;
};

//using PMediator = shared_ptr<Mediator>;

