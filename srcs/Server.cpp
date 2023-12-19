#include "../includes/Server.hpp"
#include "../includes/Config.hpp"

Server::Server() {}

Server::~Server() {}

std::string	Server::getName() const { return name; }

std::string	Server::getIP() const { return ip; } 

std::string	Server::getPort() const { return port; }

std::map< std::string, std::map<std::string, std::string> > Server::getLoc() const { return loc; }


void	Server::parseLocation(std::ifstream& file, std::string& line)
{
	size_t pos = line.find("location");
	if (pos == std::string::npos)
		return ;

	std::istringstream iss(line);
	std::string path;
	iss >> path;
	iss >> path;

	std::map<std::string, std::string> map;
	while (std::getline(file, line))
	{
		// size_t i = 0;
		// while (isspace(line[i]))
		// 	i++;
		// if (line.find(';') == std::string::npos &&
		// 	!line.empty() && i == line.length() && line[i] != '#' &&
		// 	line.find('{') == std::string::npos && line.find('}') == std::string::npos)
		// 	err("config form error");

		std::istringstream iss(line);
		std::vector<std::string> tokens;
		std::string key, token;
		iss >> key;
		while (iss >> token) 
			tokens.push_back(token);
		for (size_t i = 0; i < tokens.size(); i++) 
			map.insert(std::pair<std::string, std::string>(tokens[i], key));

		if (line.find('}') != std::string::npos)
			break;
	}
	loc.insert(std::pair< std::string, std::map<std::string, std::string> >(path, map));
}

void	Server::parseDirective(const std::string& dir, const std::string& line)
{
	size_t pos = line.find(dir);
	if (pos == std::string::npos)
		return ;

	size_t i = dir.length();
	while (isspace(line[pos + i]))
		i++;
	if (i == dir.length())
		err("directive error");
	size_t cnt = line.find(';') - pos - i;

	if (dir == "listen")
		port = line.substr(pos + i, cnt);
	else if (dir == "server_name")
		name = line.substr(pos + i, cnt);
}
