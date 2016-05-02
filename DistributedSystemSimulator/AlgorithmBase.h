#pragma once
#include "Node.h"

class CAlgorithmBase
{
public:
	CAlgorithmBase();
	~CAlgorithmBase();
	virtual void Run(CNode &io_node) = 0;
};

