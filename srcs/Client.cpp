#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include "../includes/Request.hpp"

Client::Client(): socket_fd(0), ip("") {}

Client::Client(int client_socket, std::string client_ip)
{
	socket_fd = client_socket;
	file_fd = -1;
	pipe_fd[0] = -1;
	pipe_fd[1] = -1;
	ip = client_ip;
	status = RECV_REQUEST;
	written = 0;
	file = NULL;
	body_length = 0;
	location_uri = "/";
}

Client::~Client() {}
