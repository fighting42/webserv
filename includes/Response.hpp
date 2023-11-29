#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "Webserv.hpp"

class Response
{
	private:
		std::string version;
        int         status;
        std::string status_msg;
		std::map<std::string, std::string> headers;
		std::string body;
};

#endif
