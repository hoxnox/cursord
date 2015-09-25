/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username
 *
 * Init glog, if we can*/

#ifndef __LOGGING_H__
#define __LOGGING_H__

#ifndef CFG_WITHOUT_GLOG
#	include <glog/logging.h>
#else
	#include <iostream>
	#include <sstream>
	static struct SimpleLogStream
	{
	public:
		std::stringstream& _stream()
		{
			ss_.str("");
			ss_ << time(NULL) << " ";
			return ss_;
		}
		SimpleLogStream& operator< (std::ostream& ss)
		{
			std::cerr << ss.rdbuf() << std::endl;
			return *this;
		}
		static const bool verbose = false;
	protected:
		std::stringstream ss_;
	} __simple_log_strm;
	class SilentNoop{
	public:
		SilentNoop() { }
		void operator&(SimpleLogStream&) { }
	};
	#define LOG(x) SilentNoop() & __simple_log_strm < __simple_log_strm._stream() << (x) << ": "
	#define ERROR "ERROR"
	#define INFO "INFO"
	#define WARNING "WARNING"
#ifndef NDEBUG
#	define DLOG(x) std::cerr << "[" << __DATE__ << " " << __TIME__ << "] " << (x) << ": "
#else // NDEBUG
#	define DLOG(x) void (0)
#endif
#endif

#endif // __LOGGING_H__

