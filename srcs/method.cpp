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

	std::vector<std::string> v_autoindex = client.server->findValue(client.m_location, "autoindex");
	if (!v_autoindex.empty() && v_autoindex[0] == "on")
		return handleAutoindex(client, change_list, rsrcs);

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
	//response.setContentType_지우지말아주십셩,,희희,,

	client.status = READ_FILE;
}

void    Event::handleDelete(Client& client, std::vector<struct kevent>& change_list)
{
	std::cout << "handleDelete()" << std::endl;

	std::vector<std::string> v_root = client.server->findValue(client.m_location, "root");
	std::string root = v_root.back();
	std::string rsrcs = root + client.request.getUri();
	if (access(rsrcs.c_str(), F_OK) == -1) //파일 유효성 검사
		return handleError(client, change_list, "404");
	if (std::remove(rsrcs.c_str())) //파일 삭제 실패
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
	std::vector<std::string> v_value = client.server->findValue(client.m_location, "upload_pass");
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
	if (access(error_page.c_str(), F_OK) == -1)
		return handleError(client, change_list, "404");
	client.file_fd = open(error_page.c_str(), O_RDONLY);
	if (client.file_fd == -1) 
		return handleError(client, change_list, "500");

	client.response.setContentType(error_page);

	changeEvents(change_list, client.file_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &client);
	client.status = READ_FILE;
}

void Event::handleAutoindex(Client& client, std::vector<struct kevent>& change_list, std::string uri) 
{
	// html 코드 생성
	std::string autoindex_html = "<html><head><meta charset=\"UTF-8\"></head>" \
    "<style>body { background-color: white; font-family: Trebuchet MS;}</style>" \
	"<body><h1>" + uri + " List</h1><ul>";
	autoindex_html += "<table>";
	// 목록 채우기
	struct dirent* entry;
	DIR* dp = opendir(uri.c_str());
	if (dp != NULL) 
	{
		size_t resources_pos = uri.find("resources/");
		if (resources_pos != std::string::npos) 
		uri = uri.substr(resources_pos + std::string("resources/").length());
		while ((entry = readdir(dp))) 
		{
			std::string entry_name = entry->d_name;
            std::string entry_path = entry_name;
            std::string entry_link = "<a href='" + entry_path + "'>" + entry_name + "</a>";
            // 만약 폴더라면 하이퍼링크 걸어주기
            if (entry->d_type == DT_DIR)
			{
				entry_path = uri + "/" + entry_name;
				autoindex_html += "<tr><td><a href='" + entry_path + "'>" + entry_name + "</a>" + "</td></tr>";
            }
			else
				autoindex_html += "<tr><td>" + entry_name + "</td></tr>";
		}
		closedir(dp);
	}
	autoindex_html += "</table></ul></body></html>";
	// 응답 body에 써주기
	client.response.setBody(std::vector<char>(autoindex_html.begin(), autoindex_html.end()));
	client.response.makeResponse();
	changeEvents(change_list, client.socket_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, &client);
	client.status = SEND_RESPONSE;
}
