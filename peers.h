#ifndef PEERS_H
#define PEERS_H


#include <map>
#include <string>
#include "per_id.h"


class peers
{
public:
    peers();
    bool    add(std::string& n, struct sockaddr_in& p, struct sockaddr_in& q);

private:
    std::map<std::string, per_pair> _pairs;
};

#endif // PEERS_H
