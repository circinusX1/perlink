#ifndef U_SERVER_H
#define U_SERVER_H

#include <mutex>
#include <thread>
#include "per_id.h"
#include "sock.h"
#include "peers.h"

class udp_xdea;
class u_server
{
public:
    u_server(const char* );
    virtual ~u_server();
    bool run();
    void main();

private:
    void _linkthem(udp_xdea& s, const std::string& id);

private:
    std::thread*        _t;
    peers               _ps;
};

#endif // U_SERVER_H
