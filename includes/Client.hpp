#ifndef CLIENT_HPP
#define CLIENT_HPP

#define RESET   "\033[0m"
#define BLUE    "\033[34m"
#define CYAN    "\033[36m"

#include <fcntl.h>
#include <unistd.h>
#include <fstream>

#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"

enum Status // 다음에 할 작업으로 status 바꿔주는겁니다요
{
	RECV_REQUEST, 
	READ_FILE,
    WRITE_FILE,
	SEND_RESPONSE,
	DISCONNECT
}; 

class Client
{
    private:
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

        Client();

    public:
        Client(int client_socket);
        ~Client();

        int     getSocketFd();
        int     getFileFd();
        int     getStatus();
        void    setServer(Server* server);

        void    findLocation();
        void    checkMethod();
        
        void    handleSocketRead();
        void    handleSocketWrite();
        void    handleFileRead();
        
        void    handleGet();
        void    handleDelete();
        void    handlePost();
        void    handleCgi();
        void    handleError(const std::string &error_code);
};

#endif
