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

int CursorODBC::Next(const size_t count, std::vector<nx::String>& buf /*= buf_*/)
{
	return 0;
}

} // namespace

