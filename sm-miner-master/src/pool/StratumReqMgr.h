#ifndef STRATUMREQMGR_H
#define STRATUMREQMGR_H

#include <map>
#include "sys/Mutex.h"
#include "pool/StratumShare.h"

class StratumReqMgr
{
public:
    StratumReqMgr();

    bool isDuplicate(const StratumShare& share);
    void shareSent(int reqId, const StratumShare& share);

    void remove(int reqId);
    bool getShare(int reqId, StratumShare &share, int &age);

private:
    Mutex m_mutex;

    struct ReqRecord {
        int time;
        StratumShare share;
    };

    typedef std::map<int, ReqRecord> RecordsMap;
    RecordsMap records;

    int getSeconds();
    void clenupOldRecords();
};

#endif // STRATUMREQMGR_H
