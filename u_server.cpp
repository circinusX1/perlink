
#include <unistd.h>
#include <iostream>
#include "u_server.h"
#include "per_id.h"
#include "udp_xdea.h"

extern bool __alive;

u_server::u_server(const char* key)
{

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
    udp_xdea        s(__key);
    std::string     meiotkey;
    Payload         pl;

    if(s.create(SRV_PORT))
    {
        s.bind();
        while(__alive)
        {
            SADDR_46 rem;
            int by = s.receive(&pl,sizeof(pl));
            if(by == sizeof(pl))
            {
                if(pl._verb == SRV_REGISTER || pl._verb == SRV_UNREGISTER)
                {
                    if(__meikey == pl._u.Reg.meiot)
                    {
                        ipp priv(pl._u.reg.ip, pl._u.reg.port);
                        ipp pub(s.Rsin());

                        std::cout<< "s<-c p:" << Ip2str(priv._a) << ":"<<port <<", "<< chunk << "\n";
                        std::cout<< "s<-c P:" << Ip2str(pub._a)  << ":"<<pub._p <<", "<< chunk << "\n";

                        if(pl._verb==SRV_REGISTER)
                        {
                            if(_ps.add(pl._u.reg.id, pub, priv))
                            {
                                _linkthem(s, id);
                            }
                            else
                            {
                                pl._verb = SRV_REGISTERRED;
                                s.send(&pl, sizeof(pl), s.Rsin());
                            }
                        }
                        else
                        {
                            _ps.remove(pl._u.reg.id));
                            pl._verb = SRV_UNREGISTERRED;
                            s.send(&pl, sizeof(pl), s.Rsin());
                        }
                        usleep(1000);
                    }
                }
            }
        }
    }
}

void u_server::_linkthem(udp_xdea& s, const std::string& id)
{
    const per_pair*  pp = _ps.find(id);
    if(pp)
    {
        Payload  pl;

        pl._verb = SRV_PEERING;
        pl._u._pp._link = true;
        pl._u._pp._private = pp->_b[0];
        pl._u._pp._public  = pp->_b[1];

        // A <- b/B
        std::cout << "a<-" << Ip2str(pp->_a[0]._a) <<":"<< pp->_a[0]._p<<"\n";
        std::cout << "a<-" << Ip2str(pp->_a[1]._a) <<":"<< pp->_a[1]._p<<"\n";

        s.send((const unsigned char*)&pl,sizeof(pl), pp->_a[0]);
        s.send((const unsigned char*)&pl,sizeof(pl), pp->_a[1]);

        pl._u._pp._private = pp->_a[0];
        pl._u._pp._public  = pp->_a[1];

        // A <- b/B
        std::cout << "b<-" << Ip2str(pp->_b[0]._a) <<":"<< pp->_b[0]._p<<"\n";
        std::cout << "b<-" << Ip2str(pp->_b[1]._a) <<":"<< pp->_b[1]._p<<"\n";

        s.send((const unsigned char*)&pl,sizeof(pl), pp->_b[0]);
        s.send((const unsigned char*)&pl,sizeof(pl), pp->_b[1]);

        std::cout << "\r\n";
        ::sleep(4);
    }
}

