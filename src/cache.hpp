#ifndef CACHE_H
#define CACHE_H

#include <list>
#include <string>
#include "CacheStuff.h"

struct CacheEntry {
    bool valid = false;
    bool dirty = false;
    unsigned long int tag = 0x0;
};

class Cache {
public:
    Cache(CacheInfo cache_info);

    ~Cache();
    
    void access(CacheResponse* response, bool isWrite, unsigned int setIndex, unsigned long int tag, int numBytes);

    void print();

private:
    CacheInfo cache_info;
    std::list<struct CacheEntry>** data;
    unsigned long int prefetchAddress = ~0x0;
};

#endif // CACHE_H

