#pragma once

#include <vector>

#include "Mediator.h"

class Transmit
{
	static const std::size_t SOCKET_BUF_SIZE = 4096;
public:
	Transmit(std::shared_ptr<Mediator> _pMediator)
		:pMediator_(_pMediator)
	{
	};

	virtual ~Transmit() = default;

public:
	void Start(int _num)
	{
		IsContinue_ = true;

		ThreadCount_ = _num;
		for (int i = 0; i < ThreadCount_; ++i)
		{
			shared_ptr<thread> pThread = make_shared<thread>(bind(&Transmit::TransmitFun, this, i));
			ThreadVec_.push_back(pThread);
		}

	}

	void Terminate()
	{
		IsContinue_ = false;

		for (auto it : ThreadVec_)
		{
			if (it != nullptr && it->joinable())
				it->join();
		}
		ThreadVec_.clear();
	}

	void ClientDisconnect(int _socket);
	void ClientRecvData(int _socket);
	void ClientSendData(int _socket);

private:
	virtual void TransmitFun(int _index) = 0;

protected:
	atomic<bool> IsContinue_;
	size_t ThreadCount_;

	shared_ptr<Mediator> pMediator_;

private:
	vector<shared_ptr<thread>>  ThreadVec_;
};

