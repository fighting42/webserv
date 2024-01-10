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
		std::string uri;
		std::string version;
    std::string host;
		std::map<std::string, std::string> headers;
		std::string body;
    std::string status;
    
  public:
    Request();
    ~Request();
    const std::string& getMethod() const;
	  const std::string& getUri() const;
	  const std::string& getHost() const;
	  const std::string& getStatus() const;
    void PrintRequest();
    void ReqParsing(std::string msg);
    std::vector<std::string> ReqSplit(std::string input, char delimiter);
    std::string removeSpace(std::string str);
};

#endif
