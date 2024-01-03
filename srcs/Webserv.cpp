#include "../includes/Webserv.hpp"
#include "../includes/Config.hpp"
#include "../includes/Client.hpp"

Webserv::Webserv() {}

Webserv::~Webserv() {}

void    Webserv::initServer(Config conf)
{
    if ((kq = kqueue()) == -1)
        error("kqueue() error");

    std::vector<Server> servs = conf.getConfig();
    for (std::vector<Server>::iterator it = servs.begin(); it != servs.end(); ++it)
    {
        int server_socket;
        struct sockaddr_in server_addr;

        if ((server_socket = socket(PF_INET, SOCK_STREAM, 0)) == -1)
            error("socket() error");

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        server_addr.sin_port = htons(it->getPort());

        if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
            error("bind() error");
        if (listen(server_socket, 3) == -1)
            error("listen() error");
        fcntl(server_socket, F_SETFL, O_NONBLOCK);

        change_events(change_list, server_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
        v_server.push_back(server_socket);

        std::cout << "[server start] " << it->getName() << ":" << it->getPort() << std::endl;
    }
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
    int new_events;
	struct kevent* curr_event;

    while (1)
    {
        new_events = kevent(kq, &change_list[0], change_list.size(), event_list, 8, NULL);
        if (new_events == -1)
            error("kevent error");
        change_list.clear();

        for (int i = 0; i < new_events; ++i)
        {
            curr_event = &event_list[i];

            if (std::find(v_server.begin(), v_server.end(), curr_event->ident) != v_server.end())
            { // server
                int client_socket;
                if ((client_socket = accept(curr_event->ident, NULL, NULL)) == -1)
                    error("accept() error");
                fcntl(client_socket, F_SETFL, O_NONBLOCK);

                change_events(change_list, client_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
                change_events(change_list, client_socket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
                
                Client *client = new Client(client_socket);
                v_client.push_back(client);
            }
            else
            { // client

            }

            if (curr_event->flags & EV_ERROR) {}
            else if (curr_event->filter == EVFILT_READ) {}
            else if (curr_event->filter == EVFILT_WRITE) {}
        }
    }
}
