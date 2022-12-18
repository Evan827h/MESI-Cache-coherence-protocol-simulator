#include "sim.h"
#include "membus.h"
#include "cpu.h"
#include <string>
#include<iostream>
using namespace std;

sim::sim(string file0, string file1, string file2, string file3) {

	membus membus;
	Arbiter memArbiter(membus);
	memArbiter.run(file0, file1, file2, file3);
}