#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <vector>

class Response
{
	private:
		std::string version;
		std::vector<char> send_buffer;
		std::string status;
		std::string content_type;
		//std::string status_msg;
		std::map<std::string, std::string> headers;
		std::vector<char> body;
		std::map<std::string, std::string> m_mime;
		// int		 status; // make를 위한 주석입니당 지워도됩니당

		std::vector<char> getStatusMsg() const;
		std::string size_tToString(size_t value);
		std::string getMIMEType(const std::string& file_type);
		std::map<std::string, std::string> initializeMIMEMap();
		void makeHeaderLine();
		std::string getDate(std::time_t* t);

	public:
		Response();
		Response& operator=(const Response& obj);

		/* getter setter */
		const std::vector<char>& getSendBuffer() const;
		void setBody(const std::vector<char>& obj);
		void setStatus(const std::string& obj);
		void setContentType(const std::string& resource);
		void getBody(char* buffer, int read_size);

		void print();
		void makeResponse();
		
};

#endif
