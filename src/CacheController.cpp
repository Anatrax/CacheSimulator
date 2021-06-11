/*
	Cache Simulator (Starter Code) by Justin Goins
	Oregon State University
	Spring Term 2021
*/

#include "CacheController.h"
#include <iostream>
#include <fstream>
#include <regex>
#include <cmath>

using namespace std;

CacheController::CacheController(CacheInfo ci, string tracefile) {
	// store the configuration info
	this->ci = ci;
	this->inputFile = tracefile;
	this->outputFile = this->inputFile + ".out";
	// compute the other cache parameters
	this->ci.numByteOffsetBits = log2(ci.blockSize);
	this->ci.numSetIndexBits = log2(ci.numberSets);
	// initialize the counters
	this->globalCycles = 0;
	this->globalHits = 0;
	this->globalMisses = 0;
	this->globalEvictions = 0;
	
	// create your cache structure
    this->cache = new Cache(ci);

	// manual test code to see if the cache is behaving properly
	// will need to be changed slightly to match the function prototype
	/*
	cacheAccess(false, 0);
	cacheAccess(false, 128);
	cacheAccess(false, 256);

	cacheAccess(false, 0);
	cacheAccess(false, 128);
	cacheAccess(false, 256);
	*/
}

CacheController::~CacheController() {
    delete this->cache;
}

/*
	Starts reading the tracefile and processing memory operations.
*/
void CacheController::runTracefile() {
	cout << "Input tracefile: " << inputFile << endl;
	cout << "Output file name: " << outputFile << endl;
	
	// process each input line
	string line;
	// define regular expressions that are used to locate commands
	regex commentPattern("==.*");
	regex instructionPattern("I .*");
	regex loadPattern(" (L )(.*)(,)([[:digit:]]+)$");
	regex storePattern(" (S )(.*)(,)([[:digit:]]+)$");
	regex modifyPattern(" (M )(.*)(,)([[:digit:]]+)$");

	// open the output file
	ofstream outfile(outputFile);
	// open the output file
	ifstream infile(inputFile);
	// parse each line of the file and look for commands
	while (getline(infile, line)) {
		// these strings will be used in the file output
		string opString, activityString;
		smatch match; // will eventually hold the hexadecimal address string
		unsigned long int address;
		// create a struct to track cache responses
		CacheResponse response;

		// ignore comments
		if (std::regex_match(line, commentPattern) || std::regex_match(line, instructionPattern)) {
			// skip over comments and CPU instructions
			continue;
		} else if (std::regex_match(line, match, loadPattern)) {
			cout << "Found a load op!" << endl;
			istringstream hexStream(match.str(2));
			hexStream >> std::hex >> address;
			outfile << match.str(1) << match.str(2) << match.str(3) << match.str(4);
			cacheAccess(&response, false, address, stoi(match.str(4)));
			logEntry(outfile, &response);
			
		} else if (std::regex_match(line, match, storePattern)) {
			cout << "Found a store op!" << endl;
			istringstream hexStream(match.str(2));
			hexStream >> std::hex >> address;
			outfile << match.str(1) << match.str(2) << match.str(3) << match.str(4);
			cacheAccess(&response, true, address, stoi(match.str(4)));
			logEntry(outfile, &response);
		} else if (std::regex_match(line, match, modifyPattern)) {
			cout << "Found a modify op!" << endl;
			istringstream hexStream(match.str(2));
			// first process the read operation
			hexStream >> std::hex >> address;
			outfile << match.str(1) << match.str(2) << match.str(3) << match.str(4);
			cacheAccess(&response, false, address, stoi(match.str(4)));
			logEntry(outfile, &response);
			outfile << endl;
			// now process the write operation
			hexStream >> std::hex >> address;
			outfile << match.str(1) << match.str(2) << match.str(3) << match.str(4);
			cacheAccess(&response, true, address, stoi(match.str(4)));
			logEntry(outfile, &response);
		} else {
			throw runtime_error("Encountered unknown line format in tracefile.");
		}
		outfile << endl;
	}
	// add the final cache statistics
	outfile << "L1 Cache: Hits:" << globalHits << " Misses:" << globalMisses << " Evictions:" << globalEvictions << endl;
	outfile << "Cycles:" << globalCycles << " Reads:" << globalReads << " Writes:" << globalWrites << endl;

	infile.close();
	outfile.close();
}

/*
	Report the results of a memory access operation.
*/
void CacheController::logEntry(ofstream& outfile, CacheResponse* response) {
	outfile << " " << response->cycles << " L1";
	if (response->hits > 0)
		outfile << " hit";
	if (response->misses > 0)
		outfile << " miss";
	if (response->evictions > 0)
		outfile << " eviction";
}

/*
	Calculate the block index and tag for a specified address.
*/
CacheController::AddressInfo CacheController::getAddressInfo(unsigned long int address) {
	AddressInfo ai;
    ai.setIndex = (address >> this->ci.numByteOffsetBits) & (this->ci.numberSets - 1);
    ai.tag = address >> (this->ci.numByteOffsetBits + this->ci.numSetIndexBits);
	return ai;
}

/*
	This function allows us to read or write to the cache.
	The read or write is indicated by isWrite.
	address is the initial memory address
	numByte is the number of bytes involved in the access
*/
void CacheController::cacheAccess(CacheResponse* response, bool isWrite, unsigned long int address, int numBytes) {
	// determine the index and tag
	AddressInfo ai = getAddressInfo(address);

	cout << "\tSet index: " << ai.setIndex << ", tag: " << ai.tag << endl;
	
	// your code should also calculate the proper number of cycles that were used for the operation
	// response->cycles = 0;
	
	// your code needs to update the global counters that track the number of hits, misses, and evictions
    for(unsigned long int access_block = address & ~(this->ci.blockSize-1); access_block < address+numBytes; access_block+=this->ci.blockSize) {
        this->cache->access(response, isWrite, ai.setIndex, ai.tag, numBytes);
        // // Calculate cycles for cache miss
        // if(response->misses) {
        //     if(access_block != (address & ~(this->ci.blockSize-1))) response->cycles++;
        //     else response->cycles+= this->ci.memoryAccessCycles;
        // }
        // response->cycles+= this->ci.cacheAccessCycles;
    }
	this->globalHits+= response->hits;
	this->globalMisses+= response->misses;
	this->globalCycles+= response->cycles;
	this->globalReads+= !isWrite;
	this->globalWrites+= isWrite;

	if (response->hits > 0)
		cout << "Operation at address 0x" << std::hex << address << " caused " << std::dec << response->hits << " hit(s)." << endl;
	if (response->misses > 0)
		cout << "Operation at address 0x" << std::hex << address << " caused " << std::dec << response->misses << " miss(es)." << endl;

	cout << "-----------------------------------------" << endl;
    this->cache->print();

	return;
}
