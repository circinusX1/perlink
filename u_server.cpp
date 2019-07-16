#include <unistd.h>
#include <iostream>
#include <unordered_set>
#include <assert.h>
#include <vector>
#include "u_server.h"
#include "per_id.h"
#include "udp_xdea.h"
#include "sqliter.h"

extern bool __alive;


#define provider true
#define consumer false


static void _tokeys(const std::string&  meiot, uint32_t keys[4])
{
    ::sscanf(meiot.c_str(),"%8X%8X%8X%8X", &keys[0], &keys[1], &keys[2], &keys[3]);
}

u_server::u_server(const char* key)
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

u_server::~u_server()
{
    if (_t->joinable())
    {
        _t->join();
    }
    delete _t;
}

bool u_server::run()
{
    _t = new std::thread (&u_server::main, this);
    return _t != nullptr;
}

void u_server::main()
{
    int             counter;
    udp_xdea        s;
    std::string     meiotkey;
    SrvCap          pl;
    time_t          now = 0;
    time_t          pingpers = 0;
    fd_set          rdSet;
    int             nfds;// = (int)s.socket()+1;
    timeval         tv;//   = {0,1024};
    uint32_t        keys[4];

    if(s.create(SRV_PORT))
    {
        s.bind();
        while(__alive)
        {
            SADDR_46 rem;

            now = time(0);

            FD_ZERO(&rdSet);
            FD_SET(s.socket(), &rdSet);
            tv.tv_sec=0;
            tv.tv_usec=100000;
            nfds = s.socket()+1;
            int sel = ::select(nfds, &rdSet, 0, 0, &tv);
            if(sel < 0)
            {
                std::cout << "select fatal \r\n";
                exit(errno);
            }
            if(sel > 0 && FD_ISSET(s.socket(), &rdSet))
            {
                int by = s.udp_sock::receive((char*)&pl,sizeof(pl));
                if(by == sizeof(pl))
                {
                    if(!_auth(s, pl,keys))
                    {
                        _deny(s);
                    }
                    else
                    {
                        _process(s, pl,keys);
                    }
                }
            }
            if(now - pingpers > TTLIVE || _dirty)
            {
                pingpers=now;
                _ping_pers(s, pl);
                memset(&_prev,0,sizeof(_prev));
                _dirty = false;
            }
        }
    }
}

void u_server::_delete_olies()
{
    char    *szerr;
    Sqliter __sq(&_db,__LINE__);
    std::string sql = "DELETE FROM pers WHERE D <= date('now','-30 seconds')";
    int res  = sqlite3_exec(_db, sql.c_str(), 0, 0, &szerr);
    if( res != SQLITE_OK )
    {
        std::cerr << __LINE__ << ": " << szerr << "\n";
        sqlite3_free(szerr);
    }
    else
    {
        _dirty=true;
    }
}

void u_server::_remove_peer(udp_xdea& s, SrvCap& pl, ipp& pub)
{
    char              shash[256];
    ::sprintf(shash,"%s", pl._u.pp._public.str().c_str());
    std::size_t crc_hash = std::hash<std::string>{}(shash);
    sqlite3_stmt      *statement;
    char              *szerr;
    Sqliter __sq(&_db,__LINE__);

    std::string sql = "DELETE FROM pers WHERE K='";
    sql += crc_hash; sql += "';";

    int res  = sqlite3_exec(_db, sql.c_str(), 0, 0, &szerr);
    if( res != SQLITE_OK )
    {
        std::cerr << __LINE__ << ": " << szerr << "\n";
        sqlite3_free(szerr);
    }
    else
    {
        _dirty=true;
    }
}

void u_server::_store_peer(udp_xdea& s, SrvCap& pl, ipp& pub, const uint32_t* keys)
{
    char              shash[256];
    char              shash2[256];
    ::sprintf(shash2,"%d%d%s%d%d%%d%d%", pub._a,  pub._p, pl._u.reg.id,keys[0],keys[1],keys[2],keys[3]);
    std::size_t crc_hash = std::hash<std::string>{}(shash2);
    sqlite3_stmt      *statement;
    char              *szerr;
    int               refs = 0;
    bool              newreck;
    Sqliter          __sq(&_db,__LINE__);

    if(_prev==pl)
        return;
    _prev = pl;

    int err = sqlite3_open(DB_FILE, &_db);
    if(err!=SQLITE_OK)
    {
        std::cerr << DB_PATH <<  sqlite3_errmsg(_db) << "\n";
        exit (1);
    }
    std::string sql = "SELECT R,PA,PP,T FROM pers WHERE I='";
    sql += (const char*)(pl._u.reg.id+2); sql += "';";

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
                char        sper[128];
                tper        p;

                refs          = sqlite3_column_int(statement, 0);
                uint32_t puba = sqlite3_column_int(statement, 1);
                p._pub        = ipp(sqlite3_column_int(statement, 1),
                                     sqlite3_column_int(statement, 2),
                                     sqlite3_column_int(statement, 3));
            }
        }
        sqlite3_reset(statement);
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
    sql += "datetime('now')";            sql += ",'";           // date
    sql += (pl._u.reg.id+2);             sql += "',";       // ID/NAME
    sql += std::to_string(crc_hash);     sql += ","; // KRC
    sql += std::to_string(pub._a);       sql += ",";     // date
    sql += std::to_string(pub._p);       sql += ",";     // date
    sql += std::to_string(pl._u.reg.typ);sql += ",";
    sql += std::to_string(refs);         sql += ",";
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
        pl._verb = SRV_REGISTERRED;
        pl.clear();
        s.send((const uint8_t*)&pl,sizeof(pl), s.Rsin(),keys);
        _dirty=true;
    }
}

void u_server::_ping_pers(udp_xdea& s, SrvCap& pl)
{
    sqlite3_stmt      *statement;
    char              *szerr;
    char              sper[128];
    Sqliter          __sq(&_db,__LINE__);
    std::vector<ipp>  pers;

    std::string sql = "SELECT PA,PP,T,MK0,MK1,MK2,MK3 FROM pers;";
    if ( sqlite3_prepare(_db, sql.c_str(), -1, &statement, 0 ) == SQLITE_OK )
    {
        pl._verb = SRV_PING;
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


    for(const auto& p : pers)
    {
        pl._verb = SRV_PING;
        s.send((const uint8_t*)&pl, sizeof(pl), p);

        for(const auto& q : pers)
        {
            pl._verb = SRV_SET_PEER;
            if(p._t != q._t)
            {
                pl._u.pp._public=p;
                s.send((const uint8_t*)&pl, sizeof(pl), q);

                pl._u.pp._public=q;
                s.send((const uint8_t*)&pl, sizeof(pl), p);
            }
        }
    }
}

bool u_server::_auth(udp_xdea& s, SrvCap& pl, uint32_t keys[4])
{
    SrvCap          plclear;
    sqlite3_stmt    *statement;
    bool            ret = false;
    std::string     meiot;
    Sqliter          __sq(&_db,__LINE__);

    std::string sql = "SELECT mid FROM mid WHERE ip=";
    sql += std::to_string(s.Rsin().ip4()) + " OR ip='0' ;";
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
                std::cout << "checking " << Ip2str(s.Rsin()) << ", " << plclear._u.reg.meiot << "\n";
                if(meiot == plclear._u.reg.meiot)
                {
                    ::memcpy(&pl, &plclear, sizeof(pl));
                    ret = true;
                    break;
                }
            }
        }
        sqlite3_reset(statement);
    }
    return ret;
}

void u_server::_deny(udp_xdea& s)
{
    uint32_t        ip = s.Rsin().ip4();
    sqlite3_stmt    *statement;
    char            *szerr = nullptr;
    uint8_t         reject[1]  = {SRV_REJECT};
    Sqliter          __sq(&_db,__LINE__);


    std::string sql = "SELECT cnt FROM rej WHERE ip=";
    sql += std::to_string(ip) + ";";
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
            sql += std::to_string(ip);
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
                _ufw_reject(ip);
            }

            sql = "UPDATE rej SET cnt = cnt + 1, D=";
            sql += "datetime('now')";
            sql += " WHERE ip=";
            sql += std::to_string(ip) + ";";
            res  = sqlite3_exec(_db, sql.c_str(), 0, 0, &szerr);
            if( res != SQLITE_OK )
            {
                std::cerr << szerr << "\n";
                sqlite3_free(szerr);
            }
        }
        sqlite3_finalize(statement);
    }
}

void u_server::_process(udp_xdea& s, SrvCap& pl, const uint32_t *keys)
{
    std::cout <<__FUNCTION__ << ": verb: " << int(pl._verb) << "\n";

    if(pl._verb == PER_AIH)
    {
        _update_db(s,pl);
    }
    else if(pl._verb == SRV_REGISTER)
    {
        ipp pub(s.Rsin());

        std::cout<< "REGISTERING  (SEEN)"  << IP2STR(s.Rsin())  << "\n";
        if(pl._verb==SRV_REGISTER)
        {
            _store_peer(s, pl, pub, keys);
        }
    }
    else if(pl._verb == SRV_UNREGISTER)
    {
        ipp pub(s.Rsin());

        std::cout << " dropping " << (pl._u.reg.id) << "\n";
        pl._verb = SRV_UNREGISTERED;
        s.send((const uint8_t*)&pl, sizeof(pl), s.Rsin());
        _remove_peer(s, pl, pub);
    }
    else
    {
        std::cout << Ip2str(s.Rsin().ip4()) << int(s.Rsin().port()) << " INVALID SRV verb \n";
    }
}

void u_server::_ufw_reject(uint32_t ip)
{
    /*
    std::string s = "sudo ufw insert 1 deny from "; // add no pass for ufw
    s += Ip2str(ip);
    system(s.c_str());
    */
}

void u_server::_test()
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

/**
 * @brief u_server::_update_db
 * @param s
 * @param pl
 */
void u_server::_update_db(udp_xdea& s, SrvCap& pl)
{
    char            *szerr = nullptr;
    Sqliter          __sq(&_db,__LINE__);

    std::string sql = "UPDATE pers SET D=";
    sql += "datetime('now')";
    sql += " WHERE PA=";
    sql += std::to_string(s.Rsin().ip4());
    sql +=  + " AND PP=";
    sql += std::to_string(s.Rsin().port());
    sql += ";";

    std::cout << sql << "\n";

    int res  = sqlite3_exec(_db, sql.c_str(), 0, 0, &szerr);
    if( res != SQLITE_OK )
    {
        std::cerr << szerr << "\n";
        sqlite3_free(szerr);
    }
}


