#include "per_id.h"


uint32_t     __key[4] = {0};
std::string  __meikey = "51de10940c93f85b02261266cd362885825a614183246bfcfef04a";

void ed(const uint8_t *in, uint8_t* out, int len, const uint32_t __key[4], bool encrypt)
{
    ::memcpy(out,in,len);
    for(int i=0;i<len/BLOCK_SIZE;i++)
    {
        if(encrypt)
            xtea_encipher(32,(uint32_t*)(out+(i*BLOCK_SIZE)),__key);
        else
            xtea_decipher(32,(uint32_t*)(out+(i*BLOCK_SIZE)),__key);
    }
    if(len%BLOCK_SIZE!=0)
    {
        int mod=len%BLOCK_SIZE;
        int offset=(len/BLOCK_SIZE)*BLOCK_SIZE;
        char data[BLOCK_SIZE];

        memcpy(data,out+offset,mod);
        if(encrypt)
            xtea_encipher(32,(uint32_t*)data,__key);
        else
            xtea_decipher(32,(uint32_t*)data,__key);
        memcpy(out+offset,data,mod);
    }
}
