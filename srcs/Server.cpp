#include "../includes/Server.hpp"
#include "../includes/Config.hpp"

Server::Server() 
	: name(""), ip(""), port(0) {}

Server::~Server() {}

std::string	Server::getName() const { return name; }

std::string	Server::getIP() const { return ip; } 

int	Server::getPort() const { return port; }

std::map< std::string, std::map<std::string, std::string> > Server::getLoc() const { return loc; }


bool	Server::parseLocation(std::vector<std::string> tokens, bool flag)
{
	static std::string loc_path;
	if (!flag)
	{
		loc_path = tokens[1];
		std::map<std::string, std::string> map;
		loc.insert(std::pair< std::string, std::map<std::string, std::string> >(loc_path, map));
	}
	else
	{
		for (size_t i = 1; i < tokens.size(); i++)
		{
			if (i == tokens.size() - 1)
				tokens[i] = tokens[i].substr(0, tokens[i].size() - 1);
			loc[loc_path].insert(std::pair<std::string, std::string>(tokens[i], tokens[0]));			
		}
	}
	return true;
}

void	Server::parseDirective(const std::string& dir, std::vector<std::string>& tokens)
{
	if (dir == "listen")
	{
		std::istringstream iss(tokens[1].substr(0, tokens[1].size() - 1));
		iss >> port; 
	}
	else if (dir == "server_name")
		name = tokens[1].substr(0, tokens[1].size() - 1);
}
