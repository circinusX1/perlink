#include <unistd.h>
#include <assert.h>
#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>
#include "sock.h"
#include "per_id.h"
#include "a_peer.h"
#include "udp_xdea.h"


extern int __perport;

a_peer::a_peer(const char* id):_id(id)
{
    _status = SRV_UNREGISTERED;
    _regtime = 0;
    _pingtime = 0;
    system("rm -f /tmp/peer_*");
}

a_peer::~a_peer()
{

}

void a_peer::main()
{

    int         counter;
    udp_xdea    s(__key);
    time_t      now;

    strcpy(_plout._u.reg.ip,sock::GetLocalIP("127.0.0.1"));
    strcpy(_plout._u.reg.meiot,__meikey.c_str());
    strcpy(_plout._u.reg.id,_id.c_str());
    _plout._u.reg.port = __perport;
    if(s.create(__perport))
    {
        s.set_option(SO_REUSEADDR,1);
        s.bind();
        while(__alive)
        {
            _plout._verb = SRV_REGISTER;

            now = time(0);
            if(_status==SRV_UNREGISTERED)
            {
                if(now - _regtime > REG_TICK)
                {
                    _regtime = now;
                    _plout._verb = SRV_REGISTER;
                    if(0==s.send((const char*)&_plout,sizeof(_plout),SRV_PORT,SRV_IP))
                    {
                        std::cout << "no server \r\n";
                        break;
                    }
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
        bytes = s.receive((char*)&_plin, (int)sizeof(_plin));
        FD_CLR(s.socket(), &rdSet);
        if(bytes==0)
        {
            return false;
        }

        std::cout << " GOT " << bytes << " verb: "<< int(_plin._verb)<< "from port: "
                  << int (s.Rsin().port())  <<"\n";
        if(bytes == sizeof(_plin))
        {

            switch(_plin._verb)
            {
            case SRV_REGISTERRED:
                std::cout << "SRV REGISTERRED " << "\n";
                _status = SRV_REGISTERRED;
                _regtime = time(0);
                break;
            case SRV_PEERING:
                std::cout << "SRV PEERING " << "\n";
                _peering(s);
                break;
            case SRV_UNREGISTERED:
                std::cout << "SRV RELEASED " << "\n";
                _status = SRV_UNREGISTERED;
                break;
            case PER_PING:
                std::cout << "PER_GOT PING FROM "<<Ip2str(s.Rsin().ip4raw())<<" \r\n";
                _plout._verb = PER_PONG; //back
                s.send((const uint8_t*)&_plout,sizeof(_plout), s.Rsin());
                break;
            case PER_PONG:
                std::cout << "PER_ GOT PONG FROM (droping server) " <<Ip2str(s.Rsin().sin_addr.s_addr) <<" : "<<_plin._u.reg.meiot << " \r\n";
                _per      = s.Rsin();
                _status   = PER_PERRED;
                _plout._verb = SRV_UNREGISTER;

                s.send((const char*)&_plout,sizeof(_plout),SRV_PORT,SRV_IP);
                std::cout << "LINKED to" << Ip2str(_per._a) << _per._p << "\n";
                _pipe_it();
                break;
            default:
                std::cout << "PER_ GOT " <<Ip2str(s.Rsin().ip4()) <<"\r\n";
                break;
            }
        }
        else if(bytes > 0)
        {
            _status = _plin._verb;
            std::cerr << "rejected "<< (int)_status  <<" \r\n";
            exit(2);
        }
    }
    return true;
}

/*
 */
void a_peer::_peering(udp_xdea& s)
{
    std::cout << "PER PING "<<__perport <<"->"<< Ip2str(_plin._u.pp._private._a) <<
                 ":"<<int(_plin._u.pp._private._p) << "\r\n";

    std::cout << "PER PING "<<__perport <<"->"<<  Ip2str(_plin._u.pp._public._a) <<
                 ":"<<int(_plin._u.pp._public._p) << "\r\n";

    _plin._verb = PER_PING;
    s.send((const uint8_t*)&_plin, sizeof(_plin), _plin._u.pp._private);
    s.send((const uint8_t*)&_plin, sizeof(_plin), _plin._u.pp._public);
}

void a_peer::_io(udp_xdea& s, time_t now)
{
    fd_set      rdSet;
    int         bytes=0;
    int         nfds = (int)s.socket()+1;
    timeval     tv   = {0,0xFFFF};
    FILE*       po;

    assert(_per._p);
    if(now-_pingtime > PAIRING_TOUT)
    {
        _plout._verb = PER_AIH;
        s.send((const uint8_t*)&_plout,sizeof(_plout), _per);
        _pingtime = now;
    }
    ::usleep(128);
    // select and such
    uint8_t buff[MAX_UDP+1];


    FD_ZERO(&rdSet);
    FD_SET(s.socket(), &rdSet);
    int sel = ::select(nfds, &rdSet, 0, 0, &tv);
    if(sel < 0)
    {
        std::cout << "select fatal \r\n";
        exit(5);
    }
    if(sel > 0 && FD_ISSET(s.socket(), &rdSet))
    {
        bytes = s.receive((char*)buff, (int)sizeof(buff));
        FD_CLR(s.socket(), &rdSet);
        if(bytes>0)
        {
            if(buff[0]!=PER_DATA)
                goto SEND;
            FILE* pi = ::popen(_infile.c_str(), "wb");
            if(pi)
            {
                std::cout << "saving " << buff << "\n";
                ::fwrite(buff+1, 1, bytes-1, pi);
                ::pclose(pi);
            }
        }
    }
SEND:
    po = ::popen(_outfile.c_str(), "wb");
    if(po)
    {
        buff[0] = uint8_t(PER_DATA);
        bytes  = ::fread(buff+1, 1, bytes, po);
        ::pclose(po);
        if(bytes)
        {
            std::cout << "reading " << buff << "\n";
            s.send(buff,bytes+1,_per);
            _pingtime = now;
        }
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
}

void  a_peer::_pipe_it()
{
    char pipefile[512];
    const char * ip = Ip2str(_per._a);
    ::sprintf(pipefile,"/tmp/peer_%s%s%d.in",_id.c_str(), ip, (int)_per._p);

    if(::access(pipefile,0)==0)
    {
        ::unlink(pipefile);
    }
    else
    {
        int fi = ::mkfifo(pipefile, O_RDWR|O_NONBLOCK| S_IRWXU|S_IRWXG|S_IRWXG  );
        if(fi<0)
        {
            perror("mkfifo");
            exit(3);
        }
    }

    int fdin = ::open (pipefile, O_RDWR|O_CREAT);
    if(fdin<0)
    {
        std::cerr << pipefile << ": PIPE: " << strerror(errno);
    }
    else
    {
        ::fcntl(fdin, F_SETFL, O_NONBLOCK);
        ::fcntl(fdin, F_SETPIPE_SZ, 1024 * 8912);
    }
    ::close(fdin);
    _infile = pipefile;

    ::sprintf(pipefile,"/tmp/peer_%s%s-%d.out",_id.c_str(), ip, _per._p);

    if(::access(pipefile,0)==0)
    {
        ::unlink(pipefile);
    }
    else
    {
        int fi = ::mkfifo(pipefile, O_RDWR|O_NONBLOCK| S_IRWXU|S_IRWXG|S_IRWXG  );
        if(fi<0)
        {
            perror("mkfifo");
            exit(3);
        }
    }
    fdin = ::open (pipefile, O_RDWR|O_CREAT);
    if(fdin<0)
    {
        std::cerr << pipefile << ": PIPE: " << strerror(errno);
    }
    else
    {
        ::fcntl(fdin, F_SETFL, O_NONBLOCK);
        ::fcntl(fdin, F_SETPIPE_SZ, 1024 * 8912);
    }
    ::close(fdin);
    _outfile = pipefile;

}
