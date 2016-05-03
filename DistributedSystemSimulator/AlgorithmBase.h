#pragma once
#include "Node.h"

class CNode;

class CAlgorithmBase
{
public:
	CAlgorithmBase();
	~CAlgorithmBase();
	virtual void Run(CNode *io_node) = 0;
	virtual void SetInitialProcess(CNode *io_node) = 0;
};

