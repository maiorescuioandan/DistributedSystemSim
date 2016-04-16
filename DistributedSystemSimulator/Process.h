#pragma once

#include "Command.h"
#include "CodeBase.h"
#include "MemPage.h"
#include "Utils.h"

class CProcess
{
public:
	CProcess();
	CProcess(CCodeBase i_codeBase, uint64_t i_memoryRequired, uint64_t i_deadline);
	CProcess(std::string i_configFilePath);
	~CProcess();
	uint32_t GetId();
	bool IsRunning();
	uint64_t GetMemoryRequired();
	CCodeBase GetCodeBase();
	uint16_t GetProgramPointer();
	uint64_t GetDeadline();
	uint64_t GetLastDeadlineTick();
	bool AddMemoryPage(CMemPage* i_page);
	bool ClearMemory();
private:
	void Validate();

	uint32_t m_id;
	bool m_isRunning;
	uint64_t m_memoryRequired;
	CCodeBase m_codeBase;
	uint16_t m_programPointer;
	uint64_t m_deadline;
	uint64_t m_startTime;
	uint64_t m_lastDeadlineTick;
	std::vector<CMemPage*> m_processPageVector;
};

// will define exception for each class in its file.
class CProcessException : public std::exception
{
public:
	enum ProcessExceptionEnum{
		kAddressOutsideRange	= 0,
		kDeadlineNotSet			= 1,
		kMemoryRequiredNotSet	= 2,
		kStartTimeNotSet		= 3,
		kExpectedCodeBaseConfig	= 4
	};

	CProcessException(ProcessExceptionEnum i_info, uint32_t i_line = 0);
	ProcessExceptionEnum GetExceptionInfo();
private:
	ProcessExceptionEnum m_info;
	uint32_t m_line;

};

