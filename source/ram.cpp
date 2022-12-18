#include "ram.h"
#include <fstream>
#include<iostream>
#include <stdlib.h> 
#include <time.h>

using namespace std;

ram::ram() {

    mem.resize(memSize, "0");

    srand(time(NULL));
    //Random numbers for reads
    for (int i = 0; i < 16; i++) {
        int f = rand() % 100 + 1;
        mem[i] = to_string(f);
    }

}

string ram::read(int addr) {
    if(addr < mem.size()){
        return mem[addr];
    } else {
        cout << "Could not find address " << addr << " in RAM.\n";
        return "";
    }
}

bool ram::write(int addr, string data) {
    if(addr >= 0 && addr < memSize) {
        mem[addr] = data;
        return true;
    } else {
        return false;
    }
}

vector<string> ram::getMemory() {
    return mem;
}
