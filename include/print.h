#ifndef _H_PRINT_H_
#define _H_PRINT_H_

#include "query_commands.h"
#include "table.h"
#include<vector>
#include<string>
#include<fstream>

class Print : public Command
{
public:
    Print(Table* table, bool optimised=false)
    {
        this->optimised = optimised;
        this->table = table;
        this->Name = "Print";
        this->inputFields = 3;
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