#ifndef PER_ID_H
#define PER_ID_H

#include <string>
#include "sock.h"

#define MAX_UDP     1024
#define SRV_PORT    5001
#define CLI_PORT    5002
#define SRV_IP      "127.0.0.1"
#define PAIRING_TOUT 10
#define TTLIVE       60
#define REG_TICK     2

struct ipp{
    ipp():_a(0),_p(0){}
    ipp(const char* a, int p){
        _a = (uint32_t)(inet_addr(a));
        _p = htons(p);
    }
    ipp(struct SADDR_46& sa):_a(htonl(sa.sin_addr.s_addr)), _p(htons(sa.sin_port)){}
    ipp(struct sockaddr_in& sa):_a(htonl(sa.sin_addr.s_addr)), _p(htons(sa.sin_port)){}

    bool operator==(const ipp& r){return _a==r._a && _p==r._p;}
    const ipp& operator=(const ipp& r){_a = r._a; _p = r._p; return *this;}
    const ipp& operator=(const sockaddr_in& r){_a = htonl(r.sin_addr.s_addr);
                                               _p = htons(r.sin_port); return *this;}

    uint32_t _a;
    int      _p;
}__attribute__ ((aligned));


struct per_pair
{
    ipp  _a[2];
    ipp  _b[2];
    time_t born;
}__attribute__ ((aligned));


#define BLOCK_SIZE 8
inline void xtea_encipher(unsigned int num_rounds,
                          uint32_t v[2],
                          uint32_t const __key[4])
{
    unsigned int i;
    uint32_t v0=v[0], v1=v[1], sum=0, delta=0x9E3779B9;
    for (i=0; i < num_rounds; i++) {
        v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + __key[sum & 3]);
        sum += delta;
        v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + __key[(sum>>11) & 3]);
    }
    v[0]=v0; v[1]=v1;
}


inline void xtea_decipher(unsigned int num_rounds,
                          uint32_t v[2],
                          uint32_t const __key[4])
{
    unsigned int i;
    uint32_t v0=v[0], v1=v[1], delta=0x9E3779B9, sum=delta*num_rounds;
    for (i=0; i < num_rounds; i++) {
        v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + __key[(sum>>11) & 3]);
        sum -= delta;
        v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + __key[sum & 3]);
    }
    v[0]=v0; v[1]=v1;
}

void ed(const uint8_t *in, uint8_t* out, int len, const uint32_t __key[4], bool encrypt);
extern uint32_t     __key[4];
extern std::string  __meikey;


enum {
    SRV_REGISTER=0,
    SRV_REGISTERRED,
    SRV_PEERING,
    SRV_UNREGISTER,
    SRV_UNREGISTERED,
    SRV_REJECT,
    PER_PING,
    PER_AIH, // i am here
    PER_PONG,
    PER_PERRED,
    PER_DATA=':',
};

struct  SrvCap
{
    SrvCap(){memset(this,0,sizeof(*this));}
    void set_meiot(const std::string& k){
        memset(_u.reg.meiot,0,sizeof(_u.reg.meiot));
        ::strcpy(_u.reg.meiot, k.c_str());
    }
    uint8_t _verb;
    union U
    {
        U(){}
        struct {
            char meiot[58];
            char ip[16];
            int  port;
            char id[16];
        }   reg;

        struct
        {
            bool  _link;
            ipp   _public;
            ipp   _private;
        } pp;

    } _u;
    char padding[8];
}__attribute__ ((aligned));












#endif // PER_ID_H
