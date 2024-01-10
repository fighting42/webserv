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
	if (m_location.size() == 0)
		handleError("404"); // 수정
}

void    Client::checkMethod()
{
	if (status != RECV_REQUEST)
		return;

	findLocation();
	// handleCgi();
	if (request.getMethod() == "GET")
		handleGet();
	else if (request.getMethod() == "DELETE")
		handleDelete();
	else if (request.getMethod() == "POST")
		handlePost();
}

void Client::handleSocketRead()
{
	std::cout << "handleSocketRead()" << std::endl;

	char buf[1024];
	body_length = read(this->socket_fd, buf, 1024);
	buf[body_length] = '\0';
	request.ReqParsing(buf);
	if (request.getStatus() != "200") // 다 404 리턴중임당ㅎㅎ..ㅎ..ㅎ.ㅎㅎ
		handleError(request.getStatus()); // 예진 수정
	
	std::cout << BLUE << "[request message]" << std::endl << buf << RESET << std::endl;
}

void Client::handleSocketWrite()
{
	std::cout << "handleSocketWrite()" << std::endl;

	const std::vector<char>& send_buffer = response.getSendBuffer();
	ssize_t write_size = send_buffer.size() - written > 1024 ? 1024 : send_buffer.size() - written;
	write_size = write(socket_fd, &send_buffer[written], write_size);
	if (write_size <= 0)
		handleError("500"); // 예진 수정
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
	if (body_length <= 0)
		handleError("500"); // 예진 수정
	buf[body_length] = '\0';
	body = buf;
	response.getBody(buf, body_length);
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
	if (access(idx.c_str(), F_OK) == -1)
		handleError("404"); // 예진 수정
	this->file_fd = open(idx.c_str(), O_RDONLY);
	if (this->file_fd == -1)
		handleError("500"); // 예진 수정
	//파일 내용 저장
	std::ifstream fout(idx.c_str());
	if (!fout.is_open())
		return ;
	this->body = std::string((std::istreambuf_iterator<char>(fout)), std::istreambuf_iterator<char>());
	//response.setContentType_지우지말아주십셩,,희희,,
	this->status = READ_FILE;
}

void    Client::handleDelete()
{
	std::cout << "handleDelete()" << std::endl;

	std::vector<std::string> v_root = server->findValue(this->m_location, "root");
	std::string root = v_root.back();
	std::string rsrcs = root + this->request.getUri();
	if (access(rsrcs.c_str(), F_OK) == -1) //파일 유효성 검사
		handleError("404"); // 예진 수정
	if (std::remove(rsrcs.c_str())) //파일 삭제 실패
		handleError("500"); // 예진 수정
	this->status = SEND_RESPONSE;
}

void	Client::handlePost()
{
	std::cout << "handlePost()" << std::endl;

	// 날짜시간으로 파일 생성 (post)
	// 파일에 body 내용 쓰기 (filewrite?)
	// 응답보내기 (socketwrite)
}

void    Client::handleCgi()
{
	std::cout << "handleCgi()" << std::endl;
}

void    Client::handleError(const std::string &error_code)
{
	std::cout << "handleError()" << std::endl;
	(void)error_code;
	// handleGet()이랑 비슷. server, config에서 error_page 찾아서 open()
	// server block의 error_page 우선 적용, 없으면 location, 그것도 없으면 default

	// std::cout << server->findValue(m_location, "error_page")[0] << std::endl;
	std::string error_page = "resources/error.html";
	file_fd = open(error_page.c_str(), O_RDONLY);

	//response.setContentType_지우지말아주십셩,,희희,,
	this->status = READ_FILE;
}
