#ifndef U_SERVER_H
#define U_SERVER_H

#include <mutex>
#include <thread>
#include "per_id.h"
#include "sock.h"
#include "peers.h"

class u_server
{
public:
    u_server(const char* );
    virtual ~u_server();
    bool run();

    void main();

private:
    std::thread*        _t;
    peers               _ps;
};

#endif // U_SERVER_H
