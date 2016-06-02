#pragma once
#include "AlgorithmBase.h"
#include "Node.h"

class CAlgorithmBase;

class CAlgorithmRoundRobin : public CAlgorithmBase
{
public:
	CAlgorithmRoundRobin(uint32_t i_ticksPerProcess);
	virtual ~CAlgorithmRoundRobin();
	virtual void Run(CNode *io_node);
	virtual void SetInitialProcess(CNode *io_node);
	virtual CAlgorithmRoundRobin* Clone();
	bool CheckIfProcessCanRun(CNode *i_node, CProcess *i_process);
private:
	uint32_t m_ticksLeft;
	uint32_t m_ticksPerProcess;
};

