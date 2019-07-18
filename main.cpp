#include <iostream>
#include <string>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "a_peer.h"
#include "u_server.h"

bool __alive = true;
int  __perport = CLI_PORT;
std::string __srvip=SRV_IP;
bool __provider=false;

int main(int argc, char *argv[])
{
    if(argc>=3)
    {
        if( ::strchr(argv[2],':'))
        {
            std::cout << "CLIENT  \n";
            _tokeys(__meikey,__key);
            std::cout << __key[0] << __key[1] <<__key[2] <<__key[3] << "\n";
            if(argc==4)
                __perport=::atoi(argv[3]);
            if(argc==5)
                __srvip=argv[4];
            __provider = strstr(argv[2],"p:");
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
            std::cout << "SERVER  \n";
            u_server u(argv[2]);
            u.main();
        }
    }


    return 0;
}



