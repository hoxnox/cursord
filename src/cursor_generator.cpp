/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username$*/

#include "cursor_generator.hpp"

namespace cursor {

CursorGenerator::CursorGenerator(const Cursor::Sockaddr addr, const Cursor::Args args)
	: Cursor(addr)
{
}

CursorGenerator::~CursorGenerator()
{
}

int CursorGenerator::Next(const size_t count, std::vector<nx::String>& buf)
{
	return 0;
}

} // namespace 

