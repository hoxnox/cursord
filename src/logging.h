/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username
 *
 * Init glog, if we can*/

#ifndef CFG_WITHOUT_GLOG
#	include <glog/logging.h>
#else
#	include <iostream>
#	define LOG(x) std::cerr << "[" << __TIME__ << " " << __DATE__ << "] " << (x) << ": "
#	define ERROR "ERROR"
#	define INFO "INFO"
#	define WARNING "WARNING"
#ifndef NDEBUG
#	define DLOG(x) std::cerr << "[" << __DATE__ << " " << __TIME__ << "] " << (x) << ": "
#else // NDEBUG
#	define DLOG(x) void (0)
#endif
#endif
