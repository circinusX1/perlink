
#include <iostream>
#include "peers.h"

peers::peers()
{

}

bool    peers::add(const std::string& n,  const ipp& p, const ipp& q)
{
    std::map<std::string, per_pair>::iterator a = _pairs.find(n);
    if(a != _pairs.end())
    {
        per_pair& rpp = a->second;

        if(rpp._a[0]._p==0)
        {
            rpp._a[0] = p;
            rpp._a[1] = q;
        }
        else if(rpp._b[0]._p==0)
        {
            if(rpp._a[0]==p || rpp._a[0]==q)
                return false; // same one
            if(rpp._a[1]==p || rpp._a[1]==q)
                return false; // same one

            std::cout << Ip2str(p._a) << ":" << p._p << " assoc \n";
            std::cout << Ip2str(q._a) << ":" << q._p << " assoc \n";

            rpp._b[0]= p;
            rpp._b[1]= q;
        }
        return true;
    }

    per_pair pa;
    pa._a[0]    = p;
    pa._a[1]    = q;
    _pairs[n]   = pa;
    std::cout << Ip2str(p._a) << ":" << p._p << " added \n";
    std::cout << Ip2str(q._a) << ":" << q._p << " added \n";

    return false;
}

const per_pair* peers::find(const std::string& n)
{
    const auto& r = _pairs.find(n);
    if(r != _pairs.end())
        return &r->second;
    return nullptr;
}
