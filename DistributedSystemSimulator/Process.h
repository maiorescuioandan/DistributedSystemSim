#pragma once

#include "Command.h"
#include "CodeBase.h"
#include "MemPage.h"
#include "Utils.h"

class CProcess
{
public:
	CProcess();
	CProcess(CCodeBase i_codeBase, uint32_t i_memoryRequired, uint32_t i_deadline);
	CProcess(std::string i_configFilePath);
	CProcess(CProcess* i_process);
	~CProcess();
	uint32_t GetId();
	bool IsRunning();
	void SetRunState(bool i_isRunning);
	uint32_t GetMemoryRequired();
	CCodeBase GetCodeBase();
	uint16_t GetProgramPointer();
	void IncrementProgramPointer();
	uint32_t GetDeadline();
	double GetLastDeadlineTick();
	void SetLastDeadlineTick(double i_lastDeadlineTick);
	bool AddMemoryPage(CMemPage* i_page);
	bool ClearMemory();
	void MarkPageAsDirty(uint32_t i_pageSize);
	bool IsCurrentCommandMemAccess();
	void SetWakeUpTime(double i_wakeUpTime);
	double GetWakeUpTime();
	void LinkToPages(std::vector<CMemPage*> i_memPageVector);
	uint32_t GetMemPageCount(bool i_countOnlyDirty = false);
	void CleanMemory();
	void SetMarkedDeadlineMissed(bool i_markedDeadlineMissed);
	bool GetMarkedDeadlineMissed();
	void SetSleep(bool i_sleep);
	bool IsSleeping();
	bool HasMemAllocated();
	void SetMemAllocated(bool i_hasMemAllocated);
private:
	void Validate();

	uint32_t	m_id;
	bool		m_isRunning;
	uint32_t	m_memoryRequired;
	CCodeBase	m_codeBase;
	uint16_t	m_programPointer;
	uint32_t	m_deadline;
	uint32_t	m_startTime;
	bool		m_started;
	double		m_lastDeadlineTick;
	double		m_wakeUpTime;
	bool		m_markedDeadlineMissed;
	bool		m_sleep;
	bool		m_hasMemAllocated;
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

