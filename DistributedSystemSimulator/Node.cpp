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

CNode::CNode(uint32_t i_memorySize, uint32_t i_pageSize, double i_frequency)
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
	m_cpuUsage = 0;
	m_configuredBandwith = 0;
	m_availableBandwith = 0;
	m_isAlarmUp = false;
	this->PostCreate();
	this->CreateLog();
}

CNode::CNode(std::string i_configFilePath)
{
	m_id = CMasterSingleton::GetInstance()->GetNewNodeId();
	m_algorithm = NULL;
	m_cpuUsage = 0;
	m_availableBandwith = 0;
	m_isAlarmUp = false;

	std::ifstream input_file(i_configFilePath);
	std::string line;
	uint32_t lineNumber = 0;

	bool memorySet = false;
	bool pageSizeSet = false;
	bool frequencySet = false;
	bool algorithmSet = false;
	bool bandwidthSet = false;

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

		if (stringVector[0] == "bandwidth")
		{
			m_configuredBandwith = std::stoi(stringVector[1]);
			bandwidthSet = true;
		}
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
				// this is the round robin algorithm
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
	if (!bandwidthSet)
		throw new CNodeException(CNodeException::kBandwidthNotSet);
	m_timePerTick = 1.0 / m_frequency;
	this->PostCreate();
	this->CreateLog();
	input_file.close();
}

CNode::CNode(CNode* i_node)
{
	m_runningProcessIndex = i_node->m_runningProcessIndex;
	m_id = i_node->m_id;
	m_memorySize = i_node->m_memorySize;
	m_pageSize = i_node->m_pageSize;
	m_frequency = i_node->m_frequency;
	m_ticksDone = i_node->m_ticksDone;
	m_currentTime = i_node->m_currentTime;
	m_currentTimeTemp = i_node->m_currentTimeTemp;
	m_timePerTick = i_node->m_timePerTick;
	m_cpuUsage = i_node->m_cpuUsage;
	m_configuredBandwith = i_node->m_configuredBandwith;
	m_availableBandwith = i_node->m_availableBandwith;
	m_isAlarmUp = i_node->m_isAlarmUp;
	if (m_algorithm)
		delete m_algorithm;
	m_algorithm = i_node->m_algorithm->Clone();
	m_log = CLog(i_node->m_log);
	// This is a delicate process. First we copy all the node mem pages
	// then we copy all the process pages
	// and then we have to link the processes to the proper pages they already had in the first place
	// but with the pointers from the new node
	for (uint32_t i = 0; i < m_nodePageVector.size(); ++i)
	{
		if (m_nodePageVector[i])
			delete m_nodePageVector[i];
	}
	m_nodePageVector.clear();
	for (uint32_t i = 0; i < i_node->m_nodePageVector.size(); ++i)
	{
		CMemPage* temp = new CMemPage(i_node->m_nodePageVector[i]);
		m_nodePageVector.push_back(temp);
	}
	
	for (uint32_t i = 0; i < m_processVector.size(); ++i)
	{
		if (m_processVector[i])
			delete m_processVector[i];
	}
	m_processVector.clear();
	for (uint32_t i = 0; i < i_node->m_processVector.size(); ++i)
	{
		CProcess* temp = new CProcess(i_node->m_processVector[i]);
		temp->LinkToPages(m_nodePageVector);
		m_processVector.push_back(temp);
	}
}

CNode::~CNode()
{
	// Delete all the objects
	if (m_algorithm)
		delete m_algorithm;
	for (uint32_t i = 0; i < m_nodePageVector.size(); ++i)
	{
		if (m_nodePageVector[i])
			delete m_nodePageVector[i];
	}
	m_nodePageVector.clear();
	for (uint32_t i = 0; i < m_nodePageVector.size(); ++i)
	{
		if (m_processVector[i])
			delete m_processVector[i];
	}
	m_processVector.clear();
	if (m_migrationInfo)
		delete m_migrationInfo;
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
		if ((*it)->IsOwned() == false)
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

bool CNode::MemAlloc(uint32_t i_processId, uint32_t memoryRequired)
{
	// This is used when there is no process object (before the migration takes place)
	uint32_t reqPages = uint32_t(ceil(1.0 * memoryRequired / m_pageSize));
	std::vector<CMemPage*>::iterator it = m_nodePageVector.begin();
	while (reqPages > 0 && it != m_nodePageVector.end())
	{
		// If memory page is not owned, add it to the current process
		if ((*it)->IsOwned() == false)
		{
			(*it)->SetOwnerId(i_processId);
			(*it)->MakePageClean();
			(*it)->SetPageOwnership(true);
			--reqPages;
		}
		++it;
	}

	// if we reached the end and reqPages is not 0, we must free the memory and return false, as MemAlloc failed
	if (reqPages > 0)
	{
		for (uint32_t i = 0; i < m_nodePageVector.size(); ++i)
			m_nodePageVector[i]->SetPageOwnership(false);
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
		oLog << boost::format("OK PID %1% MEMREQ %2% DEADLINE %3% STARTTIME %4%") % process->GetId() % process->GetMemoryRequired() % process->GetDeadline() %process->GetWakeUpTime();
		this->WriteLog(oLog.str());

		return true;
	}
	
	std::stringstream oLog;
	oLog << boost::format("FAIL PID %1%") % process->GetId();
	this->WriteLog(oLog.str());

	return rValue;
}

void CNode::AddProcessPostMigration(CProcess* i_process)
{
	// add the process to the destination node (this)
	m_processVector.push_back(i_process);
	// link the pages with the process (the pages are already owned by this process)
	i_process->LinkToPages(m_nodePageVector);
}

bool CNode::RemoveProcess(uint32_t i_processId)
{
	bool rValue = false;
	for (uint32_t i = 0; i < m_processVector.size(); ++i)
	{
		if (m_processVector[i]->GetId() == i_processId)
		{
			m_processVector[i]->ClearMemory();
			m_processVector.erase(m_processVector.begin() + i);
			if (i < m_runningProcessIndex)
				--m_runningProcessIndex;
		}
	}

/*	std::vector<CProcess*>::iterator it = m_processVector.begin();
	while (it != m_processVector.end() && (*it)->GetId() != i_processId)
		++it;
	if (it != m_processVector.end())
	{
		(*it)->ClearMemory();
		m_processVector.erase(it);
		rValue = true;
	}*/
	return rValue;
}

// Run for one processor clock cycle
void CNode::Tick(bool o_deadline)
{
	++m_ticksDone;
	m_algorithm->Run(this);
	m_currentTimeTemp += m_timePerTick;
	// Check if we have a migration going on and if it's time to take action
	if (m_isAlarmUp && m_migrationInfo && m_currentTimeTemp > m_migrationInfo->GetAlarmTime())
	{
		switch (m_migrationInfo->GetState())
		{
		case CMigrationInfo::kStateScheduled:
			m_migrationInfo->StopProcessOnSource();
			break;
		case CMigrationInfo::kStateInitialCopyDone:
			m_migrationInfo->CompleteMigration();
			delete m_migrationInfo;
			break;
		default:
			break;
		}
	}
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
	//m_log.Write(m_tempStringStream.str());
	//m_tempStringStream.str(std::string());
}

double CNode::GetTime()
{
	return m_currentTime;
}

void CNode::PostCreate()
{
	m_availableBandwith = m_configuredBandwith;
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

uint32_t CNode::GetPageSize()
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

void CNode::Init()
{
	m_algorithm->SetInitialProcess(this);
	ResetTicks();
}

void CNode::IncrementNopTicks()
{
	++m_nopTicks;
}

void CNode::IncrementProcTicks()
{
	++m_procTicks;
}

void CNode::ResetTicks()
{
	m_nopTicks = 0;
	m_procTicks = 0;
}

void CNode::ReportStatus()
{
	// Calculate CPU usage first
	// should avoid dividing by 1
	assert((m_nopTicks + m_procTicks) != 0);
	double cpuUsage = 1.0 * m_procTicks / (m_nopTicks + m_procTicks);
	ResetTicks();

	WriteLog(boost::str(boost::format("STATUS: CPU usage: %f \tMEMORY usage: %f") % cpuUsage % GetMemUsage() ));
	m_cpuUsage = cpuUsage;
}

double CNode::GetMemUsage()
{
	// Calculate MEM usage
	int usedPages = 0;
	for (uint32_t i = 0; i < m_nodePageVector.size(); ++i)
	{
		if (m_nodePageVector[i]->IsOwned())
			++usedPages;
	}
	double memUsage = 1.0 * usedPages / m_nodePageVector.size();
	return memUsage;
}

double CNode::GetCpuUsage()
{
	return m_cpuUsage;
}


double CNode::GetFreeMem()
{
	return (1.0 - GetMemUsage()) * m_memorySize;
}


uint32_t CNode::GetAvailableBandwidth()
{
	return m_availableBandwith;
}

void CNode::SetAvailableBandwidth(uint32_t i_newBandwidth)
{
	m_availableBandwith = i_newBandwidth;
	assert(m_availableBandwith <= m_configuredBandwith);
}

uint32_t CNode::GetId()
{
	return m_id;
}

void CNode::SetAlarm(bool i_isAlarmUp)
{
	m_isAlarmUp = i_isAlarmUp;
}

bool CNode::IsAlarmUp()
{
	return m_isAlarmUp;
}

void CNode::SetMigrationInfo(CMigrationInfo* i_migrationInfo)
{
	m_migrationInfo = i_migrationInfo;
}

CAlgorithmBase* CNode::GetProcessorAlgorithm()
{
	return m_algorithm;
}

uint32_t CNode::GetMemSize()
{
	return m_memorySize;
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

/*
Node object adds:
bandwith - configurable from file
availableBandwith - based on current happening migrations
list of MIgrationObjectInfo
alarmTime - set on adding a new object to the list of migration object info and on completing an object from list of migrationobjectinfo
isAlarmUp - used in an "if" in tick to save time that would be spent searching to the migration object list

Process object adds:
isInMIgration bool - to ignore deadlines if it is already in a migration


the migrationinfoobject
pointer to source node
pointer to destination node
migration itime (memPage*memPageSize/minBW)

what it does
on ScheduleMigration:
calculates migration time
cleans all mem pages from src process
reserve the mem pages on destination node

then in the tick method in the node, it will check for the alarm
when the alarm is hit, tthe migration helper object calculates the time left
to move the remaining mem pages to the other node (the dirty ones), but halts 
the process and moves it to the destination node, setting as a wake up time
the time calculated here. Also, it cleans of ownership the mem pages from the src node
And frees the nodes bandwith so they can migrate some more
Also, it must remove the "ignore deadline" from the migrated process when this is done.\
*/