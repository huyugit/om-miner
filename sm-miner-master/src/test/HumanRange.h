#ifndef HUMANRANGE_H
#define HUMANRANGE_H

#include <stdint.h>
#include <vector>
#include "base/StringBuffer.h"

class HumanRange
{
public:
    // NOTE: push in ASCENDANT order!
    void push(int x);

    uint32_t size() const { return ranges.size(); }

private:
    struct Range
    {
        Range(int _x1, int _x2) : x1(_x1), x2(_x2) {}
        int x1, x2;
    };

    typedef std::vector<Range> VRange;
    VRange ranges;

public:
    template<size_t CAPACITY>
    void printTo(StringBuffer<CAPACITY> &str) const
    {
        size_t n = ranges.size();
        const Range &head = ranges.front();

        if (n == 0)
        {
            str.printf("<none>");
        }
        else
        {
            bool shortForm = (n == 1 && head.x1 == head.x2);

            if (!shortForm) {
                str.printf("(");
            }

            for (VRange::const_iterator it = ranges.begin(); it != ranges.end(); it++)
            {
                if (it != ranges.begin())
                {
                    str.printf(",");
                }

                const Range &r = *it;
                if (r.x1 != r.x2)
                    str.printf("%d-%d", r.x1, r.x2);
                else
                    str.printf("%d", r.x1);
            }

            if (!shortForm) {
                str.printf(")");
            }
        }
    }
};

#endif // HUMANRANGE_H
