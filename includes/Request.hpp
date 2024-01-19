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
    std::string status;
    std::vector<char> buffer;
	  size_t content_length;
	  std::vector<char> body;
	  size_t body_size;
    std::string query_str;
    bool chunked;
    
  public:
    Request();
    ~Request();
    const std::string& getMethod() const;
	  const std::string& getUri() const;
	  const std::string& getHost() const;
	  const std::string& getStatus() const;
    const bool& getChunked() const;
    const std::string& getQueryStr() const;
    const std::vector<char>& getBody() const;
	  const std::map<std::string, std::string>& getHeaders() const;
    void PrintRequest();
    void ReqParsing(std::string msg);
    std::size_t LineParsing(std::string msg);
    std::vector<std::string> ReqSplit(std::string input, char delimiter);
    std::string removeWhiteSpace(std::string str);
    std::string checkQuery(std::string uri);
    void controlChunked(size_t found, int flag);
    ssize_t hexToDec(const std::string& hex);
};

#endif
