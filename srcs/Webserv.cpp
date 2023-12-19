#include "../includes/Webserv.hpp"
#include "../includes/Config.hpp"

Webserv::Webserv() {}

Webserv::~Webserv() {}

void    Webserv::initServer(Config conf)
{
    int server_socket;
    struct sockaddr_in server_addr;

    if ((server_socket = socket(PF_INET, SOCK_STREAM, 0)) == -1)
        err("socket() error");

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET; // IPv4
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 32bit
    server_addr.sin_port = htons(8080); // port
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
        err("bind() error");

    if (listen(server_socket, 3) == -1)
        err("listen() error");
    fcntl(server_socket, F_SETFL, O_NONBLOCK);

    int kq;
    if ((kq = kqueue()) == -1)
        err("kqueue() error");
}

void Webserv::change_events(std::vector<struct kevent> &change_list, uintptr_t ident, int16_t filter,
							uint16_t flags, uint32_t fflags, intptr_t data, void *udata)
{
	struct kevent temp_event;

	EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
	change_list.push_back(temp_event);
}

void    Webserv::startServer()
{
    change_events(change_list, server_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    while (1)
    {
        new_events = kevent(kq, &change_list[0], change_list.size(), event_list, 8, NULL);
        if (new_events == -1)
            err("kevent error");
        change_list.clear();

        for (int i = 0; i < new_events; ++i)
        {
            if (curr_event->flags & EV_ERROR) {}
            else if (curr_event->filter == EVFILT_READ)
            {
				
            }
            else if (curr_event->filter == EVFILT_WRITE)
            {
				
            }
        }
    }
}
