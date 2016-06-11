#include "stdafx.h"
#include "Command.h"

//////////////////////////////////////////////////////////////////////////
// Command Class implementation below
//////////////////////////////////////////////////////////////////////////

CCommand::CCommand(bool i_memoryAccess, uint32_t i_cmdCount /*= 1*/, uint32_t i_startAddr /*= 0*/)
{
	m_memoryAccess = i_memoryAccess;
	m_startAddr = i_startAddr;
	m_cmdCount = i_cmdCount;
}

bool CCommand::IsMemoryAccess()
{
	return m_memoryAccess;
}

uint32_t CCommand::GetStartAddr()
{
	return m_startAddr;
}

uint32_t CCommand::GetCmdCount()
{
	return m_cmdCount;
}

uint32_t CCommand::GetMaxAddr()
{
	uint32_t rValue;
	if (m_memoryAccess)
	{
		rValue =  m_startAddr + m_cmdCount - 1;
	}
	else
	{
		rValue = 0;
	}
	return rValue;
}