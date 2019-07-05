#include "pipefiles.h"

pipefiles::pipefiles()
{

}
#if 0

if(_fdout)
{
    tv.tv_sec = 0;
    tv.tv_usec = 4000;
    nfds = (int)_fdout+1;
    FD_ZERO(&_rdout);
    FD_SET(_fdout, &_rdout);
    int sel = ::select(nfds, &_rdout, 0, 0, &tv);

    if(sel < 0)
    {
        perror("select");
        exit(7);
    }
    if(sel > 0 && FD_ISSET(_fdout, &_rdout))
    {
        bytes = ::read(_fdout, buff, sizeof(buff));
        if(bytes>0)
        {
            std::cout << "sending  from" << _infile << ":" << bytes << "\n";
            FD_CLR(_fdout, &_rdout);
            s.rsend(buff,bytes,_per);
            _pingtime = now;
        }
    }
}

void  a_peer::_pipe_it()
{
    char pipefile[512];

    ::sprintf(pipefile,"/tmp/_p_%s%d_%d.in",_id.c_str(), _per._a, (int)_per._p);

    if(::access(pipefile,0)==0)
    {
        ::unlink(pipefile);
    }
    else
    {
        int fi = ::mkfifo(pipefile, O_RDWR|O_NONBLOCK| S_IRWXU|S_IRWXG|S_IRWXG  );
        if(fi<0)
        {
            perror("mkfifo");
            exit(3);
        }
    }

    int fdin = ::open (pipefile, O_RDWR|O_CREAT);
    if(fdin<0)
    {
        std::cerr << pipefile << ": PIPE: " << strerror(errno);
    }
    else
    {
        ::fcntl(fdin, F_SETFL, O_NONBLOCK);
        ::fcntl(fdin, F_SETPIPE_SZ, 1024 * 8912);
    }
    ::close(fdin);
    _infile = pipefile;

    ::sprintf(pipefile,"/tmp/_p_%s%d_%d.out",_id.c_str(), _per._a, _per._p);

    if(::access(pipefile,0)==0)
    {
        ::unlink(pipefile);
    }
    else
    {
        int fi = ::mkfifo(pipefile, O_RDWR|O_NONBLOCK| S_IRWXU|S_IRWXG|S_IRWXG  );
        if(fi<0)
        {
            perror("mkfifo");
            exit(3);
        }
    }
    if(_fdout)
        ::close(_fdout);
    _fdout = 0;

    _fdout = ::open (pipefile, O_RDWR|O_CREAT);
    if(_fdout<0)
    {
        std::cerr << pipefile << ": PIPE: " << strerror(errno);
    }
    else
    {
        ::fcntl(_fdout, F_SETFL, O_NONBLOCK);
        ::fcntl(_fdout, F_SETPIPE_SZ, 1024 * 8912);
    }
    _outfile = pipefile;

}


int pi = ::open(_infile.c_str(), O_WRONLY|O_APPEND);
if(pi)
{
    std::cout << "saving to" << _infile << "\n";
    ::write(pi, buff, bytes);
    ::close(pi);
}


#endif
