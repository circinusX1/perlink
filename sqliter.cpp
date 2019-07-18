#include "sqliter.h"



Sqlyte::Sqlyte()
{
    Sqliter __sq(&_db,__LINE__);

    char *szerr = nullptr;
    sqlite3_busy_timeout(_db, 4096);
    std::string sql =  "CREATE TABLE IF NOT EXISTS 'mid' (Id integer primary key autoincrement,"
                       "D DATETIME NOT NULL,"
                       "mid TEXT NOT NULL,"
                       "ip INTEGER);";
    int rc = sqlite3_exec(_db, sql.c_str(), 0, 0, &szerr);
    if( rc != SQLITE_OK )
    {
        std::cerr << " SQL error: " << szerr;
        exit (1);
    }
    sql =  "CREATE TABLE IF NOT EXISTS 'rej' (Id integer primary key autoincrement,"
           "D DATETIME NOT NULL,"
           "cnt INTEGER,"
           "ip INTEGER);";
    rc = sqlite3_exec(_db, sql.c_str(), 0, 0, &szerr);
    if( rc != SQLITE_OK )
    {
        std::cerr << " SQL error: " << szerr;
        exit (1);
    }
    /**
    pairs
*/
    sql =   "CREATE TABLE IF NOT EXISTS 'pers' (Id integer primary key autoincrement,"
            "D  DATETIME NOT NULL,"
            "I  TEXT,"
            "K  INTEGER UNIQUE,"
            "PA  INTEGER,"
            "PP  INTEGER,"
            "T  INTEGER,"
            "R Integer,"
            "MK0 Integer,"
            "MK1 Integer,"
            "MK2 Integer,"
            "MK3 Integer);";

    rc = sqlite3_exec(_db, sql.c_str(), 0, 0, &szerr);
    if( rc != SQLITE_OK )
    {
        std::cerr << " SQL error: " << szerr;
        exit (1);
    }
    _test();
}

Sqlyte::~Sqlyte()
{

}

bool Sqlyte::del_oldies()
{
    char    *szerr;
    Sqliter __sq(&_db,__LINE__);
    std::string sql = "DELETE FROM pers WHERE D <= date('now','-30 seconds')";
    int res  = sqlite3_exec(_db, sql.c_str(), 0, 0, &szerr);
    if( res != SQLITE_OK )
    {
        std::cerr << __LINE__ << ": " << szerr << "\n";
        sqlite3_free(szerr);
        return false;
    }
    return true;
}



void Sqlyte::_test()
{
    char            *szerr = nullptr;
    Sqliter          __sq(&_db,__LINE__);

    std::string sql = "INSERT INTO mid (D,mid,ip) ";
    sql += "VALUES (";
    sql += "datetime('now')";
    sql += ",'51de10940c93f85b02261266cd362885825a614183246bfcfef04a','0');";
    int res  = sqlite3_exec(_db, sql.c_str(), 0, 0, &szerr);
    if( res != SQLITE_OK )
    {
        std::cerr << __LINE__ << ": " << szerr << "\n";
        sqlite3_free(szerr);
    }

    sql = "INSERT INTO mid (D,mid,ip) ";
    sql += "VALUES (";
    sql += "datetime('now')";
    sql += ",'5edc15b7888ca1be1048ca995d7f8e33313f17049e30dcb9e2b47a','0');";
    res  = sqlite3_exec(_db, sql.c_str(), 0, 0, &szerr);
    if( res != SQLITE_OK )
    {
        std::cerr << __LINE__ << ": " << szerr << "\n";
        sqlite3_free(szerr);
    }
}

bool Sqlyte::remove_peer(std::size_t crc_hash)
{
    char              *szerr;
    Sqliter __sq(&_db,__LINE__);

    std::string sql = "DELETE FROM pers WHERE K='";
    sql += crc_hash; sql += "';";

    int res  = sqlite3_exec(_db, sql.c_str(), 0, 0, &szerr);
    if( res != SQLITE_OK )
    {
        std::cerr << __LINE__ << ": " << szerr << "\n";
        sqlite3_free(szerr);
        return false;
    }
    return true;
}


bool Sqlyte::store_peer(const char* name, std::size_t crc_hash, const ipp& pub,
                        const SrvCap& pl, const uint32_t* keys)
{
    sqlite3_stmt      *statement;
    char              *szerr;
    int               refs = 0;
    Sqliter          __sq(&_db,__LINE__);
    bool             dirty=false;

    int err = sqlite3_open(DB_FILE, &_db);
    if(err!=SQLITE_OK)
    {
        std::cerr << DB_PATH <<  sqlite3_errmsg(_db) << "\n";
        exit (1);
    }
    std::string sql = "SELECT R,PA,PP,T FROM pers WHERE I='";
    sql += name; sql += "';";

    std::cout << sql << "\n";

    if ( sqlite3_prepare(_db, sql.c_str(), -1, &statement, 0 ) == SQLITE_OK )
    {
        while(1)
        {
            int res = sqlite3_step(statement);
            if (res==SQLITE_ERROR)
            {
                std::cerr << sqlite3_errmsg(_db) << "\n";
                break;
            }
            if ( res == SQLITE_DONE)
            {
                break;
            }
            if ( res == SQLITE_ROW )
            {
                tper        p;

                refs          = sqlite3_column_int(statement, 0);
                p._pub        = ipp(sqlite3_column_int(statement, 1),
                                    sqlite3_column_int(statement, 2),
                                    sqlite3_column_int(statement, 3));
            }
        }
        sqlite3_reset(statement);
        dirty=true;
    }
    else
    {
        std::cerr << sqlite3_errmsg(_db) << "\n";
    }

    refs++;
    /*
           "D  DATETIME NOT NULL,"
           "I  TEXT,"
           "K  INTEGER UNIQUE,"
           "PA  INTEGER,"
           "PP  INTEGER,"
           "T  INTEGER,"
           "R Integer);";

    */

    sql = "INSERT INTO pers (D,I,K,PA,PP,T,R,MK0,MK1,MK2,MK3) VALUES (";
    sql += "datetime('now')";             sql += ",'";           // date
    sql += (pl._u.reg.id+2);              sql += "',";       // ID/NAME
    sql += std::to_string(crc_hash);      sql += ","; // KRC
    sql += std::to_string(pub._a);        sql += ",";     // date
    sql += std::to_string(pub._p);        sql += ",";     // date
    sql += std::to_string(pl._u.reg.typ); sql += ",";
    sql += std::to_string(refs);          sql += ",";
    sql += std::to_string(keys[0]);         sql += ",";
    sql += std::to_string(keys[1]);         sql += ",";
    sql += std::to_string(keys[2]);         sql += ",";
    sql += std::to_string(keys[3]);         sql += ");";

    std::cout << sql << "\n";
    int res  = sqlite3_exec(_db, sql.c_str(), 0, 0, &szerr);
    if( res != SQLITE_OK )
    {
        std::cerr << __LINE__ << ": " << szerr << "\n";
        sqlite3_free(szerr);
        __sq.commit();

        sql = "UPDATE pers SET D=";
        sql += "datetime('now')";
        sql += " WHERE K=";
        sql += std::to_string(crc_hash) + ";";
        std::cout << sql << "\n";
        res  = sqlite3_exec(_db, sql.c_str(), 0, 0, &szerr);
        if( res != SQLITE_OK )
        {
            std::cerr << __LINE__ << ": " << szerr << "\n";
            sqlite3_free(szerr);
        }
        __sq.commit();
    }
    else
    {
        // notify this was registerred
        dirty=true;
    }
    return dirty;
}

void Sqlyte::get_pers(std::vector<ipp>& pers)
{
    sqlite3_stmt      *statement;
    Sqliter          __sq(&_db,__LINE__);

    std::string sql = "SELECT PA,PP,T,MK0,MK1,MK2,MK3 FROM pers;";
    if ( sqlite3_prepare(_db, sql.c_str(), -1, &statement, 0 ) == SQLITE_OK )
    {
        while ( 1 )
        {
            int res = sqlite3_step(statement);
            if ( res == SQLITE_DONE || res==SQLITE_ERROR)
            {
                break;
            }
            if ( res == SQLITE_ROW )
            {
                ipp per(sqlite3_column_int(statement, 0),
                        sqlite3_column_int(statement, 1),
                        sqlite3_column_int(statement, 2));
                per._keys[0] = sqlite3_column_int(statement, 3);
                per._keys[1] = sqlite3_column_int(statement, 4);
                per._keys[2] = sqlite3_column_int(statement, 5);
                per._keys[3] = sqlite3_column_int(statement, 6);

                pers.push_back(per);
            }
        }
        sqlite3_reset(statement);
    }
}


bool Sqlyte::authorize(SrvCap& pl,const SrvCap& plclear, uint32_t ip4, uint32_t keys[4])
{

    sqlite3_stmt    *statement;
    bool            ret = false;
    std::string     meiot;
    Sqliter          __sq(&_db,__LINE__);

    std::string sql = "SELECT mid FROM mid WHERE ip=";
    sql += std::to_string(ip4) + " OR ip='0' ;";

    if ( sqlite3_prepare(_db, sql.c_str(), -1, &statement, 0 ) == SQLITE_OK )
    {
        while ( 1 )
        {
            int res = sqlite3_step(statement);
            if ( res == SQLITE_DONE || res==SQLITE_ERROR)
            {
                break;
            }
            if ( res == SQLITE_ROW )
            {
                meiot = (const char*)sqlite3_column_text(statement, 0);
                _tokeys(meiot, keys);
                std::cout << keys[0]  << keys[1] << keys[2] <<keys[3] << "\n";
                ed((const uint8_t*)&pl, (uint8_t*)&plclear, sizeof(pl), keys, false);
                if(meiot == plclear._u.reg.meiot)
                {
                    pl = plclear;
                    ret = true;
                    break;
                }
            }
        }
        sqlite3_reset(statement);
    }
    return ret;
}

bool Sqlyte::deny(uint32_t ip4)
{

    sqlite3_stmt    *statement;
    char            *szerr = nullptr;
    Sqliter          __sq(&_db,__LINE__);
    bool            breject = false;

    std::string sql = "SELECT cnt FROM rej WHERE ip=";
    sql += std::to_string(ip4) + ";";
    if ( sqlite3_prepare(_db, sql.c_str(), -1, &statement, 0 ) == SQLITE_OK )
    {
        int res = sqlite3_step(statement);

        if ( res == SQLITE_DONE || res==SQLITE_ERROR)
        {
            sql = "INSERT INTO rej (D,cnt,ip) ";
            sql += "VALUES (";
            sql += "datetime('now')";
            sql += ",";
            sql += "1,'";
            sql += std::to_string(ip4);
            sql += "');";
            res  = sqlite3_exec(_db, sql.c_str(), 0, 0, &szerr);
            if( res != SQLITE_OK )
            {
                std::cerr << szerr << "\n";
                sqlite3_free(szerr);
            }
        }
        else if ( res == SQLITE_ROW )
        {
            int  attemps = sqlite3_column_int(statement, 0);
            if(attemps > MAX_ATTEMPS)
            {
                breject =true;
            }

            sql = "UPDATE rej SET cnt = cnt + 1, D=";
            sql += "datetime('now')";
            sql += " WHERE ip=";
            sql += std::to_string(ip4) + ";";
            res  = sqlite3_exec(_db, sql.c_str(), 0, 0, &szerr);
            if( res != SQLITE_OK )
            {
                std::cerr << szerr << "\n";
                sqlite3_free(szerr);
            }
        }
        sqlite3_finalize(statement);
    }
    return breject;
}


void Sqlyte::update_peer(uint32_t ip, uint32_t port)
{

    char            *szerr = nullptr;
    Sqliter          __sq(&_db,__LINE__);

    std::string sql = "UPDATE pers SET D=";
    sql += "datetime('now')";
    sql += " WHERE PA=";
    sql += std::to_string(ip);
    sql +=  + " AND PP=";
    sql += std::to_string(port);
    sql += ";";

    std::cout << sql << "\n";

    int res  = sqlite3_exec(_db, sql.c_str(), 0, 0, &szerr);
    if( res != SQLITE_OK )
    {
        std::cerr << szerr << "\n";
        sqlite3_free(szerr);
    }
}


