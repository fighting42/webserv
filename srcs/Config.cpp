#include "../includes/Config.hpp"
#include "../includes/Server.hpp"

Config::Config() {}

Config::~Config() {}

std::vector<Server>	Config::getServer() const { return v_server; }

void	Config::checkBracket(int& chk)
{
	if (line.find('{') != std::string::npos)
		chk++;
	else if (line.find('}') != std::string::npos)
		chk--;
}

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

void	Config::parseToken()
{
	int chk = 0;
	std::string token;

	removeCommentSpace();
	checkBracket(chk);
	std::istringstream iss(line);
	while (iss >> token)
		v_tokens.push_back(token);

	while (std::getline(file, line))
	{
		removeCommentSpace();
		if (line.empty())
			continue;
		checkBracket(chk);
		// if (chk <= 0 && v_tokens.size() == 0)
		// 	throw "config file error";

		std::istringstream iss(line);
		while (iss >> token)
			v_tokens.push_back(token);

		if (chk == 0)
			break;
	}
}

void	Config::parseServer()
{
	v_tokens.clear();
	parseToken();

	Server *server = new Server;
	std::map<std::string, std::string> m_server;
	std::map< std::string, std::map<std::string, std::string> > m_location;

	for (size_t i = 0; i < v_tokens.size(); i++)
	{
		if (v_tokens[i] == "server" || v_tokens[i] == "{" || v_tokens[i] == "}")
			continue;
		
		if (v_tokens[i] == "location")
		{
			std::string uri = v_tokens[++i];
			i += 2;
			std::map<std::string, std::string> location;
			while (v_tokens[i] != "}")
			{
				std::string directive = v_tokens[i++];
				while (v_tokens[i].find(';') == std::string::npos)
					location.insert(std::pair<std::string, std::string>(v_tokens[i++], directive));
				v_tokens[i] = v_tokens[i].substr(0, v_tokens[i].size() - 1);
				location.insert(std::pair<std::string, std::string>(v_tokens[i], directive));
				i++;
			}
			m_location.insert(std::pair< std::string, std::map<std::string, std::string> >(uri, location));
		}
		else
		{
			std::string directive = v_tokens[i++];
			if (directive == "listen")
				server->setPort(v_tokens[i]);
			else if (directive == "server_name")
				server->setName(v_tokens[i]);
			
			while (v_tokens[i].find(';') == std::string::npos)
				m_server.insert(std::pair<std::string, std::string>(v_tokens[i++], directive));
			v_tokens[i] = v_tokens[i].substr(0, v_tokens[i].size() - 1);
			m_server.insert(std::pair<std::string, std::string>(v_tokens[i], directive));
		}
	}
	server->setServer(m_server);
	server->setLocation(m_location);
	v_server.push_back(*server);
}

void	Config::parseConfig(std::string path)
{
	file.open(path);
	if (!file)
		throw "config file error";

	while (std::getline(file, line))
	{
		removeCommentSpace();
		if (line.find("server") != std::string::npos)
			parseServer();
		// else if (!line.empty() && line.find('{') != std::string::npos \
		// 	&& line.find('}') != std::string::npos)
		// 	throw "config file error";
	}

// 	for (size_t i = 0; i < v_server.size(); i++)
// 	{
//     Server s = v_server[i];

//     std::map<std::string, std::string> m_s = s.getServer();
//     for (std::map<std::string, std::string>::iterator s_it = m_s.begin(); s_it != m_s.end(); ++s_it)
//     {
//         std::cout << s_it->second << " " << s_it->first << std::endl;
//     }

//     std::map<std::string, std::map<std::string, std::string> > m_l = s.getLocation();
//     for (std::map<std::string, std::map<std::string, std::string> >::iterator l_it = m_l.begin(); l_it != m_l.end(); ++l_it)
//     {
//         std::cout << "location " << l_it->first << std::endl;
//         std::map<std::string, std::string> l = l_it->second;
//         for (std::map<std::string, std::string>::iterator it = l.begin(); it != l.end(); ++it)
//         {
//             std::cout << it->second << " " << it->first << std::endl;
//         }
//     }
// 	std::cout << std::endl;
// }

}
