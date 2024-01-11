#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include "../includes/Request.hpp"

Client::Client() {}

Client::Client(int client_socket)
{
	socket_fd = client_socket;
	file_fd = -1;
	status = READ_SOCKET;
	written = 0;
}

Client::~Client() {}
