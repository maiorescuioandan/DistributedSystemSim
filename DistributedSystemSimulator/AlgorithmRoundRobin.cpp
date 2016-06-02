#include "stdafx.h"
#include "AlgorithmRoundRobin.h"
#include <iomanip>

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
	if (pProcess->IsRunning())
	{
		io_node->m_tempStringStream << "RUN CrtTime:" << io_node->GetCurrentTimeTemp() << "\tPID:" << pProcess->GetId() << "\tProgramPointer:" << pProcess->GetProgramPointer() << "\tIsMemAccess:" << pProcess->IsCurrentCommandMemAccess() << std::endl;

		// Step 2:
		if (pProcess->IsCurrentCommandMemAccess())
			pProcess->MarkPageAsDirty(io_node->GetPageSize());
		pProcess->IncrementProgramPointer();

		// if this was the last command, we want to do some things
		if (pProcess->GetProgramPointer() == 0)
		{
			// The process reached the deadline and will start next time it's required
			pProcess->SetRunState(false);
			// set the process to wake up after the initial deadline passed
			pProcess->SetWakeUpTime(pProcess->GetLastDeadlineTick() + pProcess->GetDeadline());
			pProcess->SetLastDeadlineTick(pProcess->GetLastDeadlineTick() + pProcess->GetDeadline());
		}
	}
	else
	{
		io_node->m_tempStringStream << "NOP CrtTime:" << io_node->GetCurrentTimeTemp() << std::endl;
	}

	// Step 3:
	--m_ticksLeft;
	if (m_ticksLeft == 0)
	{ 
		io_node->m_tempStringStream << "RR CYCLE END" << std::endl;
		m_ticksLeft = m_ticksPerProcess;
		// After each RR cycle is done, get through all the processes and check if we should update the running state.
		for (uint32_t i = 0; i < io_node->GetProcessCount(); ++i)
		{
			CheckIfProcessCanRun(io_node, io_node->GetProcess(i));
		}

		//Afterwards, find the next process that we can run.If none available, just wait.
		uint32_t initial = io_node->GetRunningProcessIndex();
		// we search for another process to run until we reach the initial process
		// if no process was good, we just stop there and run NOPs on the processor
		do {
			if (io_node->GetRunningProcessIndex() + 1 >= io_node->GetProcessCount())
				io_node->SetRunningProcess(0);
			else
				io_node->SetRunningProcess(io_node->GetRunningProcessIndex() + 1);
		} while (io_node->GetRunningProcessIndex() != initial && !io_node->GetProcess(io_node->GetRunningProcessIndex())->IsRunning());
	}
}

void CAlgorithmRoundRobin::SetInitialProcess(CNode *io_node)
{
	io_node->WriteLog("Searching for an initial process to run...");
	bool processSet = false;
	// For round-robin, we set the first process as the initial one
	io_node->SetRunningProcess(0);
	
	while (!CheckIfProcessCanRun(io_node, io_node->GetProcess(io_node->GetRunningProcessIndex())))
	{
		if (io_node->GetRunningProcessIndex() + 1 >= io_node->GetProcessCount())
		{
			// this is the case where all the processes are sleeping and we just run NOPs on the processor
			io_node->SetRunningProcess(0);
			io_node->WriteLog("No viable process found - will run NOP the next scheduled RR cycle");
			break;
		}
		else
			io_node->SetRunningProcess(io_node->GetRunningProcessIndex() + 1);
	}
}

bool CAlgorithmRoundRobin::CheckIfProcessCanRun(CNode *i_node, CProcess *i_process)
{
	bool canRun = false;

	if (i_process->IsRunning())
		canRun = true;
	else
	{
		// if the process is not running, we start it if the wake up time is bigger than the 
		// node current temp time.
		if (i_process->GetWakeUpTime() <= i_node->GetCurrentTimeTemp())
		{
			i_process->SetRunState(true);
			canRun = true;
		}
	}
	return canRun;
}

CAlgorithmRoundRobin* CAlgorithmRoundRobin::Clone()
{
	CAlgorithmRoundRobin* pReturnAlgorithm = new CAlgorithmRoundRobin(m_ticksPerProcess);
	pReturnAlgorithm->m_ticksLeft = m_ticksLeft;
	return pReturnAlgorithm;
}
