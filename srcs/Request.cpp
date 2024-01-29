#include "../includes/Request.hpp"

Request::Request()
    :method("default"), uri("default"), version("default"), host("default"), status("200"), buffer(""), query_str(""), chunked(false), body_done(false), parsing_done(false)
{
    this->body.clear();
    this->buffer.clear();
}

const std::string &Request::getMethod() const { return this->method; }
const std::string &Request::getUri() const { return this->uri; }
const std::string &Request::getHost() const { return this->host; }
const std::string &Request::getStatus() const { return this->status; }
const bool &Request::getParsingStatus() const { return this->parsing_done; }
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
    ssize_t size, chk_size, body_len=0;
    std::size_t  found_RN, found_C, i=0;
    std::string line_size, line_contents;
    std::vector<char> tmp;
    
    tmp.clear();
    found_RN = this->req_msg.find("\r\n", found+1);
    line_size = this->req_msg.substr(found+1, found_RN-(found+1));
    size = hexToDec(line_size);
    body_len += size;
    while(!this->body_done) { //contents_length가 없을 경우도 있음, 411 코드
        chk_size = 0;
        if (found >= this->req_msg.size())
            break;
        if (i > 0) {
            found_RN = this->req_msg.find("\r\n", found+2);
            line_size = this->req_msg.substr(found+2, found_RN-(found+2));
            size = hexToDec(line_size);
            body_len += size;
            if (found_RN == this->req_msg.size()-4) {
                this->body_done = true;
            }
        }
        while(chk_size < size) {
            found_C = this->req_msg.find("\r\n", found_RN + 2);
            if (found_C == std::string::npos)
                break;
            line_contents = this->req_msg.substr(found_RN + 2, found_C-(found_RN+2));
            chk_size += (line_contents.size());
            if (chk_size == size) { // 한 줄에 크기에 맞게 잘 들어왔을 때
                for (size_t i = 0; i < line_contents.size() ; i++)
                    tmp.push_back(line_contents[i]);
                break;
            }
            else if (chk_size + 2 <= size) { // 한 줄에 다 안들어 왔을 때
                for (size_t idx = 0; idx < line_contents.size() ; idx++)
                    tmp.push_back(line_contents[idx]);
                tmp.push_back('\r');
                tmp.push_back('\n');
                // for (std::vector<char>::iterator it = tmp.begin(); it != tmp.end() ; ++it)
                //     std::cout << *it;
                // std::cout << '\n';
                chk_size += 2;
            }
            else // 에러
                break;
            found_RN = found_C;
        }
        // std::cout << "최종 tmp size " << tmp.size() << "size " << size << std::endl; //삭제
        if (tmp.size() != static_cast<size_t>(body_len) || size == -1) { //size맞지 않을 때
            // std::cout << "최종 tmp :: " << std::endl;
            // for (std::vector<char>::iterator it = tmp.begin(); it != tmp.end() ; ++it)
            //     std::cout << *it;
            this->status = "400";
            return ;
        }
        found = found_C;
        i++;
    }
    this->chunked = false;
    for (size_t idx = 0; idx < tmp.size() ; idx++)
            this->body.push_back(tmp[idx]);
    this->body_size = this->body.size();
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
    return (found);
}

void Request::ReqParsing(std::string msg)
{
    std::size_t found;

    this->buffer.append(msg);
    if (this->buffer.find("\r\n\r\n") == std::string::npos)
        return ;
    std::cout << "잘 찾고있음 " << std::endl;
    found = LineParsing(this->buffer);
    if (this->chunked)
        controlChunked(found);
    else {
        if (found != this->req_msg.size() && found < this->req_msg.size()) {
            std::string buf = this->req_msg.substr(found, this->req_msg.size());
            for (size_t i = 0; i < buf.size() ; i++)
                this->body.push_back(buf[i]);
        }
        this->body_size = this->body.size();
    }
    if (this->method == "POST") {
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
    std::cout << "parsing에서 확인해보자 " << this->parsing_done << std::endl;
    this->parsing_done = true;
    return ;
}

// int main() // 파싱 테스트
// {
//     Request Req;

//     // std::string msg = "POST HTTP?name 1.1\nHost: foo.com\nContent-Type: application/x-www-form-urlencoded\nhost: localhost:8080\nTransfer-Encoding: chunked\n4\r\nWiki\r\n5\r\npedia\r\nF\r\nin\r\n\r\nchunks.\r\n0\r\n\r\n";
//     // std::string msg = "POST HTTP?name 1.1\nHost: foo.com\nContent-Type: application/x-www-form-urlencoded\nContent-Type: multipart/form-data; yejinkim123456789\nhost: localhost:8080\nTransfer-Encoding: chunked\n4\r\nWiki\r\n5\r\npedia\r\n2\r\nin\r\n7\r\nchunks.\r\n0\r\n\r\n";
//     std::string msg = "POST HTTP?name 1.1\nUser-Agent: PostmanRuntime/7.36.1\nAccept: */*\nPostman-Token: f686f1fd-497e-4782-a7ba-19d7424ab7be\nHost: localhost:8080\nAccept-Encoding: gzip, deflate, br\nConnection: keep-alive\nContent-Type: multipart/form-data; boundary=—————————————734697986470657830089921\nContent-Length: 211\r\n\r\n";
//     Req.ReqParsing(msg);
//     Req.PrintRequest();
//     return (0);
// }
