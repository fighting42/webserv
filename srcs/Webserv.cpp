#include "../includes/Webserv.hpp"

Webserv::Webserv() {}

Webserv::~Webserv() {}

void Webserv::start(int argc, char **argv)
{
	if (argc == 1)
		config.parseConfig("config/default.conf");
	else if (argc == 2)
		config.parseConfig(argv[1]);
	else
		throw "argc error";

	kqueue.initServer(config);
	kqueue.startServer();

	// Request request;
	// request.ReqParsing("GET ./resources/index.html http/1.1\ncontent_length: 9\ncontent_type: text/html");
}
