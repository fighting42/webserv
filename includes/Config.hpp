#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Server.hpp"

class Config
{
	private:
		std::vector<Server> config;

	public:
		Config();
		~Config();

		std::vector<Server>	getConfig() const;
		void	addConfig(Server& serv);
		void	parseConfig(std::string path);
};

void	error(std::string);

#endif
