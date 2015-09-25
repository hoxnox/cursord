#include <iostream>
#include <fstream>
#include <cursor_file.hpp>
#include <string.hpp>
#include <deque>
#include <cursor.hpp>
#include <nx_socket.h>

using namespace cursor;

class CursorFileUnprotected : public CursorFile
{
	public:
		CursorFileUnprotected(const Cursor::Args args)
			: CursorFile(args)
		{
		}
		int next(const size_t count, std::deque<nx::String>& buf)
		{
			return do_next(count, buf);
		}
};

int main(int argc, char * argv[])
{
	std::ios::sync_with_stdio(false);
	if (argc < 2)
	{
		std::cout << "Usage: " << argv[0] << " ipv4rg_file" << std::endl;
		return 0;
	}

	Cursor::Args args;
	args[L"name"] = nx::String::fromUTF8(argv[1]);
	args[L"type"] = L"ipv4rg";
	CursorFileUnprotected cf(args);
	const size_t BUFSZ = 1000;
	std::deque<nx::String> buf;
	while(true)
	{
		if (cf.next(BUFSZ, buf) == -1)
			return 0;
		if (buf.size() == 0)
			return 0;
		for(auto i : buf)
		{
			if(i == L"END")
				return 0;
			std::cout << i.toUTF8() << std::endl;
		}
		buf.clear();
	}
	return 0;
}

