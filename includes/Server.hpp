#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>

class Location;

class Server
{
	private:
		std::string name;
		int	port;
		std::map<std::string, std::string> m_server;
		std::map< std::string, std::map<std::string, std::string> > m_location;

		int socket_fd;

	public:
		Server();
		~Server();

		std::string	getName() const;
		int	getPort() const;
		std::map<std::string, std::string>	getServer() const;
		std::map< std::string, std::map<std::string, std::string> >	getLocation() const;
		int	getSocketFd() const;
		void	setName(std::string name);
		void	setPort(std::string port);
		void	setServer(std::map<std::string, std::string> m_server);
		void	setLocation(std::map< std::string, std::map<std::string, std::string> > m_location);
		void	setSocketFd(int server_socket);
};

#endif
