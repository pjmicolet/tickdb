#include "print.h"
#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <utility>
#include <inttypes.h>


void Print::Parse(std::vector<std::string> line)
{
    if (!table->IsValid())
    {
        std::cerr << "Table isn't valid, please load a valid file" << std::endl;
        return;
    }
    if ((int)line.size() < this->inputFields || ((int)line.size()-1) > this->inputFields)
    {
        std::cerr << "Print should be used this way: print <start time> <end time> <symbol>" << std::endl;
    }
    else
    {
        Execute(line);
    }
}

/*
print <start time> <end time> <symbol>

*/
void Print::Execute(std::vector<std::string> line)
{
    int64_t Start = atoi(line[1].c_str());
    int64_t End = atoi(line[2].c_str());
    std::string Symbol = line[3];

    //Start by getting the range index from the time column (should return 2 indexes)
    //Then generate a vector of indexes by going through the symbol list
    //Once we have the vector of correct indexes, rebuild the list; we can skip the symbol column by returning the required symbol.
    std::vector<int> range = table->GetRange("Time", Start, End);
    if(range[0] != -1 && range[1] != -1)
    {
        std::vector<int> indexes = table->GetIndexesSingle("Symbol", range, (char*)&(Symbol.at(0)), Symbol.size());
    
        if (optimised)
            table->OptimisedPrint(indexes, Symbol);
        else
            table->PrintIndexes(indexes, Symbol);
    }
}