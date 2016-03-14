/**@author hoxnox <hoxnox@gmail.com>
 * @date 20160314 09:11:14
 * @copyright hoxnox*/

#ifndef __NX_CURSOR_GENERATOR_HPP__
#define __NX_CURSOR_GENERATOR_HPP__

#include <map>
#include <functional>

#include "cursor.hpp"
#include "string.hpp"

namespace cursor {

class CursorGenerator: public Cursor
{
	public:
		CursorGenerator(const Cursor::Args args);
		~CursorGenerator();
	protected:
		virtual int do_next(const size_t count, std::deque<nx::String>& buf);
	private:
		void init(const Cursor::Args args);
		std::function<void(char *, size_t *, const size_t, char *, 
		                   size_t *, const size_t, int)> generator;
		size_t     do_next_fake_count_;
		nx::String name_;
		int        repeat_;
		char*      nextbuf_;
		size_t     nextbufsz_;
		size_t     nextbufmaxsz_;
		char*      state_;
		size_t     statesz_;
		size_t     statemaxsz_;
};

} // namespace

#endif /*__NX_CURSOR_GENERATOR_HPP__*/

