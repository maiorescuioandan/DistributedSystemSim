#pragma once

class CMemPage
{
public:
	CMemPage(uint32_t i_pageId);
	CMemPage(CMemPage* i_memPage);
	~CMemPage();
	void SetPageOwnership(bool i_owned);
	bool GetPageOwnership();
	void SetOwnerId(uint32_t i_ownerId);
	uint32_t GetOwnerId();
	uint32_t GetPageId();
	void MakePageDirty();
	void MakePageClean();
	bool IsClean();

private:
	bool m_owned;
	uint32_t m_ownerProcessId;
	uint32_t m_pageId;
	bool m_clean;
};