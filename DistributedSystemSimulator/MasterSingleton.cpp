#include "stdafx.h"
#include "MasterSingleton.h"
#include "Log.h"
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_multifile_backend.hpp>
#include <boost/log/expressions/filter.hpp>
#include <boost/log/keywords/filter.hpp>
#include <boost/log/detail/sink_init_helpers.hpp>
#include <boost/shared_ptr.hpp>

namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;
//namespace flt = boost::log::filters;

CMasterSingleton* CMasterSingleton::s_instance = NULL;
BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(my_logger, src::logger_mt)

CMasterSingleton::CMasterSingleton()
{
	m_processId = 0;
	m_nodeId = 0;
	CreateLogFile();
	AddNodeLog(0);
	AddNodeLog(1);
	AddNodeLog(2);
}

CMasterSingleton::~CMasterSingleton()
{
}

CMasterSingleton* CMasterSingleton::GetInstance()
{
	if (!s_instance)
	{
		s_instance = new CMasterSingleton();
		CLog::SetCreationTimeAndDate();
	}
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
		boost::log::trivial::severity >= boost::log::trivial::trace
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

void CMasterSingleton::AddNodeLog(uint32_t i_nodeId)
{
	//http://stackoverflow.com/questions/5960395/boost-log-select-destination-file


	//typedef sinks::synchronous_sink< sinks::text_multifile_backend > file_sink;
	//boost::shared_ptr< file_sink > sink(new file_sink);
	//
	//// Set up how the file names will be generated
	//sink->locked_backend()->set_file_name_composer(sinks::file::as_file_name_composer(
	//	expr::stream << "logs/Node" << expr::attr< uint32_t >("NodeID") << ".log"));
	//
	//// Set the log record formatter
	//sink->set_formatter
	//	(
	//	expr::format("%1%")
	//	% expr::smessage
	//	);
	//
	//// Add it to the core
	//logging::core::get()->add_sink(sink);

	// Construct the sink
	typedef sinks::synchronous_sink< sinks::text_ostream_backend > text_sink;
	boost::shared_ptr< text_sink > sink = boost::make_shared< text_sink >();

	// Add a stream to write log to
	std::ostringstream stringStream;
	stringStream << "logs/Node" << i_nodeId << ".log";

	sink->locked_backend()->add_stream(
		boost::make_shared< std::ofstream >(stringStream.str()));
	sink->set_filter(expr::attr<std::string>("Log") == stringStream.str());

	// Register the sink in the logging core
	logging::core::get()->add_sink(sink);
}

void CMasterSingleton::NodeLog(uint32_t i_nodeId, std::string i_string)
{
	// Add a stream to write log to
	std::ostringstream stringStream;
	stringStream << "logs/Node" << i_nodeId << ".log";

	using namespace boost::log::trivial;
	boost::log::sources::severity_logger< severity_level > m_lg;
	
	BOOST_LOG_SCOPED_LOGGER_TAG(m_lg, "Log", stringStream.str())
	BOOST_LOG(m_lg) << i_string;

}
