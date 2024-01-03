#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <fcntl.h>
#include <unistd.h>

class Client
{
    private:
        int fd;
    public:
        Client();
        Client(int client_socket);
        ~Client();
        void HandleSocketRead();
        void HandleSocketWrite();
};

#endif
