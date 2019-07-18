#ifndef A_PEER_H
#define A_PEER_H

#include <string>
#include <set>
#include "per_id.h"

extern bool __alive;

class udp_xdea;
class a_peer
{
public:
    a_peer(const char* id);
    virtual ~a_peer();
    void main();
    void send_to_pers(udp_xdea& s, const char* data, size_t len ,bool encrypt);
    int  rec(uint8_t* pd, size_t l, bool decrypt);

private:
    bool _receive(udp_xdea& s, time_t now);
    bool _received(udp_xdea& s, time_t now, int bytes);
    bool _srv_process(udp_xdea& s, SrvCap&  plin, time_t now);
    bool _per_process(udp_xdea& s, int bytes, time_t now);
    void _peering(udp_xdea& s, SrvCap&  plin);
    void _pipe_it();
    bool _data_in(udp_xdea& s, int bytes);
    void _i_am_here(udp_xdea& s);
    int  _rec_udp(udp_xdea& s, time_t now);
    void _show_pers(udp_xdea& s);
    void _proc_perrs(udp_xdea& s);

private:
    SrvCap              _mecap;
    std::set<SADDR_46>  _pers;
    std::string         _id;

    int             _status;
    time_t          _regtime=0;
    time_t          _pingtime=0;
    std::string     _infile;
    std::string     _outfile;
    fd_set          _rdout;
    int             _fdout = 0;
    SADDR_46        _srvsin;
    char            _type=-1;
    uint8_t*        _udpbuffer;
};

#endif // A_PEER_H
