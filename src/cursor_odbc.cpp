/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username$*/

#include "cursor_odbc.hpp"

namespace cursor { 

CursorODBC::CursorODBC(const Cursor::Sockaddr addr, const Cursor::Args args)
	: Cursor(addr)
{
}

CursorODBC::~CursorODBC()
{
}

int CursorODBC::do_next(const size_t count, std::deque<nx::String>& buf /*= buf_*/)
{
	return 0;
}

} // namespace

