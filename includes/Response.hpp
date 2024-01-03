#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include <string>
#include <map>

class Response
{
	private:
		std::string version;
		// int		 status; // make를 위한 주석입니당 지워도됩니당
		std::string status_msg;
		std::map<std::string, std::string> headers;
		std::string body;

	public:
		
};

#endif
