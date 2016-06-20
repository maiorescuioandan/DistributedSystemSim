#include "stdafx.h"
#include "MasterSingleton.h"

CMasterSingleton* CMasterSingleton::s_instance = NULL;
CLog CMasterSingleton::m_log = CLog();

CMasterSingleton::CMasterSingleton()
{
	m_processId = 0;
	m_nodeId = 0;
	m_statusReportCycle = 100;
	m_currentStatusReportCycle = 0;
	m_memoryMigrationTreshold = 1;
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

void CMasterSingleton::AddNode(CNode* i_node)
{
	i_node->Init();
	m_mainNodeVector.push_back(i_node);
}

CNode* CMasterSingleton::GetNode(uint32_t i_nodeIndex)
{
	// we need to make sure we call this with an actual node index
	assert(i_nodeIndex < m_mainNodeVector.size());
	return m_mainNodeVector[i_nodeIndex];
}

void CMasterSingleton::CreateNodeBackup()
{
	//m_backupNodeVector.clear();
	//for (std::vector<CNode*>::iterator it = m_mainNodeVector.begin(); it != m_mainNodeVector.end(); ++it) {
	//	CNode object = *it;
	//	m_backupNodeVector.push_back(new CNode(&object));
	//}
}

void CMasterSingleton::RevertNodeFromBackup()
{

}

void CMasterSingleton::RunToTime(double i_time)
{
	while (1)	
	{
		double minTime = m_mainNodeVector[0]->GetTime();
		int index = 0;
		for (uint32_t i = 1; i < m_mainNodeVector.size(); ++i)
		{
			if (m_mainNodeVector[i]->GetTime() < minTime)
			{
				index = i;
				minTime = m_mainNodeVector[i]->GetTime();
			}
		}
		if (minTime > i_time)
			return;
		bool o_deadline = false;

		if (minTime > GetCurrentStatusReportCycle())
		{
			// Report to log here
			for (uint32_t i = 0; i < m_mainNodeVector.size(); ++i)
			{
				m_mainNodeVector[i]->ReportStatus();
			}
			IncrementStatusReportCycle();

			if (IsMigrationEnabled())
			{
				for (uint32_t i = 0; i < m_mainNodeVector.size(); ++i)
				{
					if (m_mainNodeVector[i]->GetMemUsage() > m_memoryMigrationTreshold && !m_mainNodeVector[i]->IsAlarmUp())
					{
						// 	we should migrate something from this node to another node
						CMigrationInfo* migrationInfo = new CMigrationInfo();
						CNode* destinationNode = migrationInfo->FindNodeWithFreeMem(m_mainNodeVector[i], NULL);
						if (!destinationNode)
							continue;
						CProcess* process = migrationInfo->FindProcessToMigrateOnMemOverflow(m_mainNodeVector[i], destinationNode);
						if (!process)
							continue;
						if (!migrationInfo->ScheduleMigration(m_mainNodeVector[i], destinationNode, process))
							delete migrationInfo;
					}
				}
			}
		}

		m_mainNodeVector[index]->Tick(o_deadline);
		m_mainNodeVector[index]->PushRun();
	}
}

void CMasterSingleton::SetStatusReportCycle(uint32_t i_statusReportCycle)
{
	// we're setting both as  we want to run the first check after the first cycle, not before
	m_statusReportCycle = i_statusReportCycle;
	m_currentStatusReportCycle = i_statusReportCycle;
}

uint32_t CMasterSingleton::GetCurrentStatusReportCycle()
{
	return m_currentStatusReportCycle;
}

// we report the mem usage and cpu usage in the log once every m_statusReportCycle time units;
void CMasterSingleton::IncrementStatusReportCycle()
{
	m_currentStatusReportCycle += m_statusReportCycle;
}

void CMasterSingleton::SetEnableMigration(bool i_enableMigration)
{
	m_enableMigration = i_enableMigration;
}

bool CMasterSingleton::IsMigrationEnabled()
{
	return m_enableMigration;
}

uint32_t CMasterSingleton::GetNodeCount()
{
	return m_mainNodeVector.size();
}

void CMasterSingleton::SetMemoryMigrationTreshold(double i_memoryMigrationTreshold)
{
	m_memoryMigrationTreshold = i_memoryMigrationTreshold;
}

double CMasterSingleton::GetMemoryMigrationTreshold()
{
	return m_memoryMigrationTreshold;
}

void CMasterSingleton::SystemBringUp()
{
	std::ifstream input_file(c_mainConfigPath);
	std::string line;
	uint32_t lineNumber = 0;

	while (std::getline(input_file, line))
	{
		++lineNumber;
		std::vector<std::string> stringVector;
		Utils::SplitStringInVector(line, Utils::SPACE, stringVector);
		// Make the first element to lower, as we decide what to do next base on it
		// and we don't want to make a hard time for the user
		std::transform(stringVector[0].begin(), stringVector[0].end(), stringVector[0].begin(), ::tolower);

		if (stringVector[0] == "node")
		{
			CNode* pNode = new CNode(stringVector[1]);
			m_mainNodeVector.push_back(pNode);
		}
		if (stringVector[0] == "process")
		{
			CProcess* pProcess = new CProcess(stringVector[1]);
			m_mainNodeVector[m_mainNodeVector.size() - 1]->AddProcess(pProcess);
		}
	}
	input_file.close();
}
