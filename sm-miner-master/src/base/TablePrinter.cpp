#include "TablePrinter.h"

#include <cstdio>
#include <cstring>

#include "base/MinMax.h"
#include "base/VarArgs.h"
#include "base/BaseUtil.h"

#include "sys/writer/StreamWriter.h"

using namespace std;

TablePrinter::TablePrinter()
    : rightAlignment(true)
{}

void TablePrinter::writeCell(const char* format, ...)
{
    VarArgs arglist;
    VA_START(arglist, format);

    char buffer[MAX_CELL_SIZE];
    vsnprintf(buffer, sizeof(buffer), format, arglist);

    // start new line if table is empty
    if (cells.empty()) cells.push_back(VS());

    // add new cell to the last row
    VS &row = cells.back();
    row.push_back( string(buffer) );

    size_t colIndex = row.size() - 1;
    if (colSize.size() < colIndex + 1)
    {
        colSize.resize(colIndex + 1);
    }

    colSize[colIndex] = std::max(colSize[colIndex], row[colIndex].length());
}

void TablePrinter::newLine()
{
    cells.push_back(VS());
}

void TablePrinter::printTable()
{
    StreamWriter wr(stdout);
    printTable(wr);
}

const char* g_spaces[] = {
    "",
    " ",
    "  ",
    "   ",
    "    ",
    "     ",
    "      ",
    "       ",
    "        ",
    "         ",
    "          ",
    "           ",
    "            ",
    "             ",
    "              ",
    "               ",
    "                ",
    "                 ",
    "                  ",
};

void TablePrinter::printTable(Writer &wr)
{
    for (size_t rowIndex = 0; rowIndex < cells.size(); rowIndex++)
    {
        VS &row = cells[rowIndex];
        for (size_t colIndex = 0; colIndex < row.size(); colIndex++)
        {
            if (colIndex > 0) wr.printf("  ");

            const char *s = row[colIndex].c_str();

            if (rightAlignment)
            {
                char f[16];
                snprintf(f, sizeof(f), "%%%ds", colSize[colIndex]);

                wr.printf(f, s);
            }
            else {
                wr.printf("%s", s);

                int l = colSize[colIndex] - strlen(s);
                if (l > 0)
                {
                    if (l >= util::arrayLength(g_spaces))
                    {
                        l = util::arrayLength(g_spaces) - 1;
                    }

                    wr.printf(g_spaces[l]);
                }
            }
        }
        wr.printf("\n");
    }
}
