#include "../includes/Event.hpp"
#include "../includes/Client.hpp"

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
	client.status = SEND_RESPONSE;
}

void	Event::handlePost(Client& client, std::vector<struct kevent>& change_list)
{
	std::cout << "handlePost()" << std::endl;
	
	time_t now = time(0);
	struct tm* tm = localtime(&now);
	char buf[80];
	strftime(buf, sizeof(buf), "%Y%m%d %H:%M:%S", tm);
	std::string filename = buf; // 현재날짜시간 + 실제 파일 이름 있으면 추가
	std::map<std::string, std::string> m_headers = client.request.getHeaders();
	std::string extension = m_mime_type[m_headers["Content-Type"]]; // m_headers 값 잘 들어있어야됨
	std::string path = client.server->findValue(client.m_location, "upload_pass")[0] + "/";
	if (path.empty())
		path = "resources/upload/"; // default
	client.file_fd = open((path + filename + extension).c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0777);

	// body가 file이면 업로드 구현 -> .data()
	// application/x-www-form-urlencoded ??

	changeEvents(change_list, client.file_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
	client.status = WRITE_FILE;
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
