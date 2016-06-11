#pragma once

class CCommand
{
public:
	CCommand(){};
	CCommand(bool i_memoryAccess, uint32_t i_cmdCount = 1, uint32_t i_startAddr = 0);
	~CCommand(){};
	bool IsMemoryAccess();
	uint32_t GetStartAddr();
	uint32_t GetCmdCount();
	uint32_t GetMaxAddr();
private:
	bool m_memoryAccess;
	uint32_t m_startAddr;
	uint32_t m_cmdCount;
};