#!/bin/bash

# Huanle Zhang
# www.huanlezhang.com

recordId=1
savePath=/root

serverIp=192.168.21.191
serverPort=50000

debugfsPath=/sys/kernel/debug
dtcSockDir=${debugfsPath}/dtcSock
dtcMacDir=${debugfsPath}/dtcMac
dtcAthDir=${debugfsPath}/dtcAth


usage(){
cat <<EOF
Usage: program -[r|s]
	-r	run relevant tools
	-s	stop tools, and save data to files
Example: program -r
			trigger program
		 program -s
			stop program 
EOF
}

# starts from here --------
if [ $# -ne 1 ]; then 
	echo "**** Error"
	usage
	exit
fi

while getopts ":rs" opt; do
	case $opt in
	r) # run
		dtcDebugfsController.sh -s suma -i $serverIp -p $serverPort -c enable
		dtc_sock_client -i $serverIp -p $serverPort -n 100000 > /dev/null &
		echo "Running..."
		;;
	s) # stop and save data
		dtcDebugfsController.sh -c disable
		ps | grep dtc_sock_client | grep -v grep | awk '{print $1}' | xargs -r kill -9
		# modify to your needs
		cat ${dtcSockDir}/log1 > ${savePath}/sockLog1_${recordId}
		cat ${dtcSockDir}/log2 > ${savePath}/sockLog2_${recordId}
		cat ${dtcMacDir}/log1 > ${savePath}/macLog1_${recordId}
		cat ${dtcMacDir}/log2 > ${savePath}/macLog2_${recordId}
		cat ${dtcAthDir}/log1 > ${savePath}/athLog1_${recordId}
		cat ${dtcAthDir}/log2 > ${savePath}/athLog2_${recordId}
		echo "Save file done"
		;;
	\?) # error
		echo "**** Error"
		echo "unknown option"
		exit
		;;
	esac 
done
shift $(($OPTIND - 1))
