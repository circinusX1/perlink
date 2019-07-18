#ifndef SQLITER_H
#define SQLITER_H

#include <iostream>
#include <vector>
#include <stdint.h>
#include <sqlite3.h>
#include "per_id.h"


class Sqliter
{
    sqlite3* _db;
public:
    Sqliter( sqlite3 **db, int line ):_db(*db)
    {
        int err = ::sqlite3_open(DB_FILE, db);
        if(err!=SQLITE_OK)
        {
            std::cerr << DB_FILE <<", "<< line << ", "<<   sqlite3_errmsg(*db) << "\n";
            exit (1);
        }
    }
    ~Sqliter(){
         sqlite3_close(_db);
    }

    void commit()
    {
        char* szerr;
        const char c[] = "COMMIT;";
        ;int err  = sqlite3_exec(_db, (const char*)c, 0, 0, &szerr);
        if( err != SQLITE_OK )
        {
            sqlite3_free(szerr);
        }
    }
};

class Sqlyte
{
public:
    Sqlyte();
    ~Sqlyte();

    bool remove_peer(std::size_t crc_hash);
    bool del_oldies();
    bool store_peer(const char* name, std::size_t crc_hash, const ipp& pub, const SrvCap& pl,const uint32_t* keys);
    void get_pers(std::vector<ipp>& s);
    bool authorize(SrvCap& pl,const SrvCap& plclear, uint32_t ip4, uint32_t keys[4]);
    bool deny(uint32_t ip4);
    void update_peer(uint32_t ip, uint32_t port);
private:
    void _test();
private:

    sqlite3             *_db = nullptr;

};



#endif // SQLITER_H

