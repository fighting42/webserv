#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>

class Request
{
	private:
    std::string req_msg;
		std::string method;
		std::string location;
		std::string version;
		std::map<std::string, std::string> headers;
		std::string body;
    
  public:
    Request();
    ~Request();
    void ReqParsing(std::string msg);
    std::vector<std::string> ReqSplit(std::string input, char delimiter);
    std::string removeSpace(std::string str);
};

#endif
