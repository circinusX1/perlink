#include <iostream>
#include "udp_xdea.h"
#include "per_id.h"

udp_xdea::udp_xdea(const uint32_t __key[4])
{

}

udp_xdea::udp_xdea()
{

}

int udp_xdea::send(const char* buff,  int length,const uint32_t* keys, int port, const char* ip)
{
    if( keys){
        ed((const uint8_t*)buff,_buff,length, keys,true);
        return udp_sock::send(_buff,length,port,ip);
    }
    return udp_sock::send(buff,length,port,ip);
}

int udp_xdea::send(const uint8_t* buff,
                   const int length, const  SADDR_46& rsin,const uint32_t* keys)
{
    std::cout << "<--" << IP2STR(rsin) << "\n";
    if(keys){
        ed(buff,_buff,length,keys,true);
        return udp_sock::send(_buff,length,rsin);
    }
    return udp_sock::send(buff,length,rsin);
}

int udp_xdea::receive(char* buff, int length,  SADDR_46& rsin,const uint32_t* keys)
{
    if(keys)
    {
        int bytes = udp_sock::receive(_buff,length,rsin);
        if(bytes>0)
        {
            _buff[bytes] = 0;

            ed(_buff,(uint8_t*)buff,length,keys,false);
        }
    }
    return udp_sock::receive(buff,length,rsin);
}

int udp_xdea::receive(char* buff, int length, const uint32_t* keys, int port, const char* ip)
{
    if(keys)
    {
        int bytes = udp_sock::receive(_buff,length,port,ip);
        if(bytes>0)
        {
            buff[bytes] = 0;
            ::memset(buff,0,length);
            ed(_buff,(uint8_t*)buff,bytes,keys,false);
        }
        return bytes;
    }
    return udp_sock::receive(buff,length,port,ip);
}

int  udp_xdea::send(const uint8_t* buff, const int length, const  ipp& ipa, bool encrypt)
{
    SADDR_46 rsin;
    rsin.sin_family      = AF_INET;

    rsin.sin_port        = htons (ipa._p);
    rsin.sin_addr.s_addr = htonl(ipa._a);

    if(encrypt)
    {
        ed((const uint8_t*)buff,_buff,length,ipa._keys,true);
        return udp_sock::send(_buff,length,rsin);
    }
    return udp_sock::send(buff,length,rsin);
}

int udp_xdea::receive(char* buff, int length,  const  ipp& ipa, bool encrypt)
{
    SADDR_46 rsin;
    rsin.sin_port        = htons (ipa._p);
    rsin.sin_family      = AF_INET;
    rsin.sin_addr.s_addr = htonl(ipa._a);

    std::cout << "->>"<<  IP2STR(rsin) << "\n";

    if(encrypt)
    {
        int bytes = udp_sock::receive(_buff,length,rsin);
        if(bytes>0)
        {
            _buff[bytes] = 0;
            ed(_buff,(uint8_t*)buff,length,ipa._keys,false);
        }
        return bytes;
    }
    return udp_sock::receive(buff,length,rsin);

}

