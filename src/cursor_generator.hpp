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
		virtual int Next(const size_t count, std::deque<nx::String>& buf);
	private:
		void (*generator)(char* state, size_t* statesz, size_t* statemaxsz,
	                          char * next, size_t* nextsz, size_t* nextmaxsz);
		char*      nextbuf_;
		size_t     nextbufsz_;
		size_t     nextbufmaxsz_;
		char*      state_;
		size_t     statesz_;
		size_t     statemaxsz_;
};

} // namespace

#endif /*__NX_CURSOR_GENERATOR_HPP__*/

