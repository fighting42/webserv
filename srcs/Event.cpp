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
	m_mime_type.insert(std::pair<std::string, std::string>("application/octet-stream", ""));
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

void Event::changeEvents(std::vector<struct kevent>& change_list, uintptr_t ident, \
	int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata)
{
	struct kevent temp_event;

	EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
	if (flags == (EV_ADD | EV_ENABLE))
	{
		if (!udata)
			FD_SET(ident, &server_fds);
		else
			FD_SET(ident, &client_fds);
	}
	else
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
	buf[client.body_length] = '\0';
	client.request.ReqParsing(buf);
	// if (!client.request.getChunked())
		changeEvents(change_list, client.socket_fd, EVFILT_READ, EV_DISABLE | EV_DELETE, 0, 0, NULL);
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
		handleError(client, change_list, "500");
	client.written += write_size;
	if (client.written == static_cast<ssize_t>(send_buffer.size())) //다쓰면 연결해제
		client.status = DISCONNECT;

	std::cout << CYAN << "[response message]" << std::endl << &send_buffer[client.written - write_size] << RESET << std::endl;
}

void Event::readFile(Client& client, std::vector<struct kevent>& change_list)
{
	std::cout << "readFile()" << std::endl;

	char buf[1024];
	client.body_length = read(client.file_fd, buf, sizeof(buf));
	if (client.body_length <= 0)
	{
		changeEvents(change_list, client.file_fd, EVFILT_READ, EV_DISABLE | EV_DELETE, 0, 0, NULL);
		close(client.file_fd);
		handleError(client, change_list, "500");
		return ;
	}
	buf[client.body_length] = '\0';
	client.body = buf;
	client.response.getBody(buf, client.body_length);
	client.response.makeResponse();
	close(client.file_fd);
	changeEvents(change_list, client.file_fd, EVFILT_READ, EV_DISABLE | EV_DELETE, 0, 0, NULL);
	changeEvents(change_list, client.socket_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, &client);
	client.status = SEND_RESPONSE;
}

void Event::writeFile(Client& client, std::vector<struct kevent>& change_list)
{
	std::cout << "writeFile()" << std::endl;

	// 이게 맞나
	if (client.body_length == 0) // cgi를 거치지 않았을 때
	{
		// std::string body = client.request.getBody();
		// body = body.substr(3); // 개행 제거
		// client.body = body;
		// client.body_length = body.length();
	}
	write(client.file_fd, client.body.c_str(), client.body_length);
	char *tmp = new char [client.body_length];
	strcpy(tmp, client.body.c_str());
	
	client.response.getBody(tmp, client.body_length);
	client.response.makeResponse();

	close(client.file_fd);
	changeEvents(change_list, client.file_fd, EVFILT_WRITE, EV_DISABLE | EV_DELETE, 0, 0, NULL);
	changeEvents(change_list, client.socket_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, &client);
	client.status = SEND_RESPONSE;
}

void	Event::readPipe(Client& client, std::vector<struct kevent>& change_list)
{
	std::cout << "readPipe()" << std::endl;

	waitpid(client.pid, NULL, 0);
	char buf[1024];
	client.body_length = read(client.pipe_fd[0], buf, 1024);
	buf[client.body_length] = '\0';
	client.body = buf;

	close(client.pipe_fd[0]);
	changeEvents(change_list, client.pipe_fd[0], EVFILT_READ, EV_DISABLE | EV_DELETE, 0, 0, NULL);
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

	// std::string body = client.request.getBody();
	// body = body.substr(3); // 개행 제거
	// write(client.pipe_fd[1], body.c_str(), body.length());
	write(client.pipe_fd[1], &client.request.getBody(), client.request.getBody().size());
	close(client.pipe_fd[1]);
	changeEvents(change_list, client.pipe_fd[1], EVFILT_WRITE, EV_DISABLE | EV_DELETE, 0, 0, NULL);
	changeEvents(change_list, client.pipe_fd[0], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &client);
	client.status = READ_PIPE;
}
