#########################################################################
# File Name: make.sh
# Author: gzh
# mail: zhihua.ge@bitfily.com
# Created Time: 2018年01月03日 星期三 15时36分49秒
#########################################################################
#!/bin/bash

arm-linux-gnueabihf-gcc sn_read.c -o sn_read
cp sn_read /home/gzh/code/h2_linux/BuildLinux/version/a1/tools
cp sn_read /home/gzh/code/h2_linux/BuildLinux/upg_file/a1/tools
