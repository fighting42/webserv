#include "../includes/Server.hpp"
#include "../includes/Request.hpp"
#include "../includes/Webserv.hpp"

int main(int argc, char **argv)
{
    if (argc != 2)
        return 1; // error 처리 하기

    Server serv;
    Request req;
    Webserv webserv;

    serv.parseConfig(argv[1]);
    req.ReqParsing("GET ./resources/index.html http/1.1\ncontent_length: 9\ncontent_type: text/html");
    webserv.initServer(serv);

    return (0);
}
