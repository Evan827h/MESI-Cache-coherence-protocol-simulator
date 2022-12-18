#ifndef membus_H
#define membus_H

#include "cpu.h"
#include "ram.h"
#include "cache.h"
#include <map>
#include <string>
#include <functional>
using namespace std;

class cpu;
class Cache;

class membus
{
private:
	
	bool busy = false;

	ram ram0;


public:
	string readMemory(int addr);
	bool writeMemory(int addr, string data);
	
};

#endif
