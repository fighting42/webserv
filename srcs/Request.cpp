#include "../includes/Request.hpp"

Request::Request()
    :method("default"), uri("default"), version("default"), host("default"), status("200"), buffer(""), chunkedLineSize(-1), chunkedSize_chk(-1), query_str(""), pstatus(FIRST), chunked(false)
{
    this->body.clear();
    this->buffer.clear();
}

const std::string &Request::getMethod() const { return this->method; }
const std::string &Request::getUri() const { return this->uri; }
const std::string &Request::getHost() const { return this->host; }
const std::string &Request::getStatus() const { return this->status; }
const pStatus &Request::getParsingStatus() const { return this->pstatus; }
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
    ssize_t size, chk_size=0;
    std::size_t  found_RN=0, found_C=0;
    std::string line_size, line_contents;
    
    while(this->pstatus == BODY) {
        if (this->chunkedSize_chk == -1) {
            if (found == found_C && found_C != 0)
                found += 2;
            found_RN = this->buffer.find("\r\n", found);
            line_size = this->buffer.substr(found, found_RN-found);
            size = hexToDec(line_size);
            if (this->chunkedLineSize == 0 && size == 0) { //요청 완료 조건
                this->pstatus = BODY_DONE;
            }
            if (size == -1) {
                this->status = "400";
                return ;
            }
            if (this->pstatus != BODY_DONE) {
                this->chunkedLineSize = size;
                this->chunkedSize_chk = 0;
            }
        }
        if (this->chunkedSize_chk != -1) {
            while (1) {
                chk_size = 0;
                if (this->buffer.find("\r\n", found_RN+2) == std::string::npos)
                    return ;
                if (found_RN != 0)
                    found_RN += 2;
                found_C = this->buffer.find("\r\n", found_RN);
                line_contents = this->buffer.substr(found_RN, found_C-(found_RN));
                //contents_length가 없을 경우도 있음, 411 코드
                chk_size += line_contents.size();
                if (chk_size + 2 < this->chunkedLineSize) {
                    for (size_t idx = 0; idx < line_contents.size() ; idx++)
                        this->body.push_back(line_contents[idx]);
                    this->body.push_back('\r');
                    this->body.push_back('\n');
                    chk_size += 2;
                    this->chunkedLineSize -= chk_size;
                }
                else if (chk_size == this->chunkedLineSize) { // 한 줄에 크기에 맞게 잘 들어왔을 때
                    for (size_t i = 0; i < line_contents.size() ; i++)
                        this->body.push_back(line_contents[i]);
                    this->chunkedLineSize -= chk_size;
                }
                if (this->chunkedLineSize < 0) {
                    this->status = "400";
                    return ;
                }
                else if (this->chunkedLineSize == 0) {
                    this->chunkedSize_chk = -1;
                    break;
                }
                std::vector<char>::iterator it;
                for (it = this->body.begin(); it != this->body.end(); it++)
                    std::cout << *it;    
                found_RN = found_C;
            }
            found = found_C;
        }
    }
    this->chunked = false;
    this->body_size = this->body.size();
    this->found = found;
    return ;
}

std::size_t Request::LineParsing(std::string msg)
{
    this->req_msg = msg;
    std::size_t found = this->req_msg.find("\n");
    std::string first = this->req_msg.substr(0, found);
    std::vector<std::string> firstline;
    // std::cout << "msg\n" << msg << std::endl;
    firstline = ReqSplit(first, ' ');
    this->method = firstline[0];
    if (this->method != "GET" && this->method != "POST" && this->method != "DELETE")
        this->status = "405";
    // std::cout << "method\n" << this->method << std::endl;
    this->uri = checkQuery(firstline[1]);
    // if (this->uri != "HTTP/1.1")
    //     this->status = "404";
    this->version = firstline[2];
    std::size_t found_tmp;
    std::string line;
    std::vector<std::string> vline;
    this->pstatus = HEADER;
    while(1)
    {
        if (found >= this->req_msg.size())
            break;
        found_tmp = this->req_msg.find("\n", found+1);
        line = this->req_msg.substr(found+1, found_tmp - (found + 1));
        vline = ReqSplit(line, ':'); 
        if (vline == static_cast< std::vector<std::string> > (0))
            break ;
        int flag = 0;
        if (vline[0] == "Content-Type")
            flag = 1;
        vline[1] = removeWhiteSpace(vline[1], flag);
        this->headers.insert(std::make_pair(vline[0], vline[1]));
        if (vline[0] == "Host")
            this->host = vline[1];
        if (vline[0] == "Content-Length")
            this->content_length = static_cast<size_t>(atoi(vline[1].c_str()));
        if (vline[0] == "Transfer-Encoding" && vline[1] == "chunked")
            this->chunked = true;
        found = found_tmp;
    }
    this->pstatus = BODY;
    return (found);
}

void Request::ReqParsing(std::string msg)
{
    this->buffer.append(msg);
    if (this->buffer.find("\r\n\r\n") == std::string::npos && this->pstatus == FIRST)
        return ;
    if (this->pstatus == FIRST) {
        found = LineParsing(this->buffer);
        found = this->buffer.find("\r\n\r\n", found-1);
        if (found != this->buffer.size()-4) {
            std::string tmp = this->buffer.substr(found+4, this->buffer.size()-(found+4));
            this->buffer.clear();
            this->buffer.append(tmp);
            this->found = 0;
        }
        else
            this->buffer.clear();
        if (this->buffer.find("\r\n\r\n") == std::string::npos && this->pstatus == BODY)
            return ;
    }
    if (this->chunked == false && (this->content_length + 4 > this->buffer.size())) {// chunked가 아닌 body
        return ;
    }
    else if (this->chunked == false && (this->content_length + 4 < this->buffer.size())) {
        this->status = "400";
        return ;
    }
    if (this->chunked == false) {
        this->buffer = this->buffer.substr(0, this->buffer.size()-4);
        this->req_msg.append(this->buffer);
        for (size_t i = 0; i < this->buffer.size() ; i++)
            this->body.push_back(this->buffer[i]);
        this->body_size = this->body.size();
    }
    if (this->chunked == true && this->buffer.find("\r\n") == std::string::npos && this->pstatus == BODY)
        return ;
    if (this->chunked == true && this->buffer.find("\r\n") != std::string::npos && this->pstatus == BODY) {
        controlChunked(this->found);
        if (this->status != "200")
            return ;
    }
    if (this->method == "POST" && this->pstatus == BODY_DONE) {
        if (this->body_size == 0)
            this->status = "411";
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
    this->pstatus = PARSING_DONE;
    return ;
}

// int main() // 파싱 테스트
// {
//     Request Req;

//     // std::string msg = "POST HTTP?name 1.1\nHost: foo.com\nContent-Type: application/x-www-form-urlencoded\nhost: localhost:8080\nTransfer-Encoding: chunked\n4\r\nWiki\r\n5\r\npedia\r\nF\r\nin\r\n\r\nchunks.\r\n0\r\n\r\n";
//     std::string msg = "POST HTTP?name 1.1\nHost: foo.com\nContent-Type: application/x-www-form-urlencoded\nhost: localhost:8080\nTransfer-Encoding: chunked\r\n\r\n4\r\nWiki\r\n5\r\npedia\r\n2\r\nin\r\n7\r\nchunks.\r\n0\r\n\r\n";
//     // std::string msg = "POST HTTP?name 1.1\nContent-Length: 13\r\n\r\nyejinkimzzang\r\n\r\n";
//     Req.ReqParsing(msg);
//     Req.PrintRequest();
//     return (0);
// }
