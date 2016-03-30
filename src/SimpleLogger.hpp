/**@author Merder Kim <hoxnox@gmail.com>
 * @date 20151024 22:21:01*/

#ifndef __SIMPLE_LOGGER
#define __SIMPLE_LOGGER

#include <sstream>
#include <iostream>
#include <memory>
#include <cstring>

#ifdef WITH_BOOT_FORMAT
#include <boost/format.hpp>
#endif

class SimpleLogStream
{
public:
	static SimpleLogStream* GetInstance()
	{
		if (!instance_)
			instance_.reset(new SimpleLogStream());
		return instance_.get();
	}
	std::stringstream& _stream(std::ostream& ostr)
	{
		ostr_ = &ostr;
		ss_.str("");
		memset(prefix_buf_, 0, sizeof(prefix_buf_));
		time_t t = time(NULL);
		strftime(prefix_buf_, sizeof(prefix_buf_) - 1, "[%Y%m%dT%H%M%S] ", localtime(&t));
		prefix_len_ = strlen(prefix_buf_);
		ss_ << prefix_buf_;
		return ss_;
	}
	SimpleLogStream& operator< (std::ostream& ss)
	{
		if (align)
		{
			std::istream strm(ss.rdbuf());
			bool first = true;
			for(std::string line; std::getline(strm, line);)
			{
				if (first)
				{
					first = false;
				}
				else
				{
					for(size_t i = 0; i < prefix_len_; ++i)
						*ostr_ << ' ';
				}
				*ostr_ << line << std::endl;
			}
		}
		else
		{
			*ostr_ << ss.rdbuf() << std::endl;
		}
		ostr_->flush();
		return *this;
	}
	static bool verbose;
	static bool align;
private:
	static std::unique_ptr<SimpleLogStream> instance_;
	SimpleLogStream() : ostr_(&std::cout) {};
	SimpleLogStream(const SimpleLogStream&) = delete;
	SimpleLogStream(const SimpleLogStream&&) = delete;
	SimpleLogStream& operator=(const SimpleLogStream&) = delete;
	std::stringstream ss_;
	std::ostream* ostr_;
	char prefix_buf_[20];
	uint8_t prefix_len_;
};

class SilentNoop{
public:
	SilentNoop() { }
	void operator&(SimpleLogStream&) { }
};

#define VLOG !SimpleLogStream::GetInstance()->verbose ? (void)0 : SilentNoop() & *SimpleLogStream::GetInstance() < SimpleLogStream::GetInstance()->_stream(std::cout)
#define ILOG SilentNoop() & *SimpleLogStream::GetInstance() < SimpleLogStream::GetInstance()->_stream(std::cout)
#define ELOG SilentNoop() & *SimpleLogStream::GetInstance() < SimpleLogStream::GetInstance()->_stream(std::cerr)
#define SIMPLE_LOGGER_INIT std::unique_ptr<SimpleLogStream> SimpleLogStream::instance_; bool SimpleLogStream::verbose = false; bool SimpleLogStream::align = false;
#define SIMPLE_LOGGER_SET_VERBOSE SimpleLogStream::verbose = true;
#define SIMPLE_LOGGER_SET_ALIGN SimpleLogStream::align = true;

#ifdef WITH_BOOST_FORMAT

struct BoostFormatHelper
{
public:
    BoostFormatHelper(std::string file, int line, std::string fun)
        : file_(file), line_(line), fun_(fun)
    {}
    inline boost::format operator*(std::string format)
    {
        try {
            boost::format fmt(format);
            return fmt;
        }
        catch(std::exception& e)
        {
            ELOG << "Error executing boost::format."
                 << " Function: " << fun_ << " (" << file_ << ":" << line_ << ")."
                 << " Message: " << e.what();
            return boost::format();
        }
    }
private:
	std::string file_;
	int line_;
	std::string fun_;
};

#ifndef ELOGF
#define ELOGF ELOG << BoostFormatHelper(__FILE__, __LINE__, __PRETTY_FUNCTION__)
#endif
#ifndef ILOGF
#define ILOGF ILOG << BoostFormatHelper(__FILE__, __LINE__, __PRETTY_FUNCTION__)
#endif
#ifndef VLOGF
#define VLOGF VLOG << BoostFormatHelper(__FILE__, __LINE__, __PRETTY_FUNCTION__)
#endif
#endif

#endif // guard


