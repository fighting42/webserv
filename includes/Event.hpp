#ifndef EVENT_HPP
#define EVENT_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sys/event.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctime>
#include <dirent.h>

#define RESET   "\033[0m"
#define BLUE    "\033[34m"
#define CYAN    "\033[36m"

extern std::map<std::string, std::string> m_mime_type;
extern fd_set server_fds;
extern fd_set client_fds;

class Client;

class Event
{
	private:
		Event();

	public:
		static void setMimeType();
		static std::string getMimeType(std::string extension);

		static void changeEvents(std::vector<struct kevent>& change_list, uintptr_t ident, \
			int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata);

		static void	checkMethod(Client& client, std::vector<struct kevent>& change_list);
		
		static void	readSocket(Client& client, std::vector<struct kevent>& change_list);
		static void	writeSocket(Client& client, std::vector<struct kevent>& change_list);
		static void	readFile(Client& client, std::vector<struct kevent>& change_list);
		static void writeFile(Client& client, std::vector<struct kevent>& change_list);
		static void	readPipe(Client& client, std::vector<struct kevent>& change_list);
		static void	writePipe(Client& client, std::vector<struct kevent>& change_list);
		static void execCgi(Client& client);

		static void	handleGet(Client& client, std::vector<struct kevent>& change_list);
		static void	handleDelete(Client& client, std::vector<struct kevent>& change_list);
		static void	handlePost(Client& client, std::vector<struct kevent>& change_list);
		static void	handleCgi(Client& client, std::vector<struct kevent>& change_list);
		static void	handleError(Client& client, std::vector<struct kevent>& change_list, const std::string &error_code);
		static void handleAutoindex(Client& client, std::vector<struct kevent>& change_list, std::string uri);
};

#endif
