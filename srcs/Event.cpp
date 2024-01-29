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

	char buf[BUFFER_SIZE];
	client.body_length = read(client.socket_fd, buf, BUFFER_SIZE);
	buf[client.body_length] = '\0';
	if (client.body_length <= 0)
	{
		changeEvents(change_list, client.socket_fd, EVFILT_READ, EV_DISABLE | EV_DELETE, 0, 0, &client);
		return handleError(client, change_list, "500");
	}
	client.request.ReqParsing(buf);

	if (!client.request.getParsingStatus())
		changeEvents(change_list, client.socket_fd, EVFILT_TIMER, EV_ADD | EV_ENABLE | EV_ONESHOT, NOTE_SECONDS, 60, &client);
	if (client.request.getParsingStatus() || client.request.getStatus() != "200")
		changeEvents(change_list, client.socket_fd, EVFILT_READ, EV_DISABLE | EV_DELETE, 0, 0, &client);
	if (client.request.getStatus() != "200")
		handleError(client, change_list, client.request.getStatus());

	// std::cout << "parsing status: "  << client.request.getParsingStatus() << std::endl;
	std::cout << BLUE << "[request message]" << std::endl << buf << RESET << std::endl;
}

void Event::writeSocket(Client& client, std::vector<struct kevent>& change_list)
{
	std::cout << "writeSocket()" << std::endl;

	const std::vector<char>& send_buffer = client.response.getSendBuffer();
	ssize_t write_size = send_buffer.size() - client.written > BUFFER_SIZE ? BUFFER_SIZE : send_buffer.size() - client.written;
	write_size = write(client.socket_fd, &send_buffer[client.written], write_size);
	if (write_size <= 0)
		return handleError(client, change_list, "500");
	client.written += write_size;

	if (client.written == static_cast<ssize_t>(send_buffer.size())) //다쓰면 연결해제
		client.status = DISCONNECT;
	
	std::cout << CYAN << "[response message]" << std::endl << &send_buffer[client.written - write_size] << RESET << std::endl;
	
	std::map<std::string, std::string> m_headers = client.request.getHeaders();
	if (m_headers["Connection"] != "close" && client.status == DISCONNECT)
	{
		changeEvents(change_list, client.socket_fd, EVFILT_WRITE, EV_DISABLE | EV_DELETE, 0, 0, &client);
		changeEvents(change_list, client.socket_fd, EVFILT_TIMER, EV_ADD | EV_ENABLE | EV_ONESHOT, NOTE_SECONDS, 75, &client);
		changeEvents(change_list, client.socket_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &client);
		client.init();
	}
}

void Event::readFile(Client& client, std::vector<struct kevent>& change_list)
{
    std::cout << "readFile()" << std::endl;
	
	// 파일 크기 확인
    off_t fileSize = lseek(client.file_fd, 0, SEEK_END);
    lseek(client.file_fd, 0, SEEK_SET);

    // 적절한 크기의 버퍼 할당
    std::vector<char> fileContent(fileSize);

    ssize_t bytesRead = read(client.file_fd, fileContent.data(), fileSize);
    if (bytesRead == fileSize)
    {
        close(client.file_fd);
        // 읽은 파일 데이터 처리
        client.body = std::string(fileContent.begin(), fileContent.end());
        client.response.getBody(fileContent.data(), fileContent.size());
        client.response.makeResponse();
        changeEvents(change_list, client.socket_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, &client);
        client.status = SEND_RESPONSE;
    }
    else if (bytesRead <= 0)
	{
        close(client.file_fd);
        return handleError(client, change_list, "500");
	}
}

void	Event::readPipe(Client& client, std::vector<struct kevent>& change_list)
{
	std::cout << "readPipe()" << std::endl;

	waitpid(client.pid, NULL, 0);

	char buf[BUFFER_SIZE];
	client.body_length = read(client.pipe_fd[0], buf, BUFFER_SIZE);
	buf[client.body_length] = '\0';
	close(client.pipe_fd[0]);
	changeEvents(change_list, client.pipe_fd[0], EVFILT_READ, EV_DISABLE | EV_DELETE, 0, 0, &client);
	if (client.body_length <= 0)
		return handleError(client, change_list, "500");
	client.body = buf;

	client.response.getBody(buf, client.body_length);
	client.response.makeResponse();
	changeEvents(change_list, client.socket_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, &client);
	client.status = SEND_RESPONSE;
}

void	Event::writePipe(Client& client, std::vector<struct kevent>& change_list)
{
	std::cout << "writePipe()" << std::endl;

	if (client.request.getMethod() == "GET")
	{
		changeEvents(change_list, client.pipe_fd[1], EVFILT_WRITE, EV_DISABLE | EV_DELETE, 0, 0, &client);
		close(client.pipe_fd[1]);
	}
	else if (client.request.getMethod() == "POST")
	{
		std::vector<char> v_body = client.request.getBody();
		ssize_t write_size = write(client.pipe_fd[1], v_body.data(), v_body.size());
		changeEvents(change_list, client.pipe_fd[1], EVFILT_WRITE, EV_DISABLE | EV_DELETE, 0, 0, &client);
		close(client.pipe_fd[1]);
		if (write_size <= 0)
			return handleError(client, change_list, "500");
	}

	changeEvents(change_list, client.pipe_fd[0], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &client);
	client.status = READ_PIPE;
}
