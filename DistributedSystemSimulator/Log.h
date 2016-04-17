#pragma once
class CLog
{
public:
	CLog();
	~CLog();
	CLog(std::string i_logName, bool i_includeTimestamp = false);

	void Write(std::string i_string);
	static void SetCreationTimeAndDate();
private:
	std::string m_logName;
	static std::string s_creationTimeAndDate;
};

