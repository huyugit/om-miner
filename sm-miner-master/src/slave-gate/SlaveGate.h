#ifndef SLAVEGATE_H
#define SLAVEGATE_H

#include "base/NonCopyable.h"
#include "base/PollTimer.h"
#include "cmn_block.h"
#include "ms-protocol/ms_data.h"


class SlaveGate
        : private NonCopyable  // Prevent copy and assignment.
{
public:
    SlaveGate();

    int currentSlaveId;

    uint32_t numTx, numTxMs;

    void init();
    void runPollingIteration();

private:
    void runTxRx();

    void prepareTxBuffer(int slaveId);
    bool processResponse(int slaveId);

    void processMsg(SlaveData &msg);
    void processMsg(SlaveHbtData &msg);
    void processMsg(NonceContainer &noncesContainer);
    void processMsg(PsuInfoArray &psuInfoArray);
    void processMsg(PsuSpec &psuSpec);
    void processMsg(PwcQuickTestArr &arr);
    void processMsg(PwcDataArray &pwcDataArray);
    void processMsg(PwcSpiArray &pwcSpiArray);

	/* lxj add begin 20180328 */
	void processMsg(SlaveErrorArray &slaveErrorArray);
	/* lxj add end */

};

extern SlaveGate g_slaveGate;

#endif // SLAVEGATE_H
