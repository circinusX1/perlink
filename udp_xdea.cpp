
#include "udp_xdea.h"
#include "per_id.h"

udp_xdea::udp_xdea(const uint32_t __key[4])
{
    ::memcpy(_key,__key,sizeof(_key));
}

int udp_xdea::send(const char* buff,  int length, int port, const char* ip)
{
    ed((const uint8_t*)buff,_buff,length,_key,true);
    return udp_sock::send(_buff,length,port,ip);
}

int udp_xdea::send(const unsigned char* buff,
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
        _buff[bytes] = 0;
        ed(_buff,(uint8_t*)buff,length,_key,false);
    }
    return bytes;

}

