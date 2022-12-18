#include "cache.h"
#include <string>
#include<iostream>
#include <fstream>
#include<cmath>
#include <math.h>
#include<bitset>
#include <deque>
#include <sstream>
#include<tuple>
using namespace std;



Cache::Cache(Arbiter& bus_, int cs, int bs, int a, int cacheNum): bus(bus_), cacheID(cacheNum) {

	cacheSize = cs;
	blockSize = bs;
	assoc = a;
	clock = 0;
	lookupTime = (ceil(log2(cacheSize / blockSize)) * assoc);

	if (assoc > 4) {
		fullyAssoc = true;
		lookupTime = (ceil(log2(cacheSize / blockSize)) * blockSize);
	}

	if (!fullyAssoc) {
		offsetBits = int(log2(blockSize));
		totalSets = cacheSize / (blockSize * assoc);
		setBits = int(log2(totalSets));
		tagBits = addressSize - offsetBits - setBits;
	}
	else {
		offsetBits = int(log2(blockSize));
		totalSets = 1;
		tagBits = addressSize - offsetBits;
	}

	vector<pair<int,string>> dataStart;
	dataStart.resize(blockSize, make_pair(invalid, "NULL"));
	vector<tuple<bool, int, vector<pair<int, string>>>> wayStart;
	wayStart.resize(totalSets, make_tuple(false, -1, dataStart));

	if (!fullyAssoc) {
		ways.resize(assoc, wayStart);
	}
	else {
		ways.resize(blockSize, wayStart);
	}

	LRU.resize(totalSets);
	for (int i = 0; i < LRU.size(); i++) {
		for (int j = 0; j < assoc; j++) {
			LRU[i].push_back(j);
		}
	}
}

//write
bool Cache::run(int address, string data) {
	
	//mem latency
	clock = bus.getClock();
	if (clock < memLatencyCount && memLatencyCount != -1) {
		return false;
	}
	if (memLatencyCount == -1) {
		memLatencyCount = clock + m_delay;
		return false;
	}
	auto res = read(address);
	//if write hit
	if (res.first && res.second == data && getState(address) != invalid) {
		//write hit
		writeMiss = false;
	}
	else {
		//write miss
		writeMiss = true;
	}
	//returns if success and new state
	pair<bool, int> writeReturn = bus.writeMemory(address, data, writeMiss, getState(address), cacheID);
	if (writeReturn.first) {
		//write to cache
		write(address, data);
		cout << "\nFrom WRITE\tLocal change:\t";
		//set state
		setState(address, writeReturn.second);
		cout << "\n";

		memLatencyCount = -1;
		if (writeMiss) {
			writeMissCount++;
		} 
		else {
			writeHitCount++;
		}
		return true;
	}
	else
	{
		return false;
	}
	
}

//read
string Cache::run(int address) {

	auto res = read(address);
	//if read hit
	if (res.first && res.second != "NULL" && getState(address) != invalid) {
		hitCount++;
		totalLookupTime += lookupTime;
		return res.second;
	}
	else {
		//mem latency
		clock = bus.getClock();
		if (clock < memLatencyCount && memLatencyCount != -1) {
			return "";
		}
		if (memLatencyCount == -1) {
			memLatencyCount = clock + m_delay;
			missCount++;
			
			return "";
		}
		//returns new state and data
		pair<string, int> readReturn = bus.readMemory(address, getState(address), cacheID);
		if (readReturn.first != "") {
			//write new data
			write(address, readReturn.first);
			cout << "\nFrom READ\tLocal change:\t";
			//set new state
			setState(address, readReturn.second);
			cout << "\n";

			memLatencyCount = -1;
			totalLookupTime += (lookupTime + ((m_delay + 1) * 10));
			missCount++;
		}
		return "";
	}
}

bool Cache::write(int address, string data) {

	int set = getSet(address);
	int tag = getTag(address);
	int off = getOffset(address);

	int wayIndex = -1;

	for (int i = 0; i < ways.size(); i++) {
		if (get<1>(ways[i][set]) == tag) {
			wayIndex = i;
			break;
		}
	}

	if(wayIndex == -1)
		wayIndex = LRU[set].front();

	bool complulsory = false;
	if (get<2>(ways[wayIndex][set])[off].second == "NULL") complulsory = true;

	get<0>(ways[wayIndex][set]) = true;
	get<1>(ways[wayIndex][set]) = tag;
	get<2>(ways[wayIndex][set])[off].second = data;

	if (complulsory) {
		coldCount++;
	}

	LRU[set].pop_front();
	LRU[set].push_back(wayIndex);

	return false;

}

pair<bool, string> Cache::read(int address) {

	int set = getSet(address);
	int tag = getTag(address);
	int off = getOffset(address);

	int wayIndex = LRU[set].front();

	for (int i = 0; i < ways.size(); i++) {
		if (get<0>(ways[i][set]) == true && get<1>(ways[i][set]) == tag) {
			return make_pair(true, get<2>(ways[i][set])[off].second);
		}
	}

	LRU[set].pop_front();
	LRU[set].push_back(wayIndex);

	return make_pair(false, "");

}

void Cache::setState(int address, int state) {
	int set = getSet(address);
	int tag = getTag(address);
	int off = getOffset(address);
	int wayIndex = LRU[set].front();

	for (int i = 0; i < ways.size(); i++) {
		if (get<1>(ways[i][set]) == tag) {
			string oldStateString = stateToString(get<2>(ways[i][set])[off].first);
			string newStateString = stateToString(state);

			get<2>(ways[i][set])[off].first = state;
			
			cout << "Cycle: " << bus.getClock() << " CACHE" << cacheID << ": Setting state of address: " << address << " from " << oldStateString << " to " << newStateString << "\n";
			return;
		}
	}
	cout << "set tag error!\n";
}

int Cache::getState(int address) {
	int set = getSet(address);
	int tag = getTag(address);
	int off = getOffset(address);
	int wayIndex = LRU[set].front();

	for (int i = 0; i < ways.size(); i++) {
		if (get<1>(ways[i][set]) == tag) {
			return get<2>(ways[i][set])[off].first;
		}
	}
	return -1;
}

bool Cache::writeBack(int address) {
	
	if (bus.writeBack(address, read(address).second)) {
		return true;
	}
	else
	{
		return false;
	}
}

int Cache::getSet(int address) {
	if (fullyAssoc) return 0;
	string set = bitset<32>(address).to_string().substr(tagBits, setBits);
	//cout << bitset<32>(address).to_string() << "\n";
	//cout << set << "\n\n";
	return stoi(set,0, 2);
}

int Cache::getTag(int address) {
	string tag = bitset<32>(address).to_string().substr(0, tagBits);
	return stoi(tag, 0, 2);
}

int Cache::getOffset(int address) {
	string off = bitset<32>(address).to_string().substr(tagBits + setBits);
	return stoi(off, 0, 2);
}

void Cache::printCache() {
	if (!fullyAssoc) {
		for (int i = 0; i < totalSets; i++) {
			cout << LRU[i].front() << "\t";
			for (int j = ways.size() - 1; j >= 0; j--) {
				cout << get<0>(ways[j][i]) << "\t" << get<1>(ways[j][i]) << "\t";
			}
			cout << "\n";
		}
		cout << "\n\n";
	}
	else {
		for (int i = ways.size() - 1; i >= 0; i--) {
			cout << get<0>(ways[i][0]) << "\t" << get<1>(ways[i][0]) << "\n";
		}
	}
}

void Cache::printStats() {
	cout << "\nCache Size: " << cacheSize << "B\t" << "Cache Line Size: " << blockSize << "B\t" << "Associativity: " << assoc << "\n";
	cout << "Read Hit Rate: " << hitCount << "/" << (hitCount + missCount) << " : " << (hitCount / hitCount + missCount) << "\n";
	cout << "Write Hit Rate: " << writeHitCount << "/" << (writeMissCount + writeHitCount) << " : " << (writeHitCount / (writeMissCount + writeHitCount)) << "\n";
	cout << "Cold Start Rate: " << coldCount << "/" << (hitCount + missCount) << " : " << (coldCount / (hitCount + missCount)) << "\n";
	cout << "Total Lookup Time: " << totalLookupTime << " Ticks\n\n";
}

string Cache::stateToString(int state) {
	switch (state) {
	case 0:
		return "modified";
	case 1:
		return "exclusive";
	case 2:
		return "shared";
	case 3:
		return "invalid";
	}
	return "";
}