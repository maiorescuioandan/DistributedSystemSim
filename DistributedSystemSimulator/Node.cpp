#include "stdafx.h"
#include "Node.h"
#include "MasterSingleton.h"

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/make_shared.hpp>

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
	m_timePerTick = 1 / i_frequency;
	m_ticksDone = 0;

	this->CreateMemPages();
}

CNode::CNode(std::string i_configFilePath)
{
	m_id = CMasterSingleton::GetInstance()->GetNewNodeId();
	std::ifstream input_file(i_configFilePath);
	std::string line;
	uint32_t lineNumber = 0;
	bool memorySet = false;
	bool pageSizeSet = false;
	bool frequencySet = false;

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

	}

	if (!pageSizeSet)
		throw new CNodeException(CNodeException::kPageSizeNotSet);
	if (!memorySet)
		throw new CNodeException(CNodeException::kMemorySizeNotSet);
	if (!frequencySet)
		throw new CNodeException(CNodeException::kFrequencyNotSet);

	this->CreateMemPages();
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
	bool rValue = false;
	if (MemAlloc(process))
	{
		m_processVector.push_back(process);
		return true;
	}
	
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
}

double CNode::GetTime()
{
	return m_currentTime;
}

void CNode::CreateMemPages()
{
	CMemPage* memPage;
	for (uint32_t i = 0; i < m_memorySize / m_pageSize; ++i)
	{
		memPage = new CMemPage(i);
		m_nodePageVector.push_back(memPage);
	}
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