#include <iostream>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "u_server.h"
#include "a_peer.h"

bool __alive = true;

int main(int argc, char *argv[])
{
    char        gret[128];

    if(argc==3)
    {
        if(argv[1][0]=='s')
        {
            u_server u(argv[2]);
            u.main();
        }
        else
        {
            a_peer per(argv[2]);
            per.main();
        }
    }
    return 0;
}



