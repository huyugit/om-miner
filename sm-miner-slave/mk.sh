#########################################################################
# File Name: mk.sh
# Author: gzh
# mail: zhihua.ge@bitfily.com
# Created Time: 2018年01月24日 星期三 15时35分27秒
#########################################################################
#!/bin/bash

mach_list="a1 a1p a3"
not_in_list=true
elf_path="./exe/sm-miner-slave.bin"
target_path="../../h2_linux/BuildLinux"
for mach_type in $mach_list;do
	if [ "$1" = "$mach_type" ]; then
		not_in_list=false
		rm $elf_path
		make clean;make MACH_TYPE=$mach_type
	fi
done

if [ $not_in_list = true ]; then
	echo "Fuck, no such machine type!!!!"
	echo "Current support machine list:"
	echo "	A1: 49T monster"
	echo "	A1P:Another 49T monster"
	echo "	A3: 14T cute boys"
	exit 0
fi

if [ -f $elf_path ]; then
	sudo cp $elf_path $target_path/version/bitfily/miner/slave.bin
	sudo cp $elf_path $target_path/upg_file/bitfily/miner/slave.bin
	echo "Take care of my 18 palm attacks to defeat dragons"
else
	echo "Build sm-miner-slave.bin failed"
fi
