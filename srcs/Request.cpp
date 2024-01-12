#include "../includes/Request.hpp"

Request::Request()
    :method("default"), uri("default"), version("default"), body("default"), status("200")
{}

const std::string &Request::getMethod() const { return this->method; }
const std::string &Request::getUri() const { return this->uri; }
const std::string &Request::getHost() const { return this->host; }
const std::string &Request::getStatus() const { return this->status; }

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
    std::cout << "body: " << this->body << std::endl;
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

std::string Request::removeSpace(std::string str)
{
    std::string tmp;

    tmp = str.substr(2, str.size());
    return (tmp);
}

//"0/n" chunked라면 content-length(16진법)가 지금 들어온 바디 size 체크 용으로만 사용하고 마지막 문자 나올 떄까지 받아

void Request::ReqParsing(std::string msg)
{
    this->req_msg = msg;
    std::size_t found = this->req_msg.find("\n");
    std::string first = this->req_msg.substr(0, found);
    std::vector<std::string> firstline;
    firstline = ReqSplit(first, ' ');
    this->method = firstline[0];
    if (this->method != "GET" && this->method != "POST" && this->method != "DELETE")
        this->status = "405";
    this->uri = firstline[1];
    this->version = firstline[2];
    // if (this->version != "1.1")
    //     this->status = "404";
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
        vline[1] = removeSpace(vline[1]);
        this->headers.insert(std::make_pair(vline[0], vline[1]));
        // std::cout << "헤더 종류(" << vline[0] << "), 헤더 내용(" << vline[1] << ")" << std::endl;
        if (vline[0] == "Host")
            this->host = vline[1];
        found = found_tmp;
    }
    if (found != this->req_msg.size() && found < this->req_msg.size())
        this->body = this->req_msg.substr(found, this->req_msg.size());
}

// int main() //파싱 테스트
// {
//     Request Req;

//     std::string msg = "POST HTTP 1.1\nHost: foo.com\nContent-Type: application/x-www-form-urlencoded\nhosthost: localhost:8080\nContent-Length: 13\nsay=Hi&to=Mom";
//     Req.ReqParsing(msg);
//     Req.PrintRequest();
//     return (0);
// }
