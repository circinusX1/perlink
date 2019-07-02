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
    bool _receive(udp_xdea& s);
    void _io(udp_xdea& s);
    void _peering(udp_xdea& s);
private:
    ipp             _per;
    std::string     _id;
    Payload         _pl;
    int             _status;
    time_t          _regtime;
};

#endif // A_PEER_H
