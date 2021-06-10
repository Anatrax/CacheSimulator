#include <iostream>
#include <ctime>
#include <random>
#include "cache.hpp"

Cache::Cache(CacheInfo cache_info) : cache_info{cache_info} {
    srand(time(0));
    this->data = new std::list<struct CacheEntry>*[cache_info.numberSets]();
    for(unsigned int i = 0; i < cache_info.numberSets; i++) {
        this->data[i] = new std::list<struct CacheEntry>();
        for(unsigned int j = 0; j < cache_info.associativity; j++) {
            this->data[i]->emplace_front();
        }
    }
};

Cache::~Cache() {
    delete[] this->data;
}

void Cache::access(CacheResponse* response, bool isWrite, unsigned int setIndex, unsigned long int tag, int numBytes) {
    std::list<struct CacheEntry>* set = this->data[setIndex];
    for(auto & ablock : *set) {
        if(ablock.valid && (ablock.tag == tag)) {
            response->hits++;
//            if(isWrite) ablock.dirty = true;
            return;
        }
    }
    response->misses++;
    // Get next available cache block
    struct CacheEntry& block = set->front();
    if(this->cache_info.rp == ReplacementPolicy::Random) {
        // Rotate the list randomly
        int count = 0;
        int index = rand() % this->cache_info.associativity;
        while(count < index) {
            block = set->front();
            set->pop_front();
            set->push_back(block);
            count++;
        }
    }
    block = set->front();

    // Load byte into cache block
    block.valid = true;
    if(isWrite) block.dirty = true;
    block.tag = tag;

//    //check if
//    //if dirty,
//    //run cache_eviction
//    //place data into evicted block
////    ablock.valid = true;
////    if(isWrite) ablock.dirty = true;
////    ablock.tag = tag;
//    *this->data[setIndex].push_back(nextAvailableBlock); // Place back into list
}

void Cache::print() {
//    std::cout << "L1" << std::endl
//        << "----" << std::endl 
//        << "Number of Offset Bits: " << this->cache_info.numByteOffsetBits << std::endl
//        << "Number of Index Bits: " << this->cache_info.numSetIndexBits << std::endl
//        << "Number of Sets: " << this->cache_info.numberSets << std::endl
//        << "Bytes per Block: " << this->cache_info.blockSize << std::endl
//        << "Associativity: " << this->cache_info.associativity << std::endl
//        << "Replacement Policy: " << ((this->cache_info.rp == ReplacementPolicy::Random) ? "Random":"Least Recently Used (LRU)") << std::endl
//        << "Write Policy: " << ((this->cache_info.wp == WritePolicy::WriteBack) ? "Write Back":"Write Through") << std::endl
//        << "Cycles per Cache Access: " << this->cache_info.cacheAccessCycles << std::endl
//        << "Cycles per Memory Access: " << this->cache_info.memoryAccessCycles << std::endl
//        << "----" << std::endl 
//        << std::endl;
    for(unsigned int i = 0; i < this->cache_info.numberSets; i++) {
        for(auto ablock : *this->data[i]) {
            std::cout << '[' << ablock.valid << ablock.dirty << '|' << ablock.tag << ']';
        }
        std::cout << std::endl;
    }
    std::cout << "----" << std::endl;
}

