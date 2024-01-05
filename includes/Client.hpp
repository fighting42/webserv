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
        Request request;
        Response response;
        ssize_t written; //reponse의 적힌 사이즈변수
        Server server;
        std::map<std::string, std::string> m_location;

        Client();

    public:
        Client(int client_socket);
        ~Client();

        int     getSocketFd();
        int     getStatus();
        void    setStatus(Status status);
        void    setServer(Server server);
        void    findLocation();
        void    handleGet();

        void HandleSocketRead();
        void HandleSocketWrite();
};

#endif
