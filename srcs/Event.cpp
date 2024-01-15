#include "../includes/Event.hpp"
#include "../includes/Client.hpp"

void Event::changeEvents(std::vector<struct kevent>& change_list, uintptr_t ident, \
	int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata)
{
	struct kevent temp_event;

	EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
	change_list.push_back(temp_event);
}

void    Event::checkMethod(Client& client, std::vector<struct kevent>& change_list)
{
	if (client.status != RECV_REQUEST)
		return;

	client.m_location = client.server->getLocation()[client.request.getUri()];
	if (client.m_location.size() == 0)
		client.m_location = client.server->getLocation()["/"];
	// allow_method(405), client_max_body_size(413) 확인하기

	if (client.server->findValue(client.m_location, "cgi_pass").size() > 0)
		handleCgi(client, change_list);
	else if (client.request.getMethod() == "GET")
		handleGet(client, change_list);
	else if (client.request.getMethod() == "DELETE")
		handleDelete(client, change_list);
	else if (client.request.getMethod() == "POST")
		handlePost(client, change_list);
}

void Event::readSocket(Client& client, std::vector<struct kevent>& change_list)
{
	std::cout << "readSocket()" << std::endl;

	char buf[1024];
	client.body_length = read(client.socket_fd, buf, 1024);
	buf[client.body_length] = '\0';
	client.request.ReqParsing(buf);
	changeEvents(change_list, client.socket_fd, EVFILT_READ, EV_DISABLE, 0, 0, NULL);
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
		changeEvents(change_list, client.file_fd, EVFILT_READ, EV_DISABLE, 0, 0, NULL);
		close(client.file_fd);
		handleError(client, change_list, "500");
		return ;
	}
	buf[client.body_length] = '\0';
	client.body = buf;
	client.response.getBody(buf, client.body_length);
	client.response.makeResponse();
	close(client.file_fd);
	changeEvents(change_list, client.file_fd, EVFILT_READ, EV_DISABLE, 0, 0, NULL);
	changeEvents(change_list, client.socket_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
	client.status = SEND_RESPONSE;
}

void Event::writeFile(Client& client, std::vector<struct kevent>& change_list)
{
	std::cout << "writeFile()" << std::endl;
	(void)client; (void)change_list;

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
	client.response.getBody(buf, client.body_length);
	client.response.makeResponse();
	
	close(client.pipe_fd[0]);
	changeEvents(change_list, client.pipe_fd[0], EVFILT_READ, EV_DISABLE, 0, 0, NULL);
	changeEvents(change_list, client.socket_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
	client.status = SEND_RESPONSE;
}

void	Event::writePipe(Client& client, std::vector<struct kevent>& change_list)
{
	std::cout << "writePipe()" << std::endl;

	std::string body = client.request.getBody();
	body = body.substr(3); // 개행 제거
	write(client.pipe_fd[1], body.c_str(), body.length());
	close(client.pipe_fd[1]);
	changeEvents(change_list, client.pipe_fd[1], EVFILT_WRITE, EV_DISABLE, 0, 0, NULL);
	changeEvents(change_list, client.pipe_fd[0], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	client.status = READ_PIPE;
}
