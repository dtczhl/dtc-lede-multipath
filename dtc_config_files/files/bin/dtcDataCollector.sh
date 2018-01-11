#!/bin/bash

# Huanle Zhang
# www.huanlezhang.com

recordId=1
savePath=/dtc

senderIp=192.168.21.1
senderPort=50000
receiverIp=192.168.21.191
receiverPort=50000

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

readId(){
	
	if [ -f /root/autoId ]; then
		recordId=$(cat /root/autoId) 
	fi 
}

updateId(){

	if [ -f /root/autoId ]; then 
		recordId=$(( $recordId + 1 ))
		echo $recordId > /root/autoId 
	fi 
}

# starts from here --------
if [ $# -ne 1 ]; then 
	echo "**** Error"
	usage
	exit
fi

readId 

while getopts ":rls" opt; do
	case $opt in
	r) # run
		date > $savePath/ReadMe$recordId 
		myRunCmd="dtcDebugfsController.sh -s uima -r u -i $receiverIp -p $receiverPort -c enable"
		echo $myRunCmd >> $savePath/ReadMe$recordId 
		eval $myRunCmd
		if [ ! $selfPort -eq 0 ]; then
			dtc_sock_client -i $receiverIp -p $receiverPort -c $senderPort -n 100000 > /dev/null &
		else
			dtc_sock_client -i $receiverIp -p $receiverPort -n 100000 > /dev/null &
		fi 
		echo "Running..."
		;;
	l) # listen only
		date > $savePath/ReadMe$recordId 
		myListenCmd="dtc_sock_echo -i $receiverIp -p $receiverPort -d 2 &"
		echo $myListenCmd >> $savePath/ReadMe$recordId 
		eval $myListenCmd 
		myListenCmd="dtcDebugfsController.sh -s uima -r u -i $senderIp -p $senderPort -c enable"
		echo $myListenCmd >> $savePath/ReadMe$recordId 
		eval $myListenCmd 
		echo "Listening..."
		;;
	s) # stop and save data
		dtcDebugfsController.sh -c disable
		date >> $savePath/ReadMe$recordId 
		ps | grep dtc_sock_client | grep -v grep | awk '{print $1}' | xargs -r kill -9
		ps | grep dtc_sock_server | grep -v grep | awk '{print $1}' | xargs -r kill -9
		ps | grep dtc_sock_echo   | grep -v grep | awk '{print $1}' | xargs -r kill -9 
		# modify to your needs
		cat ${dtcSockDir}/log1 > ${savePath}/sockLog1_${recordId}
		cat ${dtcSockDir}/log2 > ${savePath}/sockLog2_${recordId}
		cat ${dtcMacDir}/log1 > ${savePath}/macLog1_${recordId}
		cat ${dtcMacDir}/log2 > ${savePath}/macLog2_${recordId}
		cat ${dtcAthDir}/log1 > ${savePath}/athLog1_${recordId}
		cat ${dtcAthDir}/log2 > ${savePath}/athLog2_${recordId}
		
		updateId 
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
