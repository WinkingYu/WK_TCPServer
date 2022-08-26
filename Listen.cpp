#include "Listen.h"

void Listen::ClientConnect(int _socket, int _ip, uint16_t _port)
{
	pMediator_->ClientConnect(_socket, _ip, _port);
}