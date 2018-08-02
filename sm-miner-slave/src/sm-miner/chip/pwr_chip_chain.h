#ifndef PWR_CHIP_CHAIN_H
#define PWR_CHIP_CHAIN_H

#include "chain_base.h"
#include "pwr_chip.h"
#include "pwr_chip_packet.h"
#include "ms_defines.h"
#include "stm_multi_spi.h"


class PwrChipChain : public ChainBase
{
public:
    PwrChipChain();

    void init(uint8_t _spiId, uint8_t _spiLen,
              uint8_t _boardId, uint8_t _boardSpiId);

    PwrChip& getChip(uint8_t seq);

    inline uint8_t getBoardId() const { return boardId; }
    inline uint8_t getBoardSpiId() const { return boardSpiId; }

    void setupChain();
    void testChain();
    void memTest();

    static void broadcastMinerImage();
    static void broadcastMemTestImage();
    static void broadcastBtcTestImage();

    static void broadcastImage(const void *imageBegin, const void *imageEnd,
                               bool conditional, int imageAttempts);

    static void broadcastTime();
    static void broadcastCmnConfig(bool changed);
	static void broadcastCmnConfig(bool changed , int spiId);  //chenbo add 20180109
    static void broadcastJob(bool changed);

    static void broadcastCpuDisable(int attempts);
    static void broadcastCpuEnable(int attempts);

    void downloadNonces();

    void downloadTestStats();
    void serviceNextLevel();

    bool chainSetupDone;
    bool debugOn;

private:
    static const uint8_t ALL_SPI;

    uint8_t boardId, boardSpiId;
    uint8_t serviceSeq;

    PwcReadResult downloadNoncesFrom(uint8_t parity);

    void loadConfigTo(uint8_t spiSeq);
    void downloadTestStatsFrom(uint8_t spiSeq);
    void downloadStatsFrom(uint8_t spiSeq);
};

#endif // PWR_CHIP_CHAIN_H
