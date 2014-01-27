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
		CursorFileUnprotected(const Cursor::Sockaddr addr, const Cursor::Args args)
			: CursorFile(addr, args)
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
		std::cout << "Usage: " << argv[0]
			<< " ipv4rg_file expandfile" << std::endl;
		return 0;
	}

	Cursor::Sockaddr addr;
	addr.ss_family = AF_INET;
	((sockaddr_in *)&addr)->sin_addr.s_addr = inet_addr("127.0.0.1");
	((sockaddr_in *)&addr)->sin_port = 54321;
	Cursor::Args args;
	args[L"name"] = nx::String::fromUTF8(argv[1]);
	args[L"type"] = L"ipv4rg";
	CursorFileUnprotected cf(addr, args);
	std::ofstream ofile(argv[2]);
	if (!ofile.is_open())
	{
		std::cout << "Error opening out file: " << argv[2] << std::endl;
		return 0;
	}
	const size_t BUFSZ = 1000;
	std::deque<nx::String> buf(BUFSZ);
	while(true)
	{
		cf.next(BUFSZ, buf);
		for(size_t i = 0; i < BUFSZ && i < buf.size(); ++i)
		{
			if(buf[i] == L"END")
				return 0;
			ofile << buf[i].toUTF8() << std::endl;
		}
		buf.clear();
	}

	return 0;
}

