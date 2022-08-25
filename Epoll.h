#pragma once

#include <sys/epoll.h>

#include "Listen.h"
#include "Transmit.h"

class Epoll
{
	const int TRANSMIT_EPOLL_EVENTS_NUM = 5;
public:
	Epoll(Epoll const&) = delete;
	Epoll(Epoll&&) = delete;
	Epoll& operator=(Epoll const&) = delete;
	Epoll& operator=(Epoll&&) = delete;

	Epoll() = delete;

public:
	Epoll(int _size)
	{
		fd = epoll_create(_size);
	}

	~Epoll()
	{
		close(fd);
	}

	void RegRecv(int _socket)
	{
		epoll_event event;

		event.data.fd = _socket;
		event.events = EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLERR | EPOLLHUP;

		Reg(EPOLL_CTL_ADD, _socket, &event);
	}

	void RegSend(int _socket)
	{
		epoll_event event;

		event.data.fd = _socket;
		event.events = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLRDHUP | EPOLLERR | EPOLLHUP;

		epoll_ctl(fd, EPOLL_CTL_MOD, _socket, &event);
	}

	void RegCancel(int _socket)
	{
		epoll_event event;

		epoll_ctl(fd, EPOLL_CTL_DEL, _socket, &event);
	}

	int ListenWait(int _socket)
	{
		epoll_event event;

		event.data.fd = _socket;
		event.events = EPOLLIN;

		return Wait(&event, 1, 1000);
	}

	int TransmitWait(epoll_event* _event, int _maxEvents, int _time)
	{
		return Wait(_event, _maxEvents, _time);
	}

private:
	int Reg(int _op, int _socket, epoll_event* _event)
	{
		//op参数的有效值为：
		//EPOLL_CTL_ADD：在文件描述符epfd所引用的epoll实例上注册目标文件描述符fd，并将事件事件与内部文件链接到fd。
		//EPOLL_CTL_MOD：更改与目标文件描述符fd相关联的事件事件。
		//EPOLL_CTL_DEL：从epfd引用的epoll实例中删除（注销）目标文件描述符fd。该事件将被忽略，并且可以为NULL。

		//events  成员变量可以是以下几个宏的集合：
		//EPOLLIN ：表示对应的文件描述符可以读（包括对端SOCKET正常关闭）；
		//EPOLLOUT：表示对应的文件描述符可以写；
		//EPOLLPRI：表示对应的文件描述符有紧急的数据可读（这里应该表示有带外数据到来）；
		//EPOLLERR：表示对应的文件描述符发生错误； EPOLLHUP：表示对应的文件描述符被挂断；
		//EPOLLET： 将EPOLL设为边缘触发(Edge Triggered)模式，这是相对于水平触发(Level Triggered)来说的。
		//EPOLLONESHOT：只监听一次事件，当监听完这次事件之后，如果还需要继续监听这个socket的话，需要再次把这个socket加入到EPOLL队列里。

		return epoll_ctl(fd, _op, _socket, _event);
	}

	int Wait(epoll_event* _event, int _max, int _time)
	{
		return epoll_wait(fd, _event, _max, _time);
	}
	

private:
	int fd;
};

class EpollListen :public Listen
{
	const int SOCKET_BUF_SIZE = 4096;
public:
	EpollListen(shared_ptr<Mediator> _pMediator)
		:Listen(_pMediator)
	{
		pEpoll_ = make_shared<Epoll>(1);

	}

	virtual ~EpollListen()
	{
		
	}

private:
	virtual void ListenThreadFun() override
	{
		int socket = pListenSocket_->GetSocket();
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
					LOGI("New Connect(%d) %s:%u", clientSocket, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
				}
				else
				{
					close(clientSocket);
					LOGE("Accept Error:(%d)%s !", errno, strerror(errno));
				}
			}
		}
		
	}

private:
	shared_ptr<Epoll> pEpoll_;
};

class Mediator;

class EpollTransmit :public Transmit
{
	const int TRANSMIT_EPOLL_EVENTS_NUM = 10;
public:
	EpollTransmit(shared_ptr<Mediator> _pMediator)
		:Transmit(_pMediator)
	{
		for (int i = 0; i < ThreadCount_; ++i)
			pEpollVec_[i] = make_shared<Epoll>(TRANSMIT_EPOLL_EVENTS_NUM);
	}

	virtual  ~EpollTransmit()
	{
	}
private:
	virtual void TransmitFun(int _index)
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
					ClientDisconnect(transmitSocket);
					continue;
				}

				if (transmitEvents[i].events & EPOLLOUT)
				{
					ClientSendData(transmitSocket);
				}

				if (transmitEvents[i].events & EPOLLIN)
				{
					ClientRecvData(transmitSocket);
				}
			}// end of for
		}//end of while

		LOGI("Epoll %02d Transmit Thread Exit", _index);
	}

private:
	vector<shared_ptr<Epoll>> pEpollVec_;
};

