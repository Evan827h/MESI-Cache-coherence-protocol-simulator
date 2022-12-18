#pragma once
#ifndef ARBITER_H
#define ARBITER_H

#include<string>
#include<utility>
#include<vector>
#include "cache.h"
using namespace std;

class membus;
class Cache;

class Arbiter
{
private:

	const int CACHE_SIZE = 65536;
	const int BLOCK_SIZE = 64;
	const int ASSOC = 2;

	const int modified = 0;
	const int exclusive = 1;
	const int shared = 2;
	const int invalid = 3;

	int clock = 1;

	Cache& cache0;
	Cache& cache1;
	Cache& cache2;
	Cache& cache3;


	membus& bus;

public:

	Arbiter(membus& bus_);
	void run(string file0, string file1, string file2, string file3);

	pair<string, int> readMemory(int addr, int localState, int cacheID);
	pair<bool, int> writeMemory(int addr, string data, bool miss, int localState, int cacheID);

	vector<int> snoop(int address);
	void setState(int cacheID, int addr, int state);

	string runCacheRD(int cacheID, int addr);

	bool runCacheWB(int cacheID, int addr);
	bool writeBack(int addr, string data);

	int getClock() { return clock; };
};
#endif
