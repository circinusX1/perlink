#ifndef UDP_XDEA_H
#define UDP_XDEA_H

#include "sock.h"

#define MAX_UDP 1140

class udp_xdea : public udp_sock
{
public:
    udp_xdea(const uint32_t __key[4]);

    virtual int     send(const char* buff, const int length, int port=0, const char* ip=0);
    virtual int     send(const unsigned char* buff, const int length, const  SADDR_46& rsin);
    virtual int     receive(char* buff, int length,  SADDR_46& rsin);
    virtual int     receive(char* buff, int length, int port=0, const char* ip=0  );
private:
    uint32_t    _key[4];
    uint8_t     _buff[MAX_UDP];
};

#endif // UDP_XDEA_H
