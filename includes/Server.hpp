#ifndef SERVER_HPP
#define SERVER_HPP

#include "FdSet.hpp"
#include <iostream>
#include <string>

class Server : public FdSet
{
	private: 
		std::string name;
		std::string ip;
		std::string port;

	public:
		Server();
		Server(const Server &obj);
		Server &operator=(const Server &obj);
		virtual ~Server();
};

#endif
