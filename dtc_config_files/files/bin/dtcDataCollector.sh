#!/bin/bash

# Huanle Zhang
# www.huanlezhang.com

recordId=1
savePath=/dtc

targetIp=192.168.21.191
targetPort=50000

debugfsPath=/sys/kernel/debug
dtcSockDir=${debugfsPath}/dtcSock
dtcMacDir=${debugfsPath}/dtcMac
dtcAthDir=${debugfsPath}/dtcAth


usage(){
cat <<EOF
Usage: program -[r|l|s]
	-r	run sender tools
	-l	listen only 
	-s	stop tools, and save data to files
Example: program -r
			trigger sender program
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

while getopts ":rls" opt; do
	case $opt in
	r) # run
		myRunCmd="dtcDebugfsController.sh -s uima -r u -i $targetIp -p $targetPort -c enable"
		echo $myRunCmd > $savePath/ReadMe$recordId 
		eval $myRunCmd
		date >> $savePath/ReadMe$recordId 
		dtc_sock_client -i $targetIp -p $targetPort -n 100000 > /dev/null &
		echo "Running..."
		;;
	l) # listen only
		myListenCmd="dtcDebugfsController.sh -r u -i $targetIp -p $targetPort -c enable"
		echo $myListenCmd > $savePath/ReadMe$recordId 
		eval $myListenCmd 
		echo "Listening..."
		;;
	s) # stop and save data
		dtcDebugfsController.sh -c disable
		date >> $savePath/ReadMe$recordId 
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
