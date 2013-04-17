/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username$*/

#ifndef __NX_CURSOR_HPP__
#define __NX_CURSOR_HPP__

#include <string>
#include <deque>
#include <map>

#include <logging.h>
#include <gettext.h>
#include <string.hpp>
#include <nx_socket.h>
#include "speedometer.hpp"

namespace cursor {

class Cursor
{
	public:
		typedef struct sockaddr_storage Sockaddr;
		typedef std::map<nx::String, nx::String> Args;
		Cursor(const Sockaddr addr);
		Cursor(const Sockaddr addr, const size_t shared_curr, const size_t shared_total);
		virtual ~Cursor();
		void Run();
	protected:
		int Next(const size_t count, std::deque<nx::String>& buf);
		virtual int do_next(const size_t count, std::deque<nx::String>& buf) = 0;
		nx::String initial_;
		bool       shared_;
		size_t     shared_curr_;
		size_t     shared_total_;
	private:
		void init(const Sockaddr addr);
		enum
		{
			STATE_STOP  = 1,
			STATE_ERROR = 1 << 1
		};
		std::deque<nx::String>  buf_;
		size_t                  bufsz_;
		Sockaddr                laddr_;
		timeval                 timeout_;
		int                     state_;
		char*                   recvbuf_;
		int                     recvbufsz_;
		Speedometer             speedometer_;
};

} // namespace

#endif /*__NX_CURSOR_HPP__*/

