#ifndef _H_ABSTRACT_C_
#define _H_ABSTRACT_C_
#include<string>
#include<iostream>
#include <ostream>
#include <vector>
#include <time.h>
#include <utility>
#include <inttypes.h>


/*
    This base class allows me to implement a column of any type and use a generic interface from the Table.
    The only downside is that to make sure that the interface is "easy" to use, I pass data as a char* or unsigned char*
    That means that I have to be careful when passing data to and from a column.

    Normally this can be mitigated by checking the column's type via the TYPE enum.
    The start and end integers are an easy way to check if a requested item is in bound of a column.
    Since columns don't all start at index 0 (due to them being created whenever they're first encountered in the original file) and don't necessarily end at the last row of the file
    I set start to be the first key encountered by the column and end to be the last.
    This way GetItem(x) where x is a key I can start by checking if  start<=x<=end
*/

enum TYPE
{
    NONE,
    DOUBLE,
    STRING,
    TIME
};

class AbstractColumn
{
public:
    AbstractColumn(std::string name, TYPE t, int startIndex)
    {
        this->ColumnName = name;
        type = t;
        start = startIndex;
        end = startIndex;
    }

    virtual ~AbstractColumn() {};

    std::string GetName()
    {
        return this->ColumnName;
    }

    virtual TYPE GetType() { return type;}
    virtual void AddItem(TYPE t, char* data, size_t size, int key) = 0;
    virtual bool GetData(int pos, unsigned char** data, size_t& size) = 0;
    virtual int GetSize() = 0;
    virtual void PrintItem(int item, std::ostream& stream) = 0;
    virtual void Shrink() = 0;
    virtual std::vector<int> GetRange(int64_t start, int64_t end) = 0;
    virtual std::vector<std::pair<int, int>> GetIndexes(std::vector<int>& range, char* key, size_t size) = 0;
    virtual std::vector<int> GetIndexesSingle(std::vector<int>& range, char* key, size_t size) = 0;
    virtual std::vector<int> GetAllIndexesInRange(std::vector<int>& range) = 0;
    virtual void GetUnionOfIndexes(std::vector<int>& indexesToBeFiltered) = 0;
    virtual void FilterIndexes(std::vector<int>& indexesToBeFiltered, char* key, size_t size) = 0;
    virtual void GetSpecificData(std::vector<int>& range, std::vector<double>& data) = 0;

    virtual size_t GetNumberOfIndexes() = 0;
     

private:
    std::string ColumnName;
    TYPE type;
    int start;
    int end;
};
#endif