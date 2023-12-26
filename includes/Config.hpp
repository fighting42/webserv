#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>

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

void	err(std::string);

#endif
