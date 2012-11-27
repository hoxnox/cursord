/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username$*/

#ifndef __NX_CURSOR_GENERATOR_HPP__
#define __NX_CURSOR_GENERATOR_HPP__

#include <map>

#include "cursor.hpp"
#include "string.hpp"

namespace cursor {

class CursorGenerator: public Cursor
{
	public:
		CursorGenerator(const Cursor::Sockaddr addr, const Cursor::Args args);
		~CursorGenerator();
	protected:
		virtual int Next(const size_t count, std::vector<nx::String>& buf);
};

} // namespace

#endif /*__NX_CURSOR_GENERATOR_HPP__*/

