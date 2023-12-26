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

    Config  conf;
    Request req;
    Webserv webserv;

    conf.parseConfig(path);
    req.ReqParsing("GET ./resources/index.html http/1.1\ncontent_length: 9\ncontent_type: text/html");
    Server serv = conf.getConfig()[0]; // 첫번째 서버 블록
    webserv.initServer(serv); 

    return (0);
}
