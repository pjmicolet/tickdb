#include "tickfile.h"
#include "utils.h"
#include "column.h"
#include <vector>
#include <string>
#include <inttypes.h>
#include <iostream>
#include <time.h>

void Tickfile::Parse(std::vector<std::string> line)
{
    if ((int)line.size() < this->inputFields || ((int)line.size() - 1) > this->inputFields)
    {
        std::cerr << "Product should be used this way: product <start time> <end time> <symbol> <field1> <field2>" << std::endl;
    }
    else
    {
        Execute(line);
    }
}


/*
    tickfile <file>

    Currently assume this, when I split by comma 
    split[0] == time
    split[1] == symbol
    split[2..n] == types
*/
void Tickfile::Execute(std::vector<std::string> line)
{
    std::string filename = line[1];
    std::ifstream file;
    file.open(filename, std::ifstream::in);
    std::string currLine;
    std::vector<std::string> splitLine;
    int key = 0;
    char* doubleVal = NULL;
    char* timeD = NULL;
    char* symbD = NULL;
    double val = 0;
    uint64_t averageSize = 0;
    
    if (!file.good())
    {
        std::cout << "File does not exist, keeping current table in memory" << std::endl;
        return;
    }
    //Clear table if we're loading in another file.
    table->Clear();
    table->ClearCache();
    table->SetInvalid();

    while (getline(file, currLine))
    {
        if (currLine.empty())
            continue;
        averageSize += currLine.size();
        splitLine = split(currLine, ',');
        time_t time = (time_t)(atoi(splitLine[0].c_str()));
        std::string symbol = splitLine[1];

        timeD = (char*)&time;
        symbD = (char*)&symbol.at(0);
        table->PushBack("Time", TYPE::TIME, timeD, sizeof(time_t),key);
        table->PushBack("Symbol", TYPE::STRING, symbD, symbol.size(),key);

        for (int i = 2; i <(int)splitLine.size(); i+=2)
        {
            val = atof(splitLine[i+1].c_str());
            doubleVal = (char*)&val;
            table->PushBack(splitLine[i], TYPE::DOUBLE, doubleVal, sizeof(double),key);
        }
        key++;
    }
    table->SetValid();
    file.close();
    
    //Get size in bytes, we want to use this to dynamically reconfigure the size of the cache based on how much memory we have left.
    //First start by measuring the average size of the rows in bytes
    //Also figure out roughly what the size of the table is
    //I want to leave say 10 MB just in case I do something weird.
    averageSize = (averageSize / key);
    int maxSize = 1000000000;
    int tableSize = table->GetSize();
    //Just to be safe, I make sure it's under total memory limits
    int remaining = (maxSize - tableSize) - 250000000;

    //Didn't have time to figure out a good size for the productCache
    table->ResizeCache((size_t)(remaining/(int)averageSize),10000000);
}