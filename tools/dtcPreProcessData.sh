#!/bin/bash

myInputFile=

CURRENT_PATH="$( cd "$(dirname "$0")"; pwd -P )"

usage (){
cat <<EOF
Usage: program -f fileFromDebugfs
	-f fileFromDebugfs    specify the filename for processing
Example: ./dtcPreProcessData.sh -f filename
EOF
}

# starts from here -----------------------
if [ $# -lt 1 ]; then
	usage
	exit
fi

while getopts ":f:" opt; do
	case $opt in
	f) # filename
		myInputFile="$OPTARG"
		;;
	\?)
		;;
	esac
done
shift $(($OPTIND - 1))

if [ -z "$myInputFile" ]; then
	echo "**** Filename is empty!!!"
	exit
fi

if [ ! -d "${CURRENT_PATH}/raw2text" ]; then
	echo "**** raw2text directory does not exist!!!"
	exit
fi

echo "Compile raw2text"
(
cd ${CURRENT_PATH}/raw2text && make
)

if [ -x "${CURRENT_PATH}/raw2text/raw2text" ]; then
	${CURRENT_PATH}/raw2text/raw2text -i "${myInputFile}" -o "${myInputFile}.out"
fi

if [[ $myInputFile =~ .*sock.* ]]; then
	# fileName sock, 
	awk '
		NR%2==1{sockLayer=$1" "$2;} NR%2==0{print sockLayer, $0;}
	' ${myInputFile}.out > temp
	mv temp ${myInputFile}.out
fi

echo done!!
