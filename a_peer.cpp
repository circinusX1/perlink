
#include <unistd.h>
#include <iostream>
#include <string>
#include "a_peer.h"
#include "udp_xdea.h"

a_peer::a_peer(const char* id):_id(id)
{
    _per.sin_port = 0;
}

void a_peer::main()
{
    udp_xdea      s(__key);
    char          buff[32];
    std::string   lip = __meikey; //sock::GetLocalIP("127.0.0.1");
    bool          linked = false;
    struct in_addr sin;

    lip += sock::GetLocalIP("127.0.0.1");
    lip += std::to_string(CLI_PORT);
    while(_id.length()<16)_id.append(".");
    lip += _id; // 16 characters

    if(s.create(CLI_PORT))
    {
        while(__alive)
        {
            if(_per.sin_port==0)
            {
                if(0==s.send(lip.c_str(),lip.length(),SRV_PORT,SRV_IP))
                {
                    std::cout << "no server \r\n";
                    break;
                }
                if(false == _wait_link(s))
                {
                    break;
                }
            }
            _linked(s);
        }
    }
}

bool a_peer::_wait_link(udp_xdea& s)
{
    fd_set      rdSet;
    int         bytes;
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
        bytes = s.receive((char*)&per, (int)sizeof(per));
        FD_CLR(s.socket(), &rdSet);
        if(bytes==0)
        {
            return false;
        }
    }
    if(per.link==true)
    {
        char  rec[128] = {0};
        time_t  neg = time(0) + 3;

        while(time(0) < neg && __alive)
        {
            s.send((const unsigned char*)__meikey.c_str(), __meikey.length(), per._public);
            ::usleep(1000);
            s.send((const unsigned char*)__meikey.c_str(), __meikey.length(), per._private);

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
                bytes = s.receive((char*)rec, (int)sizeof(rec)-1);
                if(bytes)rec[bytes]=0;
                FD_CLR(s.socket(), &rdSet);
            }
            if(bytes && __meikey==rec)
            {
                _per = s.Rsin();
                break;
            }
            ::usleep(50000);
        }
    }
    return true;
}

void a_peer::_linked(udp_xdea& s)
{

}
