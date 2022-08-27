#include "Epoll.h"

Epoll::Epoll(int _size)
{
	fd = epoll_create(_size);
}

Epoll::~Epoll()
{
	close(fd);
}

void Epoll::RegRecv(int _socket)
{
	epoll_event event;

	event.data.fd = _socket;
	event.events = EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLERR | EPOLLHUP;

	epoll_ctl(fd, EPOLL_CTL_ADD, _socket, &event);
}

void Epoll::RegSend(int _socket)
{
	epoll_event event;

	event.data.fd = _socket;
	event.events = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLRDHUP | EPOLLERR | EPOLLHUP;

	epoll_ctl(fd, EPOLL_CTL_MOD, _socket, &event);
}

void Epoll::RegCancel(int _socket)
{
	epoll_event event;

	epoll_ctl(fd, EPOLL_CTL_DEL, _socket, &event);
}

void Epoll::RegLinsten(int _socket)
{
	epoll_event regEvent;

	regEvent.data.fd = _socket;
	regEvent.events = EPOLLIN;

	epoll_ctl(fd, EPOLL_CTL_ADD, _socket, &regEvent);
}

int Epoll::ListenWait(int _socket)
{
	epoll_event event;

	event.data.fd = _socket;
	event.events = EPOLLIN;

	return epoll_wait(fd, &event, 1, 1000);
}

int Epoll::TransmitWait(epoll_event* _event, int _maxEvents, int _time)
{
	return epoll_wait(fd, _event, _maxEvents, _time);
}

EpollListen::EpollListen(shared_ptr<Mediator> _pMediator)
	:Listen(_pMediator)
{
	pEpoll_ = make_shared<Epoll>(1);

}

EpollListen::~EpollListen()
{

}

void EpollListen::ClientBind(int _socket)
{
	pMediator_->ClientBind(_socket);
}

void EpollListen::ListenThreadFun()
{
	LOGI("Listen Start");

	int socket = pListenSocket_->GetSocket();
	pEpoll_->RegLinsten(socket);

	while (IsContinue_)
	{
		if (pEpoll_->ListenWait(socket) > 0)
		{
			sockaddr_in clientAddr;
			bzero(&clientAddr, sizeof(sockaddr_in));
			socklen_t AddrLen = sizeof(clientAddr);

			int clientSocket = accept(socket, (sockaddr*)&clientAddr, &AddrLen);

			if (clientSocket > 0)
			{
				int flag;
				flag = fcntl(clientSocket, F_GETFL);
				fcntl(clientSocket, F_SETFL, flag | O_NONBLOCK);

				int BufSize = SOCKET_BUF_SIZE;
				if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVBUF, (char*)&BufSize, sizeof(int)) != 0)
				{
					LOGE("Set RCVBUF Error");
				}

				if (setsockopt(clientSocket, SOL_SOCKET, SO_SNDBUF, (char*)&BufSize, sizeof(int)) != 0)
				{
					LOGE("Set SNDBUF Error");
				}

				ClientConnect(clientSocket, clientAddr.sin_addr.s_addr, ntohs(clientAddr.sin_port));
				ClientBind(clientSocket);

				LOGI("New Connect(%d) %s:%u", clientSocket, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
			}
			else
			{
				close(clientSocket);
				LOGE("Accept Error:(%d)%s !", errno, strerror(errno));
			}
		}
	}

	LOGI("Listen Exit");

}

EpollTransmit::EpollTransmit(shared_ptr<Mediator> _pMediator)
	:Transmit(_pMediator)
{

}

EpollTransmit::~EpollTransmit()
{

}

void EpollTransmit::Start(int _num)
{
	for (size_t i = 0; i < _num; ++i)
		pEpollVec_.push_back(make_shared<Epoll>(TRANSMIT_EPOLL_EVENTS_NUM));

	Transmit::Start(_num);
}

void EpollTransmit::Terminate()
{
	Transmit::Terminate();
	pEpollVec_.clear();
}

void EpollTransmit::ClientBind(int _socket)
{
	LOGI(__FUNCTION__);

	int index = _socket % ThreadCount_;
	pEpollVec_[index]->RegRecv(_socket);
}

void EpollTransmit::ClientBindSend(int _socket)
{
	LOGI(__FUNCTION__);

	int index = _socket % ThreadCount_;
	pEpollVec_[index]->RegSend(_socket);
}

void EpollTransmit::TransmitFun(int _index)
{
	LOGI("Epoll %02d Transmit Thread Start", _index);

	epoll_event transmitEvents[TRANSMIT_EPOLL_EVENTS_NUM];

	while (IsContinue_)
	{
		int eventNum = pEpollVec_[_index]->TransmitWait(transmitEvents, TRANSMIT_EPOLL_EVENTS_NUM, 1000);

		for (int i = 0; i < eventNum; ++i)
		{
			int transmitSocket = transmitEvents[i].data.fd;

			if ((transmitEvents[i].events & EPOLLERR) || (transmitEvents[i].events & EPOLLHUP) || (transmitEvents[i].events & EPOLLRDHUP))
			{
				LOGI("%d Scoekt Disconnect", transmitSocket);
				ClientDisconnect(transmitSocket);
				continue;
			}

			if (transmitEvents[i].events & EPOLLOUT)
			{
				LOGI("%d Scoekt Send Data", transmitSocket);
				ClientSendData(transmitSocket);
			}

			if (transmitEvents[i].events & EPOLLIN)
			{
				LOGI("%d Scoekt Recv Data", transmitSocket);
				ClientRecvData(transmitSocket);
			}
		}// end of for
	}//end of while

	LOGI("Epoll %02d Transmit Thread Exit", _index);
}

