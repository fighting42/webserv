#include "../includes/Event.hpp"
#include "../includes/Client.hpp"

char** setEnvp(Client& client)
{
	std::vector<std::string> v_env;
	std::map<std::string, std::string> m_headers = client.request.getHeaders();

	v_env.push_back("SERVER_SOFTWARE=webserv");
	v_env.push_back("SERVER_PROTOCOL=HTTP/1.1");
	v_env.push_back("SERVER_NAME=" + client.server->getName());
	std::stringstream ss;
	ss << client.server->getPort();
	v_env.push_back("SERVER_PORT=" + ss.str());
	
	v_env.push_back("GATEWAY_INTERFACE=CGI/1.1");
	v_env.push_back("PATH_INFO=."); // localhost:8080/cgi/~~이부분~~
	v_env.push_back("SCRIPT_NAME="); // path_info 앞부분 (host + uri)
	v_env.push_back("PATH_TRANSLATED="); // 실제 경로 (root 포함)
	
	v_env.push_back("REMOTE_ADDR=" + client.ip);
	// v_env.push_back("REMOTE_HOST=");
	// v_env.push_back("REMOTE_IDENT=");
	// v_env.push_back("REMOTE_USER=");
	// v_env.push_back("AUTH_TYPE=");
	
	std::string method = client.request.getMethod();
	v_env.push_back("REQUEST_METHOD=" + method);
	if (method == "GET")
		v_env.push_back("QUERY_STRING="); // ? 뒤 값
	else if (method == "POST")
	{
		// v_env.push_back("CONTENT_LENGTH=" + m_headers["Content-Length"]);
		// white space 들어가는듯ㅜ 이상해!!!!
		v_env.push_back("CONTENT_LENGTH=12");
		v_env.push_back("CONTENT_TYPE=" + m_headers["Content-Type"]);
	}

	char** env = new char*[v_env.size() + 1];
	for (size_t i = 0; i < v_env.size(); i++)
	{
		env[i] = new char[v_env[i].length() + 1];
		std::strcpy(env[i], v_env[i].c_str());
	}

	return env; 
}

void	Event::execCgi(Client& client)
{
	int fds1[2];
	int fds2[2];
	if (pipe(fds1) == -1 || pipe(fds2) == -1)
		return ; // error

	client.pid = fork();
	if (client.pid < 0)
		return ; // error
	else if (client.pid == 0) // 자식
	{
		dup2(fds1[0], STDIN_FILENO); // fds1[0] 입력
		dup2(fds2[1], STDOUT_FILENO); // fds2[1] 출력
		close(fds1[0]);
		close(fds1[1]);
		close(fds2[0]);
		close(fds2[1]);

		std::string tmp = client.server->findValue(client.m_location, "cgi_pass")[0];
		if (client.server->findValue(client.m_location, "root").size() > 0)
			tmp = client.server->findValue(client.m_location, "root")[0] + "/" + tmp;
		char *cgi_path = new char[tmp.length() + 1];
		std::strcpy(cgi_path, tmp.c_str());

		char *args[3] = {cgi_path, NULL, NULL};
		char **envp = setEnvp(client);
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
