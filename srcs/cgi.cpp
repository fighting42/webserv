#include "../includes/Event.hpp"
#include "../includes/Client.hpp"

std::string getRealPath(Client& client, std::string path)
{
	std::string ret = "";
	std::vector<std::string> v_root = client.server->findValue(client.m_location, "root");
	if (v_root.size() != 0)
		ret += v_root[0] + "/";
	ret += path;
	return ret;
}

char** setEnvp(Client& client, std::string cgi_pass)
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

	v_env.push_back("SCRIPT_NAME="); // 모르게따
	std::string path_info = client.request.getUri().substr(client.location_uri.length());
	if (client.request.getUri().length() == client.location_uri.length())
		v_env.push_back("PATH_INFO=/");
	else
		v_env.push_back("PATH_INFO=" + path_info);
	v_env.push_back("PATH_TRANSLATED=" + cgi_pass + path_info);

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

		std::string cgi_pass = client.server->findValue(client.m_location, "cgi_pass")[0];
		cgi_pass = getRealPath(client, cgi_pass);
		char *tmp = strdup(cgi_pass.c_str());

		char *args[2] = {tmp, NULL};
		char **envp = setEnvp(client, cgi_pass);
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
