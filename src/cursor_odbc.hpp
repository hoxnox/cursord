/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username$*/

#ifndef __NX_CURSOR_ODBC_HPP__
#define __NX_CURSOR_ODBC_HPP__

#include "cursor.hpp"

namespace cursor {

class CursorODBC: public Cursor
{
	public:
		CursorODBC(const Cursor::Sockaddr addr, const Cursor::Args args,
		           const size_t shared_curr, const size_t shared_total)
			: Cursor(addr)
		{ // TODO: 
		};
		CursorODBC(const Cursor::Sockaddr addr, const Cursor::Args args);
		~CursorODBC();
	protected:
		virtual int do_next(const size_t count, std::deque<nx::String>& buf);
};

} // namespace

#endif /*__NX_CURSOR_ODBC_HPP__*/


