/*
	Cache Simulator (Starter Code) by Justin Goins
	Oregon State University
	Spring Term 2021
*/

#ifndef _CACHESTUFF_H_
#define _CACHESTUFF_H_

enum class ReplacementPolicy {
	Random,
	LRU
};

enum class WritePolicy {
	WriteThrough,
	WriteBack
};

// structure to hold information about a particular cache
struct CacheInfo {
	unsigned int numByteOffsetBits;
	unsigned int numSetIndexBits;
	unsigned int numberSets; // how many sets are in the cache
	unsigned int blockSize; // size of each block in bytes
	unsigned int associativity; // the level of associativity (N)
	ReplacementPolicy rp;
	WritePolicy wp;
	unsigned int cacheAccessCycles;
	unsigned int memoryAccessCycles;
};

// this structure can filled with information about each memory operation
struct CacheResponse {
	int hits = 0; // how many caches did this memory operation hit?
	int misses = 0; // how many caches did this memory operation miss?
	int evictions = 0; // did this memory operation involve one or more evictions?
	int dirtyEvictions = 0; // were any evicted blocks marked as dirty? (relevant for write-back cache)
	unsigned int cycles = 0; // how many clock cycles did this operation take?
};

#endif //CACHESTUFF
