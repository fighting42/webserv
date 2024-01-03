#include "../includes/Client.hpp"
#include "../includes/Request.hpp"

Client::Client() {}

Client::Client(int client_socket) : fd(client_socket) {}

Client::~Client() {}

void Client::HandleSocketRead()
{
    Request Req;
    char buf[1024];
    int msg_length = read(this->fd, buf, 1024);

    Req.ReqParsing(buf);
}

void Client::HandleSocketWrite()
{

}
