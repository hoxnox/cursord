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
	cursor::IPv4Generator ipv4gen(argv[1], strlen(argv[1]), false, true);
	char buf[128];
	memset(buf, 0, sizeof(buf));
	char state[512];
	size_t statesz = strlen(argv[1]);
	size_t bufsz = 1;
	memset(state, 0, sizeof(state));
	memcpy(state, argv[1], statesz);
	if(argc > 2)
	{
		while( strcmp(argv[2], buf) != 0 && bufsz != 0)
			ipv4gen(state, &statesz, sizeof(state), buf, &bufsz, sizeof(buf), 0);
	}
	ipv4gen(state, &statesz, sizeof(state), buf, &bufsz, sizeof(buf), 0);
	while(bufsz != 0)
	{
		std::cout << buf << std::endl;
		ipv4gen(state, &statesz, sizeof(state), buf, &bufsz, sizeof(buf), 0);
	}
	return 0;
}
