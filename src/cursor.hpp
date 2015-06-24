/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username$*/

#ifndef __NX_CURSOR_HPP__
#define __NX_CURSOR_HPP__

#include <vector>
#include <string>
#include <deque>
#include <map>

#include <logging.h>
#include <gettext.h>
#include <string.hpp>
#include "speedometer.hpp"

namespace cursor {

class Cursor
{
	public:
		typedef std::map<nx::String, nx::String> Args;
		Cursor();
		Cursor(const size_t shared_curr, const size_t shared_total);
		virtual ~Cursor();
		void Run(std::vector<std::string>& urls);
	protected:
		int Next(const size_t count, std::deque<nx::String>& buf);
		virtual int do_next(const size_t count, std::deque<nx::String>& buf) = 0;
		nx::String initial_;
		bool       shared_;
		size_t     shared_curr_;
		size_t     shared_total_;
	private:
		enum
		{
			STATE_STOP  = 1,
			STATE_ERROR = 1 << 1
		};
		std::deque<nx::String>  buf_;
		size_t                  bufsz_;
		int                     state_;
		Speedometer             speedometer_;
};

} // namespace

#endif /*__NX_CURSOR_HPP__*/

