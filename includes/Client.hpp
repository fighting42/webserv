#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <fcntl.h>
#include <unistd.h>
#include <fstream>

#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"

enum Status // 다음에 할 작업으로 status 바꿔주는겁니다요
{
	READ_SOCKET, // 구 RECV_REQUEST
	READ_FILE,
    WRITE_FILE,
	WRITE_SOCKET, // 구 SEND_RESPONSE
	DISCONNECT
}; 

class Client
{
    private:
        Client();

    public:
        int socket_fd;
        int file_fd;
        Status status;
        Request request;
        Response response;
        ssize_t written; //reponse의 적힌 사이즈변수
        
        std::string body;
        int body_length;
        
        Server *server;
        std::map<std::string, std::string> m_location;

        Client(int client_socket);
        ~Client();
};

#endif
