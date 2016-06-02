#pragma once

#include "Process.h"
#include "MemPage.h"
#include "Log.h"
#include "AlgorithmBase.h"

class CAlgorithmBase;

class CNode
{
public:
	CNode();
	~CNode();
	CNode(uint64_t i_memorySize, uint64_t i_pageSize, double i_frequency);
	CNode(std::string i_configFilePath);
	CNode(CNode* i_node);
	bool MemAlloc(CProcess* process);
	bool AddProcess(CProcess* process);
	bool RemoveProcess(uint32_t i_processId);
	void RunToTime(double i_time, bool &o_deadline);
	void PushRun();
	void WriteLog(std::string i_string);

	// Getters / setters
	double GetTime();
	double GetCurrentTimeTemp();
	uint64_t GetPageSize();
	uint32_t GetRunningProcessIndex();
	void SetRunningProcess(uint32_t i_processIndex);
	uint32_t GetProcessCount();
	CProcess* GetProcess(uint32_t i_processIndex);
	void Init();
	// the stream must be public since it is not copyable
	std::stringstream m_tempStringStream;
private:
	// Methods
	void Tick(bool o_deadline);
	void PostCreate();
	void CreateLog();
	
	uint32_t m_runningProcessIndex;
	uint32_t m_id;
	// Configuration 
	uint64_t m_memorySize;
	uint64_t m_pageSize;
	double	 m_frequency;
	std::vector<CMemPage*> m_nodePageVector;
	//CAlgorithm
	CAlgorithmBase *m_algorithm;
	// Time
	
	//counts how many ticks have been done since the program processor is running
	uint64_t	m_ticksDone;
	double		m_currentTime;
	double		m_currentTimeTemp;
	double		m_timePerTick;
	
	CLog		m_log;

	std::vector<CProcess*> m_processVector;
};

// will define exception for each class in its file.
class CNodeException : public std::exception
{
public:
	enum NodeExceptionEnum{
		kCannotGoBackInTime = 0,
		kCannotPushNothing = 1,
		kPageSizeNotSet = 2,
		kMemorySizeNotSet = 3,
		kFrequencyNotSet = 4,
		kAlgorithmNotSet = 5
	};

	CNodeException(NodeExceptionEnum i_info);
	NodeExceptionEnum GetExceptionInfo();
private:
	NodeExceptionEnum m_info;
};