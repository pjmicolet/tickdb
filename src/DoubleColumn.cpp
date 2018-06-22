#include "DoubleColumn.h"
#include <algorithm>


/*
    Storing data as a native double can be an issue if the file is mainly composed of values with less than 7 digits, as the character representation will be more compact.
    However, if they were stored as char, then any comparison operations would require multiple conversions; which would slow down computation.
    Had I more time, I'd implement some form of column compression to ensure that size isn't an issue.
*/
void DoubleColumn::AddItem(TYPE t, char* item, size_t size, int key)
{
    double* dItem = (double*)item;
    this->items.emplace_back(std::make_pair(key, *dItem));
    end=key;
}

bool BoundComparison(const std::pair<int, double> & a, const std::pair<int, double> & b)
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
    I can use binary_search and lower_bound because the indexes are inserted into the column in a sorted order.
*/
bool DoubleColumn::GetData(int index, unsigned char** data, size_t& size)
{
    struct BinCmp {
        bool operator()(const std::pair<int, double> &a, const int & b)
        {
            return (a.first < b);
        }
        bool operator()(const int & a, const std::pair<int, double> &b)
        {
            return (a < b.first);
        }
    };

    size = sizeof(double);
    if (index < start || index > end)
    {
        return false;
    }
    else
    {
        if (std::binary_search(items.begin(), items.end(), index, BinCmp()))
        {
            auto find = std::lower_bound(items.begin(), items.end(), std::make_pair(index,0), BoundComparison);
            *data = (unsigned char*)&find->second;
            return true;
        }
    }
    return false;
}

void DoubleColumn::PrintItem(int index, std::ostream& stream)
{
    stream << this->items[index].second << std::endl;
}

std::vector<int> DoubleColumn::GetAllIndexesInRange(std::vector<int>& range)
{
    std::vector<int> indexes;

    auto find1 = std::lower_bound(items.begin(), items.end(), std::make_pair(range[0], 0), BoundComparison);
    
    if (find1 == items.cend())
    {
        return indexes;
    }
    
    auto find2 = std::upper_bound(items.begin(), items.end(), std::make_pair(range[1], 0), BoundComparison);
   
    if (find2 != items.begin())
        find2--;

    for (std::vector<std::pair<int, double>>::iterator it = find1; it <= find2; it++)
    {
        indexes.emplace_back(it->first);
    }
    return indexes;
}

void DoubleColumn::GetUnionOfIndexes(std::vector<int> & currentIndexes)
{
    auto find1 = std::lower_bound(items.begin(), items.end(), std::make_pair(currentIndexes[0], 0), BoundComparison);
    
    int i = 0;
    std::vector<int> unionVector;

    if (find1 == items.cend())
    {
        currentIndexes = unionVector;
        return;
    }
    /*
        This should be O(M+N)
    */
    while (i < (int)currentIndexes.size() && find1 != items.end())
    {
        if (currentIndexes[i] == find1->first)
        {
            unionVector.push_back(currentIndexes[i]);
            i++;
            find1++;
        }
        else if (currentIndexes[i] < find1->first)
        {
            i++;
        }
        else if (find1->first < currentIndexes[i])
        {
            find1++;
        }
    }
    currentIndexes = unionVector;
}


/*
    This function tries to fetch all the items from the range vector in O(n) time (n being the number of items in range).
    The way it works is: find the index of the first item in range using lower_bound (log(m) for m items in column).
    Then find the index of the last item which is also log(m).
    This gives us a start and end index for the column.

    Then we iterate through the rest of the range vector and column to find items that match.
    When items match it means we found the correct ID in the column and we can fetch the double.

    Given index COL_INDEX for the current index of the column and RANGE_INDEX for the current index of the range vector:
    If column[COL_INDEX] == range[RANGE_INDEX] then we can fetch the double of column and push it into our data vector.
    If column[COL_INDEX] < range[RANGE_INDEX] then we know that at best we will need to iterate through range[RANGE_INDEX]-column[COL_INDEX] items before getting a matching ID.
    This is due to the fact that both vectors are monotonically increasing, and both hold unique values for their IDs, we know that, at most we will need to iterate through:
    To speed up this process we try to get as close to the index as possible by iterating through column exponentially (I'll point that in the code).
    This process is to find the smallest distance between iterator_start and iterator_end for the column vector that will hold the value in range[RANGE_INDEX].
    As long as we do not find a value in column that is larger than the range[RANGE_INDEX] then we can move the iterator_start up by an exponential value (currently we keep multiplying by 2).
    When we do hit an ID in the column that is larger than the range[RANGE_INDEX] we stop iterating as we have found where we should stop the search and we set the iterator_end.

    As we now have a smaller range, we can use lower_bound(iterator_start,iterator_end,range[RANGE_INDEX]) to find the double we are looking for.
    We then push the item in the data vector and go to the next index in range and column.

    The idea was taken from here:
    https://stackoverflow.com/questions/36669297/most-efficient-way-to-find-index-of-matching-values-in-two-sorted-arrays-using-c

    I intiially implemented something like this

    for(int i = 1; i < range.size(); i++)
    {
        while(items[currentIndex] < range[i] && currentIndex <=end)
            currentIndex++;

        data.push_back(items[currentIndex].second);

    }

    But felt that the while loop could go faster especially if the data in range is very sparse compared to the data in items.
    Since I know the data is presorted and monotonic I spent a while looking for more efficient ways of doing this, which is how i fell on the stack overflow thread and how i ended up using this idea.
    On non-sparse data the difference should be minimal, in fact, the commented code above could potentially be more lightweight.

*/
void DoubleColumn::GetSpecificData(std::vector<int>& range, std::vector<double>& data)
{
    int index = range[0];
    int currentIndex = 0;
    std::vector<std::pair<int, double>>::iterator find = items.begin();

    struct BinCmp {
        bool operator()(const std::pair<int, double> &a, const int & b)
        {
            return (a.first < b);
        }
        bool operator()(const int & a, const std::pair<int, double> &b)
        {
            return (a < b.first);
        }
    };

    if (index < start || index > end)
    {
        return;
    }
    else
    {
        //Start by looking for the first item in range and push the double from column.
        if (std::binary_search(items.begin(), items.end(), index, BinCmp()))
        {
            find = std::lower_bound(items.begin(), items.end(), std::make_pair(index, 0), BoundComparison);
            data.push_back(find->second);
            currentIndex = find - items.begin();
        }
    }
    
    if (currentIndex >(int) items.size())
        return;
    auto iter = items.begin();

    auto iterStart = items.begin();
    
    auto iterEnd = items.end();
    
    //We use this to find the last item from range in the column.
    //It gives us the last index of the column
    //We don't push the double yet because that would break the dot product ordering.
    auto find_end = std::lower_bound(find, items.end(), std::make_pair(range.back(), 0), BoundComparison);
    double lastVal = find_end->second;
    int end = find_end - items.begin();

    currentIndex++;

    for (int i = 1; i < (int)range.size()-1; i++)
    {
        if (items[currentIndex].first == range[i])
        {
            data.push_back(items[currentIndex].second);
            currentIndex++;
        }
        else if (items[currentIndex].first < range[i])
        {

            
            int diff = range[i] - items[currentIndex].first;
            int distance = (end - currentIndex) + 1;
            int smallest = diff < (distance) ? diff : (distance);

            iterStart = std::next(iter, currentIndex);
            iterEnd = std::next(iterStart, smallest);
            auto iterStartX = iterStart;

            //Try and make the iterators as close together as possible.
            for (int ii = 1; ii < smallest; ii *= 2)
            {
                auto iterNew = std::next(iterStart, ii);
                if ((*iterNew).first >= range[i])
                {
                    iterEnd = iterNew;
                    break;
                }
                iterStartX = iterNew;
            }

            //We can now find the item using lower_bound on a much smaller dataset.
            auto find2 = std::lower_bound(iterStartX, iterEnd, std::make_pair(range[i], 0), BoundComparison);

            currentIndex = find2 - items.begin();
            data.push_back(items[currentIndex].second);

            currentIndex++;
        }
    }
    data.push_back(lastVal);
}
