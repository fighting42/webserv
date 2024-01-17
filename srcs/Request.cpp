#include "../includes/Request.hpp"

Request::Request()
    :method("default"), uri("default"), version("default"), status("200"), query_str(""), chunked(false)
{}

const std::string &Request::getMethod() const { return this->method; }
const std::string &Request::getUri() const { return this->uri; }
const std::string &Request::getHost() const { return this->host; }
const std::string &Request::getStatus() const { return this->status; }
const bool &Request::getChunked() const { return this->chunked; }
const std::string &Request::getQueryStr() const { return this->query_str; }
const std::map<std::string, std::string> &Request::getHeaders() const { return this->headers; }

Request::~Request(){}

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
    std::cout << "body: " << std::endl;
    for (std::vector<char>::iterator it = this->body.begin(); it != this->body.end(); ++it) {
        std::cout << *it;
    }
    std::cout << "\033[0m" << std::endl; // reset
}

std::vector<std::string> Request::ReqSplit(std::string input, char delimiter) 
{
    std::vector<std::string> ret;
    std::stringstream ss(input);
    std::string temp;
 
    if (delimiter == ':') {
        std::size_t idx = input.find(':');
        if (idx == std::string::npos)
            return (static_cast< std::vector<std::string> > (0));
        temp = input.substr(0, idx);
        ret.push_back(temp);
        std::string temp2 = input.substr(idx, input.size());
        ret.push_back(temp2);
        return ret;
    }
    else {
        while (getline(ss, temp, delimiter)) {
            ret.push_back(temp);
        }
    }
    return ret;
}

std::string Request::removeWhiteSpace(std::string str)
{
    std::string tmp;
    size_t idx;

    tmp = str.substr(2, str.size());
    for (idx = 0; idx < tmp.size(); idx++)
    {
        if (tmp[idx] == '\t' || tmp[idx] == '\n' || tmp[idx] == '\r' || \
            tmp[idx] == '\v' || tmp[idx] == '\f' || tmp[idx] == ' ')
            break ;
    }
    tmp = tmp.substr(0, idx);
    return (tmp);
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

void Request::controlChunked(std::string msg, int flag)
{
    (void) msg;

    if (flag == 0)
    {

    }
    else
    {

    }

    return ;
}

//"0/n" chunked라면 content-length(16진법)가 지금 들어온 바디 size 체크 용으로만 사용하고 마지막 문자 나올 떄까지 받아

void Request::ReqParsing(std::string msg)
{
    if (this->chunked)
    {
        controlChunked(msg, 1);
        return ;
    }
    this->req_msg = msg;
    std::size_t found = this->req_msg.find("\n");
    std::string first = this->req_msg.substr(0, found);
    std::vector<std::string> firstline;
    firstline = ReqSplit(first, ' ');
    this->method = firstline[0];
    if (this->method != "GET" && this->method != "POST" && this->method != "DELETE")
        this->status = "405";
    this->uri = checkQuery(firstline[1]);
    // if (this->uri != "HTTP/1.1")
    //     this->status = "404";
    this->version = firstline[2];
    std::size_t found_tmp;
    std::string line;
    std::vector<std::string> vline;
    while(1)
    {
        if (found >= this->req_msg.size())
            break;
        found_tmp = this->req_msg.find("\n", found+1);
        line = this->req_msg.substr(found+1, found_tmp - (found + 1));
        vline = ReqSplit(line, ':'); 
        if (vline == static_cast< std::vector<std::string> > (0))
            break ;
        vline[1] = removeWhiteSpace(vline[1]);
        this->headers.insert(std::make_pair(vline[0], vline[1]));
        // std::cout << "헤더 종류(" << vline[0] << "), 헤더 내용(" << vline[1] << ")" << std::endl;
        if (vline[0] == "Host")
            this->host = vline[1];
        if (vline[0] == "Content-Length")
            this->content_length = static_cast<size_t>(atoi(vline[1].c_str()));
        if (vline[0] == "Transfer-Encoding" && vline[1] == "chunked")
            this->chunked = true;
        found = found_tmp;
    }
    if (this->chunked)
        controlChunked(msg, 0);
    else {
        if (found != this->req_msg.size() && found < this->req_msg.size()) {
            std::string buf = this->req_msg.substr(found, this->req_msg.size());
            for (size_t i = 0; i < buf.size() ; i++)
                this->body.push_back(buf[i]);
        }
    }
}

// int main() // 파싱 테스트
// {
//     Request Req;

//     std::string msg = "POST HTTP?name 1.1\nHost: foo.com\nContent-Type: application/x-www-form-urlencoded\nhost: localhost:8080\nContent-Length: 13\nsay=Hi&to=Mom";
//     Req.ReqParsing(msg);
//     Req.PrintRequest();
//     return (0);
// }
