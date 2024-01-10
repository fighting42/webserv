#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include "../includes/Request.hpp"

Client::Client() {}

Client::Client(int client_socket)
{
	socket_fd = client_socket;
	file_fd = -1;
	status = RECV_REQUEST;
	written = 0;
	response = Response();
}

Client::~Client() {}

int Client::getSocketFd() { return socket_fd; }

int Client::getFileFd() { return file_fd; }

int Client::getStatus() { return status; }

void	Client::setServer(Server* server) { this->server = server; }

void    Client::findLocation()
{
	m_location = server->getLocation()[request.getUri()];
	if (m_location.size() == 0) // 404
		handleError("404");
	// 예외처리~~
}

void    Client::checkMethod()
{
	findLocation();
	// handleCgi();
	if (request.getMethod() == "GET")
		handleGet();
	else if (request.getMethod() == "DELETE")
		handleDelete();
}

void Client::handleSocketRead()
{
	std::cout << "handleSocketRead()" << std::endl;

	char buf[1024];
	body_length = read(this->socket_fd, buf, 1024);
	request.ReqParsing(buf);
	if (request.getStatus() != "200")
		handleError(request.getStatus()); // 함수 끝내야할지 수정해야 합니다!
	std::cout << BLUE << "[request message]" << std::endl << buf << RESET << std::endl;
}

void Client::handleSocketWrite()
{
	std::cout << "handleSocketWrite()" << std::endl;

	const std::vector<char>& send_buffer = response.getSendBuffer();
	ssize_t write_size = send_buffer.size() - written > 1024 ? 1024 : send_buffer.size() - written;
	write_size = write(socket_fd, &send_buffer[written], write_size);
	if (write_size == -1) { //write 오류
		status = DISCONNECT;
		return;
	}
	written += write_size;
	if (written == static_cast<ssize_t>(send_buffer.size())) //다쓰면 연결해제
		status = DISCONNECT;
	
	std::cout << CYAN << "[response message]" << std::endl << &send_buffer[written - write_size] << RESET << std::endl;
}

void Client::handleFileRead()
{
	std::cout << "handleFileRead()" << std::endl;

	char buf[1024];
	body_length = read(file_fd, buf, sizeof(buf));
	response.getBody(buf, body_length);
	buf[body_length] = '\0';
	body = buf;
	if (body_length <= 0)
		status = DISCONNECT;
	response.makeResponse();
	status = SEND_RESPONSE;
}

void Client::handleGet() //디폴트 파일 말고 경로 들어왔을 때 열리도록 추가
{
	std::cout << "handleGet()" << std::endl;
	std::vector<std::string> v_root = server->findValue(this->m_location, "root");
	std::string root = v_root.back();
	std::string rsrcs = root + this->request.getUri();
	//index 파일 오픈
	std::vector<std::string> v_idx = server->findValue(this->m_location, "index");
	std::string idx = rsrcs + v_idx.back();
	if (access(idx.c_str(), F_OK) == -1) {
		handleError("404");
		return ;
	}
	this->file_fd = open(idx.c_str(), O_RDONLY);
	if (this->file_fd == -1) {
		handleError("500"); // 내부적으로 처리되어 기본적인 http상태코드 (internal server err)
		return ;
	}
	//파일 내용 저장
	std::ifstream fout(idx.c_str());
  	if (!fout.is_open())
		return ;
	this->body = std::string((std::istreambuf_iterator<char>(fout)), std::istreambuf_iterator<char>());
	// index file open(), fd(리턴값)는 file_fd에 저장
	//response.setContentType_지우지말아주십셩,,희희,,
	this->status = READ_FILE;
}

void    Client::handleDelete()
{
	std::cout << "handleDelete()" << std::endl;
	std::vector<std::string> v_root = server->findValue(this->m_location, "root");
	std::string root = v_root.back();
	std::string rsrcs = root + this->request.getUri();
	if (access(rsrcs.c_str(), F_OK) == -1) { //파일 유효성 검사
		handleError("404"); // 함수 끝나야 하는지 수정해야됨!
	}
	if (std::remove(rsrcs.c_str())) { //파일 삭제 실패
		handleError("500"); // 함수 끝나야 하는지 수정해야됨!
	}
	this->status = SEND_RESPONSE;
}

void    Client::handleCgi()
{
	std::cout << "handleCgi()" << std::endl;

	// int fds[2];
	// pid_t pid = fork();

	// char *args[3];
	// char **envp;

	// if (pid < 0)
	// 	throw "fork error"; // 500
	// else if (pid == 0) // 자식
	// {
	// 	args[0] = "cgi-bin/cgi_tester";
	// 	args[1] = NULL;
	// 	args[2] = NULL;

	// 	execve(args[0], args, envp);
	// 	throw "cgi execve error";
	// }
	// else // 부모
	// {

	// }
}

void    Client::handleError(const std::string &error_code)
{
	std::cout << "handleError()" << std::endl;

	// handleGet()이랑 비슷. server, config에서 error_page 찾아서 open()
	// server block의 error_page 우선 적용, 없으면 location, 그것도 없으면 default

	// std::cout << server->findValue(m_location, "error_page")[0] << std::endl;
	std::string error_page = "resources/error.html";
	file_fd = open(error_page.c_str(), O_RDONLY);

	//response.setContentType_지우지말아주십셩,,희희,,
	this->status = READ_FILE;
}
