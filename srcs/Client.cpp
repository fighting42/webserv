#include "../includes/Client.hpp"
#include "../includes/Request.hpp"

Client::Client() {}

Client::Client(int client_socket)
	: socket_fd(client_socket), status(RECV_REQUEST) {}

Client::~Client() {}

int Client::getSocketFd() { return socket_fd; }

int Client::getStatus() { return status; }

void	Client::setStatus(Status status) { this->status = status; }

void	Client::setConfig()
{

}

void Client::HandleSocketRead()
{
	Request Req;
	char buf[1024];
	int msg_length = read(this->socket_fd, buf, 1024);
	(void)msg_length; // make를 위한 void입니당 지워도됩니당
	Req.ReqParsing(buf);
}

void Client::HandleSocketWrite()
{

}
