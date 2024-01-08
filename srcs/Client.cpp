#include "../includes/Client.hpp"
#include "../includes/Server.hpp"
#include "../includes/Request.hpp"

Client::Client() {}

Client::Client(int client_socket)
	: socket_fd(client_socket), status(RECV_REQUEST)
{
	socket_fd = client_socket;
	file_fd = -1;
	status = RECV_REQUEST;
	written = 0;
}

Client::~Client() {}

int Client::getSocketFd() { return socket_fd; }

int Client::getStatus() { return status; }

void	Client::setStatus(Status status) { this->status = status; }

void	Client::setServer(Server* server) { this->server = server; }

void    Client::findLocation()
{
	m_location = server->getLocation()[request.getUri()];
	// location 유효성체크
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
	Request Req;
	char buf[1024];
	body_length = read(this->socket_fd, buf, 1024);
	Req.ReqParsing(buf);
	this->request = Req;
}

void Client::handleSocketWrite()
{
	// 	1. response 객체 사용, 응답 메세지 생성
	// 	2. socket_fd write()
	// 	3. setStatus(DISCONNECT)

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
}

void Client::handleFileRead()
{
	// 	1. file_fd read()
	// 	2. setStatus(SEND_RESPONSE)
}

void Client::handleGet()
{
	// index file open(), fd(리턴값)는 file_fd에 저장
	// body에 내용, body_length에 길이

	this->status = READ_FILE;
}

void    Client::handleDelete()
{
	
}
