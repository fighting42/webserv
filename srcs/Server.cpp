#include "../includes/Server.hpp"

Server::Server() {}

Server::~Server() {}

void	err(std::string str)
{
	std::cout << str << std::endl;
	exit(1);
}

void	Server::parseLocation(const std::string& line)
{
	size_t pos = line.find("location");
	if (pos == std::string::npos)
		return ;
	
	// location class func
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

void	Server::parseConfig(const std::string& path)
{
	std::ifstream conf(path);
	if (!conf)
		err("file error");

	std::string line;
	while (std::getline(conf, line))
	{
		size_t i = 0;
		while (isspace(line[i]))
			i++;
		if (line.find(';') == std::string::npos && \
			!line.empty() && i == line.length() && line[i] != '#' && \
			line.find('{') == std::string::npos && line.find('}') == std::string::npos)
			err("; error");

		parseDirective("listen", line);
		parseDirective("server_name", line);
		parseLocation(line);
	}

	// std::cout << "port: " << port << std::endl
	// 		<< "name: " << name << std::endl;
}

// int main(int argc, char **argv)
// {
// 	Server s;
// 	if (argc == 2)
// 		s.parseConfig(argv[1]);
// }
