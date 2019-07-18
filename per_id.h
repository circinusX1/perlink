#ifndef PER_ID_H
#define PER_ID_H

#include <string>
#include "sock.h"

#define MAX_UDP     10024
#define SRV_PORT    5001
#define CLI_PORT    5002
#define SRV_IP      "127.0.0.1"
#define PAIRING_TOUT 10
#define TTLIVE       15
#define REG_TICK     10
#define IAM_HERE_TO  15
#define SERVER_DOWN_TO 30

#define MAX_ATTEMPS     512
#define DB_PATH         "/usr/share/perlink"
#define DB_FILE         "/usr/share/perlink/perlink.db"

struct ipp{
    uint32_t  _a;
    uint16_t  _p;
    int       _t;
    uint32_t  _keys[4];
    ipp():_a(0),_p(0){}
    ipp(const char* a, int p,int t){
        _a = (uint32_t)(htonl(inet_addr(a)));
        _p = (int)(htons(p));
        _t = t;
    }
    ipp(uint32_t a, uint16_t p, int t):_a(a), _p(p),_t(t){}
    ipp(const ipp& r):_a(r._a), _p(r._p),_t(r._t){::memcpy(_keys,r._keys,sizeof(_keys));}
    ipp(struct SADDR_46& sa):_a(htonl(sa.sin_addr.s_addr)), _p(htons(sa.sin_port)){}
    ipp(struct sockaddr_in& sa):_a(htonl(sa.sin_addr.s_addr)), _p(htons(sa.sin_port)){}
    bool operator==(const ipp& r){return _a==r._a && _p==r._p && _t==r._t; }
    const ipp& operator=(const ipp& r){_a = r._a; _p = r._p; ::memcpy(_keys,r._keys,sizeof(_keys)); return *this;}
    const ipp& operator=(const sockaddr_in& r){_a = htonl(r.sin_addr.s_addr);_p = htons(r.sin_port); return *this;}
    std::string str()const{
        std::string ret = std::string(Ip2str((_a)));
        ret += ":";
        ret += std::to_string((_p));
        ret += ":";
        ret += std::to_string((_t));
        return ret;
    }
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
    SRV_REGISTER=1,
    SRV_REGISTERRED,
    SRV_SET_PEER,
    SRV_UNREGISTER,
    SRV_UNREGISTERED,
    SRV_REJECT,
    SRV_PING,
    PER_PING,
    PER_AIH, // 8 am here
    PER_PONG,
    PER_LINKED,
    PER_WAITING_PER,
    PER_DATA=':',
};


struct tper{
    ipp  _pub;
    ipp  _priv;
    bool _type;
};


struct  SrvCap
{
    uint8_t _verb;
    union U
    {
        U(){}
        struct {
            char    meiot[58];
            bool    typ;      // Publisher true Consumenr false
            char    id[16];
        }   reg;

        struct
        {
            ipp     _public;
            bool    _typ;      // Publisher true COnsumenr false
            char    _id[16];
        } pp;

    } _u;
    char padding[8];

    SrvCap(){memset(this,0,sizeof(*this));}
    void set_meiot(const std::string& k){
        memset(_u.reg.meiot,0,sizeof(_u.reg.meiot));
        ::strcpy(_u.reg.meiot, k.c_str());
    }
    bool operator!=(const SrvCap& r)
    {
        return memcmp(this, &r,sizeof(*this));
    }
    bool operator==(const SrvCap& r)
    {
        return !memcmp(this, &r,sizeof(*this));
    }
    const SrvCap& operator=(const SrvCap& r)
    {
        ::memcpy(this, &r,sizeof(*this));
        return *this;
    }
    void clear(){memset(&_u,0,sizeof(_u));}
}__attribute__ ((aligned));



inline void _tokeys(const std::string&  meiot, uint32_t keys[4])
{
    ::sscanf(meiot.c_str(),"%8X%8X%8X%8X", &keys[0], &keys[1], &keys[2], &keys[3]);
}








#endif // PER_ID_H
