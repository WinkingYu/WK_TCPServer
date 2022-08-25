#pragma once

#include <vector>

#include "Mediator.h"

extern class Mediator;

class Transmit
{
	static const size_t SOCKET_BUF_SIZE = 4096;
public:
	Transmit() = default;
	virtual ~Transmit() = default;

public:
	void Start(int _num)
	{
		ThreadCount_ = _num;
		for (int i = 0; i < ThreadCount_; ++i)
		{
			shared_ptr<thread> pThread = make_shared<thread>(bind(&Transmit::TransmitFun, this, i));
			ThreadVec_.push_back(pThread);
		}

	}

	void Terminate()
	{
		for (auto it : ThreadVec_)
		{
			if (it != nullptr && it->joinable())
				it->join();
		}
		ThreadVec_.clear();
	}

	void SetMediator(shared_ptr<Mediator> _pMediator)
	{
		pMediator_ = _pMediator;
	}

	void ClientDisconnect(int _socket)
	{
		pMediator_->ClientDisconnect(_socket);
	}

	void ClientRecvData(int _socket)
	{
		pMediator_->ClientRecvData(_socket);
	}

	void ClientSendData(int _socket)
	{
		pMediator_->ClientSendData(_socket);
	}

private:
	virtual void TransmitFun(int _index) = 0;

protected:
	atomic<bool> IsContinue_;
	size_t ThreadCount_;

	shared_ptr<Mediator> pMediator_;

private:
	vector<shared_ptr<thread>>  ThreadVec_;
};

