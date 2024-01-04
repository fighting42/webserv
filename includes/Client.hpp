#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <fcntl.h>
#include <unistd.h>

#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"

enum Status // 언제 값 바꿔주는지
{
	RECV_REQUEST, // client 객체 생성시
	READ_FILE, // handleGet() 함수 마지막
	SEND_RESPONSE, // handleFileRead() 함수 마지막
	DISCONNECT // handleSocketWrite() 함수 마지막 또는 error?
}; 

class Client
{
    private:
        int socket_fd;
        // int file_fd;
        Status status;
        Request req;
        Response res;
        
        // config 파일 내용. 요청메세지 보고 해당하는 server, location 블록 담기
        Server server; // host -> server
        std::map<std::string, std::string> m_location; // uri -> location

        Client();

    public:
        Client(int client_socket);
        ~Client();

        int     getSocketFd();
        int     getStatus();
        void    setStatus(Status status);
        void    setConfig(); // set server, set m_location

        void HandleSocketRead();
        void HandleSocketWrite();
};

#endif
