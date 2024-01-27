#include "../includes/Event.hpp"
#include "../includes/Client.hpp"

void Event::changeEvents(std::vector<struct kevent>& change_list, uintptr_t ident, \
	int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata)
{
	struct kevent temp_event;

	EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
	if (flags & (EV_ADD | EV_ENABLE))
	{
		if (!udata)
			FD_SET(ident, &server_fds);
		else
			FD_SET(ident, &client_fds);
	}
	else if (flags & (EV_DISABLE | EV_DELETE))
	{
		if (!udata)
			FD_CLR(ident, &server_fds);
		else
			FD_CLR(ident, &client_fds);
	}
	change_list.push_back(temp_event);
}

void Event::readSocket(Client& client, std::vector<struct kevent>& change_list)
{
	std::cout << "readSocket()" << std::endl;

	char buf[1024];
	client.body_length = read(client.socket_fd, buf, 1024);
	if (client.body_length <= 0)
	{
		changeEvents(change_list, client.socket_fd, EVFILT_READ, EV_DISABLE | EV_DELETE, 0, 0, &client);
		return handleError(client, change_list, "500");
	}
	buf[client.body_length] = '\0';
	client.request.ReqParsing(buf);

	if (client.request.getChunked())
		changeEvents(change_list, client.socket_fd, EVFILT_TIMER, EV_ADD | EV_ENABLE | EV_ONESHOT, NOTE_SECONDS, 60, &client);
	if (!client.request.getChunked() || client.request.getStatus() != "200")
		changeEvents(change_list, client.socket_fd, EVFILT_READ, EV_DISABLE | EV_DELETE, 0, 0, &client);
	if (client.request.getStatus() != "200")
		handleError(client, change_list, client.request.getStatus());

	std::cout << BLUE << "[request message]" << std::endl << buf << RESET << std::endl;
}

void Event::writeSocket(Client& client, std::vector<struct kevent>& change_list)
{
	std::cout << "writeSocket()" << std::endl;

	const std::vector<char>& send_buffer = client.response.getSendBuffer();
	ssize_t write_size = send_buffer.size() - client.written > 1024 ? 1024 : send_buffer.size() - client.written;
	write_size = write(client.socket_fd, &send_buffer[client.written], write_size);
	if (write_size <= 0)
		return handleError(client, change_list, "500");
	client.written += write_size;

	if (client.written == static_cast<ssize_t>(send_buffer.size())) //다쓰면 연결해제
		client.status = DISCONNECT;
	std::map<std::string, std::string> headers = client.request.getHeaders();
	
	// ㅠㅠㅠㅠㅠㅠㅠ눈물좔좔
	// if (headers["Connection"] == "keep-alive")
	// {
	// 	changeEvents(change_list, client.socket_fd, EVFILT_WRITE, EV_DISABLE | EV_DELETE, 0, 0, &client);
	// 	changeEvents(change_list, client.socket_fd, EVFILT_TIMER, EV_ADD | EV_ENABLE | EV_ONESHOT, NOTE_SECONDS, 75, &client);
	// 	changeEvents(change_list, client.socket_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &client);
	// 	client.status = RECV_REQUEST;
	// }

	std::cout << CYAN << "[response message]" << std::endl << &send_buffer[client.written - write_size] << RESET << std::endl;
}

void Event::readFile(Client& client, std::vector<struct kevent>& change_list)
{
	std::cout << "readFile()" << std::endl;

	char buf[1024];
	client.body_length = read(client.file_fd, buf, sizeof(buf));
	changeEvents(change_list, client.file_fd, EVFILT_READ, EV_DISABLE | EV_DELETE, 0, 0, &client);
		std::cout << client.file_fd << std::endl << buf << std::endl;
	close(client.file_fd);
	if (client.body_length <= 0)
		return handleError(client, change_list, "500");
	buf[client.body_length] = '\0';
	client.body = buf;
	client.response.getBody(buf, client.body_length);
	client.response.makeResponse();
	changeEvents(change_list, client.socket_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, &client);
	client.status = SEND_RESPONSE;
}

void Event::writeFile(Client& client, std::vector<struct kevent>& change_list)
{
	std::cout << "writeFile()" << std::endl;

	// cgi를 거치지 않았을 때
	if (client.server->findValue(client.m_location, "cgi_path").empty()) 
	{
		std::string str = "";
		for (size_t i=3; i < client.request.getBody().size(); i++)
			str += client.request.getBody()[i];
		client.body = str;
		client.body_length = str.length();
	}

	// if (client.file)
	// {
	// 	write(client.file_fd, client.file, 1024);
	// }
	ssize_t write_size = write(client.file_fd, client.body.c_str(), client.body_length);
	close(client.file_fd);
	changeEvents(change_list, client.file_fd, EVFILT_WRITE, EV_DISABLE | EV_DELETE, 0, 0, &client);
	if (write_size <= 0)
		return handleError(client, change_list, "500");
	
	char str[15] = "POST SUCCESS!\n";
	client.response.getBody(str, strlen(str));
	client.response.makeResponse();

	changeEvents(change_list, client.socket_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, &client);
	client.status = SEND_RESPONSE;
}

void	Event::readPipe(Client& client, std::vector<struct kevent>& change_list)
{
	std::cout << "readPipe()" << std::endl;

	waitpid(client.pid, NULL, 0);
	
	char buf[1024];
	client.body_length = read(client.pipe_fd[0], buf, 1024);
	close(client.pipe_fd[0]);
	changeEvents(change_list, client.pipe_fd[0], EVFILT_READ, EV_DISABLE | EV_DELETE, 0, 0, &client);
	if (client.body_length <= 0)
		return handleError(client, change_list, "500");
	buf[client.body_length] = '\0';
	client.body = buf;

	if (client.request.getMethod() == "GET")
	{
		client.response.getBody(buf, client.body_length);
		client.response.makeResponse();
		changeEvents(change_list, client.socket_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, &client);
		client.status = SEND_RESPONSE;
	}
	else if (client.request.getMethod() == "POST")
		handlePost(client, change_list);
}

void	Event::writePipe(Client& client, std::vector<struct kevent>& change_list)
{
	std::cout << "writePipe()" << std::endl;

	std::string str = "";
	for (size_t i=3; i < client.request.getBody().size(); i++)
		str += client.request.getBody()[i];

	if (client.request.getMethod() == "GET")
	{
		changeEvents(change_list, client.pipe_fd[1], EVFILT_WRITE, EV_DISABLE | EV_DELETE, 0, 0, &client);
		close(client.pipe_fd[1]);
	}
	else if (client.request.getMethod() == "POST")
	{
		ssize_t write_size = write(client.pipe_fd[1], &str, str.length());
		changeEvents(change_list, client.pipe_fd[1], EVFILT_WRITE, EV_DISABLE | EV_DELETE, 0, 0, &client);
		close(client.pipe_fd[1]);
		if (write_size <= 0)
			return handleError(client, change_list, "500");
	}

	changeEvents(change_list, client.pipe_fd[0], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &client);
	client.status = READ_PIPE;
}
