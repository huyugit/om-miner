#ifndef TABLEPRINTER_H
#define TABLEPRINTER_H

#include <string>
#include <vector>

#include "sys/writer/Writer.h"

class TablePrinter
{
public:
    TablePrinter();

    void writeCell(const char* format, ...);
    void newLine();

    void printTable();
    void printTable(Writer &wr);

    bool rightAlignment;

private:
    typedef std::vector<std::string> VS;
    typedef std::vector<VS> VVS;

    static const int MAX_CELL_SIZE = 256;

    VVS cells;
    std::vector<size_t> colSize;
};

#endif // TABLEPRINTER_H
