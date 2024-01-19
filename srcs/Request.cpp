#include "../includes/Request.hpp"

Request::Request()
    :method("default"), uri("default"), version("default"), status("200"), query_str(""), chunked(false)
{}

const std::string &Request::getMethod() const { return this->method; }
const std::string &Request::getUri() const { return this->uri; }
const std::string &Request::getHost() const { return this->host; }
const std::string &Request::getStatus() const { return this->status; }
const bool &Request::getChunked() const { return this->chunked; }
const std::vector<char> &Request::getBody() const { return this->body; }
const std::string &Request::getQueryStr() const { return this->query_str; }
const std::map<std::string, std::string> &Request::getHeaders() const { return this->headers; }

Request::~Request(){}

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

void Request::controlChunked(size_t found, int flag)
{
    std::map<std::string, std::string>::iterator it;
    ssize_t length;
    for (it = this->headers.begin(); it != this->headers.end(); it++) {
        if (it->first == "Content-Length")
            length = hexToDec(it->second);
        break;
    }
    if (found != this->req_msg.size() && found < this->req_msg.size()) {
            std::string buf = this->req_msg.substr(found, this->req_msg.size());
            for (size_t i = 0; i < buf.size() ; i++)
                this->buffer.push_back(buf[i]);
    }
    if (this->buffer.size() != static_cast<size_t>(length) || length == -1) {
        this->status = "404"; //에러 코드 맞는지 확인하기 -> 사이즈 확인
        return ;
    }
    if (flag == 0) //첫번째 chunked일 때
    {
        for (size_t i = 0; i < this->buffer.size() ; i++) {
                this->body.push_back(this->buffer[i]);
                this->body_size++;
        }
    }
    else // 첫번째 chunked가 아닐 때
    {

    }

    return ;
}

std::size_t Request::LineParsing(std::string msg)
{
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
    return (found);
}

//"0/n" chunked라면 content-length(16진법)가 지금 들어온 바디 size 체크 용으로만 사용하고 마지막 문자 나올 떄까지 받아

void Request::ReqParsing(std::string msg)
{
    std::size_t found;
    if (this->chunked) //chunked인데 첫번째 아닐 떄
    {
        found = LineParsing(msg);
        controlChunked(found, 1);
        return ;
    }
    found = LineParsing(msg);
    if (this->chunked)
        controlChunked(found, 0); // 첫번째 chunked일 때
    else {
        if (found != this->req_msg.size() && found < this->req_msg.size()) {
            std::string buf = this->req_msg.substr(found, this->req_msg.size());
            for (size_t i = 0; i < buf.size() ; i++)
                this->body.push_back(buf[i]);
        }
        this->body_size = this->body.size();
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
