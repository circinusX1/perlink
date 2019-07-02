
#include "udp_xdea.h"
#include "per_id.h"

udp_xdea::udp_xdea(const uint32_t __key[4])
{
    ::memcpy(_key,__key,sizeof(_key));
    _secured=true;
}

udp_xdea::udp_xdea()
{
    _secured=false;
}

int udp_xdea::send(const char* buff,  int length, int port, const char* ip)
{
    ed((const uint8_t*)buff,_buff,length,_key,true);
    return udp_sock::send(_buff,length,port,ip);
}

int udp_xdea::send(const uint8_t* buff,
                   const int length, const  SADDR_46& rsin)
{
    ed(buff,_buff,length,_key,true);
    return udp_sock::send(_buff,length,rsin);
}

int udp_xdea::receive(char* buff, int length,  SADDR_46& rsin)
{
    int bytes = udp_sock::receive(_buff,length,rsin);
    if(bytes>0)
    {
        _buff[bytes] = 0;
        ed(_buff,(uint8_t*)buff,length,_key,false);
    }
    return bytes;
}

int udp_xdea::receive(char* buff, int length, int port, const char* ip)
{
    int bytes = udp_sock::receive(_buff,length,port,ip);
    if(bytes>0)
    {
        buff[bytes] = 0;
        ::memset(buff,0,length);
        ed(_buff,(uint8_t*)buff,bytes,_key,false);
    }
    return bytes;

}


int  udp_xdea::send(const uint8_t* buff, const int length, const  ipp& ipa)
{
    ed((const uint8_t*)buff,_buff,length,_key,true);
    char loco[128] = {0};
    ed((const uint8_t*)_buff,(uint8_t*)loco,length,_key,false);

    SADDR_46 rsin;
    rsin.sin_port        = htons (ipa._p);
    rsin.sin_family      = AF_INET;
    rsin.sin_addr.s_addr = htonl(ipa._a);

    return udp_sock::send(_buff,length,rsin);
}

int udp_xdea::receive(char* buff, int length,   const  ipp& ipa)
{
    SADDR_46 rsin;
    rsin.sin_port        = htons (ipa._p);
    rsin.sin_family      = AF_INET;
    rsin.sin_addr.s_addr = htonl(ipa._a);

    int bytes = udp_sock::receive(_buff,length,rsin);
    if(bytes>0)
    {
        _buff[bytes] = 0;
        ed(_buff,(uint8_t*)buff,length,_key,false);
    }
    return bytes;
}


int  udp_xdea::rsend(const uint8_t* buff, const int length, const  ipp& ipa)
{
    SADDR_46 rsin;
    rsin.sin_port        = htons (ipa._p);
    rsin.sin_family      = AF_INET;
    rsin.sin_addr.s_addr = htonl(ipa._a);
    return udp_sock::send(buff,length,rsin);
}

int udp_xdea::rreceive(char* buff, int length, const  ipp& ipa)
{
    SADDR_46 rsin;
    rsin.sin_port        = htons (ipa._p);
    rsin.sin_family      = AF_INET;
    rsin.sin_addr.s_addr = htonl(ipa._a);

    int bytes = udp_sock::receive(buff,length,rsin);
    if(bytes>0)
    {
        _buff[bytes] = 0;
    }
    return bytes;
}

int udp_xdea::rreceive(char* buff, int length, int port, const char* ip)
{
    return udp_sock::receive(buff,length);
}
