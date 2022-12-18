#include "cpu.h"
#include "ram.h"
#include "membus.h"
#include "cache.h"
#include "arbiter.h"
#include <string>
#include<iostream>
#include<utility>
using namespace std;


string membus::readMemory(int addr) {
    return ram0.read(addr);
}

bool membus::writeMemory(int addr, string data) {
    return ram0.write(addr, data);
}




