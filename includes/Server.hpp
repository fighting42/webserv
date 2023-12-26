#ifndef SERVER_HPP
#define SERVER_HPP

#include "FdSet.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>

class Location;

// : public FdSet
class Server
{
	private: 
		std::string name;
		std::string ip; // 아직
		int	port;
		std::map< std::string, std::map<std::string, std::string> > loc;

	public:
		Server();
		virtual ~Server();

		std::string	getName() const;
		std::string	getIP() const;
		int	getPort() const;
		std::map< std::string, std::map<std::string, std::string> > getLoc() const;

		void	parseDirective(const std::string& dir, std::vector<std::string>& tokens);
		bool	parseLocation(std::vector<std::string> tokens, bool);
};

#endif
