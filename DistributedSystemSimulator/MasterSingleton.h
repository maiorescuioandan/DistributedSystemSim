#pragma once
#include "Log.h"

class CMasterSingleton
{
public:
	CMasterSingleton();
	~CMasterSingleton();
	static CMasterSingleton* GetInstance();
	uint32_t GetNewProcessId();
	uint32_t GetNewNodeId();

	static CMasterSingleton* s_instance;
	void MainLog(std::string i_string);

private:
	static void CreateLogFile();

	static CLog m_log;
	uint32_t	m_processId;
	uint32_t	m_nodeId;
};

