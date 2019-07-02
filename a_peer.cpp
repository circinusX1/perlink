#include <unistd.h>
#include <assert.h>
#include <iostream>
#include <string>
#include "sock.h"
#include "per_id.h"
#include "a_peer.h"
#include "udp_xdea.h"


extern int __perport;

a_peer::a_peer(const char* id):_id(id)
{
    _status = SRV_UNREGISTERED;
    _regtime = time(0) -PAIRING_TOUT;
    _pingtime = 0;
}

a_peer::~a_peer()
{

}

void a_peer::main()
{
    int         counter;
    udp_xdea    s(__key);
    time_t      now;

    _pl._verb = SRV_REGISTER;
    strcpy(_pl._u.reg.ip,sock::GetLocalIP("127.0.0.1"));
    strcpy(_pl._u.reg.meiot,__meikey.c_str());
    strcpy(_pl._u.reg.id,_id.c_str());
    _pl._u.reg.port = __perport;
    if(s.create(__perport))
    {
        s.set_option(SO_REUSEADDR,1);
        s.bind();
        while(__alive)
        {
            now = time(0);
            if(_status==SRV_UNREGISTERED)
            {
                _regtime = time(0);
                if(0==s.send((const char*)&_pl,sizeof(_pl),SRV_PORT,SRV_IP))
                {
                    std::cout << "no server \r\n";
                    break;
                }
                if(false == _receive(s))
                {
                    break;
                }
            }
            else if(_status==PER_PERRED)
            {
                _io(s,now);
            }
            else
            {
                _receive(s);
            }
            if(++counter %100 == 0)
            {
                if(_status!=PER_PERRED)
                {
                    if(time(0)-_regtime>PAIRING_TOUT)
                    {
                        _status = SRV_UNREGISTERED;
                        _regtime = time(0);
                    }
                }
            }
        }
    }
}

bool a_peer::_receive(udp_xdea& s)
{
    fd_set      rdSet;
    int         bytes=0;
    int         nfds = (int)s.socket()+1;
    timeval     tv   = {0,0xFFFF};

    FD_ZERO(&rdSet);
    FD_SET(s.socket(), &rdSet);
    int sel = ::select(nfds, &rdSet, 0, 0, &tv);
    if(sel < 0)
    {
        std::cout << "select fatal \r\n";
        return false;
    }
    if(sel > 0 && FD_ISSET(s.socket(), &rdSet))
    {
        bytes = s.receive((char*)&_pl, (int)sizeof(_pl));
        FD_CLR(s.socket(), &rdSet);
        if(bytes==0)
        {
            return false;
        }
    }
    if(bytes == sizeof(_pl))
    {
        switch(_pl._verb)
        {
        case SRV_REGISTERRED:
            _status = SRV_REGISTERRED;
            _regtime = time(0);
            break;
        case SRV_PEERING:
            _peering(s);
            break;
        case SRV_UNREGISTERED:
            _status = SRV_UNREGISTERED;
            break;
        case PER_PING:
            std::cout << "GOT PING FROM "<<Ip2str(s.Rsin().sin_addr.s_addr)<<" \r\n";
            _pl._verb = PER_PONG;
             s.send((const uint8_t*)&_pl,sizeof(_pl), s.Rsin());
            break;
        case PER_PONG:
            std::cout << "GOT PONG FROM "<<Ip2str(s.Rsin().sin_addr.s_addr)<<" \r\n";
            if(_per._p==0)
            {
                _per = s.Rsin();
                _status = PER_PERRED;
                _pl._verb = SRV_UNREGISTER;
                s.send((const char*)&_pl,sizeof(_pl),SRV_PORT,SRV_IP);
            }
            break;
        }
    }
    return true;
}

/*
 */
void a_peer::_peering(udp_xdea& s)
{
    std::cout << "PER PING "<<__perport <<":"<< Ip2str(_pl._u.pp._private._a) << _pl._u.pp._private._p << "\r\n";
    std::cout << "PER PING "<<__perport <<":"<<  Ip2str(_pl._u.pp._public._a) << _pl._u.pp._public._p << "\r\n";

    _pl._verb = PER_PING;
    s.send((const uint8_t*)&_pl,sizeof(_pl), _pl._u.pp._private);
    s.send((const uint8_t*)&_pl,sizeof(_pl), _pl._u.pp._public);
}

void a_peer::_io(udp_xdea& s, time_t now)
{
    assert(_per._p);
    std::cout << "LINKED to" << Ip2str(_per._a) << _per._p << "\n";
    s.send((const uint8_t* )"hello\r\n",7,_per);
    sleep(2);
    if(now-_pingtime > PAIRING_TOUT)
    {
        _pl._verb = PER_PING;
        s.send((const uint8_t*)&_pl,sizeof(_pl), _per);
        _pingtime = now;
    }
}

/**
 * @brief a_peer::snd
 * @param pd
 * @param l
 * @param crypt
 * @return
 */
bool a_peer::snd(const uint8_t* pd, size_t l, bool crypt)
{
    assert(pd[0]==PER_DATA);


     _pingtime = time(0);
}

/**
 * @brief a_peer::rec
 * @param pd
 * @param l
 * @param decrypt
 * @return
 */
int  a_peer::rec(uint8_t* pd, size_t l, bool decrypt)
{
    //select and such
    assert(pd[0]==PER_DATA);
    return 0;
}


