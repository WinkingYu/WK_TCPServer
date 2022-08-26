#include "Listen.h"

Listen::Listen(shared_ptr<Mediator> _pMediator)
	:pMediator_(_pMediator)
{

}

void Listen::Start(const char* _IP, uint16_t _port)
{
	IsContinue_ = true;
	pListenSocket_ = make_shared<ListenSocket>(_IP, _port);
	pListenThread_ = make_shared<thread>(bind(&Listen::ListenThreadFun, this));
}

void Listen::Terminate()
{
	IsContinue_ = false;
}

void Listen::ClientConnect(int _socket, int _ip, uint16_t _port)
{
	pMediator_->ClientConnect(_socket, _ip, _port);
}