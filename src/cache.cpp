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
    // Index into cache set
    std::list<struct CacheEntry>* set = this->data[setIndex];

    // Loop through and check blocks
    bool foundTag = false;
    bool allFull = true;
    struct CacheEntry& nextAvailableBlock = set->front(); // Init reference with non-null value
    for(auto & ablock : *set) {
        if(ablock.valid) {
            if(ablock.tag == tag) {
                response->hits++;
                std::cout << "Hit!" << std::endl;
                foundTag = true;
                nextAvailableBlock = ablock;
            }
        } else allFull = false;
    }

    // If no available blocks, must get a block to evict
    if(!foundTag && allFull) {
        // Rotate list randomly if using random replacement
        if(this->cache_info.rp == ReplacementPolicy::Random) {
            int index = rand() % this->cache_info.associativity;
            for(int count = 0; count < index; count++) {
                struct CacheEntry block = set->front();
                set->pop_front();
                set->push_back(block);
            }
        }

        // Keep rotating until one or zero avaiable blocks found
        for(unsigned int i = 0; (i < this->cache_info.associativity) && (set->front().valid); i++) {
            struct CacheEntry block = set->front();
            set->pop_front();
            set->push_back(block);
        }

        // Get the next available block (LRU unless randomized)
        // struct CacheEntry& nextAvailableBlock = set->front();
        nextAvailableBlock = set->front();

        // Save updates now if using write-back
        if(nextAvailableBlock.dirty && this->cache_info.wp == WritePolicy::WriteBack) {
            // Block to evict has been modified, write-back data to main memory
            response->cycles+= this->cache_info.memoryAccessCycles;
            nextAvailableBlock.dirty = false;
            response->dirtyEvictions++;
        }

        // Evict the block
        std::cout << "Evicting block!" << std::endl;
        nextAvailableBlock.valid = false;
        nextAvailableBlock.dirty = false;
        response->evictions++;
    }

    // Load block if missing from cache
    if(!foundTag) this->load(response);

    if(isWrite) {
        // Modify the data in the cache
        this->write(response, nextAvailableBlock, tag);

        // Write modified data to main memory if using write-through
        if(nextAvailableBlock.dirty && this->cache_info.wp == WritePolicy::WriteBack)
            this->store(response, nextAvailableBlock);
    }
    else this->read(response); // Read data from cache

    // Move block to end of LRU/Random list
    struct CacheEntry block = set->front();
    set->pop_front();
    set->push_back(block);

//////////////////////////
    // response->cycles+= this->cache_info.cacheAccessCycles;

    // for(auto & ablock : *set) {
    //     if(ablock.valid && (ablock.tag == tag)) {
    //         response->hits++;
    //         if(isWrite) {
    //             ablock.dirty = true;
    //             if(this->cache_info.wp == WritePolicy::WriteThrough) {
    //                 // Write modified data to main memory
    //                 response->cycles+= this->cache_info.memoryAccessCycles;
    //                 ablock.dirty = false;
    //             }
    //         }
    //         return;
    //     }
    // }
    // response->misses++;

    // // Check for evictions
    // if(nextAvailableBlock.valid) {
    //     // response->cycles+= this->cache_info.cacheAccessCycles;
    //     // No available blocks, must evict
    //     if(nextAvailableBlock.dirty && this->cache_info.wp == WritePolicy::WriteBack) {
    //         // Block to evict has been modified, write-back data to main memory
    //         // response->cycles+= this->cache_info.memoryAccessCycles;
    //         nextAvailableBlock.dirty = false;
    //         response->dirtyEvictions++;
    //     }
    //     response->evictions++;
    // }

    // // Load/Store byte from/into cache block
    // // response->cycles+= this->cache_info.memoryAccessCycles;
    // nextAvailableBlock.valid = true;
    // if(isWrite) {
    //     nextAvailableBlock.dirty = true;
    //     if(this->cache_info.wp == WritePolicy::WriteThrough) {
    //         response->cycles+= this->cache_info.memoryAccessCycles;
    //         nextAvailableBlock.dirty = false;
    //     }
    // }
    // nextAvailableBlock.tag = tag;
}

void Cache::load(CacheResponse* response) {
    response->misses++;
    std::cout << "Fetching block!" << std::endl;
    response->cycles+= this->cache_info.memoryAccessCycles;
}

void Cache::store(CacheResponse* response, struct CacheEntry& iter) {
    if(this->cache_info.wp == WritePolicy::WriteThrough) {
        response->cycles+= this->cache_info.memoryAccessCycles;
        iter.dirty = false;
    }
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

void Cache::read(CacheResponse* response) {
    response->cycles+= this->cache_info.cacheAccessCycles;
}

void Cache::write(CacheResponse* response, struct CacheEntry& iter, unsigned long int tag) {
    response->cycles+= this->cache_info.cacheAccessCycles;
    iter.valid = true;
    iter.dirty = true;
    iter.tag = tag;
}

