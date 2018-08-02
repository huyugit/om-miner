#include "StreamWriter.h"

#include "base/VarArgs.h"


StreamWriter::StreamWriter(FILE *_stream)
    : stream(_stream)
{}

void StreamWriter::printf(const char* format, ...)
{
    VarArgs arglist;
    VA_START(arglist, format);
    vfprintf(stream, format, arglist);
}
