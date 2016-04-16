#pragma once

#include "Process.h"
#include "MemPage.h"

class CNode
{
public:
	CNode();
	~CNode();
	CNode(uint64_t i_memorySize, uint64_t i_pageSize, double i_frequency);
	CNode(std::string i_configFilePath);
	bool MemAlloc(CProcess* process);
	bool AddProcess(CProcess* process);
	bool RemoveProcess(uint32_t i_processId);
	void RunToTime(double i_time, bool &o_deadline);
	void PushRun();

	// Getters / setters
	double GetTime();

	void CreateLogFile();
	void TestLogFile();
private:
	// Methods
	void Tick(bool o_deadline);
	void CreateMemPages();

	uint32_t m_id;
	// Configuration 
	uint64_t m_memorySize;
	uint64_t m_pageSize;
	double	 m_frequency;
	std::vector<CMemPage*> m_nodePageVector;
	//CAlgorithm
	
	// Time
	uint64_t	m_ticksDone;
	double		m_currentTime;
	double		m_currentTimeTemp;
	double		m_timePerTick;

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
		kFrequencyNotSet = 4
	};

	CNodeException(NodeExceptionEnum i_info);
	NodeExceptionEnum GetExceptionInfo();
private:
	NodeExceptionEnum m_info;
};