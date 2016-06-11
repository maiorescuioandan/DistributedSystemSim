#pragma once
#include "Command.h"

class CCodeBase
{
public:
	CCodeBase(){};
	~CCodeBase(){};
	bool AddCommand(bool i_memoryAccess, uint32_t i_cmdCount = 1, uint32_t i_startAddr = 0);
	uint16_t GetCommandCount();
	uint32_t GetMaxAddr();
	uint32_t GetAddressIndex(uint32_t i_commandIndex);
	bool IsCommandMemAccess(uint16_t i_commandIndex);
	
private:
	std::vector<CCommand> m_commandVector;
};
