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
	Epoll(int _size);
	~Epoll();

	void RegRecv(int _socket);
	void RegSend(int _socket);
	void RegCancel(int _socket);
	void RegLinsten(int _socket);
	int ListenWait(int _socket);
	int TransmitWait(epoll_event* _event, int _maxEvents, int _time);

//private:
//	int Reg(int _op, int _socket, epoll_event* _event)
//	{
//		//op参数的有效值为：
//		//EPOLL_CTL_ADD：在文件描述符epfd所引用的epoll实例上注册目标文件描述符fd，并将事件事件与内部文件链接到fd。
//		//EPOLL_CTL_MOD：更改与目标文件描述符fd相关联的事件事件。
//		//EPOLL_CTL_DEL：从epfd引用的epoll实例中删除（注销）目标文件描述符fd。该事件将被忽略，并且可以为NULL。
//
//		//events  成员变量可以是以下几个宏的集合：
//		//EPOLLIN ：表示对应的文件描述符可以读（包括对端SOCKET正常关闭）；
//		//EPOLLOUT：表示对应的文件描述符可以写；
//		//EPOLLPRI：表示对应的文件描述符有紧急的数据可读（这里应该表示有带外数据到来）；
//		//EPOLLERR：表示对应的文件描述符发生错误； EPOLLHUP：表示对应的文件描述符被挂断；
//		//EPOLLET： 将EPOLL设为边缘触发(Edge Triggered)模式，这是相对于水平触发(Level Triggered)来说的。
//		//EPOLLONESHOT：只监听一次事件，当监听完这次事件之后，如果还需要继续监听这个socket的话，需要再次把这个socket加入到EPOLL队列里。
//
//		return epoll_ctl(fd, _op, _socket, _event);
//	}
//
//	int Wait(epoll_event* _event, int _max, int _time)
//	{
//		return epoll_wait(fd, _event, _max, _time);
//	}
private:
	int fd;
};

class EpollListen :public Listen
{
	const int SOCKET_BUF_SIZE = 4096;
public:
	EpollListen(shared_ptr<Mediator> _pMediator);
	virtual ~EpollListen();

	void ClientBind(int _socket) override;

private:
	void ListenThreadFun() override;

private:
	shared_ptr<Epoll> pEpoll_;
};

class EpollTransmit :public Transmit
{
	const int TRANSMIT_EPOLL_EVENTS_NUM = 5;
public:
	EpollTransmit(shared_ptr<Mediator> _pMediator);
	virtual  ~EpollTransmit();

	void Start(int _num);
	void Terminate();

	void ClientBind(int _socket) override;

private:
	void TransmitFun(int _index) override;

private:
	vector<shared_ptr<Epoll>> pEpollVec_;
};

