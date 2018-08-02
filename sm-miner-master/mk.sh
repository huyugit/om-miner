#########################################################################
# File Name: mk.sh
# Author: gzh
# mail: zhihua.ge@bitfily.com
# Created Time: 2018年01月24日 星期三 15时35分27秒
#########################################################################
#!/bin/bash

mach_list="a1 a3"
not_in_list=true
elf_path="./build/debug/sm-miner-master"
target_path="../../h2_linux/BuildLinux"
for mach_type in $mach_list;do
	if [ "$1" = "$mach_type" ];then
		not_in_list=false
		make clean;make MACH_TYPE=$mach_type MULTI_POOL=y
	fi
done

if [ $not_in_list = true ];then
	echo "Fuck, no such machine type!!!!"
	echo "Current support machine list:"
	echo "	A1: 49T monster"
	echo "	A3: 14T cute boys"
	exit 0
fi

if [ -f "$elf_path" ]; then
	sudo cp $elf_path $target_path/version/bitfily/miner/master
	sudo cp $elf_path $target_path/upg_file/bitfily/miner/master
else
	echo "Build sm-miner-master failed"
fi
