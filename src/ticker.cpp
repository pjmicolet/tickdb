#include "command_line_parse.h"
#include "query_handler.h"

#include <iostream>
#include <string>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

void early_exit(int S)
{
    exit(1);
}

int main(int argc, char**argv)
{
    //Check for command line optimisations
    bool optimisedForPrint = false;
    bool optimisedForProduct = false;

    if (argc > 2)
    {
        std::cerr << " I thought only one optmisation could be passed through" << std::endl;
        exit(2);
    }
    if (argc > 1)
    {
        std::string line = argv[1];
        parseCommandLine(line, optimisedForPrint, optimisedForProduct);
    }

    QueryHandler* qh = new QueryHandler(optimisedForPrint, optimisedForProduct);
    signal(SIGINT, &early_exit);
    qh->StartParsing();

    return 0;
}