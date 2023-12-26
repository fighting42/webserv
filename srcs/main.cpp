#include "../includes/Config.hpp"
#include "../includes/Server.hpp"
#include "../includes/Request.hpp"
#include "../includes/Webserv.hpp"

void	error(std::string str)
{
	std::cerr << str << std::endl;
	exit(1);
}

int main(int argc, char **argv)
{
    std::string path;
    if (argc == 1)
        path = "config/default.conf";
    else if (argc == 2)
        path = argv[1];
    else
        error("argc error");

    Config  config;
    Request request;
    Webserv webserv;

    config.parseConfig(path);
    request.ReqParsing("GET ./resources/index.html http/1.1\ncontent_length: 9\ncontent_type: text/html");
    webserv.initServer(config);
    webserv.startServer();

    return 0;
}
