#include "StringColumn.h"

/*
    Instead of having <int,std::string> which will blow up the size of a string column
    I opted for a "quick" compression scheme.
    Each string has an integer attached to it, whenever we encounter a new string, assign it a new int.
    This way we just have pairs of <int,int>
*/
void StringColumn::AddItem(TYPE t, char* item, size_t size, int key)
{
    std::string stringItem = std::string(item, size);
    int compId = 0;
    if (compressed.count(stringItem))
        compId = compressed[stringItem];
    else
    {
        compId = compressedId;
        compressed[stringItem] = compressedId;
        compressedId++;
    }
    items.emplace_back(std::make_pair(key, compId));
    end=key;
}

bool StringColumn::GetData(int index, unsigned char** data, size_t& size)
{
    return false;
}

void StringColumn::PrintItem(int index, std::ostream& stream)
{
    //stream << this->items[index].second << std::endl;
}

/*
    This should be implemented for all types of columns but right now the only 2 types of commands filter by string.
    Right now the expected behaviour is this:

    First we get the time range as 2 indexes (inclusive).
    Then we feed it to this function that goes from i = index[0] to i <= index[1]
    For each item i that shares the string value push it into a vector.
    That vector represents all indexes that have the symbol *key.
 */
std::vector<int> StringColumn::GetIndexesSingle(std::vector<int>& range, char* key, size_t size)
{
    std::string stringItem = std::string(key, size);
    int compressedKey = 0;
    std::vector<int> indexes;
    if (compressed.count(stringItem))
    {
        compressedKey = compressed[stringItem];
    }
    else
    {
        std::cout << " Asking for key that doesn't exist !" << std::endl;
        return indexes;
    }

    int start = range[0];
    int end = range[1];

    for (int i = start; i <= end; i++)
    {
        if (items[i].second == compressedKey)
        {
            indexes.push_back(items[i].first);
        }
    }
    return indexes;
}


/*
    This was my second idea which I'm not using right now because it turns out it's not very efficient (in the cases I generated).
    The idea is that if the indexes are often clustered together, instead of generating a vector of indexes, we should generate a vector of index-ranges.
    This way it reduces the overall size of the vector.

    The problem is that unless we know ahead of time that there are very few Symbols, then this will often actually blow up the size of the index vector since there are very few cases where we can generate large ranges.
*/
std::vector<std::pair<int, int>> StringColumn::GetIndexes(std::vector<int>& range, char* key, size_t size)
{
    std::string stringItem = std::string(key, size);
    int compressedKey = 0;
    std::vector<std::pair<int, int>> indexes;
    if (compressed.count(stringItem))
    {
        compressedKey = compressed[stringItem];
    }
    else
    {
        std::cout << " Asking for key that doesn't exist !" << std::endl;
        return indexes;
    }

    int start = range[0];
    int end = range[1];

    int rangeStart = -1;
    int rangeEnd = -1;

    for (int i = start; i <= end; i++)
    {
        if (items[i].second == compressedKey)
        {
            if (rangeStart == -1)
            {
                rangeStart = items[i].first;
                rangeEnd = rangeStart;
            }
            else
            {
                rangeEnd=items[i].first;
            }
        }
        else
        {
            if(rangeStart != -1 )
                indexes.push_back(std::make_pair(rangeStart, rangeEnd));
            rangeStart = -1;
            rangeEnd = -1;
        }
    }
    return indexes;
}

void StringColumn::FilterIndexes(std::vector<int>&range, char* key, size_t size)
{
    std::string stringItem = std::string(key, size);
    int compressedKey = 0;
    std::vector<int> indexes;
    if (compressed.count(stringItem))
    {
        compressedKey = compressed[stringItem];
    }
    else
    {
        std::cout << " Asking for key that doesn't exist !" << std::endl;
        return;
    }
    for (auto & i : range)
    {
        if (items[i].second == compressedKey)
            indexes.emplace_back(i);
    }
    range = indexes;
}