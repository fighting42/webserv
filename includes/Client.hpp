#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <fcntl.h>
#include <unistd.h>
#include <fstream>

#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "Event.hpp"

enum Status
{
	RECV_REQUEST,
	SEND_RESPONSE,
	READ_FILE,
    READ_PIPE,
    WRITE_PIPE,
	DISCONNECT
}; 

class Client
{
    private:
        Client();

    public:
        int socket_fd;
        int file_fd;
        int pipe_fd[2];
        int pid;
        std::string ip;
        Status status;
        Request request;
        Response response;
        ssize_t written; //reponse의 적힌 사이즈변수
        
        std::string body;
        int body_length;
        
        Server *server;
        std::map<std::string, std::string> m_location;
        std::string location_uri;

        Client(int client_socket, std::string client_ip);
        ~Client();
        void init();
};

#endif
