#pragma once
#include "AlgorithmBase.h"
#include "Node.h"

class CAlgorithmRoundRobin : public CAlgorithmBase
{
public:
	CAlgorithmRoundRobin(uint32_t i_ticksPerProcess);
	~CAlgorithmRoundRobin();
	virtual void Run(CNode *io_node);
private:
	uint32_t m_ticksLeft;
	uint32_t m_ticksPerProcess;
};

