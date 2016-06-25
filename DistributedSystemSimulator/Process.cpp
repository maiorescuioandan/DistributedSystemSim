#include "stdafx.h"
#include "Process.h"
#include "MasterSingleton.h"

//////////////////////////////////////////////////////////////////////////
// Process Class implementation below
//////////////////////////////////////////////////////////////////////////

CProcess::CProcess()
{
	m_isRunning = false;
	m_sleep = false;
	m_hasMemAllocated = false;
}

CProcess::CProcess(CCodeBase i_codeBase, uint32_t i_memoryRequired, uint32_t i_deadline)
{
	m_isRunning = false;
	m_id = CMasterSingleton::GetInstance()->GetNewProcessId();
	m_codeBase = i_codeBase;
	m_memoryRequired = i_memoryRequired;
	m_deadline = i_deadline;
	m_markedDeadlineMissed = false;
	m_sleep = false;
	m_hasMemAllocated = false;
	this->Validate();
}

CProcess::CProcess(std::string i_configFilePath)
{
	m_sleep = false;
	m_isRunning = false;
	m_lastDeadlineTick = 0;
	m_started = false;
	m_hasMemAllocated = false;
	m_markedDeadlineMissed = false;
	m_id = CMasterSingleton::GetInstance()->GetNewProcessId();
	std::ifstream input_file(i_configFilePath);
	std::string line;
	uint32_t lineNumber = 0;
	bool inCodeBase = false;
	bool memorySet = false;
	bool deadlineSet = false;
	bool startTimeSet = false;

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

		if (!inCodeBase)
		{
			if (stringVector[0] == "requiredmemory")
			{
				m_memoryRequired = std::stoi(stringVector[1]);
				memorySet = true;
			}
			if (stringVector[0] == "deadline")
			{
				m_deadline = std::stoi(stringVector[1]);
				deadlineSet = true;
			}
			if (stringVector[0] == "starttime")
			{
				m_wakeUpTime = std::stoi(stringVector[1]);
				startTimeSet = true;
			}
			if (stringVector[0] == "codebase")
			{
				inCodeBase = true;
				assert(m_codeBase.GetCommandCount() == 0);
				
				if (!deadlineSet)
					throw new CProcessException(CProcessException::kDeadlineNotSet, lineNumber);
				if (!memorySet)
					throw new CProcessException(CProcessException::kMemoryRequiredNotSet, lineNumber);
				if (!startTimeSet)
					throw new CProcessException(CProcessException::kStartTimeNotSet, lineNumber);
			}
		}
		else
		{
			if (stringVector.size() != 2 && stringVector.size() != 3 && stringVector.size() != 0)
				throw new CProcessException(CProcessException::kExpectedCodeBaseConfig, lineNumber);
			if (stringVector.size() == 2)
				m_codeBase.AddCommand(stoi(stringVector[0]) != 0, stoi(stringVector[1]));
			if (stringVector.size() == 3)
				m_codeBase.AddCommand(stoi(stringVector[0]) != 0, stoi(stringVector[1]), stoi(stringVector[2]));
		}

	}
	this->Validate();
}

CProcess::CProcess(CProcess* i_process)
{
	m_id = i_process->m_id;
	m_isRunning = i_process->m_isRunning;
	m_memoryRequired = i_process->m_memoryRequired;
	m_codeBase = i_process->m_codeBase;
	m_programPointer = i_process->m_programPointer;
	m_deadline = i_process->m_deadline;
	m_startTime = i_process->m_startTime;
	m_started = i_process->m_started;
	m_lastDeadlineTick = i_process->m_lastDeadlineTick;
	m_wakeUpTime = i_process->m_wakeUpTime;
	m_markedDeadlineMissed = i_process->m_markedDeadlineMissed;
	m_hasMemAllocated = i_process->m_hasMemAllocated;
	//std::vector<CMemPage*> m_processPageVector;
}

CProcess::~CProcess()
{
	for (uint32_t i = 0; i < m_processPageVector.size(); ++i)
	{
		if (m_processPageVector[i])
			delete m_processPageVector[i];
	}
	m_processPageVector.clear();
}

uint32_t CProcess::GetId()
{
	return m_id;
}

bool CProcess::IsRunning()
{
	return m_isRunning;
}

void CProcess::SetRunState(bool i_isRunning)
{
	m_isRunning = i_isRunning;
}

uint32_t CProcess::GetMemoryRequired()
{
	return m_memoryRequired;
}

CCodeBase CProcess::GetCodeBase()
{
	return m_codeBase;
}

uint16_t CProcess::GetProgramPointer()
{
	return m_programPointer;
}

uint32_t CProcess::GetDeadline()
{
	return m_deadline;
}

double CProcess::GetLastDeadlineTick()
{
	return m_lastDeadlineTick;
}

bool CProcess::AddMemoryPage(CMemPage* i_page)
{
	i_page->SetOwnerId(m_id);
	i_page->MakePageClean();
	i_page->SetPageOwnership(true);
	m_processPageVector.push_back(i_page);
	return true;
}

bool CProcess::ClearMemory()
{
	// Must test that this code works as expected.
	// Based on http://stackoverflow.com/questions/9927163/erase-element-in-vector-while-iterating-the-same-vector
	// if we erase elements while iterating, "it" changes its value to the next element dinamically, so we don't
	// need to actually change the iterator position
	//for (std::vector<CMemPage*>::iterator it = m_processPageVector.begin(); m_processPageVector.size() != 0;)
	//{
	//	(*it)->SetPageOwnership(false);
	//	m_processPageVector.erase(it);
	//}
	while (m_processPageVector.size() > 0)
	{
		CMemPage* pMemPage = m_processPageVector[0];
		pMemPage->SetPageOwnership(false);
		m_processPageVector.erase(m_processPageVector.begin());
	}
	return true;
}

void CProcess::Validate()
{
	// substract 1 from m_memoryRequired
	// saying we need a memory size of 100 means
	// we have register addresses from 0 to 99
	if (m_codeBase.GetMaxAddr() > m_memoryRequired - 1)
		throw new CProcessException(CProcessException::kAddressOutsideRange);

	m_programPointer = 0;
}

void CProcess::IncrementProgramPointer()
{
	++m_programPointer;
	if (m_programPointer >= m_codeBase.GetCommandCount())
		m_programPointer = 0;
}

bool CProcess::IsCurrentCommandMemAccess()
{
	return m_codeBase.IsCommandMemAccess(m_programPointer);
}

void CProcess::MarkPageAsDirty(uint32_t i_pageSize)
{
	uint32_t addressIndex = m_codeBase.GetAddressIndex(m_programPointer);
	uint32_t pageNumber = floor(1.0 * addressIndex / i_pageSize);
	assert(pageNumber < m_processPageVector.size());
	m_processPageVector.at(pageNumber)->MakePageDirty();
}

void CProcess::SetLastDeadlineTick(double i_lastDeadlineTick)
{
	m_lastDeadlineTick = i_lastDeadlineTick;
}

void CProcess::SetWakeUpTime(double i_wakeUpTime)
{
	m_wakeUpTime = i_wakeUpTime;
}

double CProcess::GetWakeUpTime()
{
	return m_wakeUpTime;
}

void CProcess::LinkToPages(std::vector<CMemPage*> i_memPageVector)
{
	for (uint32_t i = 0; i < i_memPageVector.size(); ++i)
	{
		if (i_memPageVector[i]->GetOwnerId() == m_id)
			m_processPageVector.push_back(i_memPageVector[i]);
	}
}

uint32_t CProcess::GetMemPageCount(bool i_countOnlyDirty /*= false*/)
{
	// After the first migration cycle, we only want to count the dirty pages
	uint32_t result = m_processPageVector.size();
	if (i_countOnlyDirty)
	{
		result = 0;
		for (uint32_t i = 0; i < m_processPageVector.size(); ++i)
			if (!m_processPageVector[i]->IsClean())
				++result;
	}
	return result;
}

void CProcess::CleanMemory()
{
	for (uint32_t i = 0; i < m_processPageVector.size(); ++i)
	{
		m_processPageVector[i]->MakePageClean();
	}
}

void CProcess::SetMarkedDeadlineMissed(bool i_markedDeadlineMissed)
{
	m_markedDeadlineMissed = i_markedDeadlineMissed;
}

bool CProcess::GetMarkedDeadlineMissed()
{
	return m_markedDeadlineMissed;
}

void CProcess::SetSleep(bool i_sleep)
{
	m_sleep = i_sleep;
}

bool CProcess::IsSleeping()
{
	return m_sleep;
}

bool CProcess::HasMemAllocated()
{
	return m_hasMemAllocated;
}

void CProcess::SetMemAllocated(bool i_hasMemAllocated)
{
	m_hasMemAllocated = i_hasMemAllocated;
}

//////////////////////////////////////////////////////////////////////////
// ProcessException Class implementation below
//////////////////////////////////////////////////////////////////////////

CProcessException::CProcessException(ProcessExceptionEnum i_info, uint32_t i_line)
{
	m_info = i_info;
	m_line = i_line;
}

CProcessException::ProcessExceptionEnum CProcessException::GetExceptionInfo()
{
	return m_info;
}