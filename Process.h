#pragma once

#include <iostream>

#include "../WK_BASE/Log.h"

#include "Mediator.h"


class Process
{
public:
	Process(Process const&) = delete;
	Process(Process&&) = delete;
	Process& operator=(Process const&) = delete;
	Process& operator=(Process&&) = delete;

public:
	Process(std::share_ptr<Mediator> _pMediator);
	virtual ~Process();
	

};

