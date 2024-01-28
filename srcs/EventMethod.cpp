#include "../includes/Event.hpp"
#include "../includes/Client.hpp"

void    Event::checkMethod(Client& client, std::vector<struct kevent>& change_list)
{
	if (client.status != RECV_REQUEST || !client.request.getParsingStatus())
		return;

	findLocation(client);
	checkConfig(client, change_list);
	if (client.status != RECV_REQUEST)
		return;

	if (!client.server->findValue(client.m_location, "cgi_path").empty())
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

	std::string file = getRootpath(client);
	std::vector<std::string> v_autoindex = client.server->findValue(client.m_location, "autoindex");
    struct stat statbuf;
	if (!v_autoindex.empty() && v_autoindex[0] == "on")//오토인덱스 온이면
    {
	    if (stat(file.c_str(), &statbuf) != -1) //디렉토리면(파일이면) 오토인덱스하기
		{
			if (statbuf.st_mode & S_IFDIR )
				return handleAutoindex(client, change_list, file);
		}
	}

	// location에 index 설정이 되어있고, 요청 uri와 location uri가 같으면! index파일 리다이렉션
	std::vector<std::string> v_index = client.server->findValue(client.m_location, "index");
	if (v_index.size() != 0 && client.request.getUri() == client.location_uri)
		file += "/" + v_index[0];
	//index 파일 오픈
	if (access(file.c_str(), F_OK) == -1)
		return handleError(client, change_list, "404");
	client.file_fd = open(file.c_str(), O_RDONLY);
	if (client.file_fd == -1)
		return handleError(client, change_list, "500");
	changeEvents(change_list, client.file_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &client);
	//파일 내용 저장
	std::ifstream fout(file.c_str());
	if (!fout.is_open())
	{
		changeEvents(change_list, client.file_fd, EVFILT_READ, EV_DISABLE | EV_DELETE, 0, 0, &client);
		close(client.file_fd);
		return handleError(client, change_list, "500");
	}
	client.body = std::string((std::istreambuf_iterator<char>(fout)), std::istreambuf_iterator<char>());
	client.response.setContentType(file);
	client.status = READ_FILE;
}

void    Event::handleDelete(Client& client, std::vector<struct kevent>& change_list)
{
	std::cout << "handleDelete()" << std::endl;

	std::string file = getRootpath(client);
	if (access(file.c_str(), F_OK) == -1) //파일 유효성 검사
		return handleError(client, change_list, "404");
	if (std::remove(file.c_str())) //파일 삭제 실패
		return handleError(client, change_list, "500");

	char str[17] = "DELETE SUCCESS!\n";
	client.response.getBody(str, strlen(str));
	client.response.makeResponse();
	
	changeEvents(change_list, client.socket_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, &client);
	client.status = SEND_RESPONSE;
}

void	Event::handlePost(Client& client, std::vector<struct kevent>& change_list)
{
	std::cout << "handlePost()" << std::endl;
	
	time_t now = time(0);
	struct tm* tm = localtime(&now);
	char buf[80];
	strftime(buf, sizeof(buf), "%y%m%d-%H%M%S", tm);
	std::vector<std::string> v_value = client.server->findValue(client.m_location, "upload_path");
	std::map<std::string, std::string> m_headers = client.request.getHeaders();
	std::string path;
	if (v_value.size() == 0)
		path = "resources/upload/"; // default
	else
		path = v_value[0] + "/";
	if (access(path.c_str(), F_OK) == -1)
		return handleError(client, change_list, "404");
	std::string filename = buf; // 현재날짜시간 + 실제 파일 이름 있으면 추가
	std::string extension = m_mime_type[m_headers["Content-Type"]];

	client.file_fd = open((path + filename + extension).c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0777);
	if (client.file_fd == -1)
		return handleError(client, change_list, "500");
	
	if (m_headers["Content-Length"].empty() || client.request.getBody().size() == 0) // body size 0
	{
		char str[15] = "POST SUCCESS!\n";
		client.response.getBody(str, strlen(str));
		client.response.makeResponse();
		changeEvents(change_list, client.socket_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, &client);
		client.status = SEND_RESPONSE;
		return;
	}

	// body가 file이면 업로드 구현 -> .data()
	// application/x-www-form-urlencoded ??

	changeEvents(change_list, client.file_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, &client);
	client.status = WRITE_FILE;
}

void    Event::handleCgi(Client& client, std::vector<struct kevent>& change_list)
{
	std::cout << "handleCgi()" << std::endl;
	execCgi(client, change_list);
	fcntl(client.pipe_fd[0], F_SETFL, O_NONBLOCK);
	fcntl(client.pipe_fd[1], F_SETFL, O_NONBLOCK);
	changeEvents(change_list, client.pipe_fd[1], EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, &client);
	client.status = WRITE_PIPE;
}

void    Event::handleError(Client& client, std::vector<struct kevent>& change_list, const std::string &error_code)
{
	std::cout << "handleError()" << std::endl;
	client.response.setStatus(error_code);

	std::string error_page;
	if ((error_page = client.server->findErrorPage(error_code)).empty())
	{
		if ((error_page = client.server->findLocationErrorPage(client.location_uri, error_code)).empty())
			error_page = "resources/error.html";
		else
			error_page = getRootpath(client, error_page);
	}

	if (access(error_page.c_str(), F_OK) == -1)
		return handleError(client, change_list, "404");
	client.file_fd = open(error_page.c_str(), O_RDONLY);
	if (client.file_fd == -1) 
		return handleError(client, change_list, "500");

	client.response.setContentType(error_page);

	changeEvents(change_list, client.file_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &client);
	client.status = READ_FILE;
}
