#include "../includes/Config.hpp"
#include "../includes/Request.hpp"
#include "../includes/Webserv.hpp"

void	err(std::string str)
{
	std::cout << str << std::endl;
	exit(1);
}

int main(int argc, char **argv)
{
    if (argc != 2)
        err("argc error");

    Config  conf;
    Request req;
    Webserv webserv;

    conf.parseConfig(argv[1]);
    req.ReqParsing("GET ./resources/index.html http/1.1\ncontent_length: 9\ncontent_type: text/html");
    webserv.initServer(conf);

    return (0);
}
