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
		enum FileTypes
		{
			FTYPE_TEXT = 0,
			FTYPE_IPv4 = 1 << 1
		};
		CursorFile(const Cursor::Sockaddr addr, const Cursor::Args args);
		~CursorFile();
	protected:
		virtual int Next(const size_t count, std::deque<nx::String>& buf);
	private:
		std::string getnext();
		std::string getinfo(size_t info = 0);
		nx::String fname_;
		std::ifstream file_;
		FileTypes ftype_;
		nx::String suffix_;
		nx::String prefix_;
		bool repeat_;
};

} // namespace

#endif /*__NX_CURSOR_FILE_HPP__*/


