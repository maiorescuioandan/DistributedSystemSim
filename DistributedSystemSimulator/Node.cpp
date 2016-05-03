#include "stdafx.h"
#include "Node.h"
#include "MasterSingleton.h"
#include <boost/format.hpp>
#include "AlgorithmRoundRobin.h"

//////////////////////////////////////////////////////////////////////////
// Node Class implementation below
//////////////////////////////////////////////////////////////////////////

CNode::CNode()
{
}

CNode::CNode(uint64_t i_memorySize, uint64_t i_pageSize, double i_frequency)
{
	m_id = CMasterSingleton::GetInstance()->GetNewNodeId();
	m_memorySize = i_memorySize;
	m_pageSize = i_pageSize;
	m_frequency = i_frequency;
	m_currentTime = 0;
	m_currentTimeTemp = 0;
	m_timePerTick = 1.0 / i_frequency;
	m_ticksDone = 0;
	m_algorithm = NULL;

	this->PostCreate();
	this->CreateLog();
}

CNode::CNode(std::string i_configFilePath)
{
	m_id = CMasterSingleton::GetInstance()->GetNewNodeId();
	m_algorithm = NULL;
	std::ifstream input_file(i_configFilePath);
	std::string line;
	uint32_t lineNumber = 0;
	bool memorySet = false;
	bool pageSizeSet = false;
	bool frequencySet = false;
	bool algorithmSet = false;

	while (std::getline(input_file, line))
	{
		++lineNumber;
		std::vector<std::string> stringVector;
		Utils::SplitStringInVector(line, Utils::SPACE, stringVector);
		// Make the first element to lower, as we decide what to do next base on it
		// and we don't want to make a hard time for the user
		std::transform(stringVector[0].begin(), stringVector[0].end(), stringVector[0].begin(), ::tolower);

		// if there is an empty line, just skip it
		if (stringVector.size() == 0)
			continue;

		if (stringVector[0] == "memorysize")
		{
			m_memorySize = std::stoi(stringVector[1]);
			memorySet = true;
		}
		if (stringVector[0] == "pagesize")
		{
			m_pageSize = std::stoi(stringVector[1]);
			pageSizeSet = true;
		}
		if (stringVector[0] == "frequency")
		{
			m_frequency = std::stof(stringVector[1]);
			frequencySet = true;
		}
		if (stringVector[0] == "algorithm")
		{
			algorithmSet = true;
			if (stringVector[1] == "RR")
			{
				// this is the routing robot algorithm
				m_algorithm = new CAlgorithmRoundRobin(10);
			}
		}

	}

	if (!pageSizeSet)
		throw new CNodeException(CNodeException::kPageSizeNotSet);
	if (!memorySet)
		throw new CNodeException(CNodeException::kMemorySizeNotSet);
	if (!frequencySet)
		throw new CNodeException(CNodeException::kFrequencyNotSet);
	if (!algorithmSet)
		throw new CNodeException(CNodeException::kAlgorithmNotSet);
	m_timePerTick = 1.0 / m_frequency;
	this->PostCreate();
	this->CreateLog();
}

CNode::~CNode()
{
}

bool CNode::MemAlloc(CProcess* i_process)
{
	// Calculate number of required pages
	// Made explicit conversion to avoid warnings
	uint32_t reqPages = uint32_t(ceil(1.0 * i_process->GetMemoryRequired() / m_pageSize));
	std::vector<CMemPage*>::iterator it = m_nodePageVector.begin();
	while (reqPages > 0 && it != m_nodePageVector.end())
	{
		// If memory page is not owned, add it to the current process
		if ((*it)->GetPageOwnership() == false)
		{ 
			i_process->AddMemoryPage(*it);
			--reqPages;
		}
		++it;
	}

	// if we reached the end and reqPages is not 0, we must free the memory and return false, as MemAlloc failed
	if (reqPages > 0)
	{
		i_process->ClearMemory();
		return false;
	}
	return true;
}

bool CNode::AddProcess(CProcess* process)
{
	this->WriteLog("# AddProcessStart");
	bool rValue = false;
	if (MemAlloc(process))
	{
		m_processVector.push_back(process);

		std::stringstream oLog;
		oLog << boost::format("OK PID %1% MEMREQ %2% DEADLINE %3%") % process->GetId() % process->GetMemoryRequired() % process->GetDeadline();
		this->WriteLog(oLog.str());

		return true;
	}
	
	std::stringstream oLog;
	oLog << boost::format("FAIL PID %1%") % process->GetId();
	this->WriteLog(oLog.str());

	return rValue;
}

bool CNode::RemoveProcess(uint32_t i_processId)
{
	bool rValue = false;
	std::vector<CProcess*>::iterator it = m_processVector.begin();
	while (it != m_processVector.end() && (*it)->GetId() != i_processId)
		++it;
	if (it != m_processVector.end())
	{
		(*it)->ClearMemory();
		m_processVector.erase(it);
		rValue = true;
	}
	return rValue;
}

// Run for one processor clock cycle
void CNode::Tick(bool o_deadline)
{
	++m_ticksDone;
	m_algorithm->Run(this);
	m_currentTimeTemp += m_timePerTick;
}

// Run execution for a limited amount of time
// Break on deadline reached
void CNode::RunToTime(double i_time, bool &o_deadline)
{
	if (i_time < m_currentTimeTemp)
		throw new CNodeException(CNodeException::kCannotGoBackInTime);

	o_deadline = false;
	m_currentTimeTemp = m_currentTime;

	while (m_currentTimeTemp < i_time)
	{
		Tick(o_deadline);
		if (o_deadline)
			break;
	}
}

// Push the execution (logs, time)
void CNode::PushRun()
{
	if (m_currentTime == m_currentTimeTemp)
		throw new CNodeException(CNodeException::kCannotPushNothing);
	m_currentTime = m_currentTimeTemp;
	m_log.Write(m_tempStringStream.str());
	m_tempStringStream.str(std::string());
}

double CNode::GetTime()
{
	return m_currentTime;
}

void CNode::PostCreate()
{
	m_tempStringStream.precision(2);
	m_tempStringStream.setf(std::ios::fixed, std::ios::floatfield);

	m_runningProcessIndex = 0;
	CMemPage* memPage;
	for (uint32_t i = 0; i < m_memorySize / m_pageSize; ++i)
	{
		memPage = new CMemPage(i);
		m_nodePageVector.push_back(memPage);
	}
}

void CNode::CreateLog()
{
	std::stringstream ss;
	ss << "Node" << m_id;
	m_log = CLog(ss.str(), false);
	
	std::stringstream oLog;
	oLog << boost::format("Created node with id %1%") % m_id;
	this->WriteLog(oLog.str());
	
	
	CMasterSingleton::GetInstance()->MainLog(oLog.str());

}

void CNode::WriteLog(std::string i_string)
{
	m_log.Write(i_string);
}

uint32_t CNode::GetRunningProcessIndex()
{
	return m_runningProcessIndex;
}

double CNode::GetCurrentTimeTemp()
{
	return m_currentTimeTemp;
}

uint64_t CNode::GetPageSize()
{
	return m_pageSize;
}

void CNode::SetRunningProcess(uint32_t i_processIndex)
{
	m_runningProcessIndex = i_processIndex;
}

uint32_t CNode::GetProcessCount()
{
	return m_processVector.size();
}

CProcess* CNode::GetProcess(uint32_t i_processIndex)
{
	return m_processVector.at(i_processIndex);
}

//////////////////////////////////////////////////////////////////////////
// ProcessException Class implementation below
//////////////////////////////////////////////////////////////////////////

CNodeException::CNodeException(NodeExceptionEnum i_info)
{
	m_info = i_info;
}

CNodeException::NodeExceptionEnum CNodeException::GetExceptionInfo()
{
	return m_info;
}