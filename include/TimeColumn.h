#ifndef _H_TIMECOLUMN_C
#define _H_TIMECOLUMN_C

#include "abstract_column.h"
#include <string>
#include <vector>
#include <time.h>
#include <utility>
#include <inttypes.h>

/*
The columns do not use virtual IDs, instead the ID is the index of the row from the tickfile.
Whilst this increases the overall size of the column, it technically allows for a quick method of saving space when the column does not contain data from all rows.
*/
class TimeColumn : public AbstractColumn
{
public:

    virtual TYPE GetType() { return type; }
    virtual void AddItem(TYPE t, char* data, size_t size, int key);
    virtual bool GetData(int pos, unsigned char** data, size_t& size);
    virtual int GetSize()  { return (items.size() * sizeof(std::pair<int,time_t>))+sizeof(std::vector<std::pair<int,time_t>>); };
    virtual void PrintItem(int item, std::ostream& stream);
    virtual void Shrink() { items.shrink_to_fit(); }
    virtual std::vector<int> GetRange(int64_t start, int64_t end);
    virtual std::vector<std::pair<int, int>> GetIndexes(std::vector<int>& range, char* key,size_t size) { std::vector<std::pair<int, int>> a;  std::cout << "not now" << std::endl; return a; };
    virtual std::vector<int> GetIndexesSingle(std::vector<int>& range, char* key, size_t size) { std::vector<int> a;  std::cout << "not now" << std::endl; return a; };
    virtual std::vector<int> GetAllIndexesInRange(std::vector<int>& range) { std::vector<int> a; std::cout << "not now" << std::endl; return a; };
    virtual void GetUnionOfIndexes(std::vector<int>& indexesToBeFiltered) { std::vector<int> a; std::cout << "not now" << std::endl;  };
    virtual void FilterIndexes(std::vector<int>& indexesToBeFiltered, char* key, size_t size) { std::vector<int> a; std::cout << "not now" << std::endl; };
    virtual void GetSpecificData(std::vector<int>& range, std::vector<double>& data) {};

    virtual size_t GetNumberOfIndexes() { return items.size(); }
    

    TimeColumn(std::string name, TYPE t, int start) : AbstractColumn(name, t, start) { type = t; this->start = start; this->end = start; };
    ~TimeColumn() { items.clear(); };

private:
    std::vector<std::pair<int, time_t>> items;
    TYPE type;
    int start;
    int end;

};
#endif