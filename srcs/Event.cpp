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
	if (client.status != READ_SOCKET)
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
	client.status = WRITE_SOCKET;
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
	client.status = WRITE_SOCKET;
}

void	Event::writePipe(Client& client, std::vector<struct kevent>& change_list)
{
	std::cout << "writePipe()" << std::endl;

	// std::string body = client.request.getBody();
	// body = body.substr(3); // 개행 제거
	// write(client.pipe_fd[1], body.c_str(), body.length());
	close(client.pipe_fd[1]);
	changeEvents(change_list, client.pipe_fd[1], EVFILT_WRITE, EV_DISABLE, 0, 0, NULL);
	changeEvents(change_list, client.pipe_fd[0], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	client.status = READ_PIPE;
}

void Event::handleGet(Client& client, std::vector<struct kevent>& change_list) //디폴트 파일 말고 경로 들어왔을 때 열리도록 추가
{
	std::cout << "handleGet()" << std::endl;

	std::vector<std::string> v_root = client.server->findValue(client.m_location, "root");
	std::string root = v_root.back();
	std::string rsrcs = root + client.request.getUri();
	std::string file = rsrcs;
	if (rsrcs[rsrcs.find("resources") + 10] == '\0')
	{
		std::vector<std::string> v_idx = client.server->findValue(client.m_location, "index");
		std::string idx = rsrcs + v_idx.back();
		file = idx;
	}
	//index 파일 오픈
	if (access(file.c_str(), F_OK) == -1)
	{
		changeEvents(change_list, client.file_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
		close(client.file_fd);
		handleError(client, change_list, "404");
		return;
	}
	client.file_fd = open(file.c_str(), O_RDONLY);
	if (client.file_fd == -1)
	{
		changeEvents(change_list, client.file_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
		close(client.file_fd);
		handleError(client, change_list, "500");
		return;
	}
	//파일 내용 저장
	std::ifstream fout(file.c_str());
	if (!fout.is_open())
	{
		changeEvents(change_list, client.file_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
		close(client.file_fd);
		handleError(client, change_list, "500");
		return;
	}
	client.body = std::string((std::istreambuf_iterator<char>(fout)), std::istreambuf_iterator<char>());
	//response.setContentType_지우지말아주십셩,,희희,,
	changeEvents(change_list, client.file_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	client.status = READ_FILE;
}

void    Event::handleDelete(Client& client, std::vector<struct kevent>& change_list)
{
	std::cout << "handleDelete()" << std::endl;

	std::vector<std::string> v_root = client.server->findValue(client.m_location, "root");
	std::string root = v_root.back();
	std::string rsrcs = root + client.request.getUri();
	if (access(rsrcs.c_str(), F_OK) == -1) //파일 유효성 검사
	{
		handleError(client, change_list, "404");
		return;
	}
	if (std::remove(rsrcs.c_str())) //파일 삭제 실패
	{
		handleError(client, change_list, "500");
		return;
	}
	client.status = WRITE_SOCKET;
}

void	Event::handlePost(Client& client, std::vector<struct kevent>& change_list)
{
	std::cout << "handlePost()" << std::endl;
	(void)client; (void)change_list;
	// 날짜시간으로 파일 생성 (post)
	// 파일에 body 내용 쓰기 (write file)
	// 응답보내기 (write socket)
}

void    Event::handleCgi(Client& client, std::vector<struct kevent>& change_list)
{
	std::cout << "handleCgi()" << std::endl;
	execCgi(client);
	fcntl(client.pipe_fd[0], F_SETFL, O_NONBLOCK);
	fcntl(client.pipe_fd[1], F_SETFL, O_NONBLOCK);
	changeEvents(change_list, client.pipe_fd[1], EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
	client.status = WRITE_PIPE;
}

void    Event::handleError(Client& client, std::vector<struct kevent>& change_list, const std::string &error_code)
{
	std::cout << "handleError()" << std::endl;
	client.response.setStatus(error_code);

	std::string error_page;
	if ((error_page = client.server->findErrorPage(error_code)) == "")
	{
		if ((error_page = client.server->findLocationErrorPage(client.request.getUri(), error_code)) == "")
			error_page = "resources/error.html";
		else
		{
            std::vector<std::string> v_root = client.server->findValue(client.m_location, "root");
            std::string root = v_root.back();
            std::string rsrcs = root + error_page;
            std::string file = rsrcs;
            error_page = file;
		}
	}
	// access 등 예외처리 추가!
	client.file_fd = open(error_page.c_str(), O_RDONLY);
	if (client.file_fd == -1) {
        client.status = DISCONNECT;
        return;
    }

	client.response.setContentType(error_page);
	changeEvents(change_list, client.file_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	client.status = READ_FILE;
}
