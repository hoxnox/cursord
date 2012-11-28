/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username$*/

#ifndef __NX_SPEEDOMETER_HPP__
#define __NX_SPEEDOMETER_HPP__

#include <string>
#include <ctime>

namespace cursor {

class Speedometer
{
	public:
		Speedometer();
		~Speedometer();
		Speedometer& operator++();
		std::string  LastSpeedS();
		std::string  AVGSpeedS();
	private:
		void         renewLastSpeed();
		void         init();
		unsigned long           counter_;
		unsigned long           counter_point_;
		time_t                  tm_begin_;
		time_t                  tm_counter_flush_;
		time_t                  tm_point_;
		float                   last_speed_;
		const int               strbufsz_;
		char*                   strbuf_;
};

} // namespace

#endif // __NX_SPEEDOMETER_HPP__

