#include "../includes/Response.hpp"
#include "../includes/Event.hpp"

Response::Response() : status("200") {}

Response &Response::operator=(const Response &obj) 
{
	send_buffer = obj.send_buffer;
	status = obj.status;
	content_type = obj.content_type;
	headers = obj.headers;
	body = obj.body;
	return *this;
}

const std::vector<char> &Response::getSendBuffer() const { return send_buffer; }

void Response::setBody(const std::vector<char> &obj) { body = obj; }
void Response::setStatus(const std::string &obj) { status = obj; }

void Response::setContentType(const std::string &resource) 
{
    size_t dotPosition = resource.find('.');
    if (dotPosition != std::string::npos)
	{
        std::string file_type = resource.substr(dotPosition);
        content_type = Event::getMimeType(file_type);
		if (content_type == "")
			content_type = "application/octet-stream";
    }
	else
        content_type = "application/octet-stream";
}

void Response::print() {
	std::map<std::string, std::string>::iterator header_it = headers.begin();
	std::vector<char> status_msg = getStatusMsg();
	std::cout << std::string(status_msg.begin(), status_msg.end());
	while (header_it != headers.end()) {
		std::cout << header_it->first << ": " << header_it->second << std::endl;
		header_it++;
	}
}

void Response::makeResponse() {
	// 시작 줄
	std::vector<char> status_msg = getStatusMsg();
	send_buffer.insert(send_buffer.end(), status_msg.begin(), status_msg.end());

	// 헤더
	std::string header;
	makeHeaderLine();
	std::map<std::string, std::string>::const_iterator it = this->headers.begin();
	for (; it != headers.end(); ++it)
		header += it->first + ": " + it->second + "\r\n";
	send_buffer.insert(send_buffer.end(), header.begin(), header.end());

	std::vector<char> rn;
	rn.push_back('\r');
	rn.push_back('\n');
	send_buffer.insert(send_buffer.end(), rn.begin(), rn.end());

	// 바디
	send_buffer.insert(send_buffer.end(), body.begin(), body.end());

	//print();
}

std::vector<char> Response::getStatusMsg() const 
{
	static std::map<std::string, std::string> m_status;
	if (m_status.empty()) {
		m_status["200"] = "OK";
		m_status["201"] = "Created";
		m_status["204"] = "No Content";
		m_status["400"] = "Bad Request";
		m_status["404"] = "Not Found";
		m_status["405"] = "Method Not Allowed";
		m_status["413"] = "Payload Too Large";
		m_status["500"] = "Internal Server Error";
	}

	std::string status_msg = "HTTP/1.1 " + status + " " + m_status[status] + "\r\n";
	return std::vector<char>(status_msg.begin(), status_msg.end());
}

void Response::makeHeaderLine() {
	headers["Date"] = getDate(NULL);
	if (body.size() != 0)
		headers["Content-Length"] = std::to_string(body.size());
	if (content_type != "")
		headers["Content-Type"] = content_type;
}

std::string Response::getDate(std::time_t *t) {
	char buffer[128];
	std::time_t current_time;

	if (t == NULL)
		current_time = std::time(NULL);
	else
		current_time = *t;
	std::tm *timeInfo = std::gmtime(&current_time);
	std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timeInfo);

	return buffer;
}

void Response::getBody(char *buffer, int read_size) {
	for (int i = 0; i < read_size; i++)
		body.push_back(buffer[i]);
}
