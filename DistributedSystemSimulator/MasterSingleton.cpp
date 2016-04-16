#include "stdafx.h"
#include "MasterSingleton.h"
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
CMasterSingleton* CMasterSingleton::s_instance = NULL;

CMasterSingleton::CMasterSingleton()
{
	m_processId = 0;
	m_nodeId = 0;
	CreateLogFile();
}

CMasterSingleton::~CMasterSingleton()
{
}

CMasterSingleton* CMasterSingleton::GetInstance()
{
	if (!s_instance)
		s_instance = new CMasterSingleton();
	return s_instance;
}

uint32_t CMasterSingleton::GetNewProcessId()
{
	// increment after ret
	return m_processId++;
}

uint32_t CMasterSingleton::GetNewNodeId()
{
	// increment after ret
	return m_nodeId++;
}

void CMasterSingleton::CreateLogFile()
{
	boost::log::add_file_log
		(
		boost::log::keywords::file_name = "SysSim%Y%m%d_%H%M%S.log",                                        /*< file name pattern >*/
		boost::log::keywords::rotation_size = 10 * 1024 * 1024,                                   /*< rotate files every 10 MiB... >*/
		boost::log::keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0), /*< ...or at midnight >*/
		//boost::log::keywords::format = "[%TimeStamp%]: %Message%"                                 /*< log record format >*/
		boost::log::keywords::format = "%TimeStamp%: %Message%"
		);
	boost::log::core::get()->set_filter
		(
		boost::log::trivial::severity >= boost::log::trivial::info
		);
	boost::log::add_common_attributes();
}

void CMasterSingleton::TestLogFile()
{
	//	boost::log::add_common_attributes();
	using namespace boost::log::trivial;
	boost::log::sources::severity_logger< severity_level > m_lg;

	BOOST_LOG_SEV(m_lg, trace) << "A trace severity message";
	BOOST_LOG_SEV(m_lg, debug) << "A debug severity message";
	BOOST_LOG_SEV(m_lg, info) << "An informational severity message";
	BOOST_LOG_SEV(m_lg, warning) << "A warning severity message";
	BOOST_LOG_SEV(m_lg, error) << "An error severity message";
	BOOST_LOG_SEV(m_lg, fatal) << "A fatal severity message";
}

void CMasterSingleton::MainLog(std::string i_string)
{
	//boost::log::add_common_attributes();
	using namespace boost::log::trivial;
	boost::log::sources::severity_logger< severity_level > m_lg;
	BOOST_LOG_SEV(m_lg, info) << i_string;
}
