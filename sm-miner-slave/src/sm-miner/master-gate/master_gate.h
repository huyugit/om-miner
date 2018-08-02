#ifndef MASTER_GATE_H
#define MASTER_GATE_H

#include <stdint.h>
#include "format.hpp"
#include "ms_data.h"
#include "ms_packet.h"
#include "pwc_block.h"
#include "round_buffer.h"


class PwrChip;

class MasterGate
{
public:
    MasterGate();

    static inline uint32_t getTxRxBuffersSize() {
        return sizeof(txBuffer) + sizeof(rxBuffer);
    }

    void initSpi();
    void checkDataTransfer();

    void sendNonce(const PwcNonceData &nd);

    void sendPwcData(PwrChip &chip, const PwcBlock &pwcData);
    void sendPwcData(PwrChip &chip, const PwcBlock &pwcData, uint8_t index);

    uint32_t getPingTime();

    void emulateOnMiningData();

    SlaveHbtData slaveHbtInfo;
    SlaveTestInfo testInfo;
    SlaveSpiStat spiStat;

    uint32_t txId;
    uint32_t lastRxTime;

    MasterData masterData;
    PwcMiningData miningData;

    NonceContainer foundNoncesContainer;
    RoundBuffer<PwcNonceData> foundNonces;

    PwcDataArray pwcDataArray;

private:
    uint8_t txBuffer[MS_FRAME_SIZE];
    uint8_t rxBuffer[MS_FRAME_SIZE];

    void onMasterBlockReceived(MsPacket &packet);

    bool processRx(MsPacket &packet);
    void fillTx(MsPacket &packet);

    void processMsg(MasterData &md);
    void processMsg(PsuConfig &pc);
    void processMsg(PwcMiningData &md);

    void pushSlaveData(MsPacket &packet);
    void pushSlaveTestData(MsPacket &packet);
    void fillPsuInfo(PsuInfoArray &pia);
    void pushPwcQuickTestArr(MsPacket &packet);
    void fillPwcSpiArray(PwcSpiArray &arr);

	/* lxj add begin 20180328 */
	void fillSlaveErrorArray(SlaveErrorArray &arr);	
	/* lxj add end */
};

extern MasterGate g_masterGate;
extern PsuConfig g_psuConfig;

/* lxj add begin 20180328 */
extern SlaveErrorArray g_slaveError;
/* lxj add end */



#endif // MASTER_GATE_H
