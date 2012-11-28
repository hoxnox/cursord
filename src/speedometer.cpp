/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username$ */

#include "speedometer.hpp"

#include <cstring>
#include <memory>
#include <limits>

#include <gettext.h>
#include <logging.h>

namespace cursor {

/**@class Speedometer
 * @brief Used to mesure speed of some descrette process
 *
 * When the step of process happens, use operator++ to fix it.
 * Then you can use AVGSpeed() and LastSpeed() to get average and last 
 * speed of discrete process steps happened to second.
 *
 *
 * For example:
 * @code
 * 	Speedometer sp;
 * 	for(size_t i = 0; i < 10000000; ++i)
 * 	{
 * 		if(i % 1000 == 0)
 * 			++sp;
 * 	}
 * 	std:: cout << sp.AVGSpeed();
 * @endcode
 * */

Speedometer::Speedometer()
	: counter_(0)
	 ,counter_point_(0)
	 ,last_speed_(0) 
	 ,tm_begin_(0)
	 ,tm_counter_flush_(0)
	 ,tm_point_(0)
	 ,strbufsz_(std::numeric_limits<float>::digits10 + 1)
{
	strbuf_ = (char*)malloc(strbufsz_);
	memset(strbuf_, 0, strbufsz_);
}

Speedometer::~Speedometer()
{
	if(strbuf_)
		free(strbuf_);
}

inline void Speedometer::init()
{
	if(tm_begin_ == 0)
	{
		tm_begin_ = time(NULL);
		tm_counter_flush_ = tm_point_ = tm_begin_;
		counter_ = 0;
	}
}

Speedometer& Speedometer::operator++()
{
	if(tm_begin_ == 0)
		init();
	renewLastSpeed();
	if(counter_ == std::numeric_limits<unsigned long>::max())
	{
		LOG(INFO) << _("speedometer: Counter overhead. Flushing to 0.");
		counter_ = 0;
		tm_counter_flush_ = time(NULL);
	}
	if(counter_point_ == std::numeric_limits<unsigned long>::max())
	{
		LOG(INFO) << _("speedometer: Counter (point) overhead. Flushing to 0.");
		counter_ = 0;
		tm_point_ = time(NULL);
	}
	++counter_;
	++counter_point_;
	return *this;
}

std::string Speedometer::LastSpeedS()
{
	renewLastSpeed();
	memset(strbuf_, 0, strbufsz_);
	sprintf(strbuf_, "%.2f", last_speed_);
	return strbuf_;
}

std::string Speedometer::AVGSpeedS()
{
	time_t curr = time(NULL);
	float avg_speed = 0;
	if(curr - tm_counter_flush_ > 0)
		avg_speed = (float)counter_/(curr - tm_counter_flush_);
	memset(strbuf_, 0, strbufsz_);
	sprintf(strbuf_, "%.2f", avg_speed);
	return strbuf_;
}

inline void Speedometer::renewLastSpeed()
{
	time_t curr = time(NULL);
	if(curr - tm_point_ > 0)
	{
		last_speed_ = (float)counter_point_/(curr - tm_point_);
		counter_point_ = 0;
		tm_point_ = curr;
	}
}

} // namespace

