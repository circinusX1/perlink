#ifndef U_SERVER_H
#define U_SERVER_H

#include <mutex>
#include <thread>
#include <deque>
#include <sqlite3.h>

#include "per_id.h"
#include "sock.h"
#include "peers.h"

#define FAST_AUTHS  1024

struct QAuth{
    uint32_t ip;
    uint32_t key[4];
};

class udp_xdea;
class u_server
{
public:
    u_server(const char* );
    virtual ~u_server();
    bool run();
    void main();

private:
    void _store_peer(udp_xdea& s, SrvCap& pl, ipp& pub,const uint32_t *keys);
    void _remove_peer(udp_xdea& s, SrvCap& pl, ipp& pub);
    void _ping_pers(udp_xdea& s, SrvCap& pl);

    bool _auth(udp_xdea& s, SrvCap& pl, uint32_t keys[4]);
    void _deny(udp_xdea& s);
    void _process(udp_xdea& s, SrvCap& pl,const uint32_t* keys);
    void _ufw_reject(uint32_t ip);
    void _test();
    void _update_db(udp_xdea& s, SrvCap& pl);
    void _delete_olies();
private:
    std::thread*        _t;
    sqlite3             *_db = nullptr;
    SrvCap              _prev;
    bool                _dirty=false;
};

#endif // U_SERVER_H
