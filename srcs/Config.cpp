#include "../includes/Config.hpp"
#include "../includes/Server.hpp"

Config::Config() {}

Config::~Config()
{
	for (std::vector<Server *>::iterator it = v_server.begin(); it != v_server.end(); ++it)
		delete *it;
}

std::vector<Server *>	Config::getServer() const { return v_server; }

void	Config::removeCommentSpace()
{
	size_t pos = line.find('#');
    if (pos != std::string::npos) 
        line = line.substr(0, pos);
	
	size_t i = 0;
	while (isspace(line[i]))
		i++;
	line = line.substr(i);
	
	i = line.size() - 1;
	while (isspace(line[i]))
		i--;
	line = line.substr(0, i + 1);
}

void	Config::checkToken(std::string str, std::string token)
{
	if (token != str)
		throw "config file error";
}

void	Config::insertToken(std::map<std::string, std::string>& map, size_t& i)
{
	std::string directive = v_tokens[i++];
	if (directive.find(';') != std::string::npos)
		throw "config file error";

	while (v_tokens[i].find(';') == std::string::npos)
		map.insert(std::pair<std::string, std::string>(v_tokens[i++], directive));
	v_tokens[i] = v_tokens[i].substr(0, v_tokens[i].size() - 1);
	map.insert(std::pair<std::string, std::string>(v_tokens[i], directive));
}

void	Config::parseServer()
{
	for (size_t i = 0; i < v_tokens.size(); i++)
	{
		checkToken("server", v_tokens[i++]);
		checkToken("{", v_tokens[i]);

		Server *server = new Server;
		std::map<std::string, std::string> m_server;
		std::map< std::string, std::map<std::string, std::string> > m_location;
		while (v_tokens[++i] != "}")
		{
			if (v_tokens[i] == "location")
			{
				std::string uri = v_tokens[++i];
				checkToken("{", v_tokens[++i]);

				std::map<std::string, std::string> location;
				while (v_tokens[++i] != "}")
					insertToken(location, i);

				m_location.insert(std::pair< std::string, std::map<std::string, std::string> >(uri, location));
			}
			else
			{
				if (v_tokens[i] == "listen")
					server->setPort(v_tokens[i + 1]);
				else if (v_tokens[i] == "server_name")
					server->setName(v_tokens[i + 1]);

				insertToken(m_server, i);
			}
		}
		server->setServer(m_server);
		server->setLocation(m_location);
		v_server.push_back(server);
	}
}

void	Config::parseToken()
{
	int chk = 0;
	std::string token;

	while (std::getline(file, line))
	{
		removeCommentSpace();
		if (line.empty())
			continue;

		if (line.find('{') != std::string::npos)
			chk++;
		else if (line.find('}') != std::string::npos)
			chk--;
		if (chk < 0)
			throw "config file error";

		std::istringstream iss(line);
		while (iss >> token)
		{
			if (token == ";")
				throw "config file error";
			v_tokens.push_back(token);
		}
	}
	if (chk != 0)
		throw "config file error";
}

void	Config::checkConfig()
{
	for (std::vector<Server *>::iterator it = v_server.begin(); it != v_server.end(); ++it)
	{
		if ((*it)->getName() == "" || (*it)->getPort() < 0 || \
			(*it)->getServer().size() == 0 || (*it)->getLocation().size() == 0)
			throw "config file error";
	}
}

void	Config::parseConfig(std::string path)
{
	file.open(path);
	if (!file)
		throw "config file error";

	parseToken();
	parseServer();
	checkConfig();
}
