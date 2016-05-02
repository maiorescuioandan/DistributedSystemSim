#include "stdafx.h"
#include "MasterSingleton.h"

CMasterSingleton* CMasterSingleton::s_instance = NULL;
CLog CMasterSingleton::m_log = CLog();

CMasterSingleton::CMasterSingleton()
{
	m_processId = 0;
	m_nodeId = 0;
}

CMasterSingleton::~CMasterSingleton()
{
}

CMasterSingleton* CMasterSingleton::GetInstance()
{
	if (!s_instance)
	{
		s_instance = new CMasterSingleton();
		CLog::SetCreationTimeAndDate();
		CreateLogFile();
	}
	return s_instance;
}

uint32_t CMasterSingleton::GetNewProcessId()
{
	// increment after ret
	return m_processId++;
}

uint32_t CMasterSingleton::GetNewNodeId()
{
	// increment after ret
	return m_nodeId++;
}

void CMasterSingleton::CreateLogFile()
{
	m_log = CLog("MainLog", true);
}

void CMasterSingleton::MainLog(std::string i_string)
{
	m_log.Write(i_string);
}