#include "../includes/Client.hpp"
#include "../includes/Request.hpp"

Client::Client() {}

Client::Client(int client_socket) : fd(client_socket) {}

Client::~Client() {}

int Client::getFd() { return fd; }

Status  Client::getStatus() { return status; }

void	Client::setStatus(Status status) { this->status = status; }

void Client::HandleSocketRead()
{
	Request Req;
	char buf[1024];
	int msg_length = read(this->fd, buf, 1024);
	(void)msg_length; // make를 위한 void입니당 지워도됩니당
	Req.ReqParsing(buf);
}

void Client::HandleSocketWrite()
{

}
