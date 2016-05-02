#pragma once
#include "Command.h"

class CCodeBase
{
public:
	CCodeBase(){};
	~CCodeBase(){};
	bool AddCommand(bool i_memoryAccess, uint64_t i_cmdCount = 1, uint64_t i_startAddr = 0);
	uint16_t GetCommandCount();
	uint64_t GetMaxAddr();
	uint64_t GetAddressIndex(uint64_t i_commandIndex);
	bool IsCommandMemAccess(uint16_t i_commandIndex);
	
private:
	std::vector<CCommand> m_commandVector;
};
