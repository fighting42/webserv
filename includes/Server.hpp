#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>

#include "FdSet.hpp"

class Location;

class Server : public FdSet
{
	private: 
		std::string name;
		int	port;
		std::map< std::string, std::map<std::string, std::string> > loc;

	public:
		Server();
		virtual ~Server();

		std::string	getName() const;
		int	getPort() const;
		std::map< std::string, std::map<std::string, std::string> > getLoc() const;

		void	parseDirective(const std::string& dir, std::vector<std::string>& tokens);
		bool	parseLocation(std::vector<std::string> tokens, bool);
};

#endif
