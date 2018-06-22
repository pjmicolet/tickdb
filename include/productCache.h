#ifndef _H_PROD_CACHE_H
#define _H_PROD_CACHE_H

#include<utility>
#include<algorithm>
#include<unordered_map>
#include<string>
#include<list>
#include<inttypes.h>
#include<deque>

/*
The ProductCache works in this way:
From a high level we have an unordered_map
[ row index ] [<cache_data, lru_iterator>]

the Cache_data is a map
[ <TypeX,TypeY> ] [< Info >]

The Info contains 2 things
A double [SUM] which represents a particular product and an [END-INDEX].

The [SUM] represents the equivalent of having ran this request:

product [row-index][END-INDEX][TYPEX][TypeY]

*/


class ProductCache
{
public:
    ProductCache(int size)
    {
        this->size = size;
        size2 = 30;
    }
    ~ProductCache()
    {
        LRU.clear();
        cache.clear();
    }
    

    void GetData(int row, int key, int key2, double& val1, double& val2, bool& ind1, bool& ind2)
    {
        if (cache.count(row))
        {
            int indexKey1 = key % size2;
            int indexKey2 = key2 % size2;

            if (cache[row].first[indexKey1].valid && cache[row].first[indexKey1].key == key)
            {
                val1 = cache[row].first[indexKey1].key;
                ind1 = true;
            }
            if (cache[row].first[indexKey2].valid && cache[row].first[indexKey2].key == key2)
            {
                val2 = cache[row].first[indexKey2].key;
                ind2 = true;
            }
            LRU.splice(LRU.begin(), LRU, cache[row].second);
            cache[row].second = LRU.begin();
        }
        return;
    }

    void Put(int key, int sK1, int sK2, double val, double val2)
    {
        int hashK1 = sK1 % size2;
        int hashK2 = sK2 % size2;
        if (cache.count(key) == 0)
        {
            if ((int)cache.size() == size)
            {
                cache.erase(GetLRU());
            }
            Columns a;
            for (int i = 0; i < (int)size2; i++)
                a.push_back(ColumnData());
            cache[key] = std::make_pair(a, UpdateLRU(key));
            cache[key].first[hashK1].valid = true;
            cache[key].first[hashK1].data = val;
            cache[key].first[hashK1].key= sK1;

            cache[key].first[hashK2].valid = true;
            cache[key].first[hashK2].data = val2;
            cache[key].first[hashK2].key = sK2;

        }
        else
        {
            cache[key].first[hashK1].valid = true;
            cache[key].first[hashK1].data = val;
            cache[key].first[hashK1].key = sK1;
            cache[key].first[hashK2].valid = true;
            cache[key].first[hashK2].data = val2;
            cache[key].first[hashK2].key = sK2;
            LRU.splice(LRU.begin(), LRU, cache[key].second);
            cache[key].second = LRU.begin();
        }
    }

    void Resize(size_t size)
    {
        this->size = size;
    }

    void Clear()
    {
        cache.clear();
        LRU.clear();
    }

private:
    
    struct ColumnData
    {
        bool valid;
        int key;
        double data;
        ColumnData(bool v, int k, double d) : valid(v), key(k), data(d) {};
        ColumnData() : valid(false), key(0), data(0) {};
    };
    std::vector<std::vector<ColumnData>> cache2;

    int GetLRU()
    {
        int old = LRU.back();
        LRU.pop_back();
        return old;
    }

    std::list<int>::const_iterator UpdateLRU(int lastIndex)
    {
        LRU.push_front(lastIndex);
        return LRU.begin();
    }

    typedef std::vector<ColumnData> Columns;

    int size;
    size_t size2;
    std::unordered_map<int,std::pair<Columns, std::list<int>::const_iterator>> cache;
    std::list<int> LRU;
};

#endif