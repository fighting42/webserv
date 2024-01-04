#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <fcntl.h>
#include <unistd.h>

#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"

enum Status
{
	RECV_REQUEST,
	SEND_RESPONSE,
	READ_FILE,
	WRITE_FILE
    // 더 추가해야됨
}; 

class Client
{
    private:
        int fd;
        Status status;
        Request req;
        Response res;
        
        // config 파일 내용. 요청메세지 보고 해당하는 server, location 블록 담기
        Server serv; // 헤더 host -> server
        std::map<std::string, std::string> loc; // 시작줄 uri -> location

    public:
        Client();
        Client(int client_socket);
        ~Client();

        int     getFd();
        Status  getStatus();
        void    setStatus(Status status);

        void HandleSocketRead();
        void HandleSocketWrite();
};

#endif
