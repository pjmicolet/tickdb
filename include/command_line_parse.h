#ifndef _M_CMD_LN_P_
#define _M_CMD_LN_P_

#include<string>
#include<iostream>

void parseCommandLine(std::string commandLine, bool& forPrint, bool& forProduct)
{
    if (commandLine == "-Oprint")
    {
        forPrint = true;
    }
    else if (commandLine == "-Oproduct")
    {
        forProduct = true;
    }
    else
    {
        std::cerr << "Flag " << commandLine << " is not recognised " << std::endl;
        exit(2);
    }
}

#endif