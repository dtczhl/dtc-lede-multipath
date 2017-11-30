#!/bin/bash

# Huanle Zhang
# www.huanlezhang.com

FILE_PATH="$( cd "$(dirname "$0")"; pwd -P )"


# starts from here --------
if [ $# -lt 2 ]; then
	echo "**** Error "
	echo "At least two files to combine"
	exit
fi 

argc=$#
argv=($@)

file1=${argv[0]}
for (( i = 1; i <= $#-1; i++ )); do
	file2=${argv[i]}
	echo "$file1 $file2 > combine.temp_$i"
	awk '
		FNR==NR{id=$NF; NF-=1; sameId[id]=$0; next} {print sameId[$NF], $0}
		' $file1 $file2 > combine.temp_$i
	file1=combine.temp_$i
done

# reformat packet id
(( i = i - 1 ))
echo "combine.temp_$i > combine.temp_id"
awk '
	{split($NF, a, "-"); $NF=(255**3)*a[1]+(255**2)*a[2]+(255**1)*a[3]+(255**0)*a[4]; print $0} 
' combine.temp_$i > combine.temp_id 

# format timestamp
echo "combine.temp_id > combine.temp_time"
awk '
	{ 
		for(i = 1; i < NF; i=i+2) {
			printf "%d ", (1000000*$i)+$(i+1)
		}
		print $NF
	}
' combine.temp_id > combine.temp_time

mv combine.temp_time combineResult

rm -vrf combine.temp_*

echo Done!!
