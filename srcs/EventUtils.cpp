#include "../includes/Event.hpp"
#include "../includes/Client.hpp"

std::map<std::string, std::string> m_mime_type;
fd_set server_fds;
fd_set client_fds;

void Event::setMimeType()
{
	m_mime_type.insert(std::pair<std::string, std::string>("text/html", ".html"));
	m_mime_type.insert(std::pair<std::string, std::string>("text/plain", ".txt"));
	m_mime_type.insert(std::pair<std::string, std::string>("image/png", ".png"));
	m_mime_type.insert(std::pair<std::string, std::string>("multipart/form-data", ".binary"));
	m_mime_type.insert(std::pair<std::string, std::string>("application/octet-stream", "")); // .bin ??
}

std::string Event::getMimeType(std::string extension)
{
	for (std::map<std::string, std::string>::iterator it = m_mime_type.begin(); \
		it != m_mime_type.end(); ++it)
	{
		if (it->second == extension)
			return it->first;
	}
	return "";
}

void	Event::findLocation(Client& client)
{
	std::string uri = client.request.getUri();
	std::map< std::string, std::map<std::string, std::string> > m_location = client.server->getLocation();
	for (std::map< std::string, std::map<std::string, std::string> >::iterator it = m_location.begin(); \
		it != m_location.end(); ++it)
	{
		std::string l_uri = it->first;
		if (uri.length() != l_uri.length())
			l_uri += "/";
		if (strncmp(uri.c_str(), l_uri.c_str(), l_uri.length()) == 0)
		{
			client.location_uri = it->first;
			client.m_location = it->second;
			break;
		}
	}
	if (client.m_location.size() == 0)
		client.m_location = client.server->getLocation()["/"];
	
	std::vector<std::string> v_redirect = client.server->findValue(client.m_location, "redirect");
	if (!v_redirect.empty())
	{
		std::string new_uri = v_redirect[0];
		new_uri += client.request.getUri().substr(client.location_uri.length());
		if (new_uri.empty())
			new_uri = "/";
		else if (new_uri[0] != '/')
			new_uri = "/" + new_uri;

		// std::cout << "redirect uri: " << new_uri << std::endl;
		findLocation(client); // 변경된 uri의 로케이션 블록 저장
		// client.request.setUri(new_uri); // 필요함
		// client.response.setStatus("301");
		// m_status["301"] = "Moved Permanently"; // 추가 필요
	}
}

std::string	Event::getRootpath(Client& client)
{
	std::string path = "";
	std::vector<std::string> v_root = client.server->findValue(client.m_location, "root");
	if (v_root.size() != 0)
			path += v_root[0];
	if (client.request.getUri() != client.location_uri)
	{
		if (client.location_uri == "/")
			path += "/";
		path += client.request.getUri().substr(client.location_uri.length());
	}
	if (path[0] == '/')
		path = path.substr(1);
	return path;
}

std::string	Event::getRootpath(Client& client, std::string path)
{
	std::string ret_path = "";
	std::vector<std::string> v_root = client.server->findValue(client.m_location, "root");
	if (v_root.size() != 0)
		ret_path += v_root[0] + "/";
	ret_path += path;
	return ret_path;
}

char** Event::setEnvp(Client& client, std::string cgi_path)
{
	std::vector<std::string> v_env;
	std::map<std::string, std::string> m_headers = client.request.getHeaders();

	v_env.push_back("SERVER_SOFTWARE=webserv");
	v_env.push_back("SERVER_PROTOCOL=HTTP/1.1");
	v_env.push_back("GATEWAY_INTERFACE=CGI/1.1");
	v_env.push_back("SERVER_NAME=" + client.server->getName());
	std::stringstream ss;
	ss << client.server->getPort();
	v_env.push_back("SERVER_PORT=" + ss.str());
	v_env.push_back("REMOTE_ADDR=" + client.ip);

	std::string path_info = client.request.getUri().substr(client.location_uri.length());
	if (client.request.getUri().length() == client.location_uri.length())
		v_env.push_back("PATH_INFO=/");
	else
		v_env.push_back("PATH_INFO=" + path_info);

	v_env.push_back("PATH_TRANSLATED=" + cgi_path + path_info);

	std::string method = client.request.getMethod();
	v_env.push_back("REQUEST_METHOD=" + method);
	if (method == "GET")
		v_env.push_back("QUERY_STRING=" + client.request.getQueryStr());
	else if (method == "POST")
	{
		v_env.push_back("CONTENT_LENGTH=" + m_headers["Content-Length"]);
		v_env.push_back("CONTENT_TYPE=" + m_headers["Content-Type"]);
	}

	char** env = new char*[v_env.size() + 1];
	for (size_t i = 0; i < v_env.size(); i++)
		env[i] = strdup(v_env[i].c_str());

	return env; 
}

void	Event::execCgi(Client& client, std::vector<struct kevent>& change_list)
{
	int fds1[2];
	int fds2[2];
	if (pipe(fds1) == -1 || pipe(fds2) == -1)
		return handleError(client, change_list, "500");

	client.pid = fork();
	if (client.pid < 0)
		return handleError(client, change_list, "500");
	else if (client.pid == 0) // 자식
	{
		dup2(fds1[0], STDIN_FILENO); // fds1[0] 입력
		dup2(fds2[1], STDOUT_FILENO); // fds2[1] 출력
		close(fds1[0]);
		close(fds1[1]);
		close(fds2[0]);
		close(fds2[1]);

		std::string cgi_path = client.server->findValue(client.m_location, "cgi_path")[0];
		cgi_path = getRootpath(client, cgi_path);

		char *tmp = strdup(cgi_path.c_str());
		char *args[2] = {tmp, NULL};
		char **envp = setEnvp(client, cgi_path);
		execve(args[0], args, envp);
		exit(1); // error
	}
	else // 부모
	{
		close(fds1[0]);
		close(fds2[1]);
		client.pipe_fd[0] = fds2[0]; // fds2[0] read
		client.pipe_fd[1] = fds1[1]; // fds1[1] write
	}
}

void Event::handleAutoindex(Client& client, std::vector<struct kevent>& change_list, std::string uri) 
{
	// html 코드 생성
	std::string autoindex_html = "<html><head><meta charset=\"UTF-8\"></head>" \
    "<style>body { background-color: white; font-family: Trebuchet MS;}</style>" \
	"<body><h1>" + uri + " List</h1><ul>";
	autoindex_html += "<table>";
	// 목록 채우기
	struct dirent* entry;
	DIR* dp = opendir(uri.c_str());
	if (dp != NULL) 
	{
		while ((entry = readdir(dp))) 
		{
			std::string entry_name = entry->d_name;
            std::string entry_path = "http://" + client.request.getHost() + client.request.getUri();
			if (client.request.getUri() != "/")
				entry_path += "/";
			entry_path += entry_name;
			autoindex_html += "<tr><td><a href='" + entry_path + "'>" + entry_name + "</a>" + "</td></tr>";
		}
		closedir(dp);
	}
	autoindex_html += "</table></ul></body></html>";
	// 응답 body에 써주기
	client.response.setBody(std::vector<char>(autoindex_html.begin(), autoindex_html.end()));
	client.response.makeResponse();
	changeEvents(change_list, client.socket_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, &client);
	client.status = SEND_RESPONSE;
}

void	Event::checkConfig(Client& client, std::vector<struct kevent>& change_list)
{
	// allow_method(405), client_max_body_size(413) 확인하기
	std::vector<std::string> v_allow_method = client.server->findValue(client.m_location, "allow_method");
	bool methodAllowed = false;
	if (v_allow_method.size() > 0)
	{
		for (size_t i = 0; i < v_allow_method.size(); ++i) 
		{
			if (v_allow_method[i] == client.request.getMethod())
			{
				methodAllowed = true;
				break;
			}
		}
		if (!methodAllowed)
			return handleError(client, change_list, "405");
	}

	std::vector<std::string> v_client_max_body_size = client.server->findValue(client.m_location, "client_max_body_size");
	std::map<std::string, std::string> m_headers = client.request.getHeaders();
	std::stringstream ss;
	std::stringstream ss1;
	int num_client_max_body_size;
	int num_m_headers;
	if ((v_client_max_body_size.size() > 0) && (client.request.getMethod() == "POST"))
	{
		ss << v_client_max_body_size[0];
		ss >> num_client_max_body_size;
		ss1 << m_headers["Content-Length"];
		ss1 >> num_m_headers;
		// std::cout << "맥스바디사이즈 :" <<num_client_max_body_size << std::endl;
		// std::cout << "컨텐트 렝뜨: " <<m_headers["Content-Length"] << std::endl;
		// std::cout << "헤더,,,:" <<num_m_headers << std::endl;
		if (num_client_max_body_size < num_m_headers)
		{
			return handleError(client, change_list, "413");
		}
	}
}
