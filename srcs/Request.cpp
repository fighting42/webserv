#include "../includes/Request.hpp"

Request::Request()
    :method("default"), uri("default"), version("default"), status("200"), query_str(""), chunked(false), body_done(false), chunked_done(false)
{
    this->body.clear();
    this->buffer.clear();
}

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

void Request::controlChunked(size_t found)
{
    ssize_t size, chk_size;
    std::size_t  found_RN, found_C, i=0;
    std::string line_size, line_contents;
    std::vector<char> tmp;

    if (this->req_msg.find("0\r\n\r\n", found+1) != std::string::npos) //이후 확인 필요
        this->chunked_done = true;
    found_RN = this->req_msg.find("\r\n", found+1);
    line_size = this->req_msg.substr(found+1, found_RN-(found+1));
    size = hexToDec(line_size);
    while(!this->body_done) {
        chk_size = 0;
        tmp.clear();
        if (found >= this->req_msg.size())
            break;
        if (i > 0) {
            found_RN = this->req_msg.find("\r\n", found+2);
            line_size = this->req_msg.substr(found+2, found_RN-(found+2));
            size = hexToDec(line_size);
            if (this->req_msg.find("\r\n\r\n") == found_RN)
                this->body_done = true;
        }
        // std::cout << "chunked size : " << size << std::endl; //삭제
        while(chk_size < size) {
            found_C = this->req_msg.find("\r\n", found_RN + 2);
            line_contents = this->req_msg.substr(found_RN + 2, found_C-(found_RN+2));
            chk_size += (line_contents.size());
            // std::cout << "chk_size: " << chk_size << "line_size " << line_contents.size() << std::endl; //삭제
            if (chk_size + 2 < size) {
                for (size_t i = 0; i < line_contents.size() ; i++)
                    tmp.push_back(line_contents[i]);
                tmp.push_back('\r');
                tmp.push_back('\n');
                // for (std::vector<char>::iterator it = tmp.begin(); it != tmp.end() ; ++it)
                //     std::cout << *it;
                // std::cout << '\n';
                chk_size += 2;
            }
            else  if (chk_size == size) {
                // std::cout << "same" << std::endl;
                for (size_t i = 0; i < line_contents.size() ; i++)
                    tmp.push_back(line_contents[i]);
                // for (std::vector<char>::iterator it = tmp.begin(); it != tmp.end() ; ++it)
                //     std::cout << *it;
                // std::cout << '\n';
                break;
            }
            else 
                break;
            found_RN = found_C;
        }
        // std::cout << "tmp size " << tmp.size() << std::endl; //삭제
        if (tmp.size() != static_cast<size_t>(size) || size == -1) { //size맞지 않을 때
            this->buffer.clear();
            // std::cout << "error" <<std::endl; //삭제
            this->status = "400"; //에러 코드 맞는지 확인하기 -> chat gpt피셜 보통 400이나 500이 나온다고
            return ;
        }
        for (size_t i = 0; i < tmp.size() ; i++)
            this->buffer.push_back(tmp[i]);
        found = found_C;
        i++;
    }
    if (this->chunked_done)
    {
        this->chunked = false;
        for (size_t i = 0; i < this->buffer.size() ; i++)
                this->body.push_back(this->buffer[i]);
        this->body_size = this->body.size();
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

void Request::ReqParsing(std::string msg)
{
    std::size_t found;

    found = LineParsing(msg);
    if (this->chunked) {
        controlChunked(found);
        if (this->chunked == false)
            this->chunked_done = false;
    }
    else {
        if (found != this->req_msg.size() && found < this->req_msg.size()) {
            std::string buf = this->req_msg.substr(found, this->req_msg.size());
            for (size_t i = 0; i < buf.size() ; i++)
                this->body.push_back(buf[i]);
        }
        this->body_size = this->body.size();
    }
    if (this->method == "POST") {
        std::map<std::string, std::string>::iterator it;
        for (it = this->headers.begin(); it != this->headers.end(); it++) {
            if (it->first == "Content-Length")
                if(it->second == "0")
                    this->status = "411";
            break;
    }
        if (it == this->headers.end())
            this->status = "411";
    }
    return ;
}

// int main() // 파싱 테스트
// {
//     Request Req;

//     // std::string msg = "POST HTTP?name 1.1\nHost: foo.com\nContent-Type: application/x-www-form-urlencoded\nhost: localhost:8080\nTransfer-Encoding: chunked\n4\r\nWiki\r\n5\r\npedia\r\nE\r\nin\r\n\r\nchunks.\r\n0\r\n\r\n";
//     std::string msg = "POST HTTP?name 1.1\nHost: foo.com\nContent-Type: application/x-www-form-urlencoded\nhost: localhost:8080\nTransfer-Encoding: chunked\n4\r\nWiki\r\n5\r\npedia\r\n2\r\nin\r\n7\r\nchunks.\r\n0\r\n\r\n";
//     Req.ReqParsing(msg);
//     Req.PrintRequest();
//     return (0);
// }
