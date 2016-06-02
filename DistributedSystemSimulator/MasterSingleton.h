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
private:
	static void CreateLogFile();

	static CLog m_log;
	uint32_t	m_processId;
	uint32_t	m_nodeId;
	std::vector<CNode*> m_mainNodeVector;
	std::vector<CNode*> m_backupNodeVector;
};

