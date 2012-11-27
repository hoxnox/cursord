/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username$*/

#ifndef __NX_CURSOR_HPP__
#define __NX_CURSOR_HPP__

#include <string>
#include <vector>
#include <map>

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/event.h>

#include <logging.h>
#include <gettext.h>
#include <string.hpp>

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
		virtual int Next(const size_t count, std::vector<nx::String>& buf) = 0;
	private:
		enum
		{
			STATE_STOP  = 1,
			STATE_ERROR = 1 << 1
		};
		static void readCallback(bufferevent *bev, void *ptr);
		static void eventCallback(bufferevent *bev, short events, void *ptr);

		int initBufferevent();

		std::vector<nx::String> buf_;
		Sockaddr                laddr_;
		event_base*             evbase_;
		bufferevent*            bev_;
		timeval                 timeout_;
		int                     state_;
		char*                   recvbuf_;
		int                     recvbufsz_;
};

} // namespace

#endif /*__NX_CURSOR_HPP__*/

