#pragma once
#include "Log.h"
#include "Node.h"

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

	void AddNode(CNode* i_node);
	CNode* GetNode(uint32_t i_nodeIndex);
	void CreateNodeBackup();
	void RevertNodeFromBackup();
	void RunToTime(double i_time);
	void SetStatusReportCycle(uint32_t i_statusReportCycle);
	void SetEnableMigration(bool i_enableMigration);
	bool IsMigrationEnabled();
	uint32_t GetNodeCount();
	void SetMemoryMigrationTreshold(double i_memoryMigrationTreshold);
	double GetMemoryMigrationTreshold();
private:
	uint32_t GetCurrentStatusReportCycle();
	void IncrementStatusReportCycle();
	static void CreateLogFile();

	static CLog			m_log;
	bool				m_enableMigration;
	uint32_t			m_processId;
	uint32_t			m_nodeId;
	std::vector<CNode*> m_mainNodeVector;
	uint32_t			m_statusReportCycle;
	uint32_t			m_currentStatusReportCycle;
	double				m_memoryMigrationTreshold;
	//std::vector<CNode*> m_backupNodeVector;
};

