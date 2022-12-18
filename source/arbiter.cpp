#include "arbiter.h"
#include "membus.h"
#include "cache.h"
#include<string>
#include<utility>
#include<iostream>
#include<vector>
using namespace std;


Arbiter::Arbiter(membus& bus_) 
    : bus(bus_),
    cache0(*new Cache(*this, CACHE_SIZE, BLOCK_SIZE, ASSOC, 0)),
    cache1(*new Cache(*this, CACHE_SIZE, BLOCK_SIZE, ASSOC, 1)),
    cache2(*new Cache(*this, CACHE_SIZE, BLOCK_SIZE, ASSOC, 2)),
    cache3(*new Cache(*this, CACHE_SIZE, BLOCK_SIZE, ASSOC, 3))
{}

pair<string, int> Arbiter::readMemory(int addr, int localState, int cacheID) {
    vector<int> states = snoop(addr);

    for (int remoteCacheIndex = 0; remoteCacheIndex < states.size(); remoteCacheIndex++) {
        //if remote is exclusive
        if (states[remoteCacheIndex] == exclusive && remoteCacheIndex != cacheID) {
            //make remote and local shared
            cout << "\nFrom READ\t";
            setState(remoteCacheIndex, addr, shared);
            return make_pair(runCacheRD(remoteCacheIndex, addr), shared);
        }
        //if remote is shared
        else if (states[remoteCacheIndex] == shared && remoteCacheIndex != cacheID) {
            //make local shared
            return make_pair(runCacheRD(remoteCacheIndex, addr), shared);
        }
        //if remote is modifed
        else if (states[remoteCacheIndex] == modified && remoteCacheIndex != cacheID) {
            if (runCacheWB(remoteCacheIndex, addr)) {
                //make remote and local shared
                cout << "\nFrom READ\t";
                setState(remoteCacheIndex, addr, shared);
                return make_pair(runCacheRD(remoteCacheIndex, addr), shared);
            }
            else {
                //return nothing if remote cache is writing to mem
                return make_pair("", -1);
            }
        }
    }
    //if local is invalid
    return make_pair(bus.readMemory(addr), exclusive);
}


pair<bool, int> Arbiter::writeMemory(int addr, string data, bool miss, int localState, int cacheID) {
    //write miss
    if (miss) {
        //get all cache addr states
        vector<int> states = snoop(addr);

        for (int remoteCacheIndex = 0; remoteCacheIndex < states.size(); remoteCacheIndex++) {
            //if remote is modified
            if (states[remoteCacheIndex] == modified && remoteCacheIndex != cacheID) {
                if (runCacheWB(remoteCacheIndex, addr)) {
                    //set remote to invalid
                    cout << "\nFrom WRITE\t";
                    setState(remoteCacheIndex, addr, invalid);
                }
                else {
                    //return nothing if remote is reading from mem
                    return make_pair(false, -1);
                }
            }
            //if remote is exclusive or shared
            else if ((states[remoteCacheIndex] == exclusive || states[remoteCacheIndex] == shared) && remoteCacheIndex != cacheID) {
                //set remote to invalid
                cout << "\nFrom WRITE\t";
                setState(remoteCacheIndex, addr, invalid);
            }
        }
        //set local to modified
        return make_pair(bus.writeMemory(addr, data), modified);
    }
    //write hit
    else {
        //if local is modified or exclusive
        if (localState == modified || localState == exclusive) {
            //set local to modified
            return make_pair(bus.writeMemory(addr, data), modified);
        }
        //if local is shared
        else if (localState == shared) {
            vector<int> states = snoop(addr);
            for (int remoteCacheIndex = 0; remoteCacheIndex < states.size(); remoteCacheIndex++) {
                //if remote is shared
                if (states[remoteCacheIndex] == shared && remoteCacheIndex != cacheID) {
                    //set remote to invalid
                    cout << "\nFrom WRITE\t";
                    setState(remoteCacheIndex, addr, invalid);
                }
            }
            //set local to modified
            return make_pair(bus.writeMemory(addr, data), modified);
        }
        else {
            return make_pair(false, -1);
        }
    }
}

//return list of all cache states on address
vector<int> Arbiter::snoop(int address) {
    vector<int> states;
    states.push_back(cache0.getState(address));
    states.push_back(cache1.getState(address));
    states.push_back(cache2.getState(address));
    states.push_back(cache3.getState(address));
    return states;
}

void Arbiter::setState(int cacheID, int addr, int state) {
    cout << "Remote change:\t";
    switch (cacheID) {
    case 0:
        cache0.setState(addr, state);
        break;
    case 1:
        cache1.setState(addr, state);
        break;
    case 2:
        cache2.setState(addr, state);
        break;
    case 3:
        cache3.setState(addr, state);
        break;
    }
}


bool Arbiter::runCacheWB(int cacheID, int addr) {
    switch (cacheID) {
    case 0:
        return cache0.writeBack(addr);
    case 1:
        return cache1.writeBack(addr);
    case 2:
        return cache2.writeBack(addr);
    case 3:
        return cache3.writeBack(addr);
    }
}

//mem writeback
bool Arbiter::writeBack(int addr, string data) {
    return bus.writeMemory(addr, data);
}

string Arbiter::runCacheRD(int cacheID, int addr) {
    switch (cacheID) {
    case 0:
        return cache0.run(addr);
    case 1:
        return cache1.run(addr);
    case 2:
        return cache2.run(addr);
    case 3:
        return cache3.run(addr);
    }
}

void Arbiter::run(string file0, string file1, string file2, string file3) {

    cpu cpu0(file0);
    cpu cpu1(file1);
    cpu cpu2(file2);
    cpu cpu3(file3);

    cout << "Running\n";
    pair<int, string> address0 = cpu0.getAddress();
    pair<int, string> address1 = cpu1.getAddress();
    pair<int, string> address2 = cpu2.getAddress();
    pair<int, string> address3 = cpu3.getAddress();
    //Round robin running
    while(true) {

        //stop if reached end of every file
        if (address0.first == -1 && address1.first == -1 && address2.first == -1 && address3.first == -1)
            break;
        if (address0.first != -1) {
            if (address0.second != "") {
                //write addr
                bool done = false;
                while (!done) {
                    done = cache0.run(address0.first, address0.second);
                    clock++;
                }
                //get new address if cache is finished loading
                if (done) {
                    address0 = cpu0.getAddress();
                }
            }
            else {
                //read addr
                string out = "";
                while (out == "") {
                    out = cache0.run(address0.first);
                    clock++;
                }
                if (out != "") {
                    address0 = cpu0.getAddress();
                }
            }
        }

        if (address1.first != -1) {
            if (address1.second != "") {
                //write
                bool done = false;
                while (!done) {
                    done = cache1.run(address1.first, address1.second);
                    clock++;
                }
                if (done) {
                    address1 = cpu1.getAddress();
                }
            }
            else {
                //read
                string out = "";
                while (out == "") {
                    out = cache1.run(address1.first);
                    clock++;
                }
                if (out != "") {
                    address1 = cpu1.getAddress();
                }
            }
        }

        if (address2.first != -1) {
            if (address2.second != "") {
                //write
                bool done = false;
                while (!done) {
                    done = cache2.run(address2.first, address2.second);
                    clock++;
                }
                if (done) {
                    address2 = cpu2.getAddress();
                }
            }
            else {
                //read
                string out = "";
                while (out == "") {
                    out = cache2.run(address2.first);
                    clock++;
                }
                if (out != "") {
                    address2 = cpu2.getAddress();
                }
            }
        }

        if (address3.first != -1) {
            if (address3.second != "") {
                //write
                bool done = false;
                while (!done) {
                    done = cache3.run(address3.first, address3.second);
                    clock++;
                }
                if (done) {
                    address3 = cpu3.getAddress();
                }
            }
            else {
                //read
                string out = "";
                while (out == "") {
                    out = cache3.run(address3.first);
                    clock++;
                }
                if (out != "") {
                    address3 = cpu3.getAddress();
                }
            }
        }
    }
    cout << "\n\nPrinting Cache Stats:\n";
    cache0.printStats();
    cache1.printStats();
    cache2.printStats();
    cache3.printStats();

    cout << "Done." << "\n";
}