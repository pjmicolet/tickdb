#ifndef _H_PRODUCT_H_
#define _H_PRODUCT_H_

#include "query_commands.h"
#include "table.h"
#include <vector>
#include <string>
#include <fstream>

class Product : public Command
{
public:
    Product(Table* table, bool optimised=false)
    {
        this->optimised = optimised;
        this->Name = "Produce";
        this->inputFields = 5;
        this->table = table;
    }

public:
    void Parse(std::vector<std::string>);
    void Execute(std::vector<std::string>);

public:
    std::string Name;

private:
    int inputFields;
    Table* table;
    bool optimised;
};

#endif