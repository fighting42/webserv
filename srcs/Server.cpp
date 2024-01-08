#include "../includes/Server.hpp"
#include "../includes/Config.hpp"

Server::Server() : name(""), port(0) {}

Server::~Server() {}

std::string	Server::getName() const { return name; }

int	Server::getPort() const { return port; }

std::map<std::string, std::string>	Server::getServer() const { return m_server; }

std::map< std::string, std::map<std::string, std::string> > Server::getLocation() const { return m_location; }

int Server::getSocketFd() const { return socket_fd; }

void	Server::setName(std::string name)
{
	if (name.find(';') != std::string::npos)
		this->name = name.substr(0, name.size() - 1);
	else
		this->name = name;
}

void	Server::setPort(std::string port)
{ 
	if (port.find(';') != std::string::npos)
		port = port.substr(0, port.size() - 1);
	std::stringstream ss(port);
	ss >> this->port;
}

void	Server::setServer(std::map<std::string, std::string> m_server) { this->m_server = m_server; }

void	Server::setLocation(std::map< std::string, std::map<std::string, std::string> > m_location) { this->m_location = m_location; }

void	Server::setSocketFd(int server_socket) { socket_fd = server_socket; }

std::vector<std::string> Server::findValue(std::map<std::string, std::string>& map, std::string key)
{
	std::vector<std::string> v_ret;
	for (std::map<std::string, std::string>::iterator it = map.begin(); it != map.end(); ++it)
	{
		if (it->second == key)
			v_ret.push_back(it->first);
	}
	return v_ret;
}
