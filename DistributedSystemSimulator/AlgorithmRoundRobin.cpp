#include "stdafx.h"
#include "AlgorithmRoundRobin.h"


CAlgorithmRoundRobin::CAlgorithmRoundRobin(uint32_t i_ticksPerProcess)
{
	m_ticksPerProcess = i_ticksPerProcess;
	m_ticksLeft = m_ticksPerProcess;
}


CAlgorithmRoundRobin::~CAlgorithmRoundRobin()
{
}

void CAlgorithmRoundRobin::Run(CNode *io_node)
{
	CProcess* pProcess = io_node->GetProcess(io_node->GetRunningProcessIndex());
	// This is simulating the tick
	// Step 1 : we want to print: run %crtTime% %pid% %programCounter% %isMemAcess%
	// Step 2 : and make the page dirty if that's the case.
	// Step 3 : Decrement the round-robin tick counter and see if we need to change the process
	// We don't need to be concerned about incrementing the time since that is the node's job
	
	// Step 1:
	// crtTime = io_node->GetCurrentTimeTemp();
	// pid = pProcess->GetId()
	// programPointer = pProcess->GetProgramPointer()
	// isMemAcess = pProcess->IsNextCommandMemAccess();
	io_node->m_tempStringStream << "RUN " << io_node->GetCurrentTimeTemp() << " " << pProcess->GetId() << " " << pProcess->GetProgramPointer() << " " << pProcess->IsNextCommandMemAccess() << std::endl;

	// Step 2:
	if (pProcess->IsNextCommandMemAccess())
		pProcess->MarkPageAsDirty(io_node->GetPageSize());

	// Step 3:
	--m_ticksLeft;
	if (m_ticksLeft == 0)
	{ 
		if (io_node->GetRunningProcessIndex() + 1 >= io_node->GetProcessCount())
			io_node->SetRunningProcess(0);
		else
			io_node->SetRunningProcess(io_node->GetRunningProcessIndex() + 1);
	}
}