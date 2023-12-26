#ifndef FDSET_HPP
#define FDSET_HPP

typedef enum	t_type
{
	SERVER,
	CLIENT,
	RESOURCE
}	e_type;

class FdSet
{
	protected:
		int fd;
		e_type type;

	public:
        FdSet();
        ~FdSet();
};

#endif
