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
		virtual ~Cursor();
		void Run();
	protected:
		virtual int Next(const size_t count, std::deque<nx::String>& buf) = 0;
		nx::String initial_;
	private:
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

