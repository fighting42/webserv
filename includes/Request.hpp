#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>

enum pStatus
{
  FIRST,
  HEADER,
  BODY,
  BODY_DONE,
  PARSING_DONE
};

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
    std::string buffer;
	  size_t content_length;
	  std::vector<char> body;
    ssize_t chunkedLineSize;
    int chunkedSize_chk;
	  size_t body_size;
    std::string query_str;
    pStatus pstatus;
    bool chunked;
    std::size_t found;
    
  public:
    Request();
    ~Request();
    void  init();
    const std::string& getMethod() const;
	  const std::string& getUri() const;
	  const std::string& getHost() const;
	  const std::string& getStatus() const;
    const pStatus& getParsingStatus() const;
    const std::string& getQueryStr() const;
    const std::vector<char>& getBody() const;
	  const std::map<std::string, std::string>& getHeaders() const;
    void setUri(std::string newUri);
    void PrintRequest();
    void ReqParsing(std::string msg);
    std::size_t LineParsing(std::string msg);
    std::vector<std::string> ReqSplit(std::string input, char delimiter);
    std::string removeWhiteSpace(std::string str, int flag);
    std::string checkQuery(std::string uri);
    void controlChunked(size_t found);
    ssize_t hexToDec(const std::string& hex);
};

#endif
