#ifndef _H_TICKFILE_H_
#define _H_TICKFILE_H_

#include "query_commands.h"
#include "table.h"
#include <iostream>
#include <fstream>

class Tickfile : public Command
{
public:
    Tickfile(Table* table)
    {
        this->Name = "Tickfile";
        this->inputFields = 1;
        this->table = table;
    }

    void Parse(std::vector<std::string>);
    void Execute(std::vector<std::string>);

private:
    int inputFields;
    Table* table;

public:
    std::string Name;
};

#endif