#include "../includes/Event.hpp"
#include "../includes/Client.hpp"

void Event::changeEvents(std::vector<struct kevent>& change_list, uintptr_t ident, \
	int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata)
{
	struct kevent temp_event;

	EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
	change_list.push_back(temp_event);
}

void    Event::findLocation(Client& client)
{
	client.m_location = client.server->getLocation()[client.request.getUri()];
	if (client.m_location.size() == 0)
		client.m_location = client.server->getLocation()["/"];
}

void    Event::checkMethod(Client& client, std::vector<struct kevent>& change_list)
{
	if (client.status != READ_SOCKET)
		return;

	findLocation(client);
	// handleCgi(client, change_list);
	if (client.request.getMethod() == "GET")
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
	if (client.request.getStatus() != "200") // 다 404 리턴중임당ㅎㅎ..ㅎ..ㅎ.ㅎㅎ
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
	(void)client; (void)change_list;
}

void    Event::handleError(Client& client, std::vector<struct kevent>& change_list, const std::string &error_code)
{
	std::cout << "handleError()" << std::endl;
	client.response.setStatus(error_code);

	// findErrorPage(), findLocationErrorPage() 인자로 에러 코드 넣어주면 해당 에러 페이지 값 리턴!
	// findErrorPage()는 server 블록에서 찾는거, findLocationErrorPage()는 location 블록에서 찾는거!
	// 두 함수 모두 해당하는 에러페이지 없으면 default 에러페이지 저장!
	std::string error_page;
	if ((error_page = client.server->findErrorPage(error_code)) == "")
	{
		if ((error_page = client.server->findLocationErrorPage(client.request.getUri(), error_code)) == "")
			error_page = "resources/error.html"; // default error page
		else
		{
			// handleGet()처럼 location root랑 error_page 합쳐서 경로 만들기!
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
