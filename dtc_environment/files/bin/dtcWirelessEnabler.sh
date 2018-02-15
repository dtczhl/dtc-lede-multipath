#!/bin/bash

#
# start/stop wireless interfaces
#
# Huanle Zhang at UC Davis
# www.huanlezhang.com

# --------- Configuration Zone --------------------------

myInterfaces=(wlan0 wlan1)
myDrivers=(nl80211 nl80211)
# config files are in ${dtcConfig} folder
myConfigFiles=(wpa_supplicant1.conf wpa_supplicant2.conf)

# -------------------------------------------------------

usage(){
cat<<EOF
Usage: program -[s|k]
	-s start interfaces
	-k stop interfaces 
EOF
}

argumentProcess(){
	while [[ $# -gt 0 ]]; do
		key="$1"
		case $key in
			-s) # enable interfaces
				for (( i=0; i<${#myInterfaces[@]}; i++ )); do
					myCmd="wpa_supplicant -B -i${myInterfaces[$i]} -c${dtcConfig}/${myConfigFiles[$i]} -D${myDrivers[$i]}"
					dtcKillProgram.sh "$myCmd"
					eval $myCmd 
				done 
				shift
			;;
			-k) # stop interfaces
				for (( i=0; i<${#myInterfaces[@]}; i++ )); do
					myCmd="wpa_supplicant -B -i${myInterfaces[$i]} -c${dtcConfig}/${myConfigFiles[$i]} -D${myDrivers[$i]}"
					dtcKillProgram.sh "$myCmd"
				done 
				shift
			;;
			*) # unknown arguments
				echo "*** Error"
				echo "unknow argument $key in argumentProcess()"
				exit
			;;
		esac
	done 
}

# -------- start from here --------
if [ $# -ne 1 ]; then
	echo "*** Error!"
	usage
	exit
fi 

argumentProcess $@
