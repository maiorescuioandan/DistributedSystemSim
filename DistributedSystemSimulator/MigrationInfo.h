#pragma once
#include "Node.h"
#include "Process.h"

class CNode;

class CMigrationInfo
{
public:
	enum EnumMigrationState {
		kNone = 0,
		kStateScheduled = 1,
		kStateInitialCopyDone = 2
	};

	CMigrationInfo();
	~CMigrationInfo();
	bool ScheduleMigration(CNode* i_sourceNode, CNode* i_destinationNode, CProcess *i_process);
	void StopProcessOnSource();
	void CompleteMigration();
	CNode* FindNodeWithFreeCpu(CNode* i_sourceNode);
	EnumMigrationState GetState();
	double GetAlarmTime();
private:
	CNode		*m_sourceNode;
	CNode		*m_destinationNode;
	CProcess	*m_migratedProcess;
	double		m_alarmTime;
	uint32_t	m_bandwithUsed;
	EnumMigrationState m_state;
};

