#include "stdafx.h"
#include "Log.h"

std::string CLog::s_creationTimeAndDate = "";

namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

CLog::CLog()
{
}

CLog::CLog(std::string i_logName, bool i_includeTimestamp /*= false*/)
{
	m_logName = i_logName;
	// Construct the sink
	typedef sinks::synchronous_sink< sinks::text_ostream_backend > text_sink;
	boost::shared_ptr< text_sink > sink = boost::make_shared< text_sink >();

	// Add a stream to write log to
	std::ostringstream stringStream;
	stringStream << "logs/" << s_creationTimeAndDate << "_" << m_logName << ".log";

	sink->locked_backend()->add_stream(
		boost::make_shared< std::ofstream >(stringStream.str()));
	sink->set_filter(expr::attr<std::string>("Log") == m_logName);

	// TODO include timestamp formatting for 
	//if (i_includeTimestamp)
	//	sink->set_formatter
	//	(
	//	expr::format("%1%: %2%")
	//	% expr::smessage
	//	);

	// Register the sink in the logging core
	logging::core::get()->add_sink(sink);
}

CLog::CLog(CLog* i_log)
{
	m_logName = i_log->m_logName;
}


CLog::~CLog()
{
}



void CLog::SetCreationTimeAndDate()
{
	namespace pt = boost::posix_time;
	pt::ptime now = pt::second_clock::local_time();
	std::stringstream ss;

	std::string month;
	std::stringstream tempss;
	if (static_cast<int>(now.date().month()) < 10)
	{
		tempss << 0;
	}
	tempss << static_cast<int>(now.date().month());
	month = tempss.str();
	
	ss << now.date().year() << month <<	now.date().day() << "_" << now.time_of_day().hours() << now.time_of_day().minutes() << now.time_of_day().seconds();
	s_creationTimeAndDate = ss.str();
}

void CLog::Write(std::string i_string)
{
	using namespace boost::log::trivial;
	boost::log::sources::severity_logger< severity_level > lg;

	BOOST_LOG_SCOPED_LOGGER_TAG(lg, "Log", m_logName);
	BOOST_LOG(lg) << i_string;
}
