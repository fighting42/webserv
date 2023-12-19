#include "../includes/Config.hpp"
#include "../includes/Server.hpp"

Config::Config() {}

Config::~Config() {}

void	err(std::string str)
{
	std::cout << str << std::endl;
	exit(1);
}

std::vector<Server>	Config::getConfig() const
{
	return config;
}

void	Config::addConfig(Server& serv)
{
	config.push_back(serv);
}

void	Config::parseConfig(std::string path)
{
	std::ifstream file(path);
	if (!file)
		err("file error");

	std::string line;
	while (std::getline(file, line))
	{
		int chk = 0;
		if (line.find("server") != std::string::npos)
		{
			Server *serv = new Server;
			if (line.find('{') != std::string::npos)
				chk++;
			while (std::getline(file, line))
			{
				size_t i = 0;
				while (isspace(line[i]))
					i++;
				if (line.find(';') == std::string::npos &&
					!line.empty() && i == line.length() && line[i] != '#' &&
					line.find('{') == std::string::npos && line.find('}') == std::string::npos)
					err("config form error");

				serv->parseDirective("listen", line);
				serv->parseDirective("server_name", line);
				serv->parseLocation(file, line);
				// 나머지 map(serv.serv)에 넣기 ?

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

// int main(int argc, char **argv)
// {
// 	Config c;
// 	if (argc == 2)
// 		c.parseConfig(argv[1]);

// 	std::vector<Server> s = c.getConfig();
// 	for (std::vector<Server>::iterator s_it = s.begin(); s_it != s.end(); ++s_it)
// 	{
// 		std::cout << "name: " << s_it->getName() << std::endl
// 			<< "port: " << s_it->getPort() << std::endl;

// 		std::map<std::string, std::map<std::string, std::string> > l = s_it->getLoc();
// 		for (std::map<std::string, std::map<std::string, std::string> >::iterator l_it = l.begin(); l_it != l.end(); ++l_it)
// 		{
// 			std::cout << "location: " << l_it->first << std::endl;

// 		}
// 	}
// }
