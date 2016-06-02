#include "stdafx.h"
#include "MemPage.h"


//////////////////////////////////////////////////////////////////////////
// MemPage Class implementation below
//////////////////////////////////////////////////////////////////////////

CMemPage::CMemPage(uint32_t i_pageId)
{
	m_clean = true;
	m_owned = false;
	m_pageId = i_pageId;
}

CMemPage::CMemPage(CMemPage* i_memPage)
{
	m_clean = i_memPage->m_clean;
	m_owned = i_memPage->m_owned;
	m_pageId = i_memPage->m_pageId;
	m_ownerProcessId = i_memPage->m_ownerProcessId;
}

CMemPage::~CMemPage()
{

}

void CMemPage::SetPageOwnership(bool i_owned)
{
	m_owned = i_owned;
}

bool CMemPage::GetPageOwnership()
{
	return m_owned;
}

void CMemPage::SetOwnerId(uint32_t i_ownerId)
{
	m_ownerProcessId = i_ownerId;
}

uint32_t CMemPage::GetOwnerId()
{
	return m_ownerProcessId;
}

uint32_t CMemPage::GetPageId()
{
	return m_pageId;
}

void CMemPage::MakePageDirty()
{
	m_clean = false;
}

void CMemPage::MakePageClean()
{
	m_clean = true;
}

bool CMemPage::IsClean()
{
	return m_clean;
}
