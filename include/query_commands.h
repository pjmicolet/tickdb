#ifndef _H_QCOMMAND_H_
#define _H_QCOMMAND_H_

#include <string>
#include <vector>

//Abstract class for command
//Needs a name, number of inputs and what it does

class Command
{
public:
    Command(){ }
    virtual ~Command() {};
    virtual void Execute(std::vector<std::string>) = 0;
    virtual void Parse(std::vector<std::string>) = 0;

};

#endif