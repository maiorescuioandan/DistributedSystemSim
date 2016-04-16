#include "stdafx.h"
#include "CodeBase.h"


//////////////////////////////////////////////////////////////////////////
// CodeBase Class implementation below
//////////////////////////////////////////////////////////////////////////

bool CCodeBase::AddCommand(bool i_memoryAccess, uint64_t i_cmdCount /*= 1*/, uint64_t i_startAddr /*= 0*/)
{
	CCommand command(i_memoryAccess, i_cmdCount, i_startAddr);
	m_commandVector.push_back(command);
	return true;
}

// This function will return the configured command count
uint16_t CCodeBase::GetCommandCount()
{
	return m_commandVector.size();
}

uint64_t CCodeBase::GetMaxAddr()
{
	uint64_t maxAddr = 0x0;
	for (std::vector<CCommand>::iterator it = m_commandVector.begin(); it != m_commandVector.end(); ++it) {
		if (maxAddr < it->GetMaxAddr())
			maxAddr = it->GetMaxAddr();
	}
	return maxAddr;
}