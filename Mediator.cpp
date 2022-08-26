#include "Mediator.h"

void Mediator::ClientBind(int _socket)
{
	pTransmit_->ClientBind(_socket);
}