#ifndef SQLITER_H
#define SQLITER_H

#include <iostream>
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
};




#endif // SQLITER_H
