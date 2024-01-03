#include "../includes/Request.hpp"

Request::Request()
    :method("default"), location("default"), version("default"), body("default")
{}

Request::~Request(){}

std::vector<std::string> Request::ReqSplit(std::string input, char delimiter) 
{
    std::vector<std::string> ret;
    std::stringstream ss(input);
    std::string temp;
 
    while (getline(ss, temp, delimiter)) {
        ret.push_back(temp);
    }
    if (delimiter == ':') //헤더에서 localhost:8080 같은거 들어올 때 처리하기
    {
        if (ret.size() == 1)
            return (static_cast< std::vector<std::string> > (0));
    }
    return ret;
}

std::string Request::removeSpace(std::string str)
{
    std::string tmp;

    tmp = str.substr(1, str.size());
    return (tmp);
}

void Request::PrintRequest()
{
    std::cout << "\033[1;94m" << std::endl; // blue
    std::cout << this->method << std::endl;
    std::cout << this->location << std::endl;
    std::cout << this->version << std::endl;
    std::map<std::string, std::string>::iterator it;
    for (it = this->headers.begin(); it != this->headers.end(); it++) {
        if (it->first == "Content-Length")
            std::cout << "Content-Length: " << it->second << std::endl;
        break;
    }
    std::cout << "\033[0m" << std::endl; // reset
}

void Request::ReqParsing(std::string msg)
{
    this->req_msg = msg;
    std::size_t found = this->req_msg.find("\n");
    std::string first = this->req_msg.substr(0, found);
    std::vector<std::string> firstline;

    firstline = ReqSplit(first, ' ');
    this->method = firstline[0];
    this->location = firstline[1];
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
        vline[1] = removeSpace(vline[1]);
        this->headers.insert(std::make_pair(vline[0], vline[1]));
        found = found_tmp;
    }
    if (found != this->req_msg.size() && found < this->req_msg.size())
        this->body = this->req_msg.substr(found, this->req_msg.size());
}
