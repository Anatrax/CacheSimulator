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

    void load(CacheResponse* response);

    void store(CacheResponse* response, struct CacheEntry& iter);

    void print();

private:
    CacheInfo cache_info;
    std::list<struct CacheEntry>** data;

    void read(CacheResponse* response);

    void write(CacheResponse* response, struct CacheEntry& iter, unsigned long int tag);
};

#endif // CACHE_H

