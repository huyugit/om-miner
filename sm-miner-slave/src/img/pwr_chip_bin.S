
.section ".pwr_chip_bin"
.balign 4


.global g_pwr_chip_bin
g_pwr_chip_bin:
.incbin "sm-miner-pwc.bin"

.global g_end_pwr_chip_bin
g_end_pwr_chip_bin:
.word 0x12345678


.global g_pwr_chip_mt_bin
g_pwr_chip_mt_bin:
.incbin "sm-miner-pwc-mt.bin"

.global g_end_pwr_chip_mt_bin
g_end_pwr_chip_mt_bin:
.word 0x12345678


.global g_pwr_chip_bt_bin
g_pwr_chip_bt_bin:
.incbin "sm-miner-pwc-bt.bin"

.global g_end_pwr_chip_bt_bin
g_end_pwr_chip_bt_bin:
.word 0x12345678
