#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "Webserv.hpp"

class Request
{
	private:
		std::string method;
		std::string location;
		std::string version;
		std::map<std::string, std::string> headers;
		std::string body;
};

#endif
