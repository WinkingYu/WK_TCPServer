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

		int ret = epoll_ctl(fd, EPOLL_CTL_ADD, _socket, &event);
		LOGI("ret = %d", ret);
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

	void RegLinsten(int _socket)
	{
		epoll_event regEvent;

		regEvent.data.fd = _socket;
		regEvent.events = EPOLLIN;

		epoll_ctl(fd, EPOLL_CTL_ADD, _socket, &regEvent);
	}

	int ListenWait(int _socket)
	{
		epoll_event event;

		event.data.fd = _socket;
		event.events = EPOLLIN;

		return epoll_wait(fd, &event, 1, 1000);
	}

	int TransmitWait(epoll_event* _event, int _maxEvents, int _time)
	{
		return epoll_wait(fd, _event, _maxEvents, _time);
	}

private:
	int Reg(int _op, int _socket, epoll_event* _event)
	{
		//op��������ЧֵΪ��
		//EPOLL_CTL_ADD�����ļ�������epfd�����õ�epollʵ����ע��Ŀ���ļ�������fd�������¼��¼����ڲ��ļ����ӵ�fd��
		//EPOLL_CTL_MOD��������Ŀ���ļ�������fd��������¼��¼���
		//EPOLL_CTL_DEL����epfd���õ�epollʵ����ɾ����ע����Ŀ���ļ�������fd�����¼��������ԣ����ҿ���ΪNULL��

		//events  ��Ա�������������¼�����ļ��ϣ�
		//EPOLLIN ����ʾ��Ӧ���ļ����������Զ��������Զ�SOCKET�����رգ���
		//EPOLLOUT����ʾ��Ӧ���ļ�����������д��
		//EPOLLPRI����ʾ��Ӧ���ļ��������н��������ݿɶ�������Ӧ�ñ�ʾ�д������ݵ�������
		//EPOLLERR����ʾ��Ӧ���ļ��������������� EPOLLHUP����ʾ��Ӧ���ļ����������Ҷϣ�
		//EPOLLET�� ��EPOLL��Ϊ��Ե����(Edge Triggered)ģʽ�����������ˮƽ����(Level Triggered)��˵�ġ�
		//EPOLLONESHOT��ֻ����һ���¼���������������¼�֮���������Ҫ�����������socket�Ļ�����Ҫ�ٴΰ����socket���뵽EPOLL�����

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
	EpollTransmit(shared_ptr<Mediator> _pMediator)
		:Transmit(_pMediator)
	{
		
	}

	virtual  ~EpollTransmit()
	{
		
	}

	void Start(int _num)
	{
		for (size_t i = 0; i < _num; ++i)
			pEpollVec_.push_back(make_shared<Epoll>(TRANSMIT_EPOLL_EVENTS_NUM));

		Transmit::Start(_num);
	}

	void Terminate()
	{
		Transmit::Terminate();
		pEpollVec_.clear();
	}

	void ClientBind(int _socket) override;

private:
	void TransmitFun(int _index) override;

private:
	vector<shared_ptr<Epoll>> pEpollVec_;
};

