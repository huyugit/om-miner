#ifndef JSONSTAT_H
#define JSONSTAT_H

class JsonGenerator;


class JsonStat
{
public:
    static void exportStat();

private:
    static void exportMuxes(JsonGenerator &gen);
    static void exportSlaves(JsonGenerator &gen, int mid);
};

#endif // JSONSTAT_H
