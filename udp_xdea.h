#ifndef UDP_XDEA_H
#define UDP_XDEA_H

#include "sock.h"
#include "per_id.h"


class udp_xdea : public udp_sock
{
public:
    udp_xdea();
    udp_xdea(const uint32_t __key[4]);

    virtual int     send(const char* buff, const int length, const uint32_t* keys=0, int port=0, const char* ip=0);
    virtual int     send(const uint8_t* buff, const int length, const  SADDR_46& rsin, const uint32_t* keys=0);
    virtual int     receive(char* buff, int length,  SADDR_46& rsin, const uint32_t* keys=0);
    virtual int     receive(char* buff, int length, const uint32_t* keys=0, int port=0, const char* ip=0);
    virtual int     send(const uint8_t* buff, const int length, const  ipp& ipa, bool encrypt=true);
    virtual int     receive(char* buff, int length, const  ipp& ipa, bool encrypt=true);

private:
    uint8_t     _buff[MAX_UDP];
};

#endif // UDP_XDEA_H
