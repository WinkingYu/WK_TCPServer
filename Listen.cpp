#include "Listen.h"

ListenSocket::ListenSocket(const char* _ip, uint16_t _port)
{
	ListenSocket_ = socket(AF_INET, SOCK_STREAM, 0);

	int flag;
	flag = fcntl(ListenSocket_, F_GETFL);
	fcntl(ListenSocket_, F_SETFL, flag | O_NONBLOCK);

	int reuse = 1;
	setsockopt(ListenSocket_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

	sockaddr_in listenAddr;

	listenAddr.sin_family = AF_INET;
	listenAddr.sin_port = htons(_port);
	//listenAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	listenAddr.sin_addr.s_addr = inet_addr(_ip);

	if (bind(ListenSocket_, (struct sockaddr*)&listenAddr, sizeof(sockaddr_in)) == -1)
	{
		LOGE("Bind Error:(%d)%s", errno, strerror(errno));
		exit(-1);
	}

	struct sockaddr_in address;

	memset(&address, 0, sizeof(address));
	socklen_t addrLen = sizeof(address);

	if (getsockname(ListenSocket_, (struct sockaddr*)&address, &addrLen) == 0)
	{
		LOGI("Service at %s:%d", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
	}

	int listenRet = listen(ListenSocket_, 4096);

	if (listenRet == -1)
	{
		LOGE("Listen Error:(%d)%s", errno, strerror(errno));
		exit(-1);
	}

}

ListenSocket::~ListenSocket()
{
	if (ListenSocket_)
		shutdown(ListenSocket_, SHUT_RDWR);

	close(ListenSocket_);
}

int ListenSocket::GetSocket()
{
	return ListenSocket_;
}

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