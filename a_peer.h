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
    virtual ~a_peer();
    void main();
    bool snd(const uint8_t* pd, size_t l, bool crypt);
    int  rec(uint8_t* pd, size_t l, bool decrypt);
private:
    bool _receive(udp_xdea& s);
    void _io(udp_xdea& s, time_t now);
    void _peering(udp_xdea& s);
    void  _pipe_it();
private:
    ipp             _per;
    std::string     _id;
    SrvCap          _plin;
    SrvCap          _plout;
    int             _status;
    time_t          _regtime=0;
    time_t          _pingtime=0;
    std::string     _infile;
    std::string     _outfile;
};

#endif // A_PEER_H
