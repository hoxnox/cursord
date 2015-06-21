/**@author $username$ <$usermail$>
 * @date $date$
 * @copyright $username$*/

#ifndef __NX_CURSOR_FILE_HPP__
#define __NX_CURSOR_FILE_HPP__

#include <fstream>
#include "ipv4_generator.hpp"
#include "cursor.hpp"

namespace cursor {

class CursorFile : public Cursor
{
	public:
		enum FileTypes
		{
			FTYPE_TEXT       = 0,
			FTYPE_IPv4       = 1 << 1,
			FTYPE_IPv4RANGES = 1 << 2,
			FTYPE_CIDR       = 1 << 3
		};
		CursorFile(const Cursor::Sockaddr addr, const Cursor::Args args,
		           const size_t shared_curr, const size_t shared_total);
		CursorFile(const Cursor::Sockaddr addr, const Cursor::Args args);
		~CursorFile();
	protected:
		virtual int do_next(const size_t count, std::deque<nx::String>& buf);
	private:
		void init(const Cursor::Sockaddr addr, const Cursor::Args args);
		bool init_ipv4gen(uint32_t ip_low, uint32_t ip_hi);
		std::string getnext();
		std::string getinfo(size_t info = 0);
		size_t totalsz_;
		size_t passedsz_;
		nx::String fname_;
		std::ifstream file_;
		FileTypes ftype_;
		nx::String suffix_;
		nx::String prefix_;
		bool repeat_;
		bool initialized_;
		char state_[256];
		size_t statesz_;
		IPv4Generator ipv4gen_;
};

} // namespace

#endif /*__NX_CURSOR_FILE_HPP__*/


