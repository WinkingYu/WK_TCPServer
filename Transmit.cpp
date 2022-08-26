#include "Transmit.h"


void Transmit::ClientDisconnect(int _socket)
{
	pMediator_->ClientDisconnect(_socket);
}

void Transmit::ClientRecvData(int _socket)
{
	pMediator_->ClientRecvData(_socket);
}

void Transmit::ClientSendData(int _socket)
{
	pMediator_->ClientSendData(_socket);
}