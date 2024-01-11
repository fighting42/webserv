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
		std::vector<Server *> v_server;
		
		std::ifstream file;
		std::string line;
		std::vector<std::string> v_tokens;

	public:
		Config();
		~Config();

		std::vector<Server *>	getServer() const;

		void	parseConfig(std::string path);
		void	parseToken();
		void	parseServer();
		void	insertToken(std::map<std::string, std::string>& map, std::map<std::string, std::string>& map_ep, size_t& i);
		void	checkToken(std::string str, std::string token);
		void	removeCommentSpace();
		void	checkConfig();

};

#endif
