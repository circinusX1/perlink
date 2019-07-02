#ifndef PEERS_H
#define PEERS_H


#include <map>
#include <string>
#include "per_id.h"


class peers
{
public:
    peers();
    bool  add(const std::string& n,  const ipp& p, const ipp& q);
    per_pair* find(const std::string& n);
    void remove(const std::string& n);
    void del_oldies(time_t now);
private:
    std::map<std::string, per_pair> _pairs;
};

#endif // PEERS_H
