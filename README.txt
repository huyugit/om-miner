---------------------------------------
How to Build
---------------------------------------

- download and install RaspberryPi Toolchain:
http://gnutoolchains.com/raspberry/

- download and install GNU ARM Embedded Toolchain:
https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads

- build OrangePi binary file:
cd sm-miner-master
make -j8

- build STM binary file:
cd sm-miner-slave
make -j8

- download and install ST-LINK Utility:
http://www.st.com/en/embedded-software/stsw-link004.html

- flash STM on MotherBoard:
st-link_cli.exe -c SWD -Q -P sm-miner-slave.bin 0x08000000 -Rst

---------------------------------------
How to Run
---------------------------------------

./sm-miner-master -psu_v 48 -osc 50 -log_chip_stat

---------------------------------------
Explaining CLI
---------------------------------------

Usage (expected arguments):
usage:
  -help                     Display this screen (command line usage)
pool:
  -host <hostname>          Set pool hostname (default: [stratum.slushpool.com])
  -port <port>              Set pool port     (default: [3333])
  -user <username>          Set pool username (default: [swtest.def1024])
  -pswd <password>          Set pool password (default: [1])
  -exit_on_errors           Exit if pool rejects our shares
  -connect_attempts <n>     Exit if cannot connect after <n> attempts (default: 1000000)
  -auth_t <n>               Authorization timeout in seconds (default: 45)
  -subscribe_t <n>          Subscription timeout in seconds (default: 45)
  -start_diff <n>           Initial pool difficulty (default: 1024)
psu:
  -no_psu <x>               No PSU (external power): do not wait for power ack
  -psu_fan_user             PSU FAN: user speed control
  -psu_fan                  PSU FAN: speed in percents (default: 100%)
  -psu_v <v>                PSU: full power voltage (default: 42.0V)
  -psu_min_v <v>            PSU: minimal voltage (default: 42.0V)
  -psu_t <sec>              PSU: on/off delay per unit (default: 5 sec)
  -psu_start_v <v>          PSU: initial voltage (default: 44.0V)
  -psu_start_t <sec>        PSU: initial voltage hold time (default: 1 sec)
  -psu_dv <vp> <vn>         PSU: inc/dec voltage per second (default: 0.1/0.5 V)
  -psu_max_i <i> <di>       PSU: current limit thresholds (default: 70.0 +/- 5.0 A)
psu log:
  -pl_on <n>                PSU log: turn on(1)/off(0) logging (default: 1)
  -pl_all <n>               PSU log: log all messages (default: 0)
  -pl_depth <n>             PSU log: msgs to log before and after event (default: 20)
  -pl_dev <k>               PSU log: deviation from prev status (default: 0.10)
  -pl_vi_min <V>            PSU log: V in min (default: 200.0V)
  -pl_vi_max <V>            PSU log: V in max (default: 260.0V)
  -pl_vo_min <V>            PSU log: V out min (default: 40.0V)
  -pl_vo_max <V>            PSU log: V out max (default: 60.0V)
  -pl_io_min <A>            PSU log: I out min (default: 40.0A)
  -pl_io_max <A>            PSU log: I out max (default: 60.0A)
grid/env:
  -mb_hw_ver <dm>           Use mb hw ver: 0xff-auto, 0xf-6 boards, 0x1-24 boards (default: 255)
  -slave_mask <bin>         Set slave mask (default: 11111111111111111111111111111111)
  -board_mask <bin>         Set board mask (type A + B) (default: 11111111111100000000000000000000)
  -board_mask_b <bin>       Set board mask (type B) (default: 00000000000000000000000000000000)
  -slave_spi_drv <dm>       Master SPI: drv mode 0-hw, 1-sw, 2-auto (default: 2)
  -slave_spi_speed <n>      Master SPI: speed in Hz (default: 2000000)
  -ms_spi_debug             Master-Slave SPI debug mode
  -psu_v <x>                Set PSU voltage (default: 42.0)
  -fan_v <x>                Set FAN voltage (default: 12.0)
stats export/events:
  -alive_event_dt <n>       Report miner-alive event each <n> seconds (default: 3600)
pwc config:
  -spi_t_rst <t0>           SPI timing rst in MCU clycles (default: 2)
  -spi_t_tx <t0/t1/t2>      SPI timing tx time in MCU clycles (default: 2/2/4)
  -spi_t_rx <t0/t1>         SPI timing rx time in MCU clycles (default: 10/30)
  -spi_reset <cycles>       SPI chip reset cycles (default: 512)
  -pwc_clock_cfg <n>        PWC cpu clock cfg (default: 32)
                              cfg/MHz: 0/82, 32/136, 40/165, 42/172
chip:
  -osc <n>                  OSC: static oscillator register (default: 0)
  -jt <n>                   OSC: base job time in ms (default: 100)
  -ao_mode <n>              Auto OSC: mode: 0 - static, 1 - rough osc (default: 0)
  -ro_osc_min <n>           Rough OSC: min osc (default: 46)
  -ro_osc_max <n>           Rough OSC: max osc (default: 52)
  -ro_eff_z0 <n>            Rough OSC: zone 0 efficiency level (to dec osc) (default: 50)
  -ro_eff_z1 <n>            Rough OSC: zone 1 efficiency level (to inc osc) (default: 98)
  -ro_deltas_dec <n>        Rough OSC: deltas to stay at zone to dec osc (default: 2)
  -ro_deltas_inc <n>        Rough OSC: deltas to stay at zone to inc osc (default: 5)
slave mcu:
  -mcu_log_chip_stat        MCU serial: log chip stats
  -mcu_log_stratum          MCU serial: log stratum
  -mcu_log_spi <spi> <seq>  MCU serial: log spi data from chip
  -slave_log_adc            Log all ADC periodically
  -slave_test_pwr_sw        Turn on/off power switch periodically
pwc:
  -pwc_btc_ser_cfg <n>      BTC serial config (default: 0x00100011)
board:
  -brd_ocp <n>              Over Current Protection: max current per power line (default: 0x60)
  -brd_pwr_off <n>          Turn OFF board power (0 based sequence)
  -brd_pwr_off_all          Turn OFF all boards power
board user spec:
  -user_slot_mask <bin>     User slot mask (example: 101)
  -user_brd_rev <n>         User board: revision id
  -user_brd_if_found        User board: apply config if board detected
  -user_brd_spi_num <n>     User board: num of spi
  -user_brd_spi_len <n>     User board: len of spi
  -user_brd_spi_mask <bin>  User board: spi lines mask (example: 101)
  -user_brd_pwr_num <n>     User board: num of power lines
  -user_brd_pwr_len <n>     User board: len of power lines
  -user_brd_btc_num <n>     User board: num of btc per power chip
  -user_brd_btc_mask <bin>  User board: btc lines mask (example: 01111110)
board pwr switch monitoring:
  -ps_oh_curr <amp>         Low current threshold (0-no cooling) (default: 1000 ma)
  -ps_oh_t <sec>            Low current period (default: 20 sec)
  -ps_temp_lo <C>           Lo temp threshold to allow power ON (default: 60C)
  -ps_temp_hi <C>           Hi temp threshold to power OFF (default: 90C)
other:
  -polling_delay <ms>       Polling delay in milliseconds (default: 2)
statistics:
  -log_delay <sec>          Logging delay in seconds (default: 10)
  -log_chip_stat            Log chip statistics
logging:
  -trace                    Enable trace level logging
  -trace_stratum            Dump stratum protocol communication
  -log_slave <level>        Dump slave mcu protocol communication
test modes:
  -test                     Server test mode
  -test_cb                  Chip Board test mode
  -test_mb                  Mother Board flashing / test mode
  -test_fb                  Fan Board flashing / test mode
slave test config:
  -test_btc_t <ms>          Test: btc test time (default: 3000 ms)
  -test_fan_dt <ms>         Test: delay between pwr setup and measurements (default: 500 ms)
test options:
  -test_num <n>             Test: number of test iterations (default: 3)
  -test_t <sec>             Test: time in seconds (default: 60 sec)
  -test_cooling_t <sec>     Test: timeout to cool boards (default: 10 sec)
  -test_fan_wait_i <a>      Test: wait fan current before test (default: 3.0 A)
  -test_mb_hw_ver <n>       Test MB: hw ver (default: 0xf)
  -test_slave_hw_ver <n>    Test NB: slave hw ver (default: 0xe)
  -test_brd_rev <n>         Test BRD: expected revision (default: 26)
  -test_temp_min <n>        Test BRD: min temperature (default: 20)
  -test_temp_max <n>        Test BRD: max temperature (default: 40)
  -test_no_hs               Test BRD: missing heatsink
  -test_no_hs_n_max <n>     Test BRD: max number of heatsink err pin transitions (default: 0)
  -test_hs_n_min <n>        Test BRD: min number of heatsink err pin transitions (default: 10)
  -test_volt_min <n>        Test BRD: min voltage (default: 42000)
  -test_volt_max <n>        Test BRD: max voltage (default: 45000)
  -test_curr_min <n>        Test BRD: min current (default: 7000)
  -test_curr_max <n>        Test BRD: max current (default: 11000)
  -test_volt2_min <n>       Test BRD: min voltage (default: 42000)
  -test_volt2_max <n>       Test BRD: max voltage (default: 45000)
  -test_curr2_min <n>       Test BRD: min current (default: 6000)
  -test_curr2_max <n>       Test BRD: max current (default: 9000)
  -test_ocp_min <n>         Test BRD: min ocp off (default: 0x0a)
  -test_ocp_max <n>         Test BRD: max ocp off (default: 0x15)
  -tm_btc_no_sol <n>        Test BTC: max no solutions chips (default: 5)
  -tm_btc_no_js <n>         Test BTC: max no job switch chips (default: 0)
  -tm_btc_no_resp <n>       Test BTC: max no response chips (default: 0)
  -test_warn <0|1>          Test: print warnings
fan test config:
  -test_fan_on_v_dev <k>    Fan test: fan on, max voltage deviation get/cfg (default: 0.20)
  -test_fan_on_i_min <a>    Fan test: fan on, min current (default: 3.0 A)
  -test_fan_on_i_max <a>    Fan test: fan on, max current (default: 7.0 A)
special:
  -no_mcu_reset             Disable MCU reset on start-up
log levels:
  0 - NONE, 1 - INFO, 2 - DEBUG, 3 - TRACE

Board revisions table:
   Rev ADC  Rev    SPI (mask)     PWR  BTC (mask)  Volt  Flags  adcKu  adcKi     Human Name
940 - 1040   40  1 x 12 (0x1)  1 x 12    8 (0xff)    48   0x09  16.00  10.00  48V 1L12x8 v1
 500 - 560   41  1 x 12 (0x1)  1 x 12    8 (0xff)    48   0x39  16.00  10.00  48V 1L12x8 v2
 125 - 145   45   1 x 1 (0x1)   1 x 1     1 (0x1)     4   0x39   8.00   8.00   4V 1L1x1 TST

---------------------------------------
Explaining Output
---------------------------------------

*** CHIPS STAT TOTAL (30 sec):
BRD  SPI  CHIP     OSC  READ     SOL  ERR  E/S      SPEED   EFF     REJ  TST  OK  R/J  HWE     SE  JSE      JN   JT      SPEED  CR
  5    0   0/0  |    0   672  |  165   14   8%  23.62 GHs   96%  |    0   14   0   0%    0  |   0    2  |  172  145  24.53 GHs   0
  5    0   0/1  |    0   659  |  168   10   6%  24.05 GHs   96%  |    0   10   0   0%    0  |   0    1  |  175  142  24.96 GHs   0

BTC chip stat conatins information for each BTC chip. Table has following columns:
BRD     board id starting from 0
SPI     always 0
CHIP    pwc id / btc id
OSC     curent OSC set
READ    num of polling iterations while BTC chip switches task
SOL     num of correct solutions with diff 1
ERR     num of errors
E/S     error rate (err / (err + sol)) 
SPEED   speed by sol: chip speed based on solution rate
EFF     efficiency: speed by sol / speed by job
REJ     num of shares rejected for recovery due to luck of computing resources
TST     num of shares processed in recovery module
OK      num of recovedred shares
R/J     recovery rate: num of recovery / num of jobs
HWE     hw errors: num of errors in recovery module (for debug only, should be zero)
SE      num of errors on serial line between PWC and BTC chips
JSE     num of errors in task switch sequence
JN      num of processed jobs
JT      job time in ms
SPEED   speed by job: chip speed based on job rate
CR      num of chip restarts (for debug only, not applicable)


*** POWER CHIP STAT:
BRD   PWC     F    T  CFG      BF  CNST  UNQ  TOTAL      UNIQUE     STAT  LOAD  SHIFT  TIME      CPU     SS  SD
  5  0/ 0  |  1  255    1  |  142   142  142    142  0x8125667f  |   141   142      0    25  138 MHz  |   6   0
  5  0/ 1  |  1  255    1  |  142   142  142    142  0xd7aad973  |   141   142      0    25  139 MHz  |   5   0

PWC chip stat conatins information for each PWC chip. Table has following columns:
BRD     board id starting from 0
PWC     spi id (always zero) / pwc id
F       is chip found (got response during initial setup)
T       memory test result
CFG     is config loaded 
BF      num of successefull reads of vendor id register
CNST    num of successefull reads of constant memory cell
UNQ     num of successefull reads of unused memory cell
TOTAL   num of reads of above registers / memory cells
UNIQUE  value of unused memory cell on start up
STAT    num of successefull loads of stats
LOAD    num of attempts to load stats
SHIFT   num of pwc id mismatch during loading stats
TIME    pwc uptime in sec
CPU     pwc core clock
SS      shares sent
SD      shares droped


*** BOARDS SYS INFO (1 boards, 96 chips):
BRD  FOUND      REV   SPI     PWR  CHIPS  OhN  OhT  loI  T, C       TA  RevADC   OCP     HE  U, mV      I, mA  P, Wt
  5      1  BREV:40  1x12  1x12x8     96    0    0    0    31   90( 0)     987  0x60  0 (0)  44448   I0: 6190    275

BRD     board id starting from 0
FOUND   is board found during initial setup
REV     board revision id
SPI     spi configuration (1x12 - 1 spi with 12 pwc)
PWR     spi configuration (1x12x8 - 1 power line with 12 pwc with 8 btc per each pwc)
CHIPS   num of btc chips per board
OhN     number of overheats: num of board power off due to hi temperature
OhT     overheats time: num of seconds spent to cool board
loI     low current: num of board power restarts due to low current
T, C    board temperature   
TA      tmp75 alert register value / num of writes to register
RevADC  board rev in adc 
OCP     over current protection
HE      heatsink error / num of 0-1 transitions
U, mV   board voltage   
I, mA   board current
P, Wt   board power


*** BOARDS STAT TOTAL (30 secs, 1 boards, 96 chips):
BRD    SOL  ERR      bySol  E/S   JOBS  CR  W/GHs  EPWC
  5  15676  960  2244 GH/s   6%  16670   4  0.123     0

BRD     board id starting from 0
SOL     num of solutions
ERR     num of errors 
bySol   board speed by solutions
E/S     error rate
JOBS    number of processed jobs
CR      number of chip restarts
W/GHs   current board power divided by speed
EPWC    number of error pwc (no reponse from pwc)


*** MASTER STATS PER INTERVAL:
INTERVAL  sec   bySol  byDiff  byPool  byJobs  CHIP GHs  W/GHs    SOL  ERR  ERR(%)  CR
  30 sec  10s  2752.7  2935.2  3155.1  2935.0     28.67  0.106   6409  359    5.3%   0
   5 min  30s  2244.3  2151.2  2151.2  2377.7     23.38  0.129  15676  960    5.8%   4
  15 min  30s  2244.3  2151.2  2151.2  2377.7     23.38  0.129  15676  960    5.8%   4
  1 hour  30s  2244.3  2151.2  2151.2  2377.7     23.38  0.129  15676  960    5.8%   4
   total  30s  2244.3  2151.2  2151.2  2377.7     23.38  0.129  15676  960    5.8%   4

INTERVAL    interval human name
sec         actual interval size
bySol       speed by solutions
byDiff      speed by difficulty (shares filtered out by pool diff)
byPool      speed by pool (shares accepted by pool)
byJobs      speed by jobs
CHIP GHs    avg BTC chip speed
W/GHs       current power divided by speed
SOL         num of solutions
ERR         num of errors
ERR(%)      error rate
CR          number of chip restarts


*** MASTER STATS:
Date: 2017-06-11 02:10:32, UpTime: 30 secs, mbHwVer: 0x1, osc: 0
Found boards:   1
Broken SPI:     0


*** POOL STATS:
Pool: host:port: stratum.slushpool.com:3333, user: swtest.def1024, diff: 1369
extraNonce1: 0265080048fdb1, extraNonce2Size: 4, jobs: 1
INTERVAL  sec  JOBS  clean  SHARES  ok  err  POOL sol  loss  INSERVICE       %
  30 sec  10s     1      1      18  20    0      7346  0.0%        10s  100.0%
   5 min  30s     2      2      50  50    0     15026  0.0%        30s   98.6%
  15 min  30s     2      2      50  50    0     15026  0.0%        30s   98.6%
  1 hour  30s     2      2      50  50    0     15026  0.0%        30s   98.6%
   total  30s     2      2      50  50    0     15026  0.0%        30s   98.6%

INTERVAL    interval human name
sec         actual interval size
JOBS        num of stratum jobs from pool
clean       num of stratum jobs from pool with clean flag
SHARES      num of shares sent to pool
ok          num of shares accepted by pool
err         num of shares rejected by pool
POOL sol    sum of all shares diff sent to pool
loss        loss of shares due to pool rejects
INSERVICE   period while miner is connected to pool
%           percent of uptime while miner is connected to pool


*** EVENT STATS:
Legend: SE - subbsribe error (initialising issue)
        DIFF - diff changes, REC - reconnects, RECE - reconnects on error
        SHARES - sent to pool, PSS - pwc shares sent, PSD - pwc shares dropped
        DJS - default job shares, SJS - stale job shares, DUP - duplicates, LDS - low diff shares
INTERVAL  sec  SE  DIFF  REC  RECE  SHARES  PSS  PSD  DJS  SJS  DUP  LDS
  30 sec  10s   0     1    0     0      18   18    0    0    0    0    0
   5 min  30s   0     2    0     0      50   50    0    0    0    0    0
  15 min  30s   0     2    0     0      50   50    0    0    0    0    0
  1 hour  30s   0     2    0     0      50   50    0    0    0    0    0
   total  30s   0     2    0     0      50   50    0    0    0    0    0


*** SYSTEM STATS:
PSU            Serial        Part  Version        Date                  Desc
  1  0000164071006218  241119.105      3.4  2016-10-11  FLATPACK2 48/3000 HE
  0  0000000000000000                       0000-00-00

PSU            Serial      State  Time  Set,V   U,V  I,A   P,Wt  Uin,V  Tin  Tout  LED  UpTime  FSR  FS  Status  Rx  Alarm
  1  0000164071006218  FULL_PWER    25   42.0  44.0  6.6  290.5  212.0   34    40  G--       0   20  34  Normal  27
  0  0000000000000000  FULL_PWER    25   42.0   0.0  0.0    0.0    0.0    0     0  ---       0    0   0   Error   0

PSU         psu id
Serial      psu serial number
State       power state (power on, power off, etc.)
Time        num of sec in current power state
Set,V       set psu voltage
U,V         output voltage
I,A         output current
P,Wt        output power (U*I)
Uin,V       input voltage
Tin         input air temperature    
Tout        output air temperature
LED         leds state (green/yellow/red)
UpTime      psu up time (for debug purpose)
FSR         speed of fan set by software (in percent of 20000 RPM)
FS          actual fan speed (in percent of 20000 RPM)
Status      psu status
Rx          num of status messages received from psu
Alarm       alarm errors if present



FAN INFO:
   SetU  |     GetU    GetI
11.980V  |  11.977V  0.855A

POWER INFO:
Set Voltage: 42.0 V
       U, V  I, A  P, kW  W/GHs
  PSU  44.0   6.6   0.29  0.129
Board  44.4   6.2   0.28  0.123

Temp(C) (min/avr/max): 31 / 31 / 31


*** MASTER-SLAVE SPI BUS STATS:
SLAVE                       UID         VER  TIME  PING    M=>S   rx  err     %    S=>M   rx  err     %  SS  SD
    0  24004B001151343034323933  0x08160100    30   328     499  267    0  0.0%     267  267    0  0.0%  50   0

Total packets: 499 in 28717 ms, speed: 17.38 packets/sec
