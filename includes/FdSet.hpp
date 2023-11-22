#ifndef FDSET_HPP
#define FDSET_HPP

typedef enum    t_fd_type
{
	FD_SERVER,
	FD_CLIENT,
	FD_RESOURCE
}   e_fd_type;

class FdSet
{
	protected:
		int fd;
		e_fd_type fd_type;

	public:
        FdSet();
        ~FdSet();
};

#endif
