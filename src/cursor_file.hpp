/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username$*/

#ifndef __NX_CURSOR_FILE_HPP__
#define __NX_CURSOR_FILE_HPP__

#include <fstream>
#include "cursor.hpp"

namespace cursor {

class CursorFile : public Cursor
{
	public:
		CursorFile(const Cursor::Sockaddr addr, const Cursor::Args args);
		~CursorFile();
	protected:
		virtual int Next(const size_t count, std::deque<nx::String>& buf);
	private:
		std::ifstream file_;
		bool repeat_;
};

} // namespace

#endif /*__NX_CURSOR_FILE_HPP__*/


