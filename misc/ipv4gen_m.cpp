#include <ipv4_generator.hpp>
#include <iostream>
#include <gettext.h>
#include <string>
#include <cstring>

int main(int argc, char * argv[])
{
	std::ios::sync_with_stdio(false);
	if(argc < 2)
	{
		std::cout << _("Usage: ") << argv[0] << " init [value]" << std::endl;
		return 0;
	}
	std::string start;
	cursor::IPv4Generator ipv4gen(false, true);
	char buf[128];
	memset(buf, 0, sizeof(buf));
	char state[128];
	size_t statesz = sizeof(state);
	statesz = ipv4gen.init(argv[1], strlen(argv[1]), state, statesz);
	if(statesz < 0)
	{
		std::cerr << _("Error initializing ipv4 generator") << std::endl;
		return 0;
	}
	size_t bufsz = 1;
	if(argc > 2)
	{
		while( strcmp(argv[2], buf) != 0 && bufsz != 0)
			ipv4gen(state, &statesz, sizeof(state), buf, &bufsz, sizeof(buf), 0);
	}
	memset(buf, 0, sizeof(buf));
	ipv4gen(state, &statesz, sizeof(state), buf, &bufsz, sizeof(buf), 0);
	while(bufsz != 0)
	{
		std::cout << buf << " " << ipv4gen.pos() << " of "<< ipv4gen.size()  << std::endl;
		memset(buf, 0, sizeof(buf));
		ipv4gen(state, &statesz, sizeof(state), buf, &bufsz, sizeof(buf), 0);
	}
	return 0;
}
