#pragma once
#ifndef CACHE_H
#define CACHE_H
#include "arbiter.h"
#include<string>
#include<utility>
#include<vector>
#include<string>
#include <deque>
#include<tuple>
using namespace std;

class Arbiter;

class Cache
{
private:

	const int cacheID;

	const int addressSize = 32;
	const int m_delay = 2;
	
	const int modified = 0;
	const int exclusive = 1;
	const int shared = 2;
	const int invalid = 3;

	int clock;

	int cacheSize;
	int blockSize;
	int assoc;
	int lookupTime;

	int offsetBits;
	int totalSets;
	int setBits = -1;
	int tagBits;
	bool fullyAssoc = false;

	int memLatencyCount = -1;
	double missCount = 0;

	bool writeMiss = true;

	vector<deque<int>> LRU;
	//valid, tag, data
	vector<vector<tuple<bool, int, vector<pair<int, string>>>>> ways;

	double hitCount = 0;
	int coldCount = 0;
	int totalLookupTime = 0;
	double writeHitCount = 0;
	double writeMissCount = 0;

	bool write(int address, string data);
	pair<bool, string> read(int address);
	int getSet(int address);
	int getTag(int address);
	int getOffset(int address);
	void printCache();

	Arbiter& bus;

public:
	Cache(Arbiter& bus_, int cacheSize, int blockSize, int a, int cacheNum);
	bool run(int address, string data);
	string run(int address);

	void setState(int address, int state);
	int getState(int address);
	bool writeBack(int address);

	string stateToString(int state);

	double getMissCount() { return missCount; };
	double getTotalLookupTime() { return double(totalLookupTime); };
	void printStats();
};
#endif
