#include <iostream>
#include <fstream>
#include <iterator>
#include <stdint.h>
#include <stdlib.h>
#include <nx_socket.h>

int main(int argc, char * argv[])
{
	if(argc < 3)
		return 0;
	std::ifstream in(argv[1], std::ios::in);
	std::ofstream out(argv[2], std::ios::out | std::ios::binary);
	if(!in.is_open() || !out.is_open())
	{
		printf("Error file opening");
		return 0;
	}
	std::ostreambuf_iterator<char> writer(out);
	std::string str;
	while(!in.eof())
	{
		getline(in, str);
		if(str.empty())
			continue;
		size_t tab_pos = str.find('\t');
		if(tab_pos == std::string::npos)
			continue;
		std::string v1s(str.substr(0, tab_pos));
		std::string v2s(str.substr(tab_pos + 1, str.length() - tab_pos - 1));
		uint32_t v1 = (uint32_t)htonl((uint32_t)atol(v1s.c_str()));
		uint32_t v2 = (uint32_t)htonl((uint32_t)atol(v2s.c_str()));
		std::copy((char*)&v1, (char*)&v1 + 4, writer);
		std::copy((char*)&v2, (char*)&v2 + 4, writer);
	}
	return 0;
}

