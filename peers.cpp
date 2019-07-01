#include "peers.h"

peers::peers()
{

}

bool    peers::add(std::string& n, struct sockaddr_in& p, struct sockaddr_in& q)
{
    std::map<std::string, per_pair>::iterator a = _pairs.find(n);
    if(a != _pairs.end())
    {
        per_pair& rpp = a->second;
        if(rpp._a[0].sin_port==0)
        {
            rpp._a[0]=p;
            rpp._a[1]=q;
        }
        else if(rpp._b[0].sin_port==0)
        {
            rpp._b[0]=p;
            rpp._b[1]=q;
        }
            return true;
    }
    per_pair pa;
    pa._a[0]    = p;
    pa._a[1]    = q;
    _pairs[n]   = pa;
    return false;
}
