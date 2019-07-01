
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
    char            chunk[128];

    if(s.create(SRV_PORT))
    {
        s.bind();
        while(__alive)
        {
            SADDR_46 rem;
            int by = s.receive(chunk,sizeof(chunk));
            if(by>0)
            {
                chunk[by] = 0;
                //meetiot,ip,id
                std::string whole = chunk;
                std::string meio = whole.substr(0,__meikey.length());
                std::string ipid = whole.substr(__meikey.length());
                std::string ip   = whole.substr(0, ipid.length()-16);
                std::string id   = whole.substr(ipid.length()-16);

                if(__meikey == meio)
                {
                    sockaddr_in priv = s.Rsin();
                    priv.sin_addr.s_addr  =inet_addr(ip.c_str());

                    std::cout << s.ssock_addrip() <<":"<< s.Rsin().port()<< ":"<< chunk << "\n";
                    if(_ps.add(id, s.Rsin(), priv))
                    {
                        //_linkthem();
                    }
                }
            }
        }
    }
}
