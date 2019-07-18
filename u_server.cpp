#include <unistd.h>
#include <iostream>
#include <unordered_set>
#include <assert.h>
#include <vector>
#include "u_server.h"
#include "per_id.h"
#include "udp_xdea.h"
#include "sqliter.h"

extern bool __alive;


#define provider true
#define consumer false


u_server::u_server(const char* key)
{
    UNUS(key);
}

u_server::~u_server()
{
    if (_t->joinable())
    {
        _t->join();
    }
    delete _t;
}

bool u_server::run()
{
    _t = new std::thread (&u_server::main, this);
    return _t != nullptr;
}

void u_server::main()
{
    udp_xdea        s;
    std::string     meiotkey;
    SrvCap          pl;
    time_t          now = 0;
    time_t          pingpers = 0;
    fd_set          rdSet;
    int             nfds;// = (int)s.socket()+1;
    timeval         tv;//   = {0,1024};
    uint32_t        keys[4];

    if(s.create(SRV_PORT))
    {
        s.bind();
        while(__alive)
        {
            SADDR_46 rem;

            now = time(0);

            FD_ZERO(&rdSet);
            FD_SET(s.socket(), &rdSet);
            tv.tv_sec=0;
            tv.tv_usec=100000;
            nfds = s.socket()+1;
            int sel = ::select(nfds, &rdSet, 0, 0, &tv);
            if(sel < 0)
            {
                std::cout << "select fatal \r\n";
                exit(errno);
            }
            if(sel > 0 && FD_ISSET(s.socket(), &rdSet))
            {
                int by = s.udp_sock::receive((char*)&pl,sizeof(pl));
                if(by == sizeof(pl))
                {
                    if(!_auth(s, pl,keys))
                    {
                        _deny(s);
                    }
                    else
                    {
                        _process(s, pl,keys);
                    }
                }
            }
            if(now - pingpers > TTLIVE || _dirty)
            {
                pingpers=now;
                _ping_pers(s, pl);
                memset(&_prev,0,sizeof(_prev));
                _delete_olies();
                _dirty = false;
            }
        }
    }
}

void u_server::_delete_olies()
{
    _dirty=_sq.del_oldies();
}

void u_server::_remove_peer(udp_xdea& s, SrvCap& pl, ipp& pub)
{
    char              shash[256];
    UNUS(pub);
    UNUS(s);

    ::sprintf(shash,"%s", pl._u.pp._public.str().c_str());
    std::size_t crc_hash = std::hash<std::string>{}(shash);
    _dirty = _sq.remove_peer(crc_hash);
}

void u_server::_store_peer(udp_xdea& s, SrvCap& pl, ipp& pub, const uint32_t* keys)
{
    if(_prev!=pl)
    {
        char              shash2[64];

        _prev = pl;
        ::sprintf(shash2,"%d %d %s %d %d %d %d", pub._a,  pub._p, pl._u.reg.id,keys[0],
                                                keys[1],keys[2],keys[3]);
        std::size_t crc_hash = std::hash<std::string>{}(shash2);

        if((_dirty = _sq.store_peer((const char*)(pl._u.reg.id+2), crc_hash, pub, pl, keys)))
        {
            pl._verb = SRV_REGISTERRED;
            pl.clear();
            s.send((const uint8_t*)&pl,sizeof(pl), s.Rsin(),keys);
        }
    }
}


void u_server::_ping_pers(udp_xdea& s, SrvCap& pl)
{
    std::vector<ipp>  pers;

    _sq.get_pers(pers);
    for(const auto& p : pers)
    {
        pl._verb = SRV_PING;
        s.send((const uint8_t*)&pl, sizeof(pl), p);
        ::usleep(1000);

        std::cout << "LINK FOR:" << p.str() << "--------------\n";

        for(const auto& q : pers)
        {
            pl._verb = SRV_SET_PEER;
            if(p._t != q._t)
            {
                pl._u.pp._public=p;
                std::cout << "TO:" << q.str() << " IT:" << p.str()  << "\n";
                s.send((const uint8_t*)&pl, sizeof(pl), q);
                ::usleep(1000);

                pl._u.pp._public=q;
                std::cout << "TO:" << p.str() << " IT:" << q.str() << "\n";
                s.send((const uint8_t*)&pl, sizeof(pl), p);
                ::usleep(1000);
            }
        }
    }
}

bool u_server::_auth(udp_xdea& s, SrvCap& pl, uint32_t keys[4])
{
    SrvCap          plclear;

    return _sq.authorize(pl, plclear, s.Rsin().ip4(), keys);
}


void u_server::_deny(udp_xdea& s)
{
    if(_sq.deny(s.Rsin().ip4()))
    {
        _ufw_reject(s.Rsin().ip4());
    }
}


void u_server::_process(udp_xdea& s, SrvCap& pl, const uint32_t *keys)
{
    std::cout <<__FUNCTION__ << ": verb: " << int(pl._verb) << "\n";

    if(pl._verb == PER_AIH)
    {
        _update_client_time(s,pl);
    }
    else if(pl._verb == SRV_REGISTER)
    {
        ipp pub(s.Rsin());

        pub._t = pl._u.reg.typ;
        std::cout<< "REGISTERING  (SEEN)"  << pub.str() << "  ,   " << IP2STR(s.Rsin())  << "\n";
        if(pl._verb==SRV_REGISTER)
        {
            _store_peer(s, pl, pub, keys);
        }
    }
    else if(pl._verb == SRV_UNREGISTER)
    {
        ipp pub(s.Rsin());

        std::cout << " dropping " << (pl._u.reg.id) << "\n";
        pl._verb = SRV_UNREGISTERED;
        s.send((const uint8_t*)&pl, sizeof(pl), s.Rsin());
        _remove_peer(s, pl, pub);
    }
    else
    {
        std::cout << Ip2str(s.Rsin().ip4()) << int(s.Rsin().port()) << " INVALID SRV verb \n";
    }
}

void u_server::_ufw_reject(uint32_t ip)
{
    UNUS(ip);
    /*
    std::string s = "sudo ufw insert 1 deny from "; // add no pass for ufw
    s += Ip2str(ip);
    system(s.c_str());
    */
}


/**
 * @brief u_server::_update_client_time
 * @param s
 * @param pl
 */
void u_server::_update_client_time(udp_xdea& s, SrvCap& pl)
{
    UNUS(pl);
    UNUS(s);
    _sq.update_peer(s.Rsin().ip4(), s.Rsin().port());
}


