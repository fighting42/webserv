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
		std::string location; // uri로 변수명 변경..해주세요
		std::string version;
    // std::string host; // headers 중에 Host만.. 변수로 빼주세요
		std::map<std::string, std::string> headers;
		std::string body;
    
  public:
    Request();
    ~Request();
    void ReqParsing(std::string msg);
    std::vector<std::string> ReqSplit(std::string input, char delimiter);
    std::string removeSpace(std::string str);
    void PrintRequest();

    // 변수들 getter 만들어주세요..ㅜ
};

#endif
