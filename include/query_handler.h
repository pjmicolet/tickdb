#ifndef _H_QUERYHANDLER_
#define _H_QUERYHANDLER_

#include <map>
#include <fstream>
#include "query_commands.h"
#include "print.h"
#include "product.h"
#include "tickfile.h"
#include "table.h"
#include "printCache.h"
#include "productCache.h"

class QueryHandler
{
public:
    QueryHandler(bool optimisedForPrint, bool optimisedForProduct)
    {
        //Let's start off with 1000 potential items in the cache.
        //Once we load the file we can do an estimate as to how many items we could potentially have later on
        if (optimisedForPrint)
            PCache = new PrintCache(1000);
        else
            PCache = NULL;
        if (optimisedForProduct)
            ProdCache = new ProductCache(1000);
        else
            ProdCache = NULL;

        table = new Table(PCache,ProdCache,optimisedForPrint,optimisedForProduct);
        this->printOpt = optimisedForPrint;
        this->printProd = optimisedForProduct;
        Print* printCommand = new Print(table,optimisedForPrint);
        Product* produceCommand = new Product(table,optimisedForProduct);
        Tickfile* tickfileCommand = new Tickfile(table);
        command["print"] = printCommand;
        command["product"] = produceCommand;
        command["tickfile"] = tickfileCommand;
    }
    ~QueryHandler()
    {
        delete table;
        for (auto i : command)
        {
            delete i.second;
        }
    }
public:
    void StartParsing();

private:        
    Table * table;
    bool printOpt;
    bool printProd;
    PrintCache* PCache;
    ProductCache* ProdCache;
    std::map<std::string, Command*> command;

};

#endif