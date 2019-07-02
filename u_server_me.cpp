#ifdef MEEIOT_SERVER
#include <unistd.h>
#include <iostream>
#include <assert.h>
#include "u_server.h"
#include "per_id.h"
#include "udp_xdea.h"

extern bool __alive;

#define MAX_ATTEMPS     512
#define DB_PATH         "/usr/share/perlink"
#define DB_FILE         "/usr/share/perlink/perlink.db"


static const char* _datetime()
{
    static char buffer[80];
    time_t      rawtime;
    struct tm * timeinfo=::localtime(&rawtime);
    if(timeinfo != nullptr)
    {
        ::strftime(buffer, sizeof(buffer)-1, "%Y-%m-%d %H:%M:%S",timeinfo);
    }
    return buffer;
}

static void _tokeys(const std::string&  meiot, uint32_t keys[4])
{
    ::sscanf(meiot.c_str(),"%8X%8X%8X%8X", &keys[0], &keys[1], &keys[2], &keys[3]);
}

u_server::u_server(const char* key)
{
    if(::access(DB_PATH,0)!=0)
    {
        std::cerr << DB_PATH << " not created \n";
        exit (1);
    }
    int err = sqlite3_open(DB_FILE, &_db);
    if(err!=SQLITE_OK)
    {
        std::cerr << DB_PATH <<  sqlite3_errmsg(_db) << "\n";
        exit (1);
    }
    char *szerr = nullptr;
    sqlite3_busy_timeout(_db, 1024);
    std::string sql =  "CREATE TABLE IF NOT EXISTS 'mid' (Id integer primary key autoincrement,"
                       "dt DATETIME NOT NULL,"
                       "mid TEXT NOT NULL,"
                       "ip Integer);";
    int rc = sqlite3_exec(_db, sql.c_str(), 0, 0, &szerr);
    if( rc != SQLITE_OK )
    {
        std::cerr << " SQL error: " << szerr;
        exit (1);
    }
    sql =  "CREATE TABLE IF NOT EXISTS 'rej' (Id integer primary key autoincrement,"
           "dt DATETIME NOT NULL,"
           "cnt Integer,"
           "ip Integer);";
    rc = sqlite3_exec(_db, sql.c_str(), 0, 0, &szerr);
    if( rc != SQLITE_OK )
    {
        std::cerr << " SQL error: " << szerr;
        exit (1);
    }
    // _test();
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

    if(s.create(SRV_PORT))
    {
        s.bind();
        while(__alive)
        {
            SADDR_46 rem;

            s.set_keys(nullptr);
            int by = s.rreceive((char*)&pl,sizeof(pl));
            if(by == sizeof(pl))
            {
                if(!_auth(s, pl))
                {
                    _deny(s);
                }
                else
                {
                    _process(s, pl);
                }
            }
            if(++counter % 100==0)
            {
                _del_oldies();
            }
        }
    }
}

void u_server::_linkthem(udp_xdea& s, const char* id)
{
    per_pair*  pp = _ps.find(id);
    if(pp)
    {
        SrvCap  pl;

        pl._verb = SRV_PEERING;
        pl._u.pp._link = true;
        pl._u.pp._private = pp->_b[0];
        pl._u.pp._public  = pp->_b[1];

        // A <- b/B
        std::cout << "a<-" << Ip2str(pp->_a[0]._a) <<":"<< pp->_a[0]._p <<"\n";
        std::cout << "a<-" << Ip2str(pp->_a[1]._a) <<":"<< pp->_a[1]._p <<"\n";

        s.send((const uint8_t*)&pl,sizeof(pl), pp->_a[0]);
        s.send((const uint8_t*)&pl,sizeof(pl), pp->_a[1]);

        pl._u.pp._private = pp->_a[0];
        pl._u.pp._public  = pp->_a[1];

        // A <- b/B
        std::cout << "b<-" << Ip2str(pp->_b[0]._a) <<":"<< pp->_b[0]._p<<"\n";
        std::cout << "b<-" << Ip2str(pp->_b[1]._a) <<":"<< pp->_b[1]._p<<"\n";

        s.send((const uint8_t*)&pl,sizeof(pl), pp->_b[0]);
        s.send((const uint8_t*)&pl,sizeof(pl), pp->_b[1]);
        std::cout << "\r\n";
        ::sleep(4);
    }
}

void u_server::_del_oldies()
{
    time_t now = time(0);
    _ps.del_oldies(now);
}

bool u_server::_auth(udp_xdea& s, SrvCap& pl)
{
    SrvCap  plclear;

    for(const auto& q :_auths)
    {
        if(q.ip == s.Rsin().ip4())
        {
            ed((const uint8_t*)&pl, (uint8_t*)&plclear, sizeof(pl), q.key, false);
            ::memcpy(&pl, &plclear, sizeof(pl));
            s.set_keys(q.key);
            return true;
        }
    }
    int             res = sqlite3_open(DB_FILE, &_db);
    sqlite3_stmt    *statement;
    bool            ret = false;
    std::string     meiot;
    uint32_t        keys[4];

    if(res)
    {
        std::cerr <<" OPEN " << sqlite3_errmsg(_db);
        return ret;
    }
    std::string sql = "SELECT mid FROM mid WHERE ip=";
    sql += std::to_string(s.Rsin().ip4()) + " OR ip='0' ;";
    if ( sqlite3_prepare(_db, sql.c_str(), -1, &statement, 0 ) == SQLITE_OK )
    {
        while ( 1 )
        {
            res = sqlite3_step(statement);
            if ( res == SQLITE_DONE || res==SQLITE_ERROR)
            {
                break;
            }
            if ( res == SQLITE_ROW )
            {
                meiot = (const char*)sqlite3_column_text(statement, 0);
                _tokeys(meiot, keys);
                std::cout << keys[0] << keys[1] << keys[2] <<keys[3] << "\n";
                ed((const uint8_t*)&pl, (uint8_t*)&plclear, sizeof(pl), keys, false);
                std::cout << "checking " << Ip2str(s.Rsin().ip4()) << ", " << plclear._u.reg.meiot << "\n";
                if(meiot == plclear._u.reg.meiot)
                {
                    QAuth qa;
                    qa.ip = s.Rsin().ip4();
                    memcpy(qa.key, keys,sizeof(keys));
                    _auths.push_back(qa);
                    if(_auths.size()>FAST_AUTHS)
                    {
                        _auths.pop_front();
                    }
                    ::memcpy(&pl, &plclear, sizeof(pl));
                    s.set_keys(keys);
                    ret = true;
                    break;
                }
            }
        }
        sqlite3_reset(statement);
    }
    sqlite3_close(_db);
    _db=0;
    return ret;
}

void u_server::_deny(udp_xdea& s)
{
    uint32_t        ip = s.Rsin().ip4();
    sqlite3_stmt    *statement;
    std::string     buffer =_datetime();
    char            *szerr = nullptr;
    uint8_t         reject[1]  = {SRV_REJECT};

    s.::udp_sock::send((const uint8_t*)reject,1, s.Rsin());

    int res = sqlite3_open(DB_FILE, &_db);
    if(res)
    {
        std::cerr <<" OPEN " << sqlite3_errmsg(_db);
        return ;
    }

    std::string sql = "SELECT cnt FROM rej WHERE ip=";
    sql += std::to_string(ip) + ";";
    if ( sqlite3_prepare(_db, sql.c_str(), -1, &statement, 0 ) == SQLITE_OK )
    {
        res = sqlite3_step(statement);

        if ( res == SQLITE_DONE || res==SQLITE_ERROR)
        {
            sql = "INSERT INTO rej (dt,cnt,ip) ";
            sql += "VALUES (";
            sql += "'";
            sql += buffer;
            sql += "',";
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

            sql = "UPDATE rej SET cnt = cnt + 1, dt='";
            sql += buffer;
            sql += "' WHERE ip=";
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
    sqlite3_close(_db);
    _db=0;
}

void u_server::_process(udp_xdea& s, SrvCap& pl)
{
    std::cout <<__FUNCTION__ << ": verb: " << int(pl._verb) << "\n";

    if(pl._verb == SRV_REGISTER || pl._verb == SRV_UNREGISTER)
    {
        assert(__meikey == pl._u.reg.meiot);

        ipp priv(pl._u.reg.ip, pl._u.reg.port);
        ipp pub(s.Rsin());

        std::cout<< "s<-c p:" << Ip2str(priv._a) << ":"<<htons(priv._p) << "\n";
        std::cout<< "s<-c P:" << Ip2str(pub._a)  << ":"<<htons(pub._p) << "\n";

        if(pl._verb==SRV_REGISTER)
        {
            if(_ps.add(pl._u.reg.id, pub, priv))
            {
                _linkthem(s, pl._u.reg.id);
            }
            else
            {
                pl._verb = SRV_REGISTERRED;
                s.send((const uint8_t*)&pl, sizeof(pl), s.Rsin());
            }
        }
        else
        {
            std::cout << " dropping " << (pl._u.reg.id) << "\n";
            _ps.remove(pl._u.reg.id);
            pl._verb = SRV_UNREGISTERED;
            s.send((const uint8_t*)&pl, sizeof(pl), s.Rsin());
            std::deque<QAuth>::iterator q =_auths.begin();
            for(; q != _auths.end(); ++q)
            {
                if(q->ip == s.Rsin().ip4())
                {
                    _auths.erase(q);
                    break;
                }
            }
        }
        usleep(1000);
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
    std::string     buffer =_datetime();
    char            *szerr = nullptr;

    int res = sqlite3_open(DB_FILE, &_db);
    if(res)
    {
        std::cerr <<" OPEN " << sqlite3_errmsg(_db);
        return ;
    }

    std::string sql = "INSERT INTO mid (dt,mid,ip) ";
    sql += "VALUES (";
    sql += "'";
    sql += buffer;
    sql += "','51de10940c93f85b02261266cd362885825a614183246bfcfef04a','0');";
    res  = sqlite3_exec(_db, sql.c_str(), 0, 0, &szerr);
    if( res != SQLITE_OK )
    {
        std::cerr << szerr << "\n";
        sqlite3_free(szerr);
    }

    sql = "INSERT INTO mid (dt,mid,ip) ";
    sql += "VALUES (";
    sql += "'";
    sql += buffer;
    sql += "','5edc15b7888ca1be1048ca995d7f8e33313f17049e30dcb9e2b47a','0');";
    res  = sqlite3_exec(_db, sql.c_str(), 0, 0, &szerr);
    if( res != SQLITE_OK )
    {
        std::cerr << szerr << "\n";
        sqlite3_free(szerr);
    }
    sqlite3_close(_db);
    _db=0;
}

#endif
