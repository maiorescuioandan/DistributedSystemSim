#include "stdafx.h"
#include "MigrationInfo.h"
#include "MasterSingleton.h"
#include <boost/format.hpp>

CMigrationInfo::CMigrationInfo()
{
	m_state = kNone;
}

CMigrationInfo::~CMigrationInfo()
{

}

bool CMigrationInfo::ScheduleMigration(CNode* i_sourceNode, CNode* i_destinationNode, CProcess *i_process)
{
	m_sourceNode = i_sourceNode;
	m_destinationNode = i_destinationNode;
	m_migratedProcess = i_process;

	// Calculate the migration time
	m_bandwithUsed = (m_sourceNode->GetAvailableBandwidth() < m_destinationNode->GetAvailableBandwidth()) ? m_sourceNode->GetAvailableBandwidth() : m_destinationNode->GetAvailableBandwidth();
	// if there's no bandwidth available, the migration fails
	if (m_bandwithUsed == 0)
	{
		m_sourceNode->WriteLog("Could not schedule migration - bandwidth available = 0");
		return false;
	}
	double migrationTime = 1.0 * m_sourceNode->GetPageSize() * m_migratedProcess->GetMemPageCount() / m_bandwithUsed;
	// Reserve the memory on the destination node
	// The migration fails if there is not enough memory. This should not happen though
	// We should decide to move a process on another node where there is room for it
	// After the migration is done, we can use "link pages" method from node to link the process with the methods
	if (!m_destinationNode->MemAlloc(m_migratedProcess->GetId(), m_migratedProcess->GetMemoryRequired()))
	{
		m_sourceNode->WriteLog("Could not schedule migration - not enough memory on destination node");
		return false;
	}
	// Clear all pages that the migrated process currently has
	// So after the alarm goes off, we can check to move only the pages that are currently empty
	m_migratedProcess->CleanMemory();
	// Set the alarm on the source node
	m_sourceNode->SetAlarm(true);
	m_alarmTime = migrationTime + m_sourceNode->GetCurrentTimeTemp();
	// set the migration info object
	m_sourceNode->SetMigrationInfo(this);
	m_state = kStateScheduled;
	m_sourceNode->SetAvailableBandwidth(m_sourceNode->GetAvailableBandwidth() - m_bandwithUsed);
	m_destinationNode->SetAvailableBandwidth(m_destinationNode->GetAvailableBandwidth() - m_bandwithUsed);
	m_sourceNode->WriteLog(boost::str(boost::format("MIGRATION START DstNode: %d \tPID: %d\tMigrationTime: %f\tMemSize: %d\tAlarmTime: %f") % m_destinationNode->GetId() % m_migratedProcess->GetId() % migrationTime % (m_sourceNode->GetPageSize() * m_migratedProcess->GetMemPageCount()) % m_alarmTime));
	return true;
}

CNode* CMigrationInfo::FindNodeWithFreeCpu(CNode* i_sourceNode)
{
	CNode* returnValue = NULL;
	bool found = false;
	uint32_t minIndex = 0;
	double minCpu = 1;
	for (uint32_t i = 0; i < CMasterSingleton::GetInstance()->GetNodeCount(); ++i)
	{
		double cpuUsage = CMasterSingleton::GetInstance()->GetNode(i)->GetCpuUsage();
		if (minCpu > cpuUsage && i_sourceNode->GetId() != CMasterSingleton::GetInstance()->GetNode(i)->GetId())
		{ 
			minCpu = cpuUsage;
			minIndex = i;
			found = true;
		}
	}
	
	// if found is false, we will return a null pointer meaning that the migration should not happen
	if (found)
		returnValue = CMasterSingleton::GetInstance()->GetNode(minIndex);
	return returnValue;
}

CMigrationInfo::EnumMigrationState CMigrationInfo::GetState()
{
	return m_state;
}

double CMigrationInfo::GetAlarmTime()
{
	return m_alarmTime;
}

void CMigrationInfo::StopProcessOnSource()
{
	m_migratedProcess->SetRunState(false);
	// we set sleep so the process will not be scheduled again no matter what
	m_migratedProcess->SetSleep(true);

	// when calculating the migration time now, we will only count the dirty pages
	uint32_t dirtyPageCount = m_migratedProcess->GetMemPageCount(true);
	double migrationTime = 0;
	if (dirtyPageCount != 0)	
	{
		migrationTime = 1.0 * m_sourceNode->GetPageSize() * dirtyPageCount / m_bandwithUsed;
		m_alarmTime = migrationTime + m_sourceNode->GetCurrentTimeTemp();
		m_sourceNode->WriteLog(boost::str(boost::format("MIGRATION STEP 2 DstNode: %d \tPID: %d\tMigrationTime: %f\tMemSize: %d\tAlarmTime: %f") % m_destinationNode->GetId() % m_migratedProcess->GetId() % migrationTime % (m_sourceNode->GetPageSize() * m_migratedProcess->GetMemPageCount()) % m_alarmTime));
	}
	else
	{
		m_sourceNode->WriteLog(boost::str(boost::format("MIGRATION STEP 2 DstNode: %d \tPID: %d\tMigrationTime: %f\tMemSize: %d\tAlarmTime: %f - ALL PAGES CLEAN") % m_destinationNode->GetId() % m_migratedProcess->GetId() % migrationTime % (m_sourceNode->GetPageSize() * m_migratedProcess->GetMemPageCount()) % m_alarmTime));
	}
	m_state = kStateInitialCopyDone;
}

void CMigrationInfo::CompleteMigration()
{
	// set alarm up to off so migrations can happen again
	m_sourceNode->GetProcessorAlgorithm()->GetNextRunningProcess(m_sourceNode);
	m_migratedProcess->SetSleep(false);
	m_migratedProcess->SetMarkedDeadlineMissed(false);
	m_sourceNode->SetAlarm(false);
	// move the pointer to the process from the source node to the destination node
	m_sourceNode->RemoveProcess(m_migratedProcess->GetId());

	m_destinationNode->AddProcessPostMigration(m_migratedProcess);
	// wake up the process
	m_migratedProcess->SetWakeUpTime(m_destinationNode->GetCurrentTimeTemp());
	// update the bandwidth on both nodes
	m_sourceNode->SetAvailableBandwidth(m_sourceNode->GetAvailableBandwidth() + m_bandwithUsed);
	m_destinationNode->SetAvailableBandwidth(m_destinationNode->GetAvailableBandwidth() + m_bandwithUsed);
	m_sourceNode->WriteLog("MIGRATION DONE");
}
