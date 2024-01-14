#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include "../includes/Request.hpp"

Client::Client() {}

Client::Client(int client_socket, std::string client_ip)
{
	socket_fd = client_socket;
	file_fd = -1;
	pipe_fd[0] = -1;
	pipe_fd[1] = -1;
	ip = client_ip;
	status = READ_SOCKET;
	written = 0;
}

Client::~Client() {}
