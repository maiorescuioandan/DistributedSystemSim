#include "stdafx.h"
#include "Process.h"
#include "MasterSingleton.h"

//////////////////////////////////////////////////////////////////////////
// Process Class implementation below
//////////////////////////////////////////////////////////////////////////

CProcess::CProcess()
{
	m_isRunning = false;
}

CProcess::CProcess(CCodeBase i_codeBase, uint64_t i_memoryRequired, uint64_t i_deadline)
{
	m_isRunning = false;
	m_startTime = 0;
	m_id = CMasterSingleton::GetInstance()->GetNewProcessId();
	m_codeBase = i_codeBase;
	m_memoryRequired = i_memoryRequired;
	m_deadline = i_deadline;
	
	this->Validate();
}

CProcess::CProcess(std::string i_configFilePath)
{
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
				m_startTime = std::stoi(stringVector[1]);
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

CProcess::~CProcess()
{
}

uint32_t CProcess::GetId()
{
	return m_id;
}

bool CProcess::IsRunning()
{
	return m_isRunning;
}

uint64_t CProcess::GetMemoryRequired()
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

uint64_t CProcess::GetDeadline()
{
	return m_deadline;
}

uint64_t CProcess::GetLastDeadlineTick()
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

bool CProcess::IsNextCommandMemAccess()
{
	return m_codeBase.IsCommandMemAccess(m_programPointer);
}

void CProcess::MarkPageAsDirty(uint64_t i_pageSize)
{
	uint64_t addressIndex = m_codeBase.GetAddressIndex(m_programPointer);
	int pageNumber = floor(1.0 * addressIndex / i_pageSize);
	assert(pageNumber < m_processPageVector.size() - 1);
	m_processPageVector.at(pageNumber)->MakePageDirty();
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