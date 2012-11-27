/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username$*/

#include "cursor_file.hpp"

namespace cursor {

CursorFile::CursorFile(const Cursor::Sockaddr addr, const Cursor::Args args)
	: Cursor(addr)
{
}

CursorFile::~CursorFile()
{
}

int CursorFile::Next(const size_t count, std::vector<nx::String>& buf /*= buf_*/)
{
	return 0;
}

} // namespace

