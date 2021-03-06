#!/bin/bash

# Huanle Zhang
# www.huanlezhang.com

debugfsPath=/sys/kernel/debug
dtcSockDir=${debugfsPath}/dtcSock
dtcMacDir=${debugfsPath}/dtcMac
dtcAthDir=${debugfsPath}/dtcAth
isSockSend=0
isSockRecv=0
isTcpSend=0
isTcpRecv=0
isUdpSend=0
isUdpRecv=0
isMacSend=0
isMacRecv=0
isAthSend=0
isAthRecv=0
isIpSend=0
isIpRecv=0

dtcSockTimeLoc=0
dtcMacTimeLoc=0
dtcAthTimeLoc=0

dtcIp=
dtcPort=

# 0 - UDP; 1 - TCP
dtcTransport=0
TcpEnable=1
UdpEnable=2

# -1, 0, 1
controller=-1

# ash, very weired
myAshCmd=

usage (){
cat <<EOF
Usage: program -s [stuima] | -r [stuima] -i Ip_address -p Port_number -u -t -c [enable|disable]
	-s (optional)	send path
	-r (optional)	recv path
	stuima			s: sock; t: tcp; u: udp; i:ip; m: mac; a: ath10k
	-u (optional,default)	UDP tracking
	-t (optional)			TCP tracking 
	-i (required for enable)	ip address to monitor
	-p (required for enable)	port number
	-c (required for enable)	enable or disable debugfs
Example: program -s s -r sa -i 192.168.1.1 -p 12345 -u -c enable
			Enable debugfs, monitor UDP packets of sock in send path, sock and ath10k in recv path
				with target ip address of 192.168.1.1 and port number of 12345
		 program -c disable
		    Disable program 
EOF
}

# starts from here --------
if [ $# -lt 1 ]; then
	echo "**** Error"
	usage
	exit
fi

while getopts ":s:r:i:p:c:ut" opt; do
	case $opt in
	s) # send path
		myArg=${OPTARG}
		while [ ! -z "$myArg" ]; do
			firstLetter="${myArg:0:1}"
			case $firstLetter in
			s) # sock
				isSockSend=1
				;;
			t) # tcp
				isTcpSend=1
				;;
			u) # udp
				isUdpSend=1
				;;
			i) # ip
				isIpSend=1
				;;
			m) # mac
				isMacSend=1
				;;
			a) # ath
				isAthSend=1
				;;
			*) # error
				echo "**** Error"
				echo "unknown -s arguments"
				exit
				;;
			esac
			myArg=${myArg:1}
		done
		;;
	r) # recv path
		myArg=${OPTARG}
		while [ ! -z "$myArg" ]; do
			firstLetter="${myArg:0:1}"
			case $firstLetter in
			s) # sock
				isSockRecv=1
				;;
			t) # tcp
				isTcpRecv=1
				;;
			u) # udp
				isUdpRecv=1
				;;
			i) # ip
				isIpRecv=1
				;;
			m) # mac
				isMacRecv=1
				;;
			a) # ath
				isAthRecv=1
				;;
			*) # error
				echo "**** Error"
				echo "unknown -r arguments"
				exit
				;;
			esac
			myArg=${myArg:1}
		done
		;;
	i) # ip address
		dtcIp=${OPTARG}
		;;
	p) # port number
		dtcPort=${OPTARG}
		;;
	c) # control 
		if [ "${OPTARG}" == "enable" ]; then
			controller=1
		elif [ "${OPTARG}" == "disable" ]; then
			controller=0
		else
			echo "**** Error"
			echo "unknown -c arguments"
		fi
		;;
	u) # udp pakets
		dtcTransport=0
		;;
	t) # tcp packets
		dtcTransport=1
		;;
	\?) # error, unknow options
		echo "**** Error"
		echo "unknown option "
		exit
		;;
	esac
done	
shift $(($OPTIND - 1))

# check arguments

# disable 
if [ $controller -eq 0 ]; then
	if [ -d "$dtcSockDir" ]; then 
		echo -n 0 > ${dtcSockDir}/enable
	fi
	if [ -d "$dtcMacDir" ]; then
		echo -n 0 > ${dtcMacDir}/enable
	fi
	if [ -d "$dtcAthDir" ]; then
		echo -n 0 > ${dtcAthDir}/enable
	fi
	echo "DTC Debugfs Stop"
	exit 
fi

# below: enable

# check dtcSock
if [[ $isSockSend -eq 1 || $isSockRecv -eq 1 ||
	  $isIpSend -eq 1 || $isIpRecv -eq 1 ||
	  $isTcpSend -eq 1 || $isTcpRecv -eq 1 ||
	  $isUdpSend -eq 1 || $isUdpRecv -eq 1 ]]; then
	if [ ! -d "$dtcSockDir" ]; then
		echo "**** Error"
		echo "$dtcSockDir does not exist"
		exit
	fi 
	echo -n 0 > ${dtcSockDir}/enable
	echo -n 0 > ${dtcSockDir}/timeLoc
	myAshCmd="echo 1.1.1.1 1234 > ${dtcSockDir}/target"
	ash -c "$myAshCmd"
fi

# check dtcMac
if [[ $isMacSend -eq 1 || $isMacRecv -eq 1 ]]; then
	if [ ! -d "$dtcMacDir" ]; then
		echo "**** Error"
		echo "$dtcMacDir does not exist"
		exit
	fi 
	echo -n 0 > ${dtcMacDir}/enable
	echo -n 0 > ${dtcMacDir}/timeLoc
	myAshCmd="echo 1.1.1.1 1234 > ${dtcMacDir}/target"
	ash -c "$myAshCmd"
fi

# check dtcAth
if [[ $isAthSend -eq 1 || $isAthRecv -eq 1 ]]; then
	if [ ! -d "$dtcAthDir" ]; then
		echo "**** Error"
		echo "$dtcAthDir does not exist"
		exit
	fi 
	echo -n 0 > ${dtcAthDir}/enable
	echo -n 0 > ${dtcAthDir}/timeLoc
	myAshCmd="echo 1.1.1.1 1234 > ${dtcAthDir}/target"
	ash -c "$myAshCmd"
fi

# check ip address
if [[ ! $dtcIp =~ ^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$ ]]; then 
	echo "**** Error"
	echo "IP address: $dtcIp"
	exit 
fi

# check port number
if [[ ! $dtcPort =~ ^[0-9]+$ ]]; then
	echo "**** Error"
	echo "Port number: $dtcPort"
	exit 
fi

if [ $isSockSend -eq 1 ]; then
	dtcSockTimeLoc=$(( $dtcSockTimeLoc + ( 1 << 0 ) ))
fi

if [ $isSockRecv -eq 1 ]; then
	dtcSockTimeLoc=$(( $dtcSockTimeLoc + ( 1 << 16 ) ))
fi

if [ $isTcpSend -eq 1 ]; then
	dtcSockTimeLoc=$(( $dtcSockTimeLoc + ( 1 << 1 ) ))
fi

if [ $isTcpRecv -eq 1 ]; then
	dtcSockTimeLoc=$(( $dtcSockTimeLoc + ( 1 << 17 ) ))
fi 

if [ $isUdpSend -eq 1 ]; then
	dtcSockTimeLoc=$(( $dtcSockTimeLoc + ( 1 << 2 ) ))
fi 

if [ $isUdpRecv -eq 1 ]; then
	dtcSockTimeLoc=$(( $dtcSockTimeLoc + ( 1 << 18 ) ))
fi 

if [ $isIpSend -eq 1 ]; then
	dtcSockTimeLoc=$(( $dtcSockTimeLoc + ( 1 << 5 ) ))
fi 

if [ $isIpRecv -eq 1 ]; then
	dtcSockTimeLoc=$(( $dtcSockTimeLoc + ( 1 << 21 ) ))
fi 

if [ $isMacSend -eq 1 ]; then
	dtcMacTimeLoc=$(( $dtcMacTimeLoc + ( 1 << 3 ) ))
fi 

if [ $isMacRecv -eq 1 ]; then
	dtcMacTimeLoc=$(( $dtcMacTimeLoc + ( 1 << 19 ) ))
fi 

if [ $isAthSend -eq 1 ]; then
	dtcAthTimeLoc=$(( $dtcAthTimeLoc + ( 1 << 4 ) ))
fi

if [ $isAthRecv -eq 1 ]; then
	dtcAthTimeLoc=$(( $dtcAthTimeLoc + ( 1 << 20) ))
fi 

if [[ $isSockSend -eq 1 || $isSockRecv -eq 1 ||
		$isIpSend -eq 1 || $isIpRecv -eq 1 ||
		$isTcpSend -eq 1 || $isTcpRecv -eq 1 ||
		$isUdpSend -eq 1 || $isUdpRecv -eq 1 ]]; then	
	echo "${dtcIp} ${dtcPort} > ${dtcSockDir}/target"
	myAshCmd="echo ${dtcIp} ${dtcPort} > ${dtcSockDir}/target"
	ash -c "$myAshCmd"
	echo "${dtcSockTimeLoc} > ${dtcSockDir}/timeLoc"
	echo -n $dtcSockTimeLoc > ${dtcSockDir}/timeLoc
	if [ $dtcTransport -eq 0 ]; then
		echo "${UdpEnable} > ${dtcSockDir}/enable"
		echo -n $UdpEnable > ${dtcSockDir}/enable
	elif [ $dtcTransport -eq 1 ]; then
			echo "${TcpEnable} > ${dtcSockDir}/enable"
			echo -n $TcpEnable > ${dtcSockDir}/enable
	else
		echo "**** Error in try to enable dtcSockDir"
		exit
	fi 
fi

if [[ $isMacSend -eq 1 || $isMacRecv -eq 1 ]]; then
	echo "${dtcIp} ${dtcPort} > ${dtcMacDir}/target"
	myAshCmd="echo ${dtcIp} ${dtcPort} > ${dtcMacDir}/target"
	ash -c "$myAshCmd"
	echo "${dtcMacTimeLoc} > ${dtcMacDir}/timeLoc"
	echo -n $dtcMacTimeLoc > ${dtcMacDir}/timeLoc
	if [ $dtcTransport -eq 0 ]; then
		echo "${UdpEnable} > ${dtcMacDir}/enable"
		echo -n $UdpEnable > ${dtcMacDir}/enable
	elif [ $dtcTransport -eq 1 ]; then
			echo "${TcpEnable} > ${dtcMacDir}/enable"
			echo -n $TcpEnable > ${dtcMacDir}/enable
	else
		echo "**** Error in try to enable dtcMacDir"
		exit
	fi
fi

if [[ $isAthSend -eq 1 || $isAthRecv -eq 1 ]]; then
	echo "${dtcIp} ${dtcPort} > ${dtcAthDir}/target"
	myAshCmd="echo ${dtcIp} ${dtcPort} > ${dtcAthDir}/target"
	ash -c "$myAshCmd"
	echo "${dtcAthTimeLoc} > ${dtcAthDir}/timeLoc"
	echo -n $dtcAthTimeLoc > ${dtcAthDir}/timeLoc
	if [ $dtcTransport -eq 0 ]; then
		echo "${UdpEnable} > ${dtcAthDir}/enable"
		echo -n $UdpEnable > ${dtcAthDir}/enable
	elif [ $dtcTransport -eq 1 ]; then
			echo "${TcpEnable} > ${dtcAthDir}/enable"
			echo -n $TcpEnable > ${dtcAthDir}/enable
	else
		echo "**** Error in try to enable dtcMacDir"
		exit
	fi	
fi

echo "Dtc Debugfs Starts"

