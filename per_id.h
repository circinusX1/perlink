#ifndef PER_ID_H
#define PER_ID_H

#include <string>
#include "sock.h"

#define SRV_PORT    5001
#define CLI_PORT    5002
#define SRV_IP      "127.0.0.1"

struct per_id
{
    bool   link;
    struct sockaddr_in  _public;
    struct sockaddr_in  _private;
} __attribute__ ((aligned));

struct per_pair
{
    per_pair(){
        _a[0].sin_port=0;
        _a[1].sin_port=0;
        _b[0].sin_port=0;
        _b[1].sin_port=0;
    }
    struct sockaddr_in  _a[2];
    struct sockaddr_in  _b[2];
};


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

void ed(const uint8_t *in, uint8_t* out, int len, uint32_t __key[4], bool encrypt);

extern uint32_t     __key[4];
extern std::string  __meikey;


#endif // PER_ID_H
