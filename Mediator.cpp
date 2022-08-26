#include "Mediator.h"

void Mediator::SetClientManager(shared_ptr<ClientManager> _pManager)
{
	pClientManager_ = _pManager;
}

void Mediator::SetListen(shared_ptr<Listen> _pListen)
{
	pListen_ = _pListen;
}

void Mediator::SetTransmit(shared_ptr<Transmit> _pTransmit)
{
	pTransmit_ = _pTransmit;
}

void Mediator::ClientConnect(int _socket, uint32_t _IP, uint16_t _port)
{
	pClientManager_->ClientConnect(_socket, _IP, _port);
}

void Mediator::ClientDisconnect(int _socket)
{
	pClientManager_->ClientDisconnect(_socket);
}

void Mediator::ClientRecvData(int _socket)
{
	pClientManager_->ClientRecvData(_socket);
}

void Mediator::ClientSendData(int _socket)
{
	pClientManager_->ClientSendData(_socket);
}

void Mediator::ClientBind(int _socket)
{
	pTransmit_->ClientBind(_socket);
}