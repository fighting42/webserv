#ifndef KQUEUE_HPP
#define KQUEUE_HPP

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/event.h>

#define RESET   "\033[0m"
#define GREEN   "\033[32m"

class Config;
class Client;
class Server;

class Kqueue
{
  private :
    int kq;
    std::vector<struct kevent> change_list;
    std::vector<Server *> v_config;

  public:
    Kqueue();
    ~Kqueue();
    void initServer(Config& config);
    void startServer();
    void connectClient(int server_fd);
    void disconnectClient(Client& client);
};

#endif
