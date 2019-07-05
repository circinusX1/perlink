#include <iostream>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "a_peer.h"
#include "u_server.h"

bool __alive = true;
int  __perport = CLI_PORT;

static void _tokeys(const std::string&  meiot, uint32_t keys[4])
{
    ::sscanf(meiot.c_str(),"%8X%8X%8X%8X", &keys[0], &keys[1], &keys[2], &keys[3]);
}

int main(int argc, char *argv[])
{
    char        gret[128];

    if(argc>=3)
    {
        if( ::strchr(argv[2],':'))
        {
            _tokeys(__meikey,__key);
            std::cout << __key[0] << __key[1] <<__key[2] <<__key[3] << "\n";
            if(argc==4)
                __perport=::atoi(argv[3]);
            a_peer per(argv[2]);
            per.main();
        }
    }
    if(argc==2)
    {
        if(::access(DB_PATH,0)!=0)
        {
            std::cerr << DB_PATH << " not created \n";
            exit (1);
        }
        system("rm /usr/share/perlink/perlink.db");

        if(argv[1][0]=='s')
        {
            u_server u(argv[2]);
            u.main();
        }
    }


    return 0;
}



