#include "Process.h"

Process::Process(shared_ptr<Mediator> _pMediator)
	:pMediator_(_pMediator)
{
	pThreadPool_ = make_shared<ThreadPool>();
	pThreadPool_->Start(5);
}

Process::~Process()
{
	pThreadPool_->Terminate();
}

void Process::AddProcess(int _socket)
{
	function<void()> task = [this, _socket] { this->ProcessClientDataFun(_socket); };
	pThreadPool_->AddTask(task);
}

void Process::ProcessClientDataFun(int _socket)
{
	PClient client = pMediator_->GetClient(_socket);

	if (client != nullptr)
	{
		char* buf = new char[BUF_SIZE];

		size_t popCount = 0;
		do
		{
			memset(buf, 0, BUF_SIZE);
			popCount = client->PopRecvData(buf, BUF_SIZE);

			LOGI("Recv:%s", buf);

			if (popCount > 0)
				client->PushSendData(buf, popCount);
			
		}
		while (popCount > 0);

		pMediator_->ClientSendData(_socket);
		pMediator_->ClientBindRecv(_socket);

		delete[] buf;
	}
}
