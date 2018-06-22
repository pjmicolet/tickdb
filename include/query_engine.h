#include "abstract_column.h"
#include <string>
#include <vector>
#include <utility>

/*
    This header implements some utilities for the table class and any other class that can operate on AbstractColumns
*/

static std::vector<int> getIndexRange(AbstractColumn* A, double start, double end)
{
    std::vector<int>indexes = A->GetRange(start, end);
    return indexes;
}

//Initially I just thought of sticking to string keys, but who knows, someone could want to use another type of key that isn't string.
//This way it's up to the specific type of column to implement what their key is.
static std::vector<std::pair<int, int>> getIndexes(AbstractColumn* A, std::vector<int> indexRange, char* key, size_t size)
{
    return A->GetIndexes(indexRange, key, size);
}

static std::vector<int> getIndexesSingle(AbstractColumn* A, std::vector<int>& indexRange, char* key, size_t size)
{
    return A->GetIndexesSingle(indexRange, key, size);
}

static std::vector<int> getAllIndexes(AbstractColumn *A, std::vector<int>& indexRange)
{
    return A->GetAllIndexesInRange(indexRange);
}

static void getUnion(AbstractColumn *A, std::vector<int>& indexRange)
{
    A->GetUnionOfIndexes(indexRange);
}

static void filterIndexes(AbstractColumn *A, std::vector<int>& indexRange, char* key, size_t size)
{
    A->FilterIndexes(indexRange,key,size);
}