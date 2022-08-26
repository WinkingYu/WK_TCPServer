#include "Transmit.h"


void Transmit::Start(int _num)
{
	IsContinue_ = true;

	ThreadCount_ = _num;

	for (int i = 0; i < ThreadCount_; ++i)
	{
		shared_ptr<thread> pThread = make_shared<thread>(bind(&Transmit::TransmitFun, this, i));
		ThreadVec_.push_back(pThread);
	}

}

void Transmit::Terminate()
{
	IsContinue_ = false;

	for (auto it : ThreadVec_)
	{
		if (it != nullptr && it->joinable())
			it->join();
	}
	ThreadVec_.clear();
}

void Transmit::ClientDisconnect(int _socket)
{
	pMediator_->ClientDisconnect(_socket);
}

void Transmit::ClientRecvData(int _socket)
{
	pMediator_->ClientRecvData(_socket);
}

void Transmit::ClientSendData(int _socket)
{
	pMediator_->ClientSendData(_socket);
}
