#ifndef SERVER_HPP
#define SERVER_HPP

#include "FdSet.hpp"
#include <iostream>
#include <string>
#include <fstream>

class Location;

// : public FdSet
class Server
{
	private: 
		std::string name;
		std::string ip;
		std::string port;

	public:
		Server();
		virtual ~Server();

		void	parseConfig(const std::string& path);
		void	parseDirective(const std::string& dir, const std::string& line);
		void	parseLocation(const std::string& line);

		// getter 만들기
};

#endif
