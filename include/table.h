#ifndef _H_TABLE_H_
#define _H_TABLE_H_
#include "column.h"
#include "query_engine.h"
#include "printCache.h"
#include "productCache.h"

#include <map>
#include <string>
#include <tuple>
#include <iostream>
#include <utility>
#include <typeinfo>
#include <inttypes.h>
#include <limits.h>
#include <sstream>
#include<iomanip>
#include <algorithm>
#include <functional>

/*
    A table is essentially a std::map of columns.
    All requests for data in the columns should be made here.
*/
class Table
{
public:

    Table(PrintCache* cache, ProductCache* prodCache, bool optimisedForPrint, bool optimisedForProduct)
    {
        this->optimisedForPrint = optimisedForPrint;
        this->optimisedForProduct = optimisedForProduct;
        this->pCache = cache;
        this->valid = false;
        this->index = 0;
    }
    ~Table()
    {
        for (auto & i : columns)
        {
            delete i.second;
        }
    }

    void AddColumn(std::string name, TYPE t, int key)
    {
        if (t == TYPE::DOUBLE)
        {
            DoubleColumn* column = new DoubleColumn(name, t, key);
            columns[name] = (AbstractColumn*)column;
        }
        else if (t == TYPE::STRING)
        {
            StringColumn* column = new StringColumn(name, t, key);
            columns[name] = (AbstractColumn*)column;
        }
        else if (t == TYPE::TIME)
        {
            TimeColumn* column = new TimeColumn(name, t, key);
            columns[name] = (AbstractColumn*)column;
        }
        colInd[name] = this->index++;
    }


    /*
        std::string name = Column name
        TYPE t = Column Type
        char* data = Pointer to data
        size_t size = Size of data (useful for StringColumn)
        int key = Key to be attached to the data. In this specific scenario, key is the index.
        
        When the caller wants to push an item into a column it has to call the column by name, and pass the data via a char*.
        If the column doesn't exist, the table will create it, using the type specified.
    */
    void PushBack(std::string name, TYPE t, char* data, size_t size, int key)
    {
        if (columns.count(name) == 0)
        {
            AddColumn(name, t,key);
            columns[name]->AddItem(t, data, size,key);
        }
        else
        {
            columns[name]->AddItem(t, data, size,key);
        }
    }

    /*
        Estimates the size of the table
    */
    int GetSize()
    {
        int size = 0;
        for (auto & i : columns)
        {
            size += i.second->GetSize()+sizeof(AbstractColumn)+sizeof(i.first);
        }
        return size+sizeof(std::map<std::string,AbstractColumn*>);
    }

    void GetNames()
    {
        for (auto & i : columns)
        {
            std::cout << i.first << std::endl;
        }
    }

    void ShrinkColumns()
    {
        for (auto & i : columns)
        {
            i.second->Shrink();
        }
    }

    /*
        Given a column and a range of values (start,end) return the starting and ending index from that column.
    */
    std::vector<int> GetRange(std::string column, uint64_t start, uint64_t end)
    {
        return getIndexRange(columns[column], start, end);
    }

    /*
        Given a column and a range of values (indexRange[0],indexRange[1])
        For all items <key,value> in (indexRange[0],indexRange[1]) if their value matches the key specified in char* key then return the key from the pair.
        This specific method returns a vector of <startIndex,endIndex> ranges.
        See StringColumn::GetIndexes for more details.
    */
    std::vector<std::pair<int, int>> GetIndexes(std::string column, std::vector<int>& indexRange, char* key, size_t size)
    {
        return getIndexes(columns[column], indexRange, key, size);
    }

    /*
        Given a column and a range of values (indexRange[0],indexRange[1])
        For all items <key,value> in (indexRange[0],indexRange[1]) if their value matches the key specified in char* key then return the key from the pair.
        This specific method returns a vector of keys.
    */
    std::vector<int> GetIndexesSingle(std::string column, std::vector<int>& indexRange, char* key, size_t size)
    {
        return getIndexesSingle(columns[column], indexRange, key, size);
    }

    std::vector<int> GetAllIndexes(std::string column, std::vector<int>& indexRange)
    {
        return getAllIndexes(columns[column], indexRange);
    }

    void GetIndexUnion(std::string column, std::vector<int>& indexRange)
    {
        return getUnion(columns[column], indexRange);
    }

    void FilterIndexes(std::string column, std::vector<int>& indexes, char* key, size_t size)
    {
        return filterIndexes(columns[column], indexes,key,size);
    }

    /*
        Given a set of indexes we're interested in (range), if those indexes exist in both specified columns, multiply the two numbers and add them to sum.
    */
    double GetProduct(std::vector<int> range, std::string type1, std::string type2)
    {
        if (!columns.count(type1) && !columns.count(type2))
            return 0.0;

        double sum = 0;
        unsigned char* prodA = NULL;
        unsigned char* prodB = NULL;
        size_t wtv = 0;
        for (auto & id : range)
        {
            columns[type1]->GetData(id, &prodA, wtv);
            columns[type2]->GetData(id, &prodB, wtv);

            double convA = *(double*)(prodA);
            double convB = *(double*)(prodB);
            sum += convA * convB;
            
        }
        return sum;
    }

    /*
        Given a set of indexes we're interested in (range), if those indexes exist in both specified columns, multiply the two numbers and add them to sum.
        This is the optimised version; we try to find if a precomputed dot product for our given range exist.
        If it does, load the precomputed sum, add it to the current one, and move the iterator appropriately.

        We then cache the result for later use.
    */
    double GetProductOpt(std::vector<int> range, std::string type1, std::string type2)
    {
        if (!columns.count(type1) && !columns.count(type2))
            return 0.0;

        double sum = 0;

        std::vector<double> data1;
        std::vector<double> data2;
        columns[type1]->GetSpecificData(range, data1);
        columns[type2]->GetSpecificData(range, data2);

        for (int i = 0; i < (int)range.size(); i++)
        {
            sum += data1[i] * data2[i];
        }

        return sum;
    }


    /*
        This method takes a vector of indexes (range) and reconstructs the rows for printing.
        There is one "hack" in this function, which is that we know which symbol we want ahead of time (since it's requested by the user).
        This allows us to skip the symbol column.

        Given a subset of the indexes in std::vector<int> range, I have a buffer of strings which represent partially constructed rows.
        For each of the columns, I iterate through the subset of indexes to check if there are values and append the data to their corresponding string in the buffer.

    */
    void PrintIndexes(std::vector<int> range, std::string Symbol)
    {
        std::string print = "";

        int buffSize = 1000;
        std::vector<std::ostringstream> buffer(buffSize);
        //Buffer lines
        int buffIndex = 0;
        size_t size = 0;
        int colSize = 2;
        unsigned char* data = NULL;
        std::string finalString;
        std::vector<std::pair<std::string,AbstractColumn*>> columnVec;
        for (auto & col : columns)
        {
            if (col.first != "Symbol" && col.first != "Time")
                columnVec.push_back(std::make_pair(col.first,col.second));
        }

        for (int rangeInd = 0; rangeInd < (int)range.size(); rangeInd += buffSize)
        {

            finalString = "";

            int which = rangeInd + buffSize >(int)range.size() ? (int)range.size() : rangeInd + buffSize;
            int index = 0;

            for (int i = rangeInd; i < which;i++)
            {
                buffer[index].str("");
                buffer[index].clear();
                buffer[index] << std::setprecision(std::numeric_limits<double>::digits10);
                index++;
            }

            for (int iCol = 0; iCol < (int)columnVec.size(); iCol += colSize)
            {
                int which2 = iCol + colSize >(int)columnVec.size() ? (int)columnVec.size() : iCol + colSize;
                for (int iiCol = iCol; iiCol < which2; iiCol++)
                {
                    index = 0;

                    for (int i = rangeInd; i < which; i++)
                    {
                        int x = range[i];
                        data = NULL;
                        
                        if (columnVec[iiCol].second->GetData(x, &data, size))
                        {
                            if(buffer[index].str().size() == 0)
                                buffer[index] << columnVec[iiCol].first << ":" << *(double*)(data);
                            else
                                buffer[index] << "," << columnVec[iiCol].first << ":" << *(double*)(data);

                        }
                        index++;
                    }
                }
            }
            buffIndex = rangeInd + buffSize > (int)range.size() ? index : buffSize;
            for (int i = 0; i < buffIndex; i++)
            {
                finalString += buffer[i].str() + "\n";
            }
            std::cout << finalString;
        }

    }

    /*
        The optimised print uses a cache to speed up the row-building.
        It starts by looking for the cached rows, then builds the remaining rows and finally prints them.

        I think there could be improvements in how I choose to cache rows and when I  choose to cache them.
        I can foresee scenarios where the cache is thrashed by having non-cached rows over-write cached rows that will just be used.

        Another way I could have done this, to avoid thrashing is to buffer the output a bit differently; I'd start by loading all cached strings into the buffer,
        then rebuilding all rows that aren't cached; then sorting the buffer and printing.
        However this method may require more memory, which would reduce the overall size of the cache.
        
        Also I assumed that rows should be printed in-order; which is why I implemented the method below.
        
    */
    void OptimisedPrint(std::vector<int> range, std::string Symbol)
    {
        std::string print = "";
        
        //More profiling would be needed to figure out good average buffer sizes and number of columns to iterate over at a time.
        int buffSize = 1000;
        int colSize = 2;
        std::vector<std::ostringstream> buffer(buffSize);
        std::vector<int> cached(buffSize, 0);
        std::vector<int> keys(buffSize, 0);
        //Buffer lines
        int buffIndex = 0;
        size_t size = 0;
        unsigned char* data = NULL;
        std::string cacheString = "";
        std::vector<std::pair<std::string, AbstractColumn*>> columnVec;
        std::string finalString = "";
        std::vector<std::pair<int, int>> remaining;
        for (auto & col : columns)
        {
            if (col.first != "Symbol" && col.first != "Time")
                columnVec.emplace_back(std::make_pair(col.first, col.second));
        }

        for (int rangeInd = 0; rangeInd < (int)range.size(); rangeInd += buffSize)
        {
            finalString = "";
            int which = rangeInd + buffSize >(int)range.size() ? (int)range.size() : rangeInd + buffSize;
            int index = 0;
            remaining.clear();
            //First find cached items;
            for (int i = rangeInd; i < which; i++)
            {
                int x = range[i];
                keys[index] = x;

                buffer[index].str("");
                buffer[index].clear();
                buffer[index] << std::setprecision(std::numeric_limits<double>::digits10);

                if (pCache->GetString(x, &cacheString))
                {
                    buffer[index] << cacheString;
                    cached[index] = 1;
                    index++;
                    continue;
                }
                else
                {
                    cached[index] = 0;
                    remaining.push_back(std::make_pair(index, x));
                }
                index++;
            }
            int maxIndex = index;

            for (int iCol = 0; iCol < (int)columnVec.size(); iCol += colSize)
            {
                int which2 = iCol + colSize >(int)columnVec.size() ? (int)columnVec.size() : iCol + colSize;
                for (int iiCol = iCol; iiCol < which2; iiCol++)
                {
                    for (auto & i : remaining)
                    {
                        data = NULL;

                        if (columnVec[iiCol].second->GetData(i.second, &data, size))
                        {
                            if (buffer[i.first].str().size() == 0)
                                buffer[i.first] << columnVec[iiCol].first << ":" << *(double*)(data);
                            else
                                buffer[i.first] << "," << columnVec[iiCol].first << ":" << *(double*)(data);
                        }
                    }
                }
            }
            buffIndex = rangeInd + buffSize > (int)range.size() ? maxIndex : buffSize;
            for (int i = 0; i < buffIndex; i++)
            {
                pCache->Put(keys[i], buffer[i].str());
                finalString += buffer[i].str() += "\n";
            }
            std::cout << finalString;
        }
     }

    /*
    THIS WAS ONLY PARTIALLY IMPLEMENTED
    Keeping it just for show.

    This method takes a vector of index ranges (range) and reconstructs the rows for printing.
    There is one "hack" in this function, which is that we know which symbol we want ahead of time (since it's requested by the user).
    This allows us to skip the symbol column.

    Given a subset of the indexes in std::vector<int> range, I have a buffer of strings which represent partially constructed rows.
    For each of the columns, I iterate through the subset of indexes to check if there are values and append the data to their corresponding string in the buffer.

    */
    void PrintIndexes(std::vector<std::pair<int, int>> range, std::string Symbol)
    {
        std::string print = "";
        int buffSize = 100;
        std::vector<std::string> buffer(buffSize, "");
        int buffIndex = 0;
        int other = 0;
        size_t size = 0;
        unsigned char* data = NULL;
        for (auto & i : range)
        {
            //How many items in the range<int,int>
            int range = (i.second - i.first)+1;
            int index = 0;
            
            if (buffIndex + range > buffSize)
            {
                for (auto & string : buffer)
                    std::cout << string << std::endl;
                buffIndex = 0;
            }

            for (int x = i.first; x <= i.second; x++)
            {
                //This has to return true;
                columns["Time"]->GetData(x, &data, size);
                time_t* convertedData = (time_t*)data;
                buffer[buffIndex+index] = std::to_string(*convertedData) + "," + Symbol;
                index++;
            }
            for (auto & col : columns)
            {
                index = 0;
                for (int x = i.first; x <= i.second; x++)
                {
                    data = NULL;
                    if (col.first == "Symbol")
                        continue;
                    if (col.first == "Time")
                        continue;
                    if (col.second->GetData(x, &data, size))
                        buffer[buffIndex+index]+= +"," + col.first + "," + std::to_string(*(double*)(data));
                    index++;
                }
            }
            buffIndex += range;
            if (buffIndex == buffSize)
            {
                   for(auto & string : buffer)
                       std::cout << string <<"\n";
                buffIndex = 0;
            }
        }
        if (buffIndex > 0)
        {
            for (int i = 0; i < buffIndex; i++)
                std::cout << buffer[i] << "\n";
        }
        std::cout << "other " << other << std::endl;

    }

    void Clear()
    {
        for (auto & i : columns)
        {
            delete i.second;
        }
        ClearCache();
        columns.clear();
    }
    
    void ClearCache()
    {
        if (optimisedForPrint)
            pCache->Clear();
    }

    void ResizeCache(size_t sizeForPrint, size_t sizeForPrd)
    {
        if (optimisedForPrint)
            pCache->Resize(sizeForPrint);
    }

    void SetInvalid()
    {
        valid = false;
    }

    void SetValid()
    {
        valid = true;
    }

    bool IsValid()
    {
        return valid;
    }

    /*
        This returns the number of rows in a column, not its size in MB
    */
    size_t GetNumberOfItemsInColumn(std::string column)
    {
        return columns[column]->GetNumberOfIndexes();
    }


private:
    bool valid;
    bool optimisedForPrint;
    bool optimisedForProduct;
    std::map<std::string, AbstractColumn*> columns;
    std::map<std::string, int> colInd;
    int index = 0;
    PrintCache* pCache;
  //  ProductCache* prodCache;
};
#endif