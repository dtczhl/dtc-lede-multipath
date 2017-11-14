#!/bin/bash

myInputFile=

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

if [ ! -d "raw2text" ]; then
	echo "**** raw2text directory does not exist!!!"
	exit
fi

echo "Compile raw2text"
(
cd ./raw2text && make
)

if [ -x "./raw2text/raw2text" ]; then
	./raw2text/raw2text -i "${myInputFile}" -o "${myInputFile}.out"
fi

echo done!!
