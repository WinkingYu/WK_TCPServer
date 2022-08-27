#pragma once

#include <iostream>

#include "../WK_BASE/Log.h"
#include "../WK_BASE/ThreadPool.h"

#include "Mediator.h"


class Process
{
	static const size_t BUF_SIZE = 4096;

public:
	Process(Process const&) = delete;
	Process(Process&&) = delete;
	Process& operator=(Process const&) = delete;
	Process& operator=(Process&&) = delete;

public:
	Process(shared_ptr<Mediator> _pMediator);
	virtual ~Process();

	void AddProcess(int _socket);

	void ProcessClientDataFun(int _socket);

protected:
	shared_ptr<Mediator> pMediator_;

private:
	shared_ptr<ThreadPool> pThreadPool_;

};

