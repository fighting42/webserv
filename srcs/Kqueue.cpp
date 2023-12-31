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

		changeEvents(change_list, server_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
		(*it)->setSocketFd(server_socket);
		v_server.push_back(server_socket);

		std::cout << "[server start] " << (*it)->getName() << ":" << (*it)->getPort() << std::endl;
	}
}

void Kqueue::changeEvents(std::vector<struct kevent> &change_list, uintptr_t ident, int16_t filter,
							uint16_t flags, uint32_t fflags, intptr_t data, void *udata)
{
	struct kevent temp_event;

	EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
	change_list.push_back(temp_event);
}

void Kqueue::disconnectClient(int client_fd)
{
	close(client_fd);
	for (std::vector<Client *>::iterator it = v_client.begin(); it != v_client.end(); ++it)
	{
		if ((*it)->getSocketFd() == client_fd)
		{
			v_client.erase(it);
			delete *it;
			break;
		}
	}
	// std::cout << "[disconnect client] " << client_fd << std::endl;
}

void Kqueue::connectClient(int server_fd)
{
	int client_socket;
	if ((client_socket = accept(server_fd, NULL, NULL)) == -1)
		throw "accept() error";
	fcntl(client_socket, F_SETFL, O_NONBLOCK);

	changeEvents(change_list, client_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // 나눌까?
	changeEvents(change_list, client_socket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL); // 나눌까?
	
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
	// std::cout << "[connect new client] " << client_socket << std::endl;
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
				else
					disconnectClient(curr_event->ident);
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
						client->checkMethod();
						break;
					case READ_FILE: 
						client->handleFileRead();
						break;
					case DISCONNECT:
						disconnectClient(client->getSocketFd());
						break;
					}

					// --- request test ---
					// read()로 http 요청 읽기 (curl로 보낸 요청메세지 buf에 담아 출력)
					// 요거 테스트 하고싶으면 위 switch문 잠깐 주석달기!
					// char buf[1024];
					// int n = read(curr_event->ident, buf, sizeof(buf));
					// buf[n] = '\0';
					// if (n > 0)
					// 	std::cout << "[request message]" << std::endl << buf << std::endl;
					// else
					// 	disconnectClient(curr_event->ident);
					// --- request test end ---
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
						break;
					}

					// --- response test ---
					// 응답 생성 후 write()로 보내기 (응답메세지 buf에 담아 전송. 현재 buf 값은 예시! 요청에 따라서 달라져야함)
					// char buf[1024] = "HTTP/1.1 200 OK\nContent-type:text/plain\nContent-Length:6\n\ntest!\n\n";
					// write(curr_event->ident, buf, sizeof(buf));
					// std::cout << "[response message]" << std::endl << buf << std::endl;
					// disconnectClient(curr_event->ident); 
					// --- response test end ---
				}
			}
		}
	}
}
