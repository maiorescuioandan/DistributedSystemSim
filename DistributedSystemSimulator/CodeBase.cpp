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

uint16_t CCodeBase::GetCommandCount()
{
	// The number of processor ticks required to reach the deadline of the current code base (in this case, the process)
	uint16_t count = 0;
	for (std::vector<CCommand>::iterator it = m_commandVector.begin(); it != m_commandVector.end(); ++it) {
		count += it->GetCmdCount();
	}
	return count;
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

bool CCodeBase::IsCommandMemAccess(uint16_t i_commandIndex)
{
	int signedCommandIndex = i_commandIndex;
	for (std::vector<CCommand>::iterator it = m_commandVector.begin(); it != m_commandVector.end(); ++it) {
		signedCommandIndex -= it->GetCmdCount();
		if (signedCommandIndex < 0)
			return it->IsMemoryAccess();
	}
	return false;
}

uint64_t CCodeBase::GetAddressIndex(uint64_t i_commandIndex)
{
	int signedCommandIndex = i_commandIndex;
	int addressIndex = 0;
	for (std::vector<CCommand>::iterator it = m_commandVector.begin(); it != m_commandVector.end(); ++it)
	{
		int temp = signedCommandIndex - it->GetCmdCount();
		if ( temp < 0)
		{
			// we should only hit this on a memory access
			assert(it->IsMemoryAccess());
			addressIndex = signedCommandIndex + it->GetStartAddr();
			return addressIndex;
		}
		else 
			signedCommandIndex -= it->GetCmdCount();

	}
	// we should never reach this;
	assert(1 == 0);
	return 0;
}
