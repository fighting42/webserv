#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "Config.hpp"
#include "Kqueue.hpp"

class Webserv
{
	private:
		Config	config;
		Kqueue	kqueue;

	public:
		Webserv();
		~Webserv();

		void start(int argc, char **argv);
};

#endif
