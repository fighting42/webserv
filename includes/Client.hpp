#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "FdSet.hpp"

class Client : public FdSet
{
    private:

    public:
        Client();
        virtual ~Client();
};

#endif
