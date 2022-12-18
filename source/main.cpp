#include "sim.h"
#include<string>
using namespace std;

int main()
{
    string file0Name = "./file0.txt";
    string file1Name = "./file1.txt";
    string file2Name = "./file2.txt";
    string file3Name = "./file3.txt";
    sim simulator(file0Name, file1Name, file2Name, file3Name);
    return 0;
}