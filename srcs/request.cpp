#include "../includes/Request.hpp"

Request::Request()
    :method("default"), location("default"), version("default"), body("default"), headers("defaut", "default")
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
    if (delimiter == ':')
    {
        if (ret.size() == 1)
            return static_cast< std::vector<std::string> > (NULL);
    }
    return ret;
}

std::string Request::removeSpace(std::string str)
{
    
}

void Request::ReqParsing()
{
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
        found_tmp = this->req_msg.find("\n", found);
        line = this->req_msg.substr(found+1, found_tmp);
        vline = ReqSplit(line, ':');
        if (vline == static_cast< std::vector<std::string> > (NULL))
            break ;
        vline[1] = removeSpace(vline[1]);
        this->headers.insert(vline[0], vline[1]);
        found = found_tmp;
    }
    if (found != this->req_msg.size())
        this->body = this->req_msg.substr(found, this->req_msg.size());
}
