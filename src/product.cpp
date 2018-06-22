#include "product.h"
#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <utility>
#include <inttypes.h>


void Product::Parse(std::vector<std::string> line)
{
    if (!table->IsValid())
    {
        std::cerr << "Table isn't valid, please load a valid file" << std::endl;
        return;
    }
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
print <start time> <end time> <symbol>

*/
void Product::Execute(std::vector<std::string> line)
{
    int64_t Start = atoi(line[1].c_str());
    int64_t End = atoi(line[2].c_str());
    std::string Symbol = line[3];
    std::string Type1= line[4];
    std::string Type2 = line[5];
    
    //Start by getting the range index from the time column (should return 2 indexes)
    //Then generate a vector of indexes by going through the symbol list
    std::vector<int> range = table->GetRange("Time", Start, End);

    if (range[0] != -1 && range[1] != -1)
    {
        std::pair<std::string, std::string> smallest = table->GetNumberOfItemsInColumn(Type1) < table->GetNumberOfItemsInColumn(Type2) ? std::make_pair(Type1,Type2) : std::make_pair(Type2,Type1);
                
        std::vector<int> indexes = table->GetAllIndexes(smallest.first, range);


        if (indexes.size() == 0)
        {
            std::cout << "0\n";
            return;
        }

        table->GetIndexUnion(smallest.second, indexes);
        //If there's nothing in the smallest column then there will be nothing to do
        if (indexes.size() == 0)
        {
            std::cout << "0\n";
            return;
        }

        table->FilterIndexes("Symbol", indexes, (char*)&Symbol.at(0), Symbol.size());

        if (indexes.size() > 0)
        {
            //Once we have the vector of correct indexes, rebuild the list; we can skip the symbol column by returning the required symbol.
            if (optimised)
                std::cout << table->GetProductOpt(indexes, Type1, Type2) << std::endl;
            else
                std::cout << table->GetProduct(indexes, Type1, Type2) << std::endl;
        }
        else
        {
            std::cout << "0\n";
        }
    }
    else
    {
        std::cout << "0\n";
    }
}