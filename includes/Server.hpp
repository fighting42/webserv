#ifndef SERVER_HPP
#define SERVER_HPP

#include "FdSet.hpp"
#include <iostream>
#include <string>

class Location;

class Server : public FdSet
{
	private: 
		std::string name;
		std::string ip;
		std::string port;

	public:
		Server();
		virtual ~Server();

};

#endif
