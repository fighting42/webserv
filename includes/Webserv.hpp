#ifndef WEBSERV_HPP
#define WEBSERV_HPP

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

#include "FdSet.hpp"

class Server;

class Webserv
{
    private :
		int kq;
		std::vector<struct kevent> change_list;
		struct kevent event_list[8];
		int server_socket;
		int new_events;
		struct kevent* curr_event;
		struct sockaddr_in server_addr;

    public :
        Webserv();
        ~Webserv();
		void initServer(Server serv);
        void startServer();
        void change_events(std::vector<struct kevent> &change_list, uintptr_t ident, int16_t filter,
							uint16_t flags, uint32_t fflags, intptr_t data, void *udata);

};

void	error(std::string str);

#endif
