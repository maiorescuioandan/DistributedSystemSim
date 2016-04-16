#pragma once

class CMasterSingleton
{
public:
	CMasterSingleton();
	~CMasterSingleton();
	static CMasterSingleton* GetInstance();
	uint32_t GetNewProcessId();
	uint32_t GetNewNodeId();

	static CMasterSingleton* s_instance;
	void TestLogFile();
	void MainLog(std::string i_string);
private:
	void CreateLogFile();


	uint32_t m_processId;
	uint32_t m_nodeId;
};

