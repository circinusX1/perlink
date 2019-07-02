#ifndef UDP_XDEA_H
#define UDP_XDEA_H

#include "sock.h"
#include "per_id.h"


class udp_xdea : public udp_sock
{
public:
    udp_xdea();
    udp_xdea(const uint32_t __key[4]);

    virtual int     send(const char* buff, const int length, int port=0, const char* ip=0);
    virtual int     send(const uint8_t* buff, const int length, const  SADDR_46& rsin);
    virtual int     receive(char* buff, int length,  SADDR_46& rsin);
    virtual int     receive(char* buff, int length, int port=0, const char* ip=0  );
    virtual int     send(const uint8_t* buff, const int length, const  ipp& ipa);
    virtual int     receive(char* buff, int length,   const  ipp& ipa);

    virtual int     rsend(const uint8_t* buff, const int length, const  ipp& ipa);
    virtual int     rreceive(char* buff, int length, const  ipp& ipa);
    virtual int     rreceive(char* buff, int length, int port=0, const char* ip=0  );
    void set_keys(const uint32_t* keys)
    {
        if(keys){
            ::memcpy(_key,keys,sizeof(_key));
        }
        else {
            memset(_key,0,sizeof(_key));
        }
    }
private:
    bool        _secured = false;
    uint32_t    _key[4] = {0,0,0,0};
    uint8_t     _buff[sizeof(SrvCap)];
};

#endif // UDP_XDEA_H
