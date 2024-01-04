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

class Config;
class Client;

class Kqueue
{
  private :
    int kq;
    std::vector<struct kevent> change_list;
    struct kevent event_list[8];

    std::vector<int> v_server;
    std::vector<Client *> v_client;

  public:
    Kqueue();
    ~Kqueue();
    void initServer(Config config);
    void startServer();
    void change_events(std::vector<struct kevent> &change_list, uintptr_t ident, int16_t filter, 
                        uint16_t flags, uint32_t fflags, intptr_t data, void *udata);
    void connect_client(int server_fd);
    void disconnect_client(int client_fd);

    bool isServer(int fd);
    bool isClient(int fd);
};

#endif
