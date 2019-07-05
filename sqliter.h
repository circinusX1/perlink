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




#endif // SQLITER_H
