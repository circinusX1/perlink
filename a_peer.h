#ifndef A_PEER_H
#define A_PEER_H

#include <string>
#include "per_id.h"


extern bool __alive;

class udp_xdea;
class a_peer
{
public:
    a_peer(const char* id);
    void main();

private:
    bool _wait_link(udp_xdea& s);
    void _linked(udp_xdea& s);
private:
    struct sockaddr_in _per;
    std::string        _id;
};

#endif // A_PEER_H
