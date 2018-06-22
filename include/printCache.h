#ifndef _H_PRINT_CACHE_H_
#define _H_PRINT_CACHE_H_

#include<utility>
#include<algorithm>
#include<unordered_map>
#include<string>
#include<list>

/*
    To optimise doing multiple prints I've chosen to implement an LRU cache.
    When doing a search we use the row's index as the key, and ping the cache before reconstructing the line.
    If we have it, then GetString returns true and we simply add it to the buffer.
    Else we have to reconstruct the line and put it in the cache.
*/

/*
    At a later data I should make an abstract class that implements caches for different commands;
    That way I can optimise for any new query.
*/

class PrintCache
{
public:
    PrintCache(int size)
    {
        this->size = size;
    }
    ~PrintCache()
    {
        LRU.clear();
        cache.clear();
    }

    bool GetString(int key, std::string* string)
    {
        if (cache.count(key))
        {
            *string = cache[key].first;
            LRU.splice(LRU.begin(), LRU, cache[key].second);
            cache[key].second = LRU.begin();
            return true;
        }
        return false;
    }

    void Put(int key, std::string string)
    {
        if (cache.count(key) == 0)
        {
            if ((int)cache.size() == size)
            {
                cache.erase(GetLRU());
            }
            cache[key] = std::make_pair(string, UpdateLRU(key));

        }
        else
        {
            cache[key].first = string;
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


private:
    int size;
    std::unordered_map<int, std::pair<std::string,std::list<int>::const_iterator>> cache;
    std::list<int> LRU;
};

#endif