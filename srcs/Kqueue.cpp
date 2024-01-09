#include "../includes/Kqueue.hpp"
#include "../includes/Config.hpp"
#include "../includes/Client.hpp"

Kqueue::Kqueue() {}

Kqueue::~Kqueue() {}

void	Kqueue::initServer(Config &config)
{
	if ((kq = kqueue()) == -1)
		throw "kqueue() error";

	v_config = config.getServer();
	for (std::vector<Server *>::iterator it = v_config.begin(); it != v_config.end(); ++it)
	{
		int server_socket;
		struct sockaddr_in server_addr;

		if ((server_socket = socket(PF_INET, SOCK_STREAM, 0)) == -1)
			throw "socket() error";

		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		server_addr.sin_port = htons((*it)->getPort());

		if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
			throw "bind() error";
		if (listen(server_socket, 3) == -1)
			throw "listen() error";
		fcntl(server_socket, F_SETFL, O_NONBLOCK);

		changeEvents(server_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
		(*it)->setSocketFd(server_socket);
		v_server.push_back(server_socket);

		std::cout << GREEN << "[server start] " << (*it)->getName() << ":" << (*it)->getPort() << RESET << std::endl;
	}
}

void Kqueue::changeEvents(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata)
{
	struct kevent temp_event;

	EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
	change_list.push_back(temp_event);
}

void Kqueue::disconnectClient(int client_fd)
{
	for (std::vector<Client *>::iterator it = v_client.begin(); it != v_client.end(); ++it)
	{
		if ((*it)->getSocketFd() == client_fd)
		{
			v_client.erase(it);
			delete *it;
			break;
		}
	}
	close(client_fd);
	std::cout << "[disconnect client] " << client_fd << std::endl;
}

void Kqueue::connectClient(int server_fd)
{
	int client_socket;
	if ((client_socket = accept(server_fd, NULL, NULL)) == -1)
		throw "accept() error";
	fcntl(client_socket, F_SETFL, O_NONBLOCK);

	changeEvents(client_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	
	Client *client = new Client(client_socket);
	for (std::vector<Server *>::iterator it = v_config.begin(); it != v_config.end(); ++it)
	{
		if ((*it)->getSocketFd() == server_fd)
		{
			client->setServer(*it);
			break;
		}
	}
	v_client.push_back(client);
	std::cout << "[connect new client] " << client_socket << std::endl;
}

bool Kqueue::isServer(int fd)
{
	if (std::find(v_server.begin(), v_server.end(), fd) != v_server.end())
		return true;
	else
		return false;
}

bool Kqueue::isClient(int fd)
{
	for (std::vector<Client *>::iterator it = v_client.begin(); it != v_client.end(); ++it)
	{
		if ((*it)->getSocketFd() == fd)
			return true;
		else if ((*it)->getFileFd() == fd)
			return true;
	}
	return false;
}

Client* Kqueue::getClient(int fd)
{
	std::vector<Client *>::iterator it;
	for (it = v_client.begin(); it != v_client.end(); ++it)
	{
		if ((*it)->getSocketFd() == fd)
			break;
		else if ((*it)->getFileFd() == fd)
			break;
	}
	return *it;
}

void	Kqueue::startServer()
{
	int new_events;
	struct kevent* curr_event;

	while (1)
	{
		new_events = kevent(kq, &change_list[0], change_list.size(), event_list, 8, NULL);
		if (new_events == -1)
			throw "kevent error";
		change_list.clear();

		for (int i = 0; i < new_events; ++i)
		{
			curr_event = &event_list[i];
			if (curr_event->flags & EV_ERROR)
			{
				if (isServer(curr_event->ident))
					throw "server socket error";
				// else
					// disconnectClient(curr_event->ident);
			}
			else if (curr_event->filter == EVFILT_READ)
			{
				if (isServer(curr_event->ident))
					connectClient(curr_event->ident);
				else if (isClient(curr_event->ident))
				{
					Client *client = getClient(curr_event->ident); (void)client;
					switch (client->getStatus())
					{
					case RECV_REQUEST:
						client->handleSocketRead();
						changeEvents(client->getSocketFd(), EVFILT_READ, EV_DISABLE, 0, 0, NULL);
						client->checkMethod();
						changeEvents(client->getFileFd(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
						break;
					case READ_FILE: 
						client->handleFileRead();
						changeEvents(client->getFileFd(), EVFILT_READ, EV_DISABLE, 0, 0, NULL);
						close(client->getFileFd());
						changeEvents(client->getSocketFd(), EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
						break;
					case DISCONNECT:
						// disconnectClient(client->getSocketFd());
						break;
					}
				}
			}
			else if (curr_event->filter == EVFILT_WRITE)
			{
				if (isClient(curr_event->ident))
				{
					Client *client = getClient(curr_event->ident);
					switch (client->getStatus())
					{
					case SEND_RESPONSE:
						client->handleSocketWrite();
						break;
					case DISCONNECT:
						disconnectClient(client->getSocketFd());
						changeEvents(client->getSocketFd(), EVFILT_WRITE, EV_DISABLE, 0, 0, NULL);
						break;
					}
				}
			}
		}
	}
}
