#include "../includes/Request.hpp"

void  Request::init()
{
    this->body.clear();
    this->buffer.clear();
    this->req_msg.clear();
    this->method = "default";
    this->uri = "default";
    this->version = "default";
    this->host = "default";
    this->status = "200";
    this->headers.clear();
    this->buffer = "";
    this->query_str = "";
    this->pstatus = FIRST;
    this->chunked = false;
}

void Request::PrintRequest()
{
    std::cout << "\033[1;94m" << std::endl; // blue
    std::cout << "method: " << this->method << std::endl;
    std::cout << "uri: " << this->uri << std::endl;
    std::cout << "version: " << this->version << std::endl;
    std::cout << "host: " << this->host << std::endl;
    std::map<std::string, std::string>::iterator it;
    for (it = this->headers.begin(); it != this->headers.end(); it++) {
        if (it->first == "Content-Length")
            std::cout << "Content-Length: " << it->second << std::endl;
        break;
    }
    std::cout << "body: ";
    for (std::vector<char>::iterator it = this->body.begin(); it != this->body.end(); ++it) {
        std::cout << *it;
    }
    std::cout << "\033[0m" << std::endl; // reset
}

void Request::setUri(std::string newUri)
{
    this->uri = newUri;
    return ;
}

std::string Request::removeWhiteSpace(std::string str, int flag)
{
    std::string tmp;
    size_t idx;

    tmp = str.substr(2, str.size());
    if (flag == 1) {
        std::string dump;
        dump = tmp.substr(0, 20);
        // std::cout << "dump : " << dump << std::endl;
        // std::cout << "yejin의 부탁 : " << tmp << std::endl;
        if (dump == "multipart/form-data;") {
            return (tmp);
        }
    }
    for (idx = 0; idx < tmp.size(); idx++)
    {
        if (tmp[idx] == '\t' || tmp[idx] == '\n' || tmp[idx] == '\r' || \
            tmp[idx] == '\v' || tmp[idx] == '\f' || tmp[idx] == ' ')
            break ;
    }
    tmp = tmp.substr(0, idx);
    return (tmp);
}

ssize_t Request::hexToDec(const std::string& hex) 
{
	ssize_t ret = 0;
    size_t i;

	for (i = 0; i < hex.size(); i++) {
		ret *= 16;
		if (hex[i] >= '0' && hex[i] <= '9')
			ret += hex[i] - '0'; // '0' ~ '9'까지의 경우
        else if (hex[i] >= 'a' && hex[i] <= 'f')
			ret += hex[i] - 'a' + 10; // 'a' ~ 'f'까지의 경우
        else if (hex[i] >= 'A' && hex[i] <= 'F')
			ret += hex[i] - 'A' + 10; // 'A' ~ 'F'까지의 경우
        else
			return (-1);
	}
	return (ret);
}

std::string Request::checkQuery(std::string uri)
{
    size_t idx;
    for (idx = 0; idx < uri.size() ; idx++) {
        if (uri[idx] == '?')
            break ;
    }
    if (idx == uri.size())
        return (uri);
    else {
        std::string q_str;
        std::string u_str;
        u_str = uri.substr(0, idx);
        q_str = uri.substr(idx+1, uri.size()-idx-1);
        this->query_str = q_str;
        return (u_str);
    }
}
