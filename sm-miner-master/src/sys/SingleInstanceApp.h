#ifndef SINGLEINSTANCEAPP_H
#define SINGLEINSTANCEAPP_H

class SingleInstanceApp
{
public:
    static void lock(const char *fileName);
};

#endif // SINGLEINSTANCEAPP_H
