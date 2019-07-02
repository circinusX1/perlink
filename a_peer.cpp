
#include <unistd.h>
#include <assert.h>
#include <iostream>
#include <string>
#include "per_id.h"
#include "a_peer.h"
#include "udp_xdea.h"

#define PAIRING_TOUT 10

a_peer::a_peer(const char* id):_id(id)
{
    _status = SRV_UNREGISTERED;
}

void a_peer::main()
{
    udp_xdea      s(__key);
    char          buff[32];
    bool          linked = false;
    struct in_addr sin;
    int            k=0;


    _pl._verb = SRV_REGISTER;
    strcpy(_pl._u.reg,sock::GetLocalIP("127.0.0.1"));
    strcpy(_pl._u.reg.meiot,__meikey.c_str());
    strcpy(_pl._u.reg.id,__meikey.c_str());
    _pl.Payload._u.Reg.port = CLI_PORT;

    if(s.create(CLI_PORT))
    {
        while(__alive)
        {
            if(_status==SRV_UNREGISTERED)
            {
                if(0==s.send(&_pl,sizeof(_pl),SRV_PORT,SRV_IP))
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
                _io(s);
            }
            else
            {
                _receive(s);
            }
        }
    }
}

bool a_peer::_receive(udp_xdea& s)
{
    fd_set      rdSet;
    int         bytes=0;
    per_id      per;
    int nfds = (int)s.socket()+1;
    timeval  tv   = {0,0xFFFF};

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
                _regtime = time(0) + PAIRING_TOUT;
                break;
            case SRV_PEERING:
                _perring(s);
                break;
            case SRV_UNREGISTERED:
                status = SRV_UNREGISTERED;
                break;
            case PER_PONG:
                if(_per._p==0)
                {
                    _per = s.Rsin();
                    _status = PER_PERRED;
                }
                break;
            case PER_DATA:
                s.send(_pl._u.data, sizeof(_pl._u.data),_per);
                break;
        }
    }
}

bool a_peer::_peering(udp_xdea& s)
{
    _pl._verb = PER_PING;

    s.send(&_pl,sizeof(_pl), _pl._u._pp._private);
    s.send(&_pl,sizeof(_pl), _pl._u._pp._public);
}


void a_peer::_io(udp_xdea& s)
{
    assert(_per._p);

    std::cout << "LINKED to" << Ip2str(_per._a) << "\n";



}

