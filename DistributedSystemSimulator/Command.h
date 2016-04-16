#pragma once

class CCommand
{
public:
	CCommand(){};
	CCommand(bool i_memoryAccess, uint64_t i_cmdCount = 1, uint64_t i_startAddr = 0);
	~CCommand(){};
	bool IsMemoryAccess();
	uint64_t GetStartAddr();
	uint64_t GetCmdCount();
	uint64_t GetMaxAddr();
private:
	bool m_memoryAccess;
	uint64_t m_startAddr;
	uint64_t m_cmdCount;
};