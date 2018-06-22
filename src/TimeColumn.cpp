#include "TimeColumn.h"
#include <algorithm>
#include <iterator>
#include <algorithm>

void TimeColumn::AddItem(TYPE t, char* item, size_t size, int key)
{
    time_t* time = (time_t*)item;
    this->items.emplace_back(std::make_pair(key, *time));
    end=key;
}

bool BoundComparison2(const std::pair<int, time_t> & a, const std::pair<int, time_t> & b)
{
    return a.first < b.first;
}

/*
Given an index, see if the column has it.
If it does, point to it using the char* and return true;
If not return false.

In order to compress columns; I don't use virtual-ids, which is why each column has a pair<int,type>
This allows me to save on space, especially in cases where the TypeX column is rarely declared in the original file.
It also means that I could potentially apply more compression to the column in case I need space.

The only downside is that now I have to check for the index whenever an item is needed.
Using binary_search + lower_bound should find the item in O(log(n)) time.

*/
bool TimeColumn::GetData(int index, unsigned char** data, size_t& size)
{
    struct BinCmp {
        bool operator()(const std::pair<int, time_t> &a, const int & b)
        {
            return (a.first < b);
        }
        bool operator()(const int & a, const std::pair<int, time_t> &b)
        {
            return (a < b.first);
        }
    };

    size = sizeof(time_t);
    if (index < start || index > end)
        return false;
    else
    {
        if (std::binary_search(items.begin(), items.end(), index, BinCmp()))
        {
            auto find = std::lower_bound(items.begin(), items.end(), std::make_pair(index, 0), BoundComparison2);
            *data = (unsigned char*)&find->second;
            return true;
        }
    }
    return false;
}

void TimeColumn::PrintItem(int index, std::ostream& stream)
{
    stream << this->items[index].second << std::endl;
}


bool BoundComparison(const std::pair<int, time_t> & a, const std::pair<int, time_t> & b)
{
    return a.second < b.second;
}

//I assume that time_t is a 32bit int for now
//Use std::lower_bound and std::higher_bound to get the range index
/*
    I can get the bounds using std::lower_bound and std::upper_bound easily because the Time column is sorted (since the file is also sorted).
*/
std::vector<int> TimeColumn::GetRange(int64_t start, int64_t end)
{
    std::vector<int> range;
    time_t convertedStart = (time_t)start;
    time_t convertedEnd = (time_t)end;
    int startIndex = -1;
    int endIndex = -1;
   
    auto lowerIndex = std::lower_bound(items.begin(), items.end(), std::make_pair(0, convertedStart), BoundComparison);
    auto highIndex = std::upper_bound(items.begin(), items.end(), std::make_pair(0, convertedEnd), BoundComparison);

    if(lowerIndex != items.cend())
        startIndex = lowerIndex->first;

    /* For upper bound I have a small trick that i do just in case someone asks for values that don't exist in the file
       If we have
       1,Symbol,...
       2,Symbol,...

       and someone asks for print 0 0 Symbol then we need to check that if the highIndex is the first item in the list then its value is equal to the requested end range
    */
    if (highIndex == items.cend())
        endIndex = items.back().first;
    else if (highIndex == items.begin())
    {
        if(highIndex->second == end)
            endIndex = (highIndex)->first;
    }
    else
        endIndex = (highIndex-1)->first;

    range.push_back(startIndex);
    range.push_back(endIndex);
    return range;
}