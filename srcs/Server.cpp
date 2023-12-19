#include "../includes/Server.hpp"
#include "../includes/Config.hpp"

Server::Server() {}

Server::~Server() {}

std::string	Server::getName() const { return name; }

std::string	Server::getIP() const { return ip; } 

std::string	Server::getPort() const { return port; }

std::map< std::string, std::map<std::string, std::string> > Server::getLoc() const { return loc; }


bool	Server::parseLocation(std::ifstream& file, std::vector<std::string> tokens, bool flag)
{
	if (!flag)
	{
		std::map<std::string, std::string> map;
		loc.insert(std::pair< std::string, std::map<std::string, std::string> >(tokens[1], map));
	}
	else
	{
		for (size_t i = 1; i < tokens.size(); i++)
		{
			if (i == tokens.size() - 1)
				tokens[i] = tokens[i].substr(0, tokens[i].size() - 1);
			(--loc.end())->second.insert(std::pair<std::string, std::string>(tokens[i], tokens[0]));			
		}
	}
	return true;
}

void	Server::parseDirective(const std::string& dir, std::vector<std::string>& tokens)
{
	// 여러개 올 수도 있나???
	if (dir == "listen")
		port = tokens[1].substr(0, tokens[1].size() - 1);
	else if (dir == "server_name")
		name = tokens[1].substr(0, tokens[1].size() - 1);
}
