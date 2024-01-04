#include "../includes/Webserv.hpp"

int main(int argc, char **argv)
{
	try
	{
		Webserv webserv;
		webserv.start(argc, argv);
	}
	catch(const char* expn)
	{
		std::cerr << "[error] " << expn << std::endl;
	}

	return 0;
}
