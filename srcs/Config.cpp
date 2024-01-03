#include "../includes/Config.hpp"
#include "../includes/Server.hpp"

Config::Config() {}

Config::~Config() {}

std::vector<Server>	Config::getConfig() const { return config; }

void	Config::addConfig(Server& serv)
{
	config.push_back(serv);
}

void	Config::parseConfig(std::string path)
{
	std::ifstream file(path);
	if (!file)
		throw "config file error";

	std::string line;
	while (std::getline(file, line))
	{
		int chk = 0;
		bool flag = false;
		if (line.find("server") != std::string::npos)
		{
			Server *serv = new Server;
			if (line.find('{') != std::string::npos)
				chk++;
			while (std::getline(file, line))
			{
				if (line.find(';') == std::string::npos && !line.empty() && \
					line.find('{') == std::string::npos && line.find('}') == std::string::npos)
					throw "config file error";
				std::istringstream iss(line);
				std::vector<std::string> tokens;
				std::string token;
				while (iss >> token)
					tokens.push_back(token);

				if (line.find("listen") != std::string::npos)
					serv->parseDirective("listen", tokens);
				else if (line.find("server_name") != std::string::npos)
					serv->parseDirective("server_name", tokens);
				else if (line.find("location") != std::string::npos || flag)
					flag = serv->parseLocation(tokens, flag);
				// else ? 나머지도 저장 해야되나??

				if (flag && line.find('}') != std::string::npos)
					flag = false;

				if (line.find('{') != std::string::npos)
					chk++;
				else if (line.find('}') != std::string::npos)
					chk--;
				if (chk == 0)
					break;
			}
			addConfig(*serv);
		}
	}
}
