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
//		//op��������ЧֵΪ��
//		//EPOLL_CTL_ADD�����ļ�������epfd�����õ�epollʵ����ע��Ŀ���ļ�������fd�������¼��¼����ڲ��ļ����ӵ�fd��
//		//EPOLL_CTL_MOD��������Ŀ���ļ�������fd��������¼��¼���
//		//EPOLL_CTL_DEL����epfd���õ�epollʵ����ɾ����ע����Ŀ���ļ�������fd�����¼��������ԣ����ҿ���ΪNULL��
//
//		//events  ��Ա�������������¼�����ļ��ϣ�
//		//EPOLLIN ����ʾ��Ӧ���ļ����������Զ��������Զ�SOCKET�����رգ���
//		//EPOLLOUT����ʾ��Ӧ���ļ�����������д��
//		//EPOLLPRI����ʾ��Ӧ���ļ��������н��������ݿɶ�������Ӧ�ñ�ʾ�д������ݵ�������
//		//EPOLLERR����ʾ��Ӧ���ļ��������������� EPOLLHUP����ʾ��Ӧ���ļ����������Ҷϣ�
//		//EPOLLET�� ��EPOLL��Ϊ��Ե����(Edge Triggered)ģʽ�����������ˮƽ����(Level Triggered)��˵�ġ�
//		//EPOLLONESHOT��ֻ����һ���¼���������������¼�֮���������Ҫ�����������socket�Ļ�����Ҫ�ٴΰ����socket���뵽EPOLL�����
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

