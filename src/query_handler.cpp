#include "query_handler.h"
#include <iostream>
#include<string>
#include "utils.h"

void QueryHandler::StartParsing()
{
    std::string test = "";
    std::vector<std::string> parsed;
    while (true)
    {
        std::getline(std::cin, test);
        if (test.size() < 1)
            continue;

        parsed = split(test, ' ');
        if (command.count(parsed[0]))
        {
            Command* cmd = command[parsed[0]];
            cmd->Parse(parsed);
        }
        else
        {
            if (parsed[0] == "Exit")
                return;
            std::cout << "This command doesn't exist " << parsed[0] << std::endl;
        }

        test = "";
    }
}