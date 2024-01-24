#include "../includes/Kqueue.hpp"
#include "../includes/Config.hpp"
#include "../includes/Client.hpp"
#include "../includes/Event.hpp"

Kqueue::Kqueue() {}

Kqueue::~Kqueue() {}

void	Kqueue::initServer(Config &config)
{
	if ((kq = kqueue()) == -1)
		throw "kqueue() error";

	Event::setMimeType();
	FD_ZERO(&client_fds);
	FD_ZERO(&server_fds);
	v_config = config.getServer();
	for (std::vector<Server *>::iterator it = v_config.begin(); it != v_config.end(); ++it)
	{
		int server_socket;
		struct sockaddr_in server_addr;

		if ((server_socket = socket(PF_INET, SOCK_STREAM, 0)) == -1)
			throw "socket() error";
		
		//포트 중복 허용
		int option = 1;
		setsockopt( server_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		server_addr.sin_port = htons((*it)->getPort());

		if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
			throw "bind() error";
		if (listen(server_socket, 3) == -1)
			throw "listen() error";
		fcntl(server_socket, F_SETFL, O_NONBLOCK);

		(*it)->setSocketFd(server_socket);
		Event::changeEvents(change_list, server_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

		std::cout << GREEN << "[server start] " << (*it)->getName() << ":" << (*it)->getPort() << RESET << std::endl;
	}
}

void Kqueue::disconnectClient(Client& client)
{
	std::cout << "[disconnect client] " << client.ip << ", " << client.socket_fd << std::endl;
	Event::changeEvents(change_list, client.socket_fd, EVFILT_WRITE, EV_DISABLE | EV_DELETE, 0, 0, &client);
	close(client.socket_fd);

	delete &client;
}

void Kqueue::connectClient(int server_fd)
{
	int client_socket;
	struct sockaddr_in client_addr;
	socklen_t addr_len = sizeof(client_addr);

	if ((client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len)) == -1)
		throw "accept() error";
	fcntl(client_socket, F_SETFL, O_NONBLOCK);

	char client_ip[INET_ADDRSTRLEN];
	if (inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip)) == NULL)
		throw "inet_ntop() error";

	Client *client = new Client(client_socket, client_ip);
	for (std::vector<Server *>::iterator it = v_config.begin(); it != v_config.end(); ++it)
	{
		if ((*it)->getSocketFd() == server_fd)
		{
			client->server = *it;
			break;
		}
	}
	Event::changeEvents(change_list, client_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, client);

	std::cout << "[connect new client] " << client_ip << ", " << client_socket << std::endl;
}

void	Kqueue::startServer()
{
	int new_events;
	struct kevent event_list[8];
	struct kevent* curr_event;

	while (1)
	{
		new_events = kevent(kq, &change_list[0], change_list.size(), event_list, 8, NULL);
		if (new_events == -1)
			throw "kevent error";
		change_list.clear();

		for (int i = 0; i < new_events; i++)
		{
			curr_event = &event_list[i];
			if (curr_event->flags & EV_ERROR)
			{
				if (FD_ISSET(curr_event->ident, &server_fds))
					throw "server socket error";
			}
			else if ((curr_event->flags & EV_EOF && FD_ISSET(curr_event->ident, &client_fds)) || \
				(curr_event->filter == EVFILT_TIMER && FD_ISSET(curr_event->ident, &client_fds)))
				disconnectClient(*static_cast<Client *>(curr_event->udata));
			else if (curr_event->filter == EVFILT_READ)
			{
				if (FD_ISSET(curr_event->ident, &server_fds))
					connectClient(curr_event->ident);
				else if (FD_ISSET(curr_event->ident, &client_fds))
				{
					Client *client = static_cast<Client *>(curr_event->udata);
					switch ((int)client->status)
					{
					case RECV_REQUEST:
						Event::readSocket(*client, change_list);
						Event::checkMethod(*client, change_list);
						break;
					case READ_FILE: 
						Event::readFile(*client, change_list);
						break;
					case READ_PIPE:
						Event::readPipe(*client, change_list);
						break;
					}
				}
			}
			else if (curr_event->filter == EVFILT_WRITE)
			{
				if (FD_ISSET(curr_event->ident, &client_fds))
				{
					Client *client = static_cast<Client *>(curr_event->udata);
					switch ((int)client->status)
					{
					case SEND_RESPONSE:
						Event::writeSocket(*client, change_list);
						break;
					case WRITE_FILE:
						Event::writeFile(*client, change_list);
						break;
					case WRITE_PIPE:
						Event::writePipe(*client, change_list);
						break;
					case DISCONNECT:
						disconnectClient(*client);
						break;
					}
				}
			}
		}
	}
}
